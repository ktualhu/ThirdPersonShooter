// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSItem.generated.h"

class ACSCharacter;
class USoundBase;
class USkeletalMeshComponent;

UCLASS()
class COOPGAME_API ACSItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACSItem();

	virtual void SetOwningPawn(ACSCharacter* Character);

	virtual ACSCharacter* GetOwningPawn() const;

	virtual void OnEquip();

	virtual void AttachItemToCharacter(FName SocketName);

	virtual void PickupItem(ACSCharacter* Character);

	virtual USkeletalMeshComponent* GetSkeletalMeshComponent() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual bool CanPickUp();

	virtual void DetachWeaponFromCharacter();


protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Base Info")
	ACSCharacter* Character;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Base Info")
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* EquipSound;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
