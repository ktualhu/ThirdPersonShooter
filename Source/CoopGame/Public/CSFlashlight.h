// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CSItem.h"
#include "CSFlashlight.generated.h"

class USpotLightComponent;

UENUM()
enum class EFlashlightState : uint8
{
	ON,
	OFF
};

UCLASS()
class COOPGAME_API ACSFlashlight : public ACSItem
{
	GENERATED_BODY()

	ACSFlashlight();

protected:

	UPROPERTY(EditAnywhere, Category = "Flashlight")
	TSubclassOf<USpotLightComponent> SpotLightClass;

	UPROPERTY(BlueprintReadOnly)
	EFlashlightState FlashlightState;

	float DefaultFlashlightPower;

	UPROPERTY(BlueprintReadOnly)
	float CurrentFlashlightPower;

	UPROPERTY(EditAnywhere, Category = "Flashlight")
	USpotLightComponent* SpotLight;

	void SetFlashlightState(EFlashlightState FlashlightState);

	UPROPERTY(EditDefaultsOnly, Category = "Flashlight Powerup")
	float PowerupDecrease;

	UPROPERTY(EditDefaultsOnly, Category = "Flashlight Sound")
	USoundBase* ToggleOnSound;

	UPROPERTY(EditDefaultsOnly, Category = "Flashlight Sound")
	USoundBase* ToggleOffSound;

	UPROPERTY(EditDefaultsOnly, Category = "Flashlight Sound")
	USoundBase* NotPowerSound;

public:

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	void PowerupFlashlight();
	
};
