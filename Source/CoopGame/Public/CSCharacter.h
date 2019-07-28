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
class USoundCue;
class UAudioComponent;

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

	/*  Weapon initialization  */

	void InitAllWeapons();

	/*  Movement  */

	void MoveForvard(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();

	void BeginJump();

	/*  Zoom  */

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

	void SetFOVCameraView(float DeltaTime);

	/*  Fire  */

	void StartFire();

	void StopFire();

	/*  Sprint  */

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

	void HandleSprintWidget(float Delta);

	bool CanSprint();

	// if CurrentSprintProgress < 100.0f or Character is sprinting now then show Sprint Widget 
	void SetSprintWidget();

	/*  Reloading Weapon  */

	void ReloadMagazine();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadMagazine();

	void ServerReloadMagazine_Implementation();

	bool ServerReloadMagazine_Validate();

	/*  Equiping Weapon  */

	void EquipWeapon(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon = nullptr);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipWeapon(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon = nullptr);

	void SetCurrentWeapon(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon = nullptr);

	void GetFirstWeaponSlot();

	void GetSecondWeaponSlot();

	void AddWeapon(ACSWeapon* NewWeapon, ACSWeapon* SecondWeapon);

	/*  Health and Death  */

	UFUNCTION()
	void OnHealthChanged(UCSHealthComponent* HealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/*  Net  */

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentWeapon(ACSWeapon* NewWeapon);

	/*  Aiming  */

	UFUNCTION(BlueprintCallable)
	FRotator GetAimOffset() const;

protected:

	/*  Camera  */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComponent;

	// Default field of view
	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float SprintFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;

	/*  AudioComponent and Sounds  */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAudioComponent* AudioComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* StepSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundCue* DeathSound;

	/*  Weapons  */

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Weapon")
	TArray<TSubclassOf<ACSWeapon>> StarterWeaponClasses;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon, BlueprintReadOnly, Category = "Weapon")
	ACSWeapon* CurrentWeapon;

	UPROPERTY(Replicated)
	ACSWeapon* BackWeapon;

	// check if we're equiping shotgun for animation purposes

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<ACSWeapon> ShotgunWeapon;

	UPROPERTY(BlueprintReadOnly)
	bool IsShotgunEquiped;

	/*  Reloading, Sprinting, Zooming, Dying bools  */

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	bool ReloadingNow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	bool ChangingWeaponNow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player")
	bool IsZoomingNow;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player")
	bool IsSprintingNow;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;

	/*  Sprint and Walking  */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Walking")
	float SprintMultiplier;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float DefaultWalkSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	float DefaultSprintProgress;

	UPROPERTY(BlueprintReadOnly)
	float CurrentSprintProgress;

	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	float SprintDecrease;

	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	float SprintIncrease;

	UPROPERTY(BlueprintReadOnly)
	bool ShowSprintWidget;

	/*  Health  */

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player")
	UCSHealthComponent* HealthComponent;

	bool bWantsToZoom;

	UPROPERTY(Replicated)
	TArray<ACSWeapon*> Weapons;

	FActorSpawnParameters SpawnParams;

	UPROPERTY(Replicated)
	uint8 WeaponIndex;

	FTimerHandle TimerHandle_RemoveWeaponTime;

	FTimerHandle TimerHandle_BreakTime;

	FTimerHandle TimerHandle_DeathTime;

	float EquipTime;

	float BreakTime;

	UPROPERTY(Replicated)
	bool bIsRagdoll;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	TArray<ACSWeapon*> GetCharacterWeapons() const;

	void EndReloading();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndReloading();

	bool isMoving();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player")
	bool IsFiringNow;
};
