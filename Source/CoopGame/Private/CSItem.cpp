// Fill out your copyright notice in the Description page of Project Settings.


#include "CSItem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "..\Public\CSCharacter.h"
#include "..\Public\CSFlashlight.h"

// Sets default values
ACSItem::ACSItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh Component"));
	RootComponent = SkeletalMeshComponent;

	SetOwningPawn(nullptr);
}

// Called when the game starts or when spawned
void ACSItem::BeginPlay()
{
	Super::BeginPlay();

	SkeletalMeshComponent->SetSimulatePhysics(true);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//SkeletalMeshComponent->SetWorldScale3D(FVector(1.0f));
	
}

void ACSItem::SetOwningPawn(ACSCharacter* NewOwner)
{
	if (Character != NewOwner)
	{
		Instigator = NewOwner;
		Character = NewOwner;
		// Net owner for RPC calls.
		SetOwner(NewOwner);
	}
}

ACSCharacter* ACSItem::GetOwningPawn() const
{
	if (Character)
	{
		return Character;
	}
	return nullptr;
}

void ACSItem::OnEquip()
{
	if (EquipSound)
	{
		UGameplayStatics::PlaySound2D(this, EquipSound);
	}

	if (Character)
	{
		AttachItemToCharacter(Character->WeaponAttachSocketName);
	}

}

void ACSItem::AttachItemToCharacter(FName SocketName)
{
	if (Character)
	{
		DetachWeaponFromCharacter();
		SkeletalMeshComponent->SetHiddenInGame(false);
		SkeletalMeshComponent->SetActive(true);
		SkeletalMeshComponent->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	}
}

void ACSItem::DetachWeaponFromCharacter()
{
	SkeletalMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	SkeletalMeshComponent->SetHiddenInGame(true);
	SkeletalMeshComponent->SetActive(false);
}

bool ACSItem::CanPickUp()
{
	if (GetOwningPawn())
	{
		return false;
	}
	return true;
}

void ACSItem::PickupItem(ACSCharacter* Character)
{
	if (CanPickUp())
	{
		if (this->GetClass() == Character->FlashlightClass.Get())
		{
			SkeletalMeshComponent->SetSimulatePhysics(false);
			SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			//SkeletalMeshComponent->SetWorldScale3D(FVector(0.6f));

			if (EquipSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
			}
			
			Character->SetFlashlight(Cast<ACSFlashlight>(this));
			SetOwningPawn(Character);

			AttachItemToCharacter(Character->FlashlightAttachSocketName);
		}
	}
}

USkeletalMeshComponent* ACSItem::GetSkeletalMeshComponent() const
{
	return SkeletalMeshComponent;
}

// Called every frame
void ACSItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

