// Fill out your copyright notice in the Description page of Project Settings.


#include "..\Public\ACSProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

// Sets default values
AACSProjectile::AACSProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Projectile Component"));
	CollisionComponent->InitSphereRadius(5.0f);
	CollisionComponent->SetCollisionProfileName("Projectile");
	CollisionComponent->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComponent->CanCharacterStepUpOn = ECB_No;

	//CollisionComponent->OnComponentDestroyed(true).AddDynamic(this, &AACSProjectile::OnDestroyed);
	RootComponent = CollisionComponent;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->InitialSpeed = 3000.f;
	ProjectileMovementComponent->MaxSpeed = 3000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = true;

	SetReplicates(true);
}

// Called when the game starts or when spawned
void AACSProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (LaunchSound)
	{
		UGameplayStatics::PlaySound2D(this, LaunchSound);
	}
}

// Called every frame
void AACSProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AACSProjectile::MoveProjectile(FVector ShotDirection)
{
	ProjectileMovementComponent->SetVelocityInLocalSpace(ShotDirection * ProjectileMovementComponent->InitialSpeed);
}

void AACSProjectile::LifeSpanExpired()
{
	if (Role < ROLE_Authority)
	{
		ServerLifeSpanExpired();
	}
	MulticastLifeSpanExpired();
	UGameplayStatics::ApplyRadialDamage(this, 50.0f, CollisionComponent->GetComponentLocation(),
		200.0f, DamageType, TArray<AActor*>(), this, GetInstigatorController(), false, ECollisionChannel::ECC_Visibility);
	Super::LifeSpanExpired();
}

void AACSProjectile::ServerLifeSpanExpired_Implementation()
{
	LifeSpanExpired();
}

bool AACSProjectile::ServerLifeSpanExpired_Validate()
{
	return true;
}

void AACSProjectile::MulticastLifeSpanExpired_Implementation()
{
	if (ExplosionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, CollisionComponent->GetComponentLocation(), CollisionComponent->GetComponentRotation());
	}
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySound2D(this, ExplosionSound);
	}
	DrawDebugSphere(GetWorld(), CollisionComponent->GetComponentLocation(), 200.0f, 24, FColor::Red, false, 2.0f, 0, 2.0f);
}
