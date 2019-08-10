// Fill out your copyright notice in the Description page of Project Settings.


#include "..\Public\CSWeapon.h"
#include "..\Public\CSPistol.h"
#include "..\Public\CSShotgun.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "CoopGame.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "..\Public\CSCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimMontage.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw debug lines for weapons"), 
	ECVF_Cheat
);

// Sets default values
ACSWeapon::ACSWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Comp"));
	SphereComponent->SetSphereRadius(50.0f);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ACSWeapon::OnOverlapped);
	SphereComponent->SetupAttachment(SkeletalMeshComponent);

	WeaponName = "Assault Rifle";

	MuzzleSocketName = "Muzzle Socket";
	TracerTargetName = "Target";

	BaseDamage = 450.0f;
	RateOfFire = 600;

	MagazineCapacity = 30;
	MaxBullets = 300;

	CountOfBulletsInMagazine = MagazineCapacity;
	CountOfBulletsOnCharacter = MaxBullets;

	FMath::Clamp(CountOfBulletsInMagazine, 0, MagazineCapacity);
	FMath::Clamp(CountOfBulletsOnCharacter, 0, MaxBullets);

	ReloadingTimeRifleHipAndIronsights = 2.067f;

	SetReplicates(true);
	bNetUseOwnerRelevancy = true;

	DoesHaveOwner = false;

	// When you drop your weapon
	ImpulseMultiplier = 1500.f;

	FiringRange = 4800.0f;

	SpreadOnIdle = 60.0f;
	SpreadOnJogging = 350.0f;
	SpreadOnZooming = 15.0f;
	SpreadOnZoomingMoving = 30.0f;
	SpreadOnCrouchingIdle = 50.0f;
	SpreadOnCrouchingMoving = 150.0f;
	SpreadOnCrouchingZoomingIdle = 5.0f;
	SpreadOnCrouchingZoomingMoving = 10.0f;
}

void ACSWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / RateOfFire;
}

// trace the world from eyes of actor to crosshair location(screen center)
void ACSWeapon::Fire()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	if (CanShoot())
	{
		AActor* Owner = GetOwner();

		if (Owner)
		{
			FVector EyeLocation;
			FRotator EyeRotation;
			Owner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

			FVector ShotDirection = EyeRotation.Vector();

			FVector TraceEnd = EyeLocation + (ShotDirection * FiringRange);
			FVector TracerEndPoint = TraceEnd + CalculateSpread();

			EPhysicalSurface SurfaceType = SurfaceType_Default;

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(Owner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			CountOfBulletsInMagazine--;

			PlayFireSoundEffect();

			FHitResult HitResult;
			if (GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TracerEndPoint, COLLISION_WEAPON, QueryParams))
			{
				// blocking hit and process damage
				AActor* HitActor = HitResult.GetActor();

				SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

				float ActualDamage = BaseDamage;
				if (SurfaceType  == SURFACE_FLESHVULNERABLE)
				{
					ActualDamage *= 5.0f;
				}

				// check if we hit one of our base character
				if (Cast<ACSBaseCharacter>(HitActor))
				{
					FPointDamageEvent PointDamage;
					PointDamage.DamageTypeClass = DamageType;
					PointDamage.HitInfo = HitResult;
					PointDamage.ShotDirection = ShotDirection;
					PointDamage.Damage = ActualDamage;

					HitActor->TakeDamage(PointDamage.Damage, PointDamage, Cast<APawn>(HitActor)->GetController(), Owner);
				}

				PlayImpactEffects(SurfaceType, HitResult.ImpactPoint);

				TracerEndPoint = HitResult.ImpactPoint;
			}

			if (DebugWeaponDrawing > 0)
			{
				DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, false, 1.f, 0, 1.f);
			}

			PlayFireEffects(TracerEndPoint);

			if (Role == ROLE_Authority)
			{
				HitScanTrace.TraceTo = TracerEndPoint;
				HitScanTrace.SurfaceType = SurfaceType;
			}

			LastFireTime = GetWorld()->TimeSeconds;
		}
	}
	else if(!CheckForAmmo())
	{
		if (EmptyMagazineSound)
		{
			UGameplayStatics::PlaySound2D(this, EmptyMagazineSound);
		}
	}
}

