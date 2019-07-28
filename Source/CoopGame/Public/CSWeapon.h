// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSWeapon.generated.h"

class USkeletalMeshComponent;
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
class COOPGAME_API ACSWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACSWeapon();

	virtual void BeginPlay() override;

	virtual void SetOwningPawn(ACSCharacter* Character);

	virtual void OnEquip();

	virtual void OnEnterInventory(ACSCharacter* NewOwner);

	virtual void AttachWeaponToCharacter(FName SocketName);

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

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

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

	virtual void DetachWeaponFromCharacter();

	/*  Net  */

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_MyCharacter();

	UFUNCTION()
	void OnRep_HitScanTrace();

	UFUNCTION()
	void OnRep_Reload();

protected:

	UPROPERTY(Transient, ReplicatedUsing = OnRep_MyCharacter, EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	ACSCharacter* Character;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* ImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCameraShake;

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
	USoundBase* EquipWeaponSound;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	/* Bullets per minute fired by weapon */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	/* Derived from RateOfFire*/
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 MagazineCapacity;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int32 MaxBullets;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	int32 CountOfBulletsInMagazine;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	int32 CountOfBulletsOnCharacter;

	float ReloadingTimeRifleHipAndIronsights;

	FTimerHandle TimerHandle_ReloadingTime;

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;


	/*  Animation  */
	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	UAnimMontage* ReloadAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	UAnimMontage* EquipAnim;

	bool bPendingEquip;

	FTimerHandle TimerHandle_EquipWeaponTime;;

public:
	virtual void StartFire();

	virtual void StopFire();

	virtual void Reload(bool bFromReplication = false);

	virtual bool CanReload();

	virtual bool CanShoot();

public:
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
	bool ReloadNow;

	bool IsFireNow;

private:
	float AnimDuration;
};
