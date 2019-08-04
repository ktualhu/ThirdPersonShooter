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

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh Component"));
	RootComponent = SkeletalMeshComponent;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Comp"));
	SphereComponent->SetSphereRadius(50.0f);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ACSWeapon::OnOverlapped);
	SphereComponent->SetupAttachment(SkeletalMeshComponent);

	WeaponName = "Assault Rifle";

	MuzzleSocketName = "Muzzle Socket";
	TracerTargetName = "Target";

	BaseDamage = 100.0f;
	RateOfFire = 600;

	MagazineCapacity = 30;
	MaxBullets = 900;

	CountOfBulletsInMagazine = MagazineCapacity;
	CountOfBulletsOnCharacter = MaxBullets;

	FMath::Clamp(CountOfBulletsInMagazine, 0, MagazineCapacity);
	FMath::Clamp(CountOfBulletsOnCharacter, 0, MaxBullets);

	ReloadingTimeRifleHipAndIronsights = 2.067f;

	//Character = Cast<ACSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	SetReplicates(true);
	bNetUseOwnerRelevancy = true;

	DoesHaveOwner = false;

	ImpulseMultiplier = 1500.f;
}

void ACSWeapon::BeginPlay()
{
	Super::BeginPlay();

	SkeletalMeshComponent->SetSimulatePhysics(true);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	TimeBetweenShots = 60 / RateOfFire;
}

// trace the world from eyes of actor to crosshair location(screen center)
void ACSWeapon::Fire()
{
	// Проверяем, если клиент вызывает функцию Fire(), тогда мы вызываем серверную реализацию этйо функции
	// Логика функции Fire() исполняется на стороне сервера
	// Клиент отправляет запрос серверу, что ему необходима эта функцию и сервер вызывает её
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

			FVector TraceEnd = EyeLocation + (ShotDirection * 10000);
			FVector TracerEndPoint = TraceEnd; // Particle target param

			EPhysicalSurface SurfaceType = SurfaceType_Default;

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(Owner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			CountOfBulletsInMagazine--;

			PlayFireSoundEffect();

			FHitResult HitResult;
			if (GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
			{
				// blocking hit and process damage
				AActor* HitActor = HitResult.GetActor();

				SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());

				float ActualDamage = BaseDamage;
				if (SurfaceType  == SURFACE_FLESHVULNERABLE)
				{
					ActualDamage *= 5.0f;
				}

				UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, HitResult, Owner->GetInstigatorController(), this, DamageType);

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
	/*if (!bFromReplication && Role < ROLE_Authority)
	{
		ServerReload();
	}*/

	/*if (bFromReplication || CanReload())
	{*/
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
	//}
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


void ACSWeapon::SetOwningPawn(ACSCharacter* NewOwner)
{
	if (Character != NewOwner)
	{
		Instigator = NewOwner;
		Character = NewOwner;
		// Net owner for RPC calls.
		SetOwner(NewOwner);
	}
}

ACSCharacter* ACSWeapon::GetOwningPawn() const
{
	if (Character)
	{
		return Character;
	}
	return nullptr;
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
		//UGameplayStatics::SpawnEmitterAtLocation(this, MuzzleEffect, MuzzleLocation, MuzzleRotation, true);
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
	//float Duration = PlayWeaponAnimations(EquipAnim);
	if (ReloadNow || bPendingEquip)
	{
		return;
	}

	bPendingEquip = true;

	GetWorldTimerManager().SetTimer(TimerHandle_EquipWeaponStopAnimationTime, this, &ACSWeapon::OnEquipStopAnimation, 0.05f, false);

	if (EquipWeaponSound)
	{
		UGameplayStatics::PlaySound2D(this, EquipWeaponSound);
	}

	if (Character)
	{
		AttachWeaponToCharacter(Character->WeaponAttachSocketName);
	}

}

void ACSWeapon::OnUnEquip(FName SocketName)
{
	//float Duration = PlayWeaponAnimations(UnEquipAnim);

	FTimerDelegate UnEquipDelegate = FTimerDelegate::CreateUObject(this, &ACSWeapon::OnUnEquipStopAnimation, SocketName);
	GetWorldTimerManager().SetTimer(TimerHandle_UnEquipWeaponStopAnimationTime, UnEquipDelegate, 0.05f, false);
	//GetWorldTimerManager().SetTimer(TimerHandle_EquipWeaponStopAnimationTime, this, &ACSWeapon::OnUnEquipStopAnimation, Duration, false);
}

// Half of equip animation 
// Here we detach previous weapon and attach new weapon
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
	AttachWeaponToCharacter(SocketName);
	if (UnEquipWeaponSound)
	{
		UGameplayStatics::PlaySound2D(this, UnEquipWeaponSound);
	}
	GetWorldTimerManager().ClearTimer(TimerHandle_UnEquipWeaponStopAnimationTime);
	StopWeaponAnimation(UnEquipAnim);
	Character->EndUnEquiping();
}

void ACSWeapon::AttachWeaponToCharacter(FName SocketName)
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

	// через секунду после выброса оружия, поставить setactive(false)

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
		if (CanPickUp())
		{
			ACSCharacter* Character = Cast<ACSCharacter>(OtherActor);

			if (Character && Character->CurrentWeapon && Character->CurrentWeapon->ReloadNow)
			{
				return;
			}

			SkeletalMeshComponent->SetSimulatePhysics(false);
			SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			if (Character)
			{
				ACSWeapon* Weapon;

				if (WeaponType.Get() == Character->PistolWeapon.Get())
				{
					Weapon = Character->GetLightWeaponSlot();
				}

				else if (WeaponType.Get() == Character->ShotgunWeapon.Get())
				{
					Weapon = Character->GetMiddleWeaponSlot();
				}

				else
				{
					Weapon = Character->GetHardWeaponSlot();
				}

				HandlePickupWeapon(Character, Weapon);
			}

		}
	}
}

bool ACSWeapon::CanPickUp()
{
	if (GetOwningPawn())
	{
		return false;
	}
	return true;
}

void ACSWeapon::HandlePickupWeapon(ACSCharacter* Character, ACSWeapon* Weapon)
{
	// if character does not have that type of weapon
	if (!Weapon)
	{
		
		SphereComponent->SetActive(false);
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

