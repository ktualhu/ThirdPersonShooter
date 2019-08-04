// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemPickup.h"
#include "AmmoPickup.generated.h"


class ACSWeapon;

/**
 * 
 */
UCLASS()
class COOPGAME_API AAmmoPickup : public AItemPickup
{
	GENERATED_BODY()	

private:
	TArray<ACSWeapon*> CharacterWeapons;

private:

	ACSWeapon* GetCharacterWeaponByAmmoType(TArray<ACSWeapon*> CharacterWeapons);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	int32 QuantityOfAmmo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	TSubclassOf<ACSWeapon> AmmoType;

	virtual bool AddAmmo(ACSWeapon* Weapon)
	{
		return AddAmmo(Weapon, QuantityOfAmmo);
	}

	bool AddAmmo(ACSWeapon* Weapon, int32 QuantityOfAmmo);



protected:
	virtual void OnOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