void ACSWeapon::SetCurrentSpread(float& Min, float& Max)
{
	EPlayerMovementState CurrentPlayerState = Character->GetCurrentPlayerMovementState();
	float CurrentSpread;

	switch (CurrentPlayerState)
	{
		case IDLE:
			CurrentSpread = SpreadOnIdle;
			break;
		case JOGGING:
			CurrentSpread = SpreadOnJogging;
			break;
		case ZOOMING_IDLE:
			CurrentSpread = SpreadOnZooming;
			break;
		case ZOOMING_MOVING:
			CurrentSpread = SpreadOnZoomingMoving;
			break;
		case CROUCHING:
			CurrentSpread = SpreadOnCrouchingIdle;
			break;
		case CROUCHING_MOVING:
			CurrentSpread = SpreadOnCrouchingMoving;
			break;
		case CROUCHING_ZOOMING_IDLE:
			CurrentSpread = SpreadOnCrouchingZoomingIdle;
			break;
		case CROUCHING_ZOOMING_MOVING:
			CurrentSpread = SpreadOnCrouchingZoomingMoving;
			break;
		default:
			break;
	}

	Max = CurrentSpread;
	Min = CurrentSpread * (-1);
}

FVector ACSWeapon::CalculateSpread()
{
	float Min, Max;
	SetCurrentSpread(Min, Max);

	float DeviationX = FMath::RandRange(Min, Max);
	float DeviationY = FMath::RandRange(Min, Max);
	float DeviationZ = FMath::RandRange(Min, Max);

	FVector Deviation = FVector(DeviationX, DeviationY, DeviationZ);

	return Deviation;
}

void ACSWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX for 
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

float ACSWeapon::PlayWeaponAnimations(UAnimMontage* Animation, float InPlayRate, FName StartSectionName)
{
	float Duration = 0.0f;
	if (Character)
	{
		if (Animation)
		{
			Duration = Character->PlayAnimMontage(Animation, InPlayRate, StartSectionName);
		}
	}
	return Duration;
}

void ACSWeapon::StopWeaponAnimation(UAnimMontage* Animation)
{
	if (Character)
	{
		if (Animation)
		{
			Character->StopAnimMontage(Animation);
		}
	}
	GetWorldTimerManager().ClearTimer(TimerHandle_ReloadingTime);
}

void ACSWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ACSWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ACSWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ACSWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ACSWeapon::ServerFire_Validate()
{
	return true;
}

int32 ACSWeapon::GetCountOfBulletsInMagazine() const
{
	return CountOfBulletsInMagazine;
}

int32 ACSWeapon::GetCountOfBulletsOnCharacter() const
{
	return CountOfBulletsOnCharacter;
}

void ACSWeapon::SetCountOfBulletsOnCharacter(int32 Bullets)
{
	CountOfBulletsOnCharacter += Bullets;
}

int32 ACSWeapon::GetMagazineCapacity() const
{
	return MagazineCapacity;
}

int32 ACSWeapon::GetMaxBullets() const
{
	return MaxBullets;
}

bool ACSWeapon::CheckForAmmo()
{
	if (CountOfBulletsInMagazine > 0)
	{
		return true;
	}
	return false;
}


bool ACSWeapon::CanReload()
{
	if (CountOfBulletsInMagazine < MagazineCapacity && CountOfBulletsOnCharacter > 0 && !bPendingEquip)
	{
		return true;
	}
	return false;
}

bool ACSWeapon::CanShoot()
{
	if (CheckForAmmo() && !ReloadNow && !bPendingEquip)
	{
		return true;
	}
	return false;
}

