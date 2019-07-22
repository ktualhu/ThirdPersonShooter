// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CSCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USoundBase;
class ACSWeapon;
class UCShealthComponent;

UCLASS()
class COOPGAME_API ACSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACSCharacter();

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName BackWeaponAttachSocketName;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForvard(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();

	void BeginJump();

	void BeginZoom();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginZoom();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastBeginZoom();

	void EndZoom();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndZoom();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastEndZoom();

	void StartFire();

	void StopFire();

	void BeginSprint();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginSprint();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastBeginSprint();

	void EndSprint();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndSprint();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastEndSprint();

	void ReloadMagazine();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadMagazine();

	void ServerReloadMagazine_Implementation();

	bool ServerReloadMagazine_Validate();

	void EquipWeapon(ACSWeapon* NewWeapon);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipWeapon(ACSWeapon* NewWeapon);

	void SetCurrentWeapon(ACSWeapon* NewWeapon);

	void GetFirstWeaponSlot();

	void GetSecondWeaponSlot();

	void AddWeapon(ACSWeapon* NewWeapon);

	/*UFUNCTION(Reliable, NetMulticast)
	void MulticastReloadMagazine();*/

	/*void TakeFirstWeapon();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTakeFirstWeapon();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastTakeFirstWeapon();

	void MulticastTakeFirstWeapon_Implementation();

	void TakeSecondWeapon();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTakeSecondWeapon();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastTakeSecondWeapon();

	void MulticastTakeSecondWeapon_Implementation();*/

	void AttachWeapon();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAttachWeapon();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastAttachWeapon();

	void MulticastAttachWeapon_Implementation();

	void DetachWeapon();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDetachWeapon();

	UFUNCTION(Reliable, NetMulticast)
	void MulticastDetachWeapon();

	void MulticastDetachWeapon_Implementation();

	/*UFUNCTION(Server, Reliable, WithValidation)
	void ServerDetachWeapon();*/

	void InitAllWeapons();

	void SaveWeaponInfoAfterDetach(uint8 WeaponIndex);

	void RemoveWeapon();

	void TakeWeapon();

	UFUNCTION()
	void OnHealthChanged(UCSHealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	FRotator GetAimOffset() const;

	UFUNCTION()
	void OnRep_CurrentWeapon(ACSWeapon* NewWeapon);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* StepSound;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Player")
	TArray<TSubclassOf<ACSWeapon>> StarterWeaponClasses;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon, BlueprintReadOnly, Category = "Weapon")
	ACSWeapon* CurrentWeapon;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player")
	bool ReloadingNow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player")
	bool ChangingWeaponNow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player")
	bool IsZoomingNow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player")
	bool IsSprintingNow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Walking")
	float SprintMultiplier;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float DefaultWalkSpeed;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player")
	UCSHealthComponent* HealthComponent;

	UPROPERTY(Replicated)
	ACSWeapon* BackWeapon;

	// Default field of view
	float DefaultFOV;

	bool bWantsToZoom;

	UPROPERTY(Replicated)
	TArray<ACSWeapon*> Weapons;

	FActorSpawnParameters SpawnParams;

	UPROPERTY(Replicated)
	uint8 WeaponIndex;

	FTimerHandle TimerHandle_RemoveWeaponTime;

	FTimerHandle TimerHandle_BreakTime;

	float EquipTime;

	float BreakTime;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	void EndReloading();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndReloading();

	bool isMoving();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player")
	bool IsFiringNow;
};
