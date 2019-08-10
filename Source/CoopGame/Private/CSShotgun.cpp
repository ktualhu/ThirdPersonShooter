// Fill out your copyright notice in the Description page of Project Settings.


#include "CSShotgun.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "CoopGame.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "..\Public\CSCharacter.h"
#include "Animation/AnimMontage.h"

ACSShotgun::ACSShotgun()
{
	BaseDamage = 2200.0f;

	MagazineCapacity = 6;
	MaxBullets = 100;

	CountOfBulletsInMagazine = MagazineCapacity;
	CountOfBulletsOnCharacter = MaxBullets;

	MinQuantityOfLineTraces = 5;
	MaxQuantityOfLineTraces = 8;

	// Only for shotgun
	MinRandomDeviation = -100.0f;
	MaxRandomDeviation = 100.0f;

	FireDelay = 1.300f;

	FiringRange = 1500.0f;

	IsAbleToFire = true;
}

void ACSShotgun::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFireShotgun();
	}

	if (CanShoot())
	{
		AActor* Owner = GetOwner();

		if (Owner)
		{
			if (Character && FireAnimation)
			{
				FireDelay = Character->PlayAnimMontage(FireAnimation);
			}

			IsAbleToFire = false;
			IsFireNow = true;

			QuantityOfLineTraces = FMath::RandRange(MinQuantityOfLineTraces, MaxQuantityOfLineTraces);

			FVector EyeLocation;
			FRotator EyeRotation;
			Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

			FVector ShotDirection = EyeRotation.Vector();
			FVector TraceEnd = EyeLocation + (ShotDirection * FiringRange) + CalculateSpread();

			CountOfBulletsInMagazine--;

			PlayFireSoundEffect();
			PlayFireEffects(TraceEnd);

			//GetWorld()->GetTimerManager().SetTimer(TimerHandle_FireShotgunDelay, this, &ACSShotgun::StopFire, FireShotgunDelay, false);
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_FireShotgunDelay,this,&ACSShotgun::ShotgunAbleToFire,FireDelay,false);

			for (int i = 0; i < QuantityOfLineTraces; i++)
			{
				float TempDeviation = FMath::RandRange(MinRandomDeviation, MaxRandomDeviation);
				FVector TraceEndPoint = TraceEnd + ShotgunDeviation(MinRandomDeviation, MaxRandomDeviation);

				EPhysicalSurface SurfaceType = SurfaceType_Default;

				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(Owner);
				QueryParams.AddIgnoredActor(this);
				QueryParams.bTraceComplex = true;
				QueryParams.bReturnPhysicalMaterial = true;

				FHitResult HitResult;
				if (GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TraceEndPoint, COLLISION_WEAPON, QueryParams))
				{
					// blocking hit and process damage
					AActor* HitActor = HitResult.GetActor();

					SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

					float ActualDamage = BaseDamage;
					if (SurfaceType == SURFACE_FLESHVULNERABLE)
					{
						ActualDamage *= 5.0f;
					}

					UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult, Owner->GetInstigatorController(), this, DamageType);

					PlayImpactEffects(SurfaceType, HitResult.ImpactPoint);

					TraceEndPoint = HitResult.ImpactPoint;
				}
			}

			if (ShotgunReloadSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ShotgunReloadSound, this->GetActorLocation());
			}

		}
	}
}

void ACSShotgun::ServerFireShotgun_Implementation()
{
	Fire();
}

bool ACSShotgun::ServerFireShotgun_Validate()
{
	return true;
}

void ACSShotgun::StartFire()
{
	if (IsAbleToFire)
	{
		Fire();
	}
}

void ACSShotgun::StopFire()
{
	
	
	IsFireNow = false;
}

bool ACSShotgun::CanShoot()
{
	if (Super::CanShoot() && IsAbleToFire && !IsFireNow)
	{
		return true;
	}
	return false;
}

FVector ACSShotgun::ShotgunDeviation(float MinDeviation, float MaxDeviation)
{
	float DeviationX = FMath::RandRange(MinDeviation, MaxDeviation);
	float DeviationY = FMath::RandRange(MinDeviation, MaxDeviation);
	float DeviationZ = FMath::RandRange(MinDeviation, MaxDeviation);

	FVector Deviation = FVector(DeviationX, DeviationY, DeviationZ);

	return Deviation;
}

void ACSShotgun::ShotgunAbleToFire()
{
	IsFireNow = false;
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_FireShotgunDelay);
	if (Character)
	{
		Character->IsFiringNow = false;
	}

	IsAbleToFire = true;
	
}
