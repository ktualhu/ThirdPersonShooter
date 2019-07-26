// Fill out your copyright notice in the Description page of Project Settings.


#include "..\Public\CSBarrel.h"
#include "..\Public\CSHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"

// Implement Explosive barrel

// Sets default values
ACSBarrel::ACSBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ExplosionForceStrenght = 300.f;
	ExplosionImpulseStrenght = 500.f;
	ExplosionRadius = 300.f;

	HealthComponent = CreateDefaultSubobject<UCSHealthComponent>(TEXT("Health Component"));

	BarrelMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Barrel Mesh Component"));
	BarrelMeshComponent->SetSimulatePhysics(true);
	BarrelMeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = BarrelMeshComponent;

	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("Radial Force Component"));
	RadialForceComponent->bAutoActivate = false;
	RadialForceComponent->bImpulseVelChange = true;
	RadialForceComponent->SetupAttachment(BarrelMeshComponent);
	RadialForceComponent->ForceStrength = ExplosionForceStrenght;
	RadialForceComponent->ImpulseStrength = ExplosionImpulseStrenght;
	RadialForceComponent->Radius = ExplosionRadius;
	
	bIsExplosed = false;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ACSBarrel::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ACSBarrel::OnHealthChanged);
	}
}

void ACSBarrel::OnHealthChanged(UCSHealthComponent* HealthComp, float Health, float HealthDelta, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	if (!bIsExplosed && Health <= 0.0f)
	{
		bIsExplosed = true;
		MulticastExplosionEffects();
	}
}

// Called every frame
void ACSBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACSBarrel::PlayExplosionEffects()
{
	if (BarrelMeshComponent)
	{
		if (ExplosionEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, this->GetActorLocation());
		}

		if (ExplosionSound)
		{
			UGameplayStatics::PlaySound2D(this, ExplosionSound);
		}

		if (ExplosionMaterial)
		{
			BarrelMeshComponent->SetMaterial(0, ExplosionMaterial);
		}
		BarrelMeshComponent->AddImpulse(FVector::UpVector * 200.f);
		RadialForceComponent->FireImpulse();
		if (Role == ROLE_Authority)
		{
			UGameplayStatics::ApplyRadialDamage(this, 80.f, GetActorLocation(), ExplosionRadius, DamageType, TArray<AActor*>(), this, GetInstigatorController(), true);
		}
	}
}

void ACSBarrel::MulticastExplosionEffects_Implementation()
{
	PlayExplosionEffects();
}

void ACSBarrel::GetLifetimeReplicatedProps(TArray < class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACSBarrel, bIsExplosed);
}
