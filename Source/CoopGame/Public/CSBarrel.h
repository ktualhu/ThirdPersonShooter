// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSBarrel.generated.h"

class UCSHealthComponent;
class UParticleSystem;
class USoundBase;
class UMaterial;
class URadialForceComponent;

UCLASS()
class COOPGAME_API ACSBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACSBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHealthChanged(UCSHealthComponent* HealthComp, float Health, float HealthDelta, const UDamageType* DamageType,
		AController* InstigatedBy, AActor* DamageCauser);
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Barrel Health")
	UCSHealthComponent* HealthComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion")
	USoundBase* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion")
	UMaterial* ExplosionMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explosion")
	UStaticMeshComponent* BarrelMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explosion")
	URadialForceComponent* RadialForceComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion")
	float ExplosionImpulseStrenght;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion")
	float ExplosionForceStrenght;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(Replicated)
	bool bIsExplosed;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void PlayExplosionEffects();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastExplosionEffects();
};
