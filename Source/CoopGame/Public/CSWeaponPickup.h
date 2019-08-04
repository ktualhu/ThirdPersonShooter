// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AmmoPickup.h"
#include "CSWeaponPickup.generated.h"

class ACSWeapon;
class ACSPistol;
class ACSShotgun;
class ACSSniperRifle;
class ACSProjectileWeapon;
class ACSCharacter;

/**
 * 
 */
UCLASS()
class COOPGAME_API ACSWeaponPickup : public AAmmoPickup
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category = "WeaponType")
	TSubclassOf<ACSWeapon> WeaponType;

	// All posible weapon types
	UPROPERTY(EditDefaultsOnly, Category = "WeaponType")
	TSubclassOf<ACSPistol> PistolWeaponType;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponType")
	TSubclassOf<ACSShotgun> ShotgunWeaponType;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponType")
	TSubclassOf<ACSWeapon> RifleWeaponType;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponType")
	TSubclassOf<ACSSniperRifle> SniperRifleWeaponType;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponType")
	TSubclassOf<ACSProjectileWeapon> ProjectileWeaponWeaponType;

	virtual void OnOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:

	void HandlePickupWeapon(ACSCharacter* Character, ACSWeapon* Weapon);
};