void ACSWeapon::Reload(bool bFromReplication)
{
	float AnimDuration = PlayWeaponAnimations(ReloadAnim);

	ReloadNow = true;
	Character->ReloadingNow = ReloadNow;

	GetWorldTimerManager().SetTimer(TimerHandle_ReloadingTime, this, &ACSWeapon::ReloadingEnd, AnimDuration, true);

	int32 AmmoToReload = MagazineCapacity - CountOfBulletsInMagazine;
	if (CountOfBulletsOnCharacter >= AmmoToReload)
	{
		CountOfBulletsInMagazine += AmmoToReload;
		CountOfBulletsOnCharacter -= AmmoToReload;
	}
	else
	{
		CountOfBulletsInMagazine += CountOfBulletsOnCharacter;
		CountOfBulletsOnCharacter -= CountOfBulletsOnCharacter;
	}
}

void ACSWeapon::ServerReload_Implementation()
{
	Reload();
}

bool ACSWeapon::ServerReload_Validate()
{
	return true;
}

void ACSWeapon::OnRep_Reload()
{
	if (ReloadNow)
	{
		Reload(true);
	}
	else
	{
		ReloadingEnd();
	}
}

void ACSWeapon::OnRep_MyCharacter()
{
	
}

void ACSWeapon::OnEnterInventory(ACSCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void ACSWeapon::ReloadingEnd()
{
	Character->EndReloading();
	StopWeaponAnimation(ReloadAnim);
	ReloadNow = false;
	Character->ReloadingNow = ReloadNow;
}

void ACSWeapon::PlayFireEffects(FVector TraceEnd)
{
	FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);
	FRotator MuzzleRotation = SkeletalMeshComponent->GetSocketRotation(MuzzleSocketName);

	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, SkeletalMeshComponent, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		
		UParticleSystemComponent* TracerComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComponent)
		{
			TracerComponent->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	AActor* Owner = Cast<APawn>(GetOwner());
	if (Owner)
	{
		APlayerController* PC = Cast<APlayerController>(Owner->GetInstigatorController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCameraShake);
		}
	}

}

void ACSWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		if (BodyImpactSurfaceSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, BodyImpactSurfaceSound, ImpactPoint);
		}
		break;
	default:
		SelectedEffect = ImpactEffect;
		if (DefaultImpactSurfaceSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, DefaultImpactSurfaceSound, ImpactPoint);
		}
		break;
	}

	if (ImpactEffect)
	{
		FVector MuzzleLocation = SkeletalMeshComponent->GetSocketLocation("Muzzle Socket");
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void ACSWeapon::PlayFireSoundEffect()
{
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
}

void ACSWeapon::OnEquip()
{
	if (ReloadNow || bPendingEquip)
	{
		return;
	}

	bPendingEquip = true;

	GetWorldTimerManager().SetTimer(TimerHandle_EquipWeaponStopAnimationTime, this, &ACSWeapon::OnEquipStopAnimation, 0.05f, false);

	if (EquipSound)
	{
		UGameplayStatics::PlaySound2D(this, EquipSound);
	}

	if (Character)
	{
		AttachItemToCharacter(Character->WeaponAttachSocketName);
	}

}

void ACSWeapon::OnUnEquip(FName SocketName)
{
	if (UnEquipWeaponSound)
	{
		UGameplayStatics::PlaySound2D(this, UnEquipWeaponSound);
	}

	FTimerDelegate UnEquipDelegate = FTimerDelegate::CreateUObject(this, &ACSWeapon::OnUnEquipStopAnimation, SocketName);
	GetWorldTimerManager().SetTimer(TimerHandle_UnEquipWeaponStopAnimationTime, UnEquipDelegate, 0.05f, false);
}

void ACSWeapon::OnEquipFinished()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_EquipWeaponTime);
	StopWeaponAnimation(EquipAnim);

	
}

void ACSWeapon::OnEquipStopAnimation()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_EquipWeaponStopAnimationTime);
	bPendingEquip = false;
	StopWeaponAnimation(EquipAnim);
	Character->EndEquiping();
}

