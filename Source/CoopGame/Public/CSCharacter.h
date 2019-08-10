// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "..\Public\CSBaseCharacter.h"
#include "CSCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USoundBase;
class ACSWeapon;
class USoundCue;
class ACSShotgun;
class ACSPistol;
class ACSFlashlight;
class ACSBaseCharacter;
class UAnimMontage;

UENUM()
enum EPlayerMovementState
{
	IDLE,
	JOGGING,
	ZOOMING_IDLE,
	ZOOMING_MOVING,
	CROUCHING,
	CROUCHING_MOVING,
	CROUCHING_ZOOMING_IDLE,
	CROUCHING_ZOOMING_MOVING
};

UCLASS()
class COOPGAME_API ACSCharacter : public ACSBaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACSCharacter();

	/*  Weapon Sockets  */

	// Weapon on hands
	UPROPERTY(VisibleDefaultsOnly, Category = "ItemSockets")
	FName WeaponAttachSocketName;

	// Pistol
	UPROPERTY(VisibleDefaultsOnly, Category = "ItemSockets")
	FName PistolAttachSocketName;

	// Shotgun
	UPROPERTY(VisibleDefaultsOnly, Category = "ItemSockets")
	FName ShotgunAttachSocketName;

	// Rifle / Grenade launcher
	UPROPERTY(VisibleDefaultsOnly, Category = "ItemSockets")
	FName BackWeaponAttachSocketName;

	UPROPERTY(VisibleDefaultsOnly, Category = "ItemSockets")
	FName FlashlightAttachSocketName;

	/*  Weapons For Pickup  */

	// check if we're equiping shotgun for animation purposes
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ACSShotgun> ShotgunWeapon;

	// check if we're equiping pistol for animation purposes
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ACSPistol> PistolWeapon;

	// check if we're equiping pistol for animation purposes
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ACSWeapon> MainWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ACSFlashlight> FlashlightClass;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon, BlueprintReadOnly, Category = "Weapons")
	ACSWeapon* CurrentWeapon;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool ReloadingNow;

	AActor* GrabbedItem;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitAllBaseCharacterFeatures();

	/*  Weapon initialization(when we're spawning with all weapons)  */

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

	void FireShotgun();

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

	void EquipWeapon(ACSWeapon* NewWeapon = nullptr, ACSWeapon* PrevWeapon = nullptr, ACSWeapon* ThirdWeapon = nullptr);

	void EquipWeaponAfterPickup(ACSWeapon* NewWeapon, ACSWeapon* CurrentWeapon = nullptr);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipWeapon(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon = nullptr);

	void SetCurrentWeapon(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon = nullptr, ACSWeapon* ThirdWeapon = nullptr);

	void GetFirstWeaponSlot();

	void GetSecondWeaponSlot();

	void GetThirdWeaponSlot();

	void FlashlightPowerOnOff();

	/*  Pickup  */

	void DrawDebugTraceLineForPickup(FVector& StartPoint, FVector& EndPoint);

	void GrabItem();

	void ShowGrabItemWidget();

	/*  Death  */

	virtual void PlayHit(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser, bool bkilled) override;

	void StopPlayHitAnimation();

	virtual void OnDeath(float DamageAmount, struct FDamageEvent const& DamageEvent, class APawn* EventInstigator, class AActor* DamageCauser) override;

	/*  Net  */

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentWeapon(ACSWeapon* NewWeapon);

	/*  Aiming  */

	UFUNCTION(BlueprintCallable)
	FRotator GetAimOffset() const;

	/*  Animations  */
	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	UAnimMontage* HitAnimation1;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	UAnimMontage* HitAnimation2;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	UAnimMontage* HitAnimation3;

	UPROPERTY(EditDefaultsOnly, Category = "Sounds")
	UAnimMontage* HitAnimation4;

	virtual void ClearWidgetsAfterDeath() override;

	void CheckPlayerState();

