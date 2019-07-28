// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CSWeapon.h"
#include "CSShotgun.generated.h"

class USoundBase;

/**
 * 
 */
UCLASS()
class COOPGAME_API ACSShotgun : public ACSWeapon
{
	GENERATED_BODY()

public:
	ACSShotgun();

	bool IsFireNow;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	int32 MinQuantityOfLineTraces;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	int32 MaxQuantityOfLineTraces;

	int32 QuantityOfLineTraces;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	float MinRandomDeviation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	float MaxRandomDeviation;

	float FireShotgunDelay;

	bool IsAbleToFire;

	FTimerHandle TimerHandle_FireShotgunDelay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* ShotgunReloadSound;

protected:
	virtual void Fire() override;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireShotgun();

	virtual void StartFire() override;
	virtual void StopFire() override;

	virtual bool CanShoot() override;

	UFUNCTION()
	void ShotgunAbleToFire();

	FVector CalculateDeviation();
	
};
