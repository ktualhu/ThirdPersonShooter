// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ACSProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class USoundBase;

UCLASS()
class COOPGAME_API AACSProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AACSProjectile();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovementComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* LaunchSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* ExplosionSound;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void MoveProjectile(FVector);

	virtual void LifeSpanExpired() override;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLifeSpanExpired();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastLifeSpanExpired();

	/*UFUNCTION()
	void OnDestroyed(UActorComponent* ActorComponent, USphereComponent* SphereComponent);*/
};
