// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CSBaseCharacter.generated.h"

class USoundCue;
class UAudioComponent;
class UPawnNoiseEmitterComponent;

UCLASS(ABSTRACT)
class COOPGAME_API ACSBaseCharacter : public ACharacter
{
	GENERATED_BODY()

	UPawnNoiseEmitterComponent* NoiseEmitter;

public:
	// Sets default values for this character's properties
	ACSBaseCharacter();

	UFUNCTION(BlueprintCallable)
	virtual bool IsAlive() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/*  Health and Death  */

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	virtual bool Die(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);

	UFUNCTION(BlueprintCallable)
	virtual void OnDeath(float DamageAmount, struct FDamageEvent const& DamageEvent, class APawn* EventInstigator, class AActor* DamageCauser);

	virtual void SetRagdollPhysics();

	virtual void PlayHit(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser, bool bkilled);

	virtual void ClearWidgetsAfterDeath();

protected:

	/*  Sounds  */

	UAudioComponent* AudioComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* TakeHitSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* DeathSound;

	/*  Movement  */

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float DefaultWalkSpeed;

	/*  Health  */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float DefaultHealth;

	UPROPERTY(BlueprintreadOnly, Category = "Health")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bDied;

	/*  Physics  */

	bool bIsRagdoll;

public:	

	bool IsDyingNow;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
