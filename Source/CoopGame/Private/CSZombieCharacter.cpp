// Fill out your copyright notice in the Description page of Project Settings.


#include "CSZombieCharacter.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "CoopGame.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

#include "..\Public\CSZombieAIController.h"
#include "..\Public\CSCharacter.h"

#include "Perception/PawnSensingComponent.h"

ACSZombieCharacter::ACSZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	ZombieSightRadius = 1500.0f;
	ZombieHearingThreshold = 500.0f;
	ZombieLOSHearingThreshold = 1000.0f;
	ZombiePeripheralVisionAngle = 60.0f;

	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensingComponent");
	PawnSensingComponent->SightRadius = ZombieSightRadius;
	PawnSensingComponent->HearingThreshold = ZombieHearingThreshold;
	PawnSensingComponent->LOSHearingThreshold = ZombieLOSHearingThreshold;
	PawnSensingComponent->SetPeripheralVisionAngle(ZombiePeripheralVisionAngle);

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f, false);
	GetCapsuleComponent()->SetCapsuleRadius(45.0f);

	MeleeCapsuleHalfHeight = 100.0f;
	MeleeCapsuleRadius = 65.0f;

	MeleeCollisionComponent = CreateDefaultSubobject<UCapsuleComponent>("MeleeCollisionComponent");
	MeleeCollisionComponent->SetCapsuleHalfHeight(MeleeCapsuleHalfHeight);
	MeleeCollisionComponent->SetCapsuleRadius(MeleeCapsuleRadius);
	MeleeCollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeleeCollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	MeleeCollisionComponent->SetupAttachment(GetCapsuleComponent());

	AudioComponent = CreateDefaultSubobject<UAudioComponent>("AudioComponent");
	AudioComponent->bAutoActivate = false;
	AudioComponent->bAutoDestroy = false;
	AudioComponent->SetupAttachment(RootComponent);

	DefaultHealth = 2000.0f;
	CurrentHealth = DefaultHealth;
	MeleeDamage = 175.0f;
	MeleeAttackCooldown = 1.2f;

	SensedTimeOut = 5.0f;
}

void ACSZombieCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &ACSZombieCharacter::OnSeePlayer);
		PawnSensingComponent->OnHearNoise.AddDynamic(this, &ACSZombieCharacter::OnHearNoise);
	}

	if (MeleeCollisionComponent)
	{
		MeleeCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ACSZombieCharacter::OnMeleeComponentBeginOverlap);
	}

	AudioLoop(false);
}

void ACSZombieCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsWonder && (GetWorld()->GetTimeSeconds() - LastHeardTime) > SensedTimeOut)
	{
		IsWonder = false;
		LastHeardTime = 0.0f;
	}

}

void ACSZombieCharacter::OnSeePlayer(APawn* Pawn)
{
	if(!IsAlive())
	{
		return;
	}

	IsWonder = false;
	SensedTarget = true;

	//LastHeardTime = GetWorld()->GetTimeSeconds();

	ACSZombieAIController* ZombieAIController = Cast<ACSZombieAIController>(GetController());
	ACSCharacter* SensedPawn = Cast<ACSCharacter>(Pawn);

	if (ZombieAIController && SensedPawn && SensedPawn->IsAlive())
	{
		ZombieAIController->SetTargetEnemy(SensedPawn);
	}
}

void ACSZombieCharacter::OnHearNoise(APawn* Pawn, const FVector& Location, float Volume)
{
	if(!IsAlive())
	{
		return;
	}

	IsWonder = false;
	SensedTarget = true;

	ACSZombieAIController* ZombieAIController = Cast<ACSZombieAIController>(GetController());
	ACSCharacter* SensedPawn = Cast<ACSCharacter>(Pawn);

	if (ZombieAIController && SensedPawn && SensedPawn->IsAlive())
	{
		ZombieAIController->SetTargetEnemy(SensedPawn);
	}
	
	LastHeardTime = GetWorld()->GetTimeSeconds();
}

void ACSZombieCharacter::OnMeleeComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		TimerHandle_MeleeAttack.Invalidate();
		DoMeleeAttack(OtherActor);
		GetWorldTimerManager().SetTimer(TimerHandle_MeleeAttack, this, &ACSZombieCharacter::RetriggerMeleeAttack, MeleeAttackCooldown, true);
	}
}

float ACSZombieCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!SensedTarget)
	{
		ACSBaseCharacter* SensedPawn = Cast<ACSBaseCharacter>(DamageCauser);

		if (SensedPawn && SensedPawn->IsAlive())
		{
			OnSeePlayer(SensedPawn);
		}
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);;
}

