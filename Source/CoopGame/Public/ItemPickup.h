// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemPickup.generated.h"

class UStaticMesh;
class UCapsuleComponent;
class USoundBase;

UCLASS()
class COOPGAME_API AItemPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemPickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Mesh")
	UCapsuleComponent* CapsuleComp;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundBase* PickupSound;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
