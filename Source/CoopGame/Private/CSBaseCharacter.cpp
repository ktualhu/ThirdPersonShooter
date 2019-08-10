// Fill out your copyright notice in the Description page of Project Settings.


#include "CSBaseCharacter.h"
#include "..\Public\CSCharacter.h"
#include "CoopGame.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "..\Public\CSDamageType.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"

// Sets default values
ACSBaseCharacter::ACSBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	NoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>("NoiseEmitterComponent");

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	DefaultHealth = 1000.0f;
	CurrentHealth = DefaultHealth;
}

// Called when the game starts or when spawned
void ACSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	//HealthComponent->OnHealthChanged.AddDynamic(this, &ACSBaseCharacter::OnHealthChanged);
	
}

float ACSBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (CurrentHealth <= 0.0f)
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.0f)
	{
		CurrentHealth -= ActualDamage;
		if (CurrentHealth <= 0.0f)
		{
			bool bCanDie = true;

			if (DamageEvent.DamageTypeClass)
			{
				UCSDamageType* DmgType = Cast<UCSDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
				bCanDie = (DmgType == nullptr || DmgType);
			}

			if (bCanDie)
			{
				Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
			}
		}
		else
		{
			PlayHit(ActualDamage, DamageEvent, EventInstigator, DamageCauser, false);
		}
	}

	return ActualDamage;
}

bool ACSBaseCharacter::Die(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	EventInstigator = GetDamageInstigator(EventInstigator, *DamageType);

	OnDeath(DamageAmount, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
	return false;
}

void ACSBaseCharacter::OnDeath(float DamageAmount, FDamageEvent const& DamageEvent, APawn* Pawn, AActor* DamageCauser)
{
	if (IsDyingNow)
	{
		return;
	}

	IsDyingNow = true;
	bDied = true;

	PlayHit(DamageAmount, DamageEvent, Pawn->Controller, DamageCauser, true);

	FPointDamageEvent PointDamage = *((FPointDamageEvent*)(&DamageEvent));

	//DetachFromControllerPendingDestroy();

	GetMovementComponent()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	
	SetActorEnableCollision(true);

	SetRagdollPhysics();

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		GetMesh()->AddImpulseAtLocation(PointDamage.ShotDirection * 10000, PointDamage.HitInfo.ImpactPoint, PointDamage.HitInfo.BoneName);
	}
	
	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		// for radial damage
	}
}

void ACSBaseCharacter::SetRagdollPhysics()
{
	if (!bIsRagdoll)
	{
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		UCharacterMovementComponent* CharacterComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
		if (CharacterComp)
		{
			CharacterComp->StopMovementImmediately();
			CharacterComp->DisableMovement();
			CharacterComp->SetComponentTickEnabled(false);
		}
		SetLifeSpan(10.0f);
		bIsRagdoll = true;
	}
}

void ACSBaseCharacter::PlayHit(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser, bool bKilled)
{
	if (bKilled && DeathSound)
	{
		UGameplayStatics::SpawnSoundAttached(DeathSound, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true);
	}
	else if (TakeHitSound)
	{
		UGameplayStatics::SpawnSoundAttached(TakeHitSound, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget, true);
	}
}

void ACSBaseCharacter::ClearWidgetsAfterDeath()
{
}

bool ACSBaseCharacter::IsAlive() const
{
	if (CurrentHealth > 0.0f)
	{
		return true;
	}
	return false;
}

// Called every frame
void ACSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