void ACSZombieCharacter::PlayHit(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser, bool bkilled)
{
	//ACSBaseCharacter::PlayHit(DamageAmount, DamageEvent, EventInstigator, DamageCauser, bkilled);
	Super::PlayHit(DamageAmount, DamageEvent, EventInstigator, DamageCauser, bkilled);

	if (!bkilled && HitAnimMontage && !HitPlaying)
	{
		HitPlaying = true;
		const float Duration = PlayAnimMontage(HitAnimMontage);

		GetWorldTimerManager().SetTimer(TimerHandle_HitAnimation, this, &ACSZombieCharacter::StopPlayAnimations, Duration, true);
	}
	else if(bkilled)
	{
		StopPlayAnimations();
		if(AudioComponent)
		{
			AudioComponent->Stop();
		}
	}
}

void ACSZombieCharacter::StopPlayAnimations()
{
	if(HitPlaying)
	{
		StopAnimMontage(HitAnimMontage);
		GetWorldTimerManager().ClearTimer(TimerHandle_HitAnimation);
		HitPlaying = false;
	}
}

void ACSZombieCharacter::AudioLoop(const bool SensedTarget)
{
	if(IsAlive())
	{
		if (SensedTarget)
		{
			AudioComponent->SetSound(HuntingSound);
			AudioComponent->Play();
		}
		else
		{
			if (IsWonder)
			{
				AudioComponent->SetSound(WonderingSound);
				AudioComponent->Play();
			}
			else
			{
				AudioComponent->SetSound(IdleSound);
				AudioComponent->Play();
			}
		}
	}
}

void ACSZombieCharacter::DoMeleeAttack(AActor* AttackedActor)
{
	if (!IsAlive())
	{
		return;
	}

	if (LastMeleeAttack > GetWorld()->GetTimeSeconds() - MeleeAttackCooldown)
	{
		if (!TimerHandle_MeleeAttack.IsValid())
		{
			GetWorldTimerManager().SetTimer(TimerHandle_MeleeAttack, this, &ACSZombieCharacter::RetriggerMeleeAttack, MeleeAttackCooldown, true);
		}

		return;
	}

	if (AttackedActor)
	{
		if (AttackedActor->IsA(ACSCharacter::StaticClass()))
		{
			LastMeleeAttack = GetWorld()->GetTimeSeconds();
				
			FPointDamageEvent ZombiePointDamage;
			ZombiePointDamage.DamageTypeClass = ZombieDamageType;
			ZombiePointDamage.Damage = MeleeDamage;

			AttackedActor->TakeDamage(ZombiePointDamage.Damage, ZombiePointDamage, GetController(), this);

			PlayAttackEffects();
		}
	}
}

void ACSZombieCharacter::RetriggerMeleeAttack()
{
	TArray<AActor*> OverlappedActors;
	MeleeCollisionComponent->GetOverlappingActors(OverlappedActors, ACSBaseCharacter::StaticClass());

	if (OverlappedActors.Num() == 0)
	{
		TimerHandle_MeleeAttack.Invalidate();
		//StopAnimMontage(MeleeAttackAnimMontage);
		return;
	}

	for (int i = 0; i < OverlappedActors.Num(); i++)
	{
		ACSBaseCharacter* OverlappedPlayer = Cast<ACSBaseCharacter>(OverlappedActors[i]);
		if (OverlappedPlayer && OverlappedPlayer->IsAlive())
		{
			DoMeleeAttack(OverlappedPlayer);
		}
	}

}

void ACSZombieCharacter::PlayAttackEffects()
{
	uint8 PickRandomAttackAnimation = FMath::RandRange(1, 4);
	switch (PickRandomAttackAnimation)
	{
		case 1:
			PlayAnimMontage(MeleeAttackAnimMontage1);
			break;
		case 2:
			PlayAnimMontage(MeleeAttackAnimMontage2);
			break;
		case 3:
			PlayAnimMontage(MeleeAttackAnimMontage3);
			break;
		case 4:
			PlayAnimMontage(MeleeAttackAnimMontage4);
			break;
	}
	PlayCharacterSound(MeleeAttackSound);
}

UAudioComponent* ACSZombieCharacter::PlayCharacterSound(USoundCue* MeleeAttackSound)
{
	if (MeleeAttackSound)
	{
		return UGameplayStatics::SpawnSoundAttached(MeleeAttackSound, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget);
	}
	return nullptr;
}
