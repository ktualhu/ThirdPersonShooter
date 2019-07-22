// Fill out your copyright notice in the Description page of Project Settings.


#include "..\Public\CSHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCSHealthComponent::UCSHealthComponent()
{
	DefaultHealth = 1000.f;
	BasePercentHealth = 100.f;

	FMath::Clamp(CurrentPercentHealth, 0.0f, BasePercentHealth);
	FMath::Clamp(Health, 0.f, DefaultHealth);
	FMath::Clamp(ProgressBarCurrentPercentHealth, 0.0f, 1.0f);

	SetIsReplicated(true);
}


// Called when the game starts
void UCSHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* Owner = GetOwner();

		if (Owner)
		{
			Owner->OnTakeAnyDamage.AddDynamic(this, &UCSHealthComponent::HandleTakeAnyDamage);
		}
	}
	
	Health = DefaultHealth;
	CurrentPercentHealth = BasePercentHealth;
	ProgressBarCurrentPercentHealth = 1.0f;
}

void UCSHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0)
	{
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.f, DefaultHealth);
	CurrentPercentHealth = Health * 100 / DefaultHealth;
	ProgressBarCurrentPercentHealth = CurrentPercentHealth / BasePercentHealth;

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}

void UCSHealthComponent::GetLifetimeReplicatedProps(TArray < class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCSHealthComponent, Health);
	DOREPLIFETIME(UCSHealthComponent, CurrentPercentHealth);
	DOREPLIFETIME(UCSHealthComponent, ProgressBarCurrentPercentHealth);
}




