// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CSBaseCharacter.h"
#include "CSZombieCharacter.generated.h"

class UPawnSensingComponent;
class UAnimMontage;
class UCapsuleComponent;
class USoundCue;
class UAudioComponent;

/**
 * 
 */
UCLASS(ABSTRACT)
class COOPGAME_API ACSZombieCharacter : public ACSBaseCharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "AI")
	class UPawnSensingComponent* PawnSensingComponent;

	float LastHeardTime;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SensedTimeOut;

	bool SensedTarget;

public:

	ACSZombieCharacter();

	UPROPERTY(EditDefaultsOnly, Category = "Behavior")
	class UBehaviorTree* BotBehavior;

	virtual void Tick(float DeltaTime) override;

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSeePlayer(APawn* Pawn);

	UFUNCTION()
	void OnHearNoise(APawn* Pawn, const FVector& Location, float Volume);

	UFUNCTION()
	void OnMeleeComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	virtual void PlayHit(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser, bool bkilled) override;

	void StopPlayAnimations();

	void AudioLoop(bool SensedTarget);

	void DoMeleeAttack(AActor* AttackedActor);

	void RetriggerMeleeAttack();

	void PlayAttackEffects();

	UAudioComponent* PlayCharacterSound(USoundCue* MeleeAttackSound);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* HitAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* MeleeAttackAnimMontage1;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* MeleeAttackAnimMontage2;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* MeleeAttackAnimMontage3;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* MeleeAttackAnimMontage4;

	FTimerHandle TimerHandle_HitAnimation;

	FTimerHandle TimerHandle_MeleeAttack;

	bool HitPlaying;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* IdleSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* WonderingSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* HuntingSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	USoundCue* MeleeAttackSound;

	UPROPERTY(EditDefaultsOnly, Category = "Perception")
	float ZombieSightRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Perception")
	float ZombiePeripheralVisionAngle;

	UPROPERTY(EditDefaultsOnly, Category = "Perception")
	float ZombieHearingThreshold;

	UPROPERTY(EditDefaultsOnly, Category = "Perception")
	float ZombieLOSHearingThreshold;

	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	UCapsuleComponent* MeleeCollisionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float MeleeCapsuleHalfHeight;

	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float MeleeCapsuleRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float MeleeDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Melee")
	float MeleeAttackCooldown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee")
	TSubclassOf<UDamageType> ZombieDamageType;

	float LastMeleeAttack;

	UPROPERTY(BlueprintReadOnly)
	bool IsWonder;

};
