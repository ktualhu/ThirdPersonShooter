// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "..\Public\CSItem.h"
#include "CSWeapon.generated.h"

class USkeletalMeshComponent;
class USphereComponent;
class UDamageType;
class UParticleSystem;
class UCameraShake;
class USoundBase;
class ACSCharacter;
class UAnimMontage;

USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};

UCLASS()
class COOPGAME_API ACSWeapon : public ACSItem
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACSWeapon();

	virtual void BeginPlay() override;

	virtual void OnEquip() override;

	virtual void OnUnEquip(FName SocketName);

	virtual void OnEnterInventory(ACSCharacter* NewOwner);

	virtual void AttachItemToCharacter(FName SocketName) override;

	virtual bool OnDropping();

	virtual void PickupItem(ACSCharacter* Character) override;

	FName GetMuzzleSocket() const;

	/*  Ammo Info  */
	virtual int32 GetCountOfBulletsInMagazine() const;

	virtual int32 GetCountOfBulletsOnCharacter() const;

	virtual void SetCountOfBulletsOnCharacter(int32 Bullets);

	virtual int32 GetMagazineCapacity() const;

	virtual int32 GetMaxBullets() const;

protected:

	/*  Fire and Fire effects  */
	void PlayFireEffects(FVector TraceEnd);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	void PlayFireSoundEffect();

	UFUNCTION(BlueprintCallable)
	virtual void Fire();

	virtual void SetCurrentSpread(float& Min, float& Max);

	FVector CalculateSpread();

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerFire();

	/*  Reload and Ammo  */

	virtual bool CheckForAmmo();

	virtual void ReloadingEnd();

	UFUNCTION(Reliable, Client)
	void ClientReload();

	void ClientReload_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReload();

	void ServerReload_Implementation();

	bool ServerReload_Validate();

	/*  Animation methods  */

	float PlayWeaponAnimations(UAnimMontage* Animation, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	void StopWeaponAnimation(UAnimMontage* Animation);

	/*  Equip  */

	virtual void OnEquipFinished();

	virtual void OnEquipStopAnimation();

	UFUNCTION()
	virtual void OnUnEquipStopAnimation(FName SocketName);

	virtual void DetachWeaponFromCharacter() override;

	/*  Pickup  */

	UFUNCTION()
	virtual void OnOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//virtual bool CanPickUp();

	virtual void HandleItemPickup(ACSCharacter* Character, ACSWeapon* Weapon, ACSWeapon* CurrentWeapon = nullptr);

	/*  Net  */

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_MyCharacter();

	UFUNCTION()
	void OnRep_HitScanTrace();

	UFUNCTION()
	void OnRep_Reload();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Base Info")
	FName WeaponName;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Base Info")
	TSubclassOf<ACSWeapon> WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Base Info")
	USphereComponent* SphereComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Base Info")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon Base Info")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon Base Info")
	FName TracerTargetName;

	/*  VFX  */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UParticleSystem* ImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TSubclassOf<UCameraShake> FireCameraShake;

	/*  Sounds  */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* EmptyMagazineSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* ReloadMagazineSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* BodyImpactSurfaceSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* DefaultImpactSurfaceSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* UnEquipWeaponSound;

	/*  Firing  */

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UPROPERTY(EditDefaultsOnly, Category = "Firing")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Firing")
	float RateOfFire;

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, Category = "Firing")
	float FiringRange;

	/*  Firing Spread  */

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadOnIdle;
	
	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadOnJogging;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadOnZooming;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadOnZoomingMoving;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadOnCrouchingIdle;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadOnCrouchingMoving;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadOnCrouchingZoomingIdle;

	UPROPERTY(EditDefaultsOnly, Category = "Spread")
	float SpreadOnCrouchingZoomingMoving;

	/*  Ammo  */

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 MagazineCapacity;

	UPROPERTY(EditDefaultsOnly, Category = "Ammo")
	int32 MaxBullets;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ammo")
	int32 CountOfBulletsInMagazine;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ammo")
	int32 CountOfBulletsOnCharacter;

	/*  Reloading  */

	float ReloadingTimeRifleHipAndIronsights;

	FTimerHandle TimerHandle_ReloadingTime;

	/*  Pickup  */

	bool DoesHaveOwner;

	/*  Drop  Weapon  */

	UPROPERTY(EditDefaultsOnly, Category = "Drop Weapon")
	float ImpulseMultiplier;

	/*  Animation  */
	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	UAnimMontage* ReloadAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	UAnimMontage* EquipAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	UAnimMontage* UnEquipAnim;

	bool bPendingEquip;

	FTimerHandle TimerHandle_EquipWeaponTime;

	FTimerHandle TimerHandle_EquipWeaponStopAnimationTime;

	//FDelegateBase TimerDelegate_UnEquipWeapon;

	FTimerHandle TimerHandle_UnEquipWeaponStopAnimationTime;

public:
	virtual void StartFire();

	virtual void StopFire();

	virtual void Reload(bool bFromReplication = false);

	virtual bool CanReload();

	virtual bool CanShoot();

public:
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload, BlueprintReadOnly)
	bool ReloadNow;

	UPROPERTY(BlueprintReadOnly)
	bool IsFireNow;

	bool IsAbleToFire;

private:
	float AnimDuration;
};
