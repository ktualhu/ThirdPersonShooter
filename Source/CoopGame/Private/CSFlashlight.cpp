// Fill out your copyright notice in the Description page of Project Settings.


#include "CSFlashlight.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ACSFlashlight::ACSFlashlight()
{
	/*SpotLight = CreateDefaultSubobject<USpotLightComponent>("Spot Light Component");
	SpotLight->SetupAttachment(RootComponent);*/

	//SpotLight->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "LightSocket");

	SetFlashlightState(EFlashlightState::OFF);
}

void ACSFlashlight::BeginPlay()
{
	Super::BeginPlay();

	DefaultFlashlightPower = 100.f;
	CurrentFlashlightPower = DefaultFlashlightPower;
	PowerupDecrease = 1.0f;

	/*FVector LightLocation = RootComponent->GetSocketLocation("LightSocket");
	FRotator LightRotation = RootComponent->GetSocketRotation("LightSocket");

	LightRotation.Yaw += 90.0f;

	SpotLight->SetWorldLocationAndRotation(LightLocation, LightRotation);*/
}

void ACSFlashlight::SetFlashlightState(EFlashlightState FlashlightState)
{
	if (FlashlightState == EFlashlightState::ON)
	{
		//SpotLight->SetVisibility(true, false);
	}
	else
	{
		//SpotLight->SetVisibility(false, false);
	}
	this->FlashlightState = FlashlightState;
}

void ACSFlashlight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FlashlightState == EFlashlightState::ON && CurrentFlashlightPower > 0.0f && GetOwningPawn())
	{
		FMath::Clamp(CurrentFlashlightPower, 0.0f, DefaultFlashlightPower);
		CurrentFlashlightPower -= PowerupDecrease * DeltaTime;
	}
	else if (CurrentFlashlightPower <= 0.0f)
	{
		SetFlashlightState(EFlashlightState::OFF);
	}
}

void ACSFlashlight::PowerupFlashlight()
{
	if (FlashlightState == EFlashlightState::OFF)
	{
		if (CurrentFlashlightPower > 0.0f)
		{
			if (ToggleOnSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, ToggleOnSound, GetActorLocation());
			}
			SetFlashlightState(EFlashlightState::ON);
		}
		else
		{
			if (NotPowerSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, NotPowerSound, GetActorLocation());
			}
		}
	}
	else
	{
		if (ToggleOffSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ToggleOffSound, GetActorLocation());
		}
		SetFlashlightState(EFlashlightState::OFF);
	}
}