protected:

	/*  Player State  */

	EPlayerMovementState PlayerMovementState;

	/*  Components  */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComponent;

	/*  Zooming  */

	bool GettingHitNow;

	bool bWantsToZoom;

	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Zooming")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Zooming")
	float SprintFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Zooming")
	float SniperRifleFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Zooming", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;

	UPROPERTY(BlueprintReadOnly)
	bool IsSniperRifleZooming;

	UPROPERTY(BlueprintReadOnly)
	bool ShowGrabWidget;

	uint8 PickedAnimationNumber;

	/*  Weapons  */

	UPROPERTY(BlueprintReadOnly)
	ACSFlashlight* Flashlight;

	UPROPERTY(Replicated)
	TArray<ACSWeapon*> Weapons;

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Weapons")
	TArray<TSubclassOf<ACSWeapon>> StarterWeaponClasses;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ACSWeapon> FirstWeaponClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ACSWeapon> SecondWeaponClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ACSWeapon> ThirdWeaponClass;

	// check if we're equiping sniper rifle for zooming purposes
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ACSWeapon> SniperRifleWeapon;

	FActorSpawnParameters SpawnParams;

	/*  Weapon Slots  */

	// pistol [waist]
	ACSWeapon* LightSlot;

	// shotgun / uzi(maybe) [left leg]
	ACSWeapon* MiddleSlot;

	// rifle / grenade laucher [back]
	ACSWeapon* HardSlot;

	UPROPERTY(BlueprintReadOnly)
	bool IsShotgunEquiped;

	UPROPERTY(BlueprintReadOnly)
	bool IsPistolEquiped;

	bool IsFlashlightPickedup;

	bool IsFlashlightEquiped;

	/*  Gameplay bools  */

	UPROPERTY(BlueprintReadOnly)
	bool IsUnarmed;

	UPROPERTY(BlueprintReadOnly)
	bool EquipingNow;

	UPROPERTY(BlueprintReadOnly)
	bool ChangingWeaponNow;

	UPROPERTY(BlueprintReadOnly)
	bool IsZoomingNow;

	UPROPERTY(BlueprintReadOnly)
	bool IsSprintingNow;

	bool IsCrouchingNow;


	/*  Sprint and Walking  */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint and Walking")
	float SprintMultiplier;

	UPROPERTY(EditDefaultsOnly, Category = "Sprint and Walking")
	float DefaultSprintProgress;

	UPROPERTY(BlueprintReadOnly)
	float CurrentSprintProgress;

	UPROPERTY(EditDefaultsOnly, Category = "Sprint and Walking")
	float SprintDecrease;

	UPROPERTY(EditDefaultsOnly, Category = "Sprint and Walking")
	float SprintIncrease;

	UPROPERTY(BlueprintReadOnly)
	bool ShowSprintWidget;

	

	/*  Time  */

	FTimerHandle TimerHandle_RemoveWeaponTime;

	FTimerHandle TimerHandle_BreakTime;

	FTimerHandle TimerHandle_DeathTime;

	FTimerHandle TimerHandle_GettingHitTime;

	float BreakTime;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	EPlayerMovementState GetCurrentPlayerMovementState() const;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	TArray<ACSWeapon*> GetCharacterWeapons() const;

	void EndReloading();

	void EndEquiping();

	void EndUnEquiping();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndReloading();

	bool isMoving();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Player")
	bool IsFiringNow;

	// Weapon slots
	ACSWeapon* GetLightWeaponSlot() const;

	ACSWeapon* GetMiddleWeaponSlot() const;

	ACSWeapon* GetHardWeaponSlot() const;

	void SetFlashlight(ACSFlashlight* Flashlight);

	void AddWeapon(ACSWeapon* NewWeapon, ACSWeapon* SecondWeapon = nullptr, ACSWeapon* ThirdWeapon = nullptr);

	/*  Drop Weapon  */

	void DropWeapon();
};
