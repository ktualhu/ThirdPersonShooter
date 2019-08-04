// Fill out your copyright notice in the Description page of Project Settings.


#include "CSWeaponPickup.h"
#include "..\Public\CSCharacter.h"
#include "..\Public\CSWeapon.h"
#include "..\Public\CSPistol.h"
#include "..\Public\CSShotgun.h"
#include "Engine/World.h"

void ACSWeaponPickup::OnOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		ACSCharacter* Character = Cast<ACSCharacter>(OtherActor);
		ACSWeapon* Weapon;
			
		if (WeaponType.Get() == PistolWeaponType.Get())
		{
			Weapon = Character->GetLightWeaponSlot();
		}

		else if (WeaponType.Get() == ShotgunWeaponType.Get())
		{
			Weapon = Character->GetMiddleWeaponSlot();
		}

		else
		{
			Weapon = Character->GetHardWeaponSlot();
		}

		HandlePickupWeapon(Character, Weapon);
	}
}

void ACSWeaponPickup::HandlePickupWeapon(ACSCharacter* Character, ACSWeapon* Weapon)
{
	if (Weapon && Weapon->GetCountOfBulletsOnCharacter() == Weapon->GetMaxBullets())
	{
		return;
	}

	else
	{
		if (!Weapon)
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ACSWeapon* FirstWeaponSlot = GetWorld()->SpawnActor<ACSWeapon>(WeaponType, SpawnInfo);
			Character->AddWeapon(FirstWeaponSlot);
		}
		else if (Weapon && Weapon->GetClass() == WeaponType.Get() && Weapon->GetCountOfBulletsOnCharacter() < Weapon->GetMaxBullets())
		{
			AddAmmo(Weapon);
		}
		else
		{
			return;
		}

		Destroy();
	}
}