void ACSWeapon::OnUnEquipStopAnimation(FName SocketName)
{
	AttachItemToCharacter(SocketName);
	if (UnEquipWeaponSound)
	{
		UGameplayStatics::PlaySound2D(this, UnEquipWeaponSound);
	}
	GetWorldTimerManager().ClearTimer(TimerHandle_UnEquipWeaponStopAnimationTime);
	StopWeaponAnimation(UnEquipAnim);
	Character->EndUnEquiping();
}

void ACSWeapon::AttachItemToCharacter(FName SocketName)
{
	if (Character)
	{
		DetachWeaponFromCharacter();

		SkeletalMeshComponent->SetHiddenInGame(false);
		SkeletalMeshComponent->SetActive(true);
		SkeletalMeshComponent->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	}
}

bool ACSWeapon::OnDropping()
{
	DetachWeaponFromCharacter();
	SkeletalMeshComponent->SetSimulatePhysics(true);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SkeletalMeshComponent->SetHiddenInGame(false);
	SkeletalMeshComponent->SetActive(true);

	SkeletalMeshComponent->AddImpulseAtLocation(Character->GetActorForwardVector() * ImpulseMultiplier, GetActorLocation());
	return true;
}

void ACSWeapon::DetachWeaponFromCharacter()
{
	SkeletalMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	SkeletalMeshComponent->SetHiddenInGame(true);
	SkeletalMeshComponent->SetActive(false);
}

void ACSWeapon::OnOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		ACSCharacter* Character = Cast<ACSCharacter>(OtherActor);

		
	}
}

void ACSWeapon::PickupItem(ACSCharacter* Character)
{
	if (CanPickUp())
	{
		if (Character->CurrentWeapon && Character->CurrentWeapon->ReloadNow)
		{
			return;
		}

		SkeletalMeshComponent->SetSimulatePhysics(false);
		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		ACSWeapon* Item;

		if (WeaponType.Get() == Character->PistolWeapon.Get())
		{
			Item = Character->GetLightWeaponSlot();
		}

		else if (WeaponType.Get() == Character->ShotgunWeapon.Get())
		{
			Item = Character->GetMiddleWeaponSlot();
		}

		else
		{
			Item = Character->GetHardWeaponSlot();
		}

		if (!Character->CurrentWeapon)
		{
			HandleItemPickup(Character, Item);
		}
		else if (Character->CurrentWeapon)
		{
			HandleItemPickup(Character, Item, Character->CurrentWeapon);
		}

	}
}

FName ACSWeapon::GetMuzzleSocket() const
{
	return MuzzleSocketName;
}

void ACSWeapon::HandleItemPickup(ACSCharacter* Character, ACSWeapon* Weapon, ACSWeapon* CurrentWeapon)
{

	if (CurrentWeapon && Weapon && CurrentWeapon->GetClass() == Weapon->GetClass())
	{
		Character->DropWeapon();
		Character->AddWeapon(this);
	}

	// if character does not have that type of weapon
	if (!Weapon)
	{
		Character->AddWeapon(this);
	}
	// if character has that type of weapon, but does not have max ammo, then we add ammo
	else if (Weapon && Weapon->GetClass() == WeaponType.Get() && Weapon->GetCountOfBulletsOnCharacter() < Weapon->GetMaxBullets())
	{
		Weapon->SetCountOfBulletsOnCharacter(Weapon->GetCountOfBulletsOnCharacter() + MagazineCapacity);
		Destroy();
	}
}

void ACSWeapon::GetLifetimeReplicatedProps(TArray < class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ACSWeapon, HitScanTrace, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ACSWeapon, ReloadNow, COND_SkipOwner);

	DOREPLIFETIME(ACSWeapon, CountOfBulletsInMagazine);
	DOREPLIFETIME(ACSWeapon, CountOfBulletsOnCharacter);
	DOREPLIFETIME(ACSWeapon, Character);
}

void ACSWeapon::ClientReload_Implementation()
{
	Reload();
}

