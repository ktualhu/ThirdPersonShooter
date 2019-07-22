// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CSWeapon.h"
#include "CSProjectileWeapon.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AACSProjectile;

/**
 * 
 */
UCLASS()
class COOPGAME_API ACSProjectileWeapon : public ACSWeapon
{
	GENERATED_BODY()
		
public:
	ACSProjectileWeapon();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USphereComponent* ProjectileComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	UStaticMeshComponent* ProjectileMeshComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AACSProjectile> Projectile;

protected:
	virtual void Fire() override;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireProjectile();

	virtual void StartFire() override;
	virtual void StopFire() override;

private:

};
