// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "..\Public\CSCharacter.h"
#include "..\Public\CSWeapon.h"
#include "Kismet/GameplayStatics.h"


ACSWeapon* AAmmoPickup::GetCharacterWeaponByAmmoType(TArray<ACSWeapon*> CharacterWeapons)
{
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *(Cast<ACSWeapon>(AmmoType)->GetClass()->GetName()));
	for (ACSWeapon* Weapon : CharacterWeapons)
	{
		// Comparing AmmoType object class(defined in editor) with each object classes from overlapped character Weapons array
		if (Weapon->GetClass() == AmmoType.Get())
		{
			return Weapon;
		}
	}
	return nullptr;
}

void AAmmoPickup::OnOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		ACSCharacter* Character = Cast<ACSCharacter>(OtherActor);
		if (Character)
		{
			CharacterWeapons = Character->GetCharacterWeapons();

			ACSWeapon* Weapon = GetCharacterWeaponByAmmoType(CharacterWeapons);
			if (Weapon)
			{
				if (Weapon->GetCountOfBulletsOnCharacter() < Weapon->GetMaxBullets())
				{
					int32 AmmoOnCharacter = Weapon->GetCountOfBulletsOnCharacter();
					int32 MaxAmmo = Weapon->GetMaxBullets();

					UE_LOG(LogTemp, Warning, TEXT("on character %d"), AmmoOnCharacter);
					UE_LOG(LogTemp, Warning, TEXT("max ammo %d"), MaxAmmo);

					if ((AmmoOnCharacter + QuantityOfAmmo) > MaxAmmo)
					{
						
						Weapon->SetCountOfBulletsOnCharacter(MaxAmmo - AmmoOnCharacter);
					}
					else
					{
						Weapon->SetCountOfBulletsOnCharacter(QuantityOfAmmo);
					}

					if (PickupSound)
					{
						UGameplayStatics::PlaySound2D(this, PickupSound);
					}

					Destroy();
				}
			}
		}
	}
}