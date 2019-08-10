// Fill out your copyright notice in the Description page of Project Settings.


#include "..\Public\CSCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "..\Public\CSWeapon.h"
#include "..\Public\CSPistol.h"
#include "..\Public\CSShotgun.h"
#include "..\Public\CSShotgun.h"
#include "..\Public\CSFlashlight.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "CoopGame.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
ACSCharacter::ACSCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm Component"));
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->SetupAttachment(RootComponent);

	this->GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	this->GetMovementComponent()->GetNavAgentPropertiesRef().bCanJump = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	InitAllBaseCharacterFeatures();
}

// Called when the game starts or when spawned
void ACSCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACSCharacter::InitAllBaseCharacterFeatures()
{
	// BASE
	PlayerMovementState = EPlayerMovementState::IDLE;
	BreakTime = 5.f;
	IsUnarmed = true;

	// Item sockets
	WeaponAttachSocketName = "weapon_socket";
	BackWeaponAttachSocketName = "back_weapon_socket";
	PistolAttachSocketName = "pistol_socket";
	ShotgunAttachSocketName = "shotgun_socket";
	FlashlightAttachSocketName = "flashlight_socket";

	// Weapons array initialization
	// Set weapon inventory capacity
	Weapons.SetNum(3, false);

	// Camera settings on view changing
	DefaultFOV = CameraComponent->FieldOfView;
	SprintFOV = 100.0f;
	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20.0f;

	// Sprint Settings
	DefaultSprintProgress = 100.0f;
	CurrentSprintProgress = DefaultSprintProgress;
	SprintMultiplier = 1.2f;
	SprintDecrease = 0.05f;
	SprintIncrease = 0.1f;
}

// If you want spawn with all weapons
//void ACSCharacter::InitAllWeapons()
//{
//	FActorSpawnParameters SpawnInfo;
//	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
//
//	ACSWeapon* FirstWeaponSlot = GetWorld()->SpawnActor<ACSWeapon>(FirstWeaponClass, SpawnInfo);
//	ACSWeapon* SecondWeaponSlot = GetWorld()->SpawnActor<ACSWeapon>(SecondWeaponClass, SpawnInfo);
//	ACSWeapon* ThirdWeaponSlot = GetWorld()->SpawnActor<ACSWeapon>(ThirdWeaponClass, SpawnInfo);
//
//	AddWeapon(FirstWeaponSlot, SecondWeaponSlot, ThirdWeaponSlot);
//}

void ACSCharacter::MoveForvard(float Value)
{
	if (IsZoomingNow)
	{
		AddMovementInput(GetActorForwardVector() * (Value / 2));
	}
	else
	{
		AddMovementInput(GetActorForwardVector() * Value);
	}
}

void ACSCharacter::MoveRight(float Value)
{
	if (!IsSprintingNow)
	{
		if (IsZoomingNow)
		{
			AddMovementInput(GetActorRightVector() * (Value / 2));
		}
		else
		{
			AddMovementInput(GetActorRightVector() * Value);
		}
	}
}

void ACSCharacter::BeginCrouch()
{
	if (!IsSprintingNow)
	{
		IsCrouchingNow = true;
		Crouch();
	}
}

void ACSCharacter::EndCrouch()
{
	if (!IsSprintingNow)
	{
		IsCrouchingNow = false;
		UnCrouch();
	}
}

void ACSCharacter::BeginJump()
{
	if (!IsSprintingNow)
	{
		Jump();
	}
}

void ACSCharacter::BeginZoom()
{
	if (Role < ROLE_Authority)
	{
		ServerBeginZoom();
	}
	MulticastBeginZoom();
}

void ACSCharacter::ServerBeginZoom_Implementation()
{
	BeginZoom();
}

bool ACSCharacter::ServerBeginZoom_Validate()
{
	return true;
}

void ACSCharacter::MulticastBeginZoom_Implementation()
{
	if (!IsSprintingNow)
	{
		if (CurrentWeapon && CurrentWeapon->GetClass() == SniperRifleWeapon.Get())
		{
			IsSniperRifleZooming = true;
		}
		else
		{
			bWantsToZoom = true;
			IsZoomingNow = true;
		}
	}
}


void ACSCharacter::EndZoom()
{
	if (Role < ROLE_Authority)
	{
		ServerEndZoom();
	}
	MulticastEndZoom();
	
}

void ACSCharacter::ServerEndZoom_Implementation()
{
	EndZoom();
}

bool ACSCharacter::ServerEndZoom_Validate()
{
	return true;
}

void ACSCharacter::MulticastEndZoom_Implementation()
{
	if (IsSniperRifleZooming)
	{
		IsSniperRifleZooming = false;
	}
	else
	{
		bWantsToZoom = false;
		IsZoomingNow = false;
	}
}

void ACSCharacter::SetFOVCameraView(float DeltaTime)
{
	float TargetFOV;

	if (IsSprintingNow)
	{
		TargetFOV = IsSprintingNow ? SprintFOV : DefaultFOV;
	}

	else if (IsZoomingNow)
	{
		TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	}

	else if (IsSniperRifleZooming)
	{
		TargetFOV = IsSniperRifleZooming ? SniperRifleFOV : DefaultFOV;
	}

	else
	{
		TargetFOV = DefaultFOV;
	}

	float NewFOV = FMath::FInterpTo(CameraComponent->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComponent->SetFieldOfView(NewFOV);
}

void ACSCharacter::FireShotgun()
{
	if (CurrentWeapon && !IsSprintingNow && !ChangingWeaponNow)
	{
		if (CurrentWeapon->GetClass() == ShotgunWeapon.Get())
		{
			CurrentWeapon->StartFire();
		}
	}
}

void ACSCharacter::StartFire()
{
	if (CurrentWeapon && !IsSprintingNow && !ChangingWeaponNow && !GettingHitNow)
	{

		if (CurrentWeapon->GetClass() != ShotgunWeapon.Get())
		{
			CurrentWeapon->StartFire();
			IsFiringNow = true;
		}
	}
	
}

void ACSCharacter::StopFire()
{
	if (CurrentWeapon && !IsSprintingNow)
	{
		if (CurrentWeapon->GetClass() != ShotgunWeapon.Get())
		{
			CurrentWeapon->StopFire();
			IsFiringNow = false;
		}
	}
}

void ACSCharacter::BeginSprint()
{
	if (isMoving() && CanSprint() && !bIsCrouched && !IsSniperRifleZooming)
	{
		UE_LOG(LogTemp, Warning, TEXT("Begin run"));
		if (Role < ROLE_Authority)
		{
			ServerBeginSprint();
		}
		MulticastBeginSprint();
	}
}

void ACSCharacter::ServerBeginSprint_Implementation()
{
	BeginSprint();
}

bool ACSCharacter::ServerBeginSprint_Validate()
{
	return true;
}

void ACSCharacter::MulticastBeginSprint_Implementation()
{
	IsSprintingNow = true;
	GetCharacterMovement()->MaxWalkSpeed *= SprintMultiplier;
}

void ACSCharacter::EndSprint()
{
	if (IsSprintingNow)
	{
		if (Role < ROLE_Authority)
		{
			ServerEndSprint();
		}
		MulticastEndSprint();
	}
}

void ACSCharacter::ServerEndSprint_Implementation()
{
	EndSprint();
}

bool ACSCharacter::ServerEndSprint_Validate()
{
	return true;
}

void ACSCharacter::MulticastEndSprint_Implementation()
{
	IsSprintingNow = false;
	GetCharacterMovement()->MaxWalkSpeed /= SprintMultiplier;
}

void ACSCharacter::HandleSprintWidget(float Delta)
{
	FMath::Clamp(CurrentSprintProgress, 0.0f, DefaultSprintProgress);
	CanSprint();

	if (IsSprintingNow)
	{
		if (CurrentSprintProgress > 0.0f)
		{
			CurrentSprintProgress -= SprintDecrease * Delta;
		}
	}
	else
	{
		if (CurrentSprintProgress < 100.f)
		{
			CurrentSprintProgress += SprintIncrease * Delta;
		}
	}

	SetSprintWidget();
}

bool ACSCharacter::CanSprint()
{
	if (CurrentSprintProgress > 1.0f)
	{
		return true;
	}

	else
	{
		EndSprint();
		return false;
	}
}

void ACSCharacter::SetSprintWidget()
{
	if (CurrentSprintProgress < 100.f)
	{
		ShowSprintWidget = true;
	}
	else
	{
		ShowSprintWidget = false;
	}
}

void ACSCharacter::ReloadMagazine()
{
	if (CurrentWeapon && CurrentWeapon->CanReload() && !IsSprintingNow && !EquipingNow)
	{
		StopFire();
		CurrentWeapon->Reload();	
	}
}


void ACSCharacter::EndReloading()
{
	if (Role < ROLE_Authority)
	{
		ServerEndReloading();
	}
}

void ACSCharacter::EndEquiping()
{
	EquipingNow = false;
}

void ACSCharacter::EndUnEquiping()
{
	CurrentWeapon->SetOwningPawn(this);
	CurrentWeapon->OnEquip();
}

void ACSCharacter::ServerEndReloading_Implementation()
{
	EndReloading();
}

bool ACSCharacter::ServerEndReloading_Validate()
{
	return true;
}

void ACSCharacter::ServerReloadMagazine_Implementation()
{
	ReloadMagazine();
}

bool ACSCharacter::ServerReloadMagazine_Validate()
{
	return true;
}

void ACSCharacter::EquipWeapon(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon, ACSWeapon* ThirdWeapon)
{
	if (ReloadingNow)
	{
		return;
	}
	if (NewWeapon)
	{
		EquipingNow = true;
		if (NewWeapon->GetClass() == ShotgunWeapon.Get() && !IsShotgunEquiped)
		{
			IsShotgunEquiped = true;
		}

		else if (NewWeapon->GetClass() != ShotgunWeapon.Get() && IsShotgunEquiped)
		{
			IsShotgunEquiped = false;
		}

		if (NewWeapon->GetClass() == PistolWeapon.Get() && !IsPistolEquiped)
		{
			IsPistolEquiped = true;
		}

		else if (NewWeapon->GetClass() != PistolWeapon.Get() && IsPistolEquiped)
		{
			IsPistolEquiped = false;
		}

		if (CurrentWeapon && CurrentWeapon == NewWeapon && !IsUnarmed)
		{
			IsUnarmed = true;
		}

		if (!CurrentWeapon && IsUnarmed)
		{
			IsUnarmed = false;
		}

		if (PrevWeapon)
		{
			if (Role == ROLE_Authority)
			{
				SetCurrentWeapon(NewWeapon, PrevWeapon);
			}
			else
			{
				ServerEquipWeapon(NewWeapon);
			}
		}
		
		else
		{
			if (Role == ROLE_Authority)
			{
				SetCurrentWeapon(NewWeapon);
			}
			else
			{
				ServerEquipWeapon(NewWeapon);
			}
		}
	}
	else
	{
		IsUnarmed = true;
		CurrentWeapon->SetOwningPawn(nullptr);
		CurrentWeapon = nullptr;
	}
}

void ACSCharacter::EquipWeaponAfterPickup(ACSWeapon* NewWeapon, ACSWeapon* CurrentWeapon)
{
	
}

void ACSCharacter::ServerEquipWeapon_Implementation(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon)
{
	EquipWeapon(NewWeapon, PrevWeapon);
}

bool ACSCharacter::ServerEquipWeapon_Validate(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon)
{
	return true;
}

void ACSCharacter::SetCurrentWeapon(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon, ACSWeapon* ThirdWeapon)
{
	if (IsUnarmed)
	{
		if (CurrentWeapon && CurrentWeapon->GetClass() == ShotgunWeapon.Get())
		{
			CurrentWeapon->AttachItemToCharacter(ShotgunAttachSocketName);
		}

		else if (CurrentWeapon && CurrentWeapon->GetClass() == PistolWeapon.Get())
		{
			CurrentWeapon->AttachItemToCharacter(PistolAttachSocketName);
		}

		else
		{
			CurrentWeapon->AttachItemToCharacter(BackWeaponAttachSocketName);
		}
		CurrentWeapon = nullptr;
		return;
	}

	CurrentWeapon = NewWeapon;
	if (NewWeapon)
	{

		if (!PrevWeapon)
		{
			CurrentWeapon->OnEquip();
		}

		if (PrevWeapon && PrevWeapon->GetClass() == ShotgunWeapon.Get())
		{
			PrevWeapon->SetOwningPawn(this);
			PrevWeapon->OnUnEquip(ShotgunAttachSocketName);
		}

		else if (PrevWeapon && PrevWeapon->GetClass() == PistolWeapon.Get())
		{
			PrevWeapon->SetOwningPawn(this);
			PrevWeapon->OnUnEquip(PistolAttachSocketName);
		}

		else if (PrevWeapon && PrevWeapon->GetClass() != ShotgunWeapon.Get() && PrevWeapon->GetClass() != PistolWeapon.Get())
		{
			PrevWeapon->SetOwningPawn(this);
			PrevWeapon->OnUnEquip(BackWeaponAttachSocketName);
		}

	}
}

void ACSCharacter::OnRep_CurrentWeapon(ACSWeapon* NewWeapon)
{
	SetCurrentWeapon(NewWeapon);
}


// Equip weapon from first slot
void ACSCharacter::GetFirstWeaponSlot()
{
	if (LightSlot)
	{
		// if we have current weapon, then we just swap it with our new weapon
		// else if we came from unarmed state, then we just equip our new weapon
		(CurrentWeapon && !ReloadingNow && !EquipingNow) ? EquipWeapon(Weapons[0], CurrentWeapon) : EquipWeapon(Weapons[0]);
	}
}

void ACSCharacter::GetSecondWeaponSlot()
{
	if (MiddleSlot)
	{
		// if we have current weapon, then we just swap it with our new weapon
		// else if we came from unarmed state, then we just equip our new weapon
		(CurrentWeapon && !ReloadingNow && !EquipingNow) ? EquipWeapon(Weapons[1], CurrentWeapon) : EquipWeapon(Weapons[1]);
	}
}

void ACSCharacter::GetThirdWeaponSlot()
{
	if (HardSlot)
	{
		// if we have current weapon, then we just swap it with our new weapon
		// else if we came from unarmed state, then we just equip our new weapon
		(CurrentWeapon && !ReloadingNow && !EquipingNow) ? EquipWeapon(Weapons[2], CurrentWeapon) : EquipWeapon(Weapons[2]);
	}
}

void ACSCharacter::FlashlightPowerOnOff()
{
	if (Flashlight)
	{
		Flashlight->PowerupFlashlight();
	}
}

void ACSCharacter::DropWeapon()
{
	if (CurrentWeapon && !ReloadingNow && !EquipingNow)
	{
		if (CurrentWeapon->OnDropping())
		{
			if (CurrentWeapon->GetClass() == PistolWeapon.Get())
			{
				LightSlot = nullptr;
			}
			else if (CurrentWeapon->GetClass() == ShotgunWeapon.Get())
			{
				MiddleSlot = nullptr;
			}
			else
			{
				HardSlot = nullptr;
			}
			EquipWeapon();
		}
	}
}

void ACSCharacter::DrawDebugTraceLineForPickup(FVector& StartPoint, FVector& EndPoint)
{
	FVector EyeLocation;
	FRotator EyeRotation;

	FVector ShotDirection;

	FVector TraceEnd;

	if (!IsUnarmed)
	{
		EyeLocation = CurrentWeapon->GetSkeletalMeshComponent()->GetSocketLocation(CurrentWeapon->GetMuzzleSocket());
		EyeRotation = CurrentWeapon->GetSkeletalMeshComponent()->GetSocketRotation(CurrentWeapon->GetMuzzleSocket());

		ShotDirection = EyeRotation.Vector();

		TraceEnd = EyeLocation + (ShotDirection * 2000);
	}
	else
	{
		GetActorEyesViewPoint(EyeLocation, EyeRotation);

		ShotDirection = EyeRotation.Vector();

		TraceEnd = EyeLocation + (ShotDirection * 600);
	}

	StartPoint = EyeLocation;
	EndPoint = TraceEnd;
}

void ACSCharacter::GrabItem()
{
	FVector EyeLocation;
	FVector TraceEnd;
	
	DrawDebugTraceLineForPickup(EyeLocation, TraceEnd);


	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, NULL);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult HitResult = FHitResult(ForceInit);

	bool IsHit;
	
	IsHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		EyeLocation,
		TraceEnd,
		ECC_GameTraceChannel3,
		TraceParams
	);

	if (IsHit)
	{
		float DistanceBetween;
		
		DistanceBetween = FVector::Dist(EyeLocation, HitResult.GetActor()->GetActorLocation());

		if (DistanceBetween <= 285.f)
		{
			GrabbedItem = HitResult.GetActor();
			ACSItem* Temp = Cast<ACSItem>(GrabbedItem);
			if (Temp)
			{
				Temp->PickupItem(this);
			}
		}
	}
}

void ACSCharacter::ShowGrabItemWidget()
{
	FVector EyeLocation;
	FVector TraceEnd;
	DrawDebugTraceLineForPickup(EyeLocation, TraceEnd);

	FCollisionQueryParams TraceParams(FName(TEXT("InteractTrace")), true, NULL);
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult HitResult = FHitResult(ForceInit);

	bool IsHit;

	IsHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		EyeLocation,
		TraceEnd,
		ECC_GameTraceChannel3,
		TraceParams
	);

	if (IsHit)
	{
		float DistanceBetween;

		DistanceBetween = FVector::Dist(EyeLocation, HitResult.GetActor()->GetActorLocation());

		if (DistanceBetween <= 285.f)
		{
			GrabbedItem = HitResult.GetActor();
			ACSItem* Temp = Cast<ACSItem>(GrabbedItem);
			if (Temp && Temp != CurrentWeapon && Temp != HardSlot)
			{
				ShowGrabWidget = true;
			}
			else if (Temp && Temp == CurrentWeapon)
			{
				ShowGrabWidget = false;
			}
			else
			{
				ShowGrabWidget = false;
			}
		}
		else
		{
			ShowGrabWidget = false;
		}
	}
	else
	{
		ShowGrabWidget = false;
	}

}

void ACSCharacter::PlayHit(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser, bool bkilled)
{
	Super::PlayHit(DamageAmount, DamageEvent, EventInstigator, DamageCauser, bkilled);

	if (!bkilled && (HitAnimation1 && HitAnimation2 && HitAnimation3 && HitAnimation4) && !GettingHitNow)
	{
		uint8 PickRandomAnimation = FMath::RandRange(1, 4);
		float Duration;

		switch (PickRandomAnimation)
		{
			case 1:
				Duration = PlayAnimMontage(HitAnimation1);
				PickedAnimationNumber = 1;
				break;
			case 2:
				Duration = PlayAnimMontage(HitAnimation2);
				PickedAnimationNumber = 2;
				break;
			case 3:
				Duration = PlayAnimMontage(HitAnimation3);
				PickedAnimationNumber = 3;
				break;
			case 4:
				Duration = PlayAnimMontage(HitAnimation4);
				PickedAnimationNumber = 4;
				break;
		}

		if (TakeHitSound)
		{
			UGameplayStatics::SpawnSoundAttached(TakeHitSound, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget);
		}

		GettingHitNow = true;
		GetWorldTimerManager().SetTimer(TimerHandle_GettingHitTime, this, &ACSCharacter::StopPlayHitAnimation, Duration);
	}
}

void ACSCharacter::StopPlayHitAnimation()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_GettingHitTime);
	switch (PickedAnimationNumber)
	{
		case 1:
			StopAnimMontage(HitAnimation1);
			break;
		case 2:
			StopAnimMontage(HitAnimation2);
			break;
		case 3:
			StopAnimMontage(HitAnimation3);
			break;
		case 4:
			StopAnimMontage(HitAnimation4);
			break;
	}
	GettingHitNow = false;
}

void ACSCharacter::OnDeath(float DamageAmount, FDamageEvent const& DamageEvent, APawn* EventInstigator, AActor* DamageCauser)
{
	Super::OnDeath(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DeathSound)
	{
		UGameplayStatics::SpawnSoundAttached(DeathSound, RootComponent, NAME_None, FVector::ZeroVector, EAttachLocation::SnapToTarget);
	}
	
}

void ACSCharacter::AddWeapon(ACSWeapon* FirstWeapon, ACSWeapon* SecondWeapon, ACSWeapon* ThirdWeapon)
{
	if (LightSlot && MiddleSlot && HardSlot)
	{
		return;
	}

	if (FirstWeapon)
	{
		FirstWeapon->OnEnterInventory(this);

		if (FirstWeapon->GetClass() == PistolWeapon.Get())
		{
			LightSlot = FirstWeapon;
			Weapons[0] = LightSlot;
		}

		else if (FirstWeapon->GetClass() == ShotgunWeapon.Get())
		{
			MiddleSlot = FirstWeapon;
			Weapons[1] = MiddleSlot;
		}

		else
		{
			HardSlot = FirstWeapon;
			Weapons[2] = HardSlot;
		}

		if (CurrentWeapon)
		{
			EquipWeapon(FirstWeapon, CurrentWeapon);
		}
		else
		{
			EquipWeapon(FirstWeapon);
		}

	}
}

bool ACSCharacter::isMoving()
{
	if (GetMovementComponent()->Velocity.Size() == 0.0f)
	{
		return false;
	}
	return true;
}

ACSWeapon* ACSCharacter::GetLightWeaponSlot() const
{
	return LightSlot;
}

ACSWeapon* ACSCharacter::GetMiddleWeaponSlot() const
{
	return MiddleSlot;
}

ACSWeapon* ACSCharacter::GetHardWeaponSlot() const
{
	return HardSlot;
}

void ACSCharacter::SetFlashlight(ACSFlashlight* Flashlight)
{
	this->Flashlight = Flashlight;
}

// Called every frame
void ACSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsAlive())
	{
		CheckPlayerState();

		SetFOVCameraView(DeltaTime);

		HandleSprintWidget(DeltaTime);

		ShowGrabItemWidget();
	}
}

EPlayerMovementState ACSCharacter::GetCurrentPlayerMovementState() const
{
	return PlayerMovementState;
}

void ACSCharacter::ClearWidgetsAfterDeath()
{
	ShowGrabWidget = false;
	IsSniperRifleZooming = false;
}

void ACSCharacter::CheckPlayerState()
{
	if (isMoving())
	{
		if (IsCrouchingNow)
		{
			if (IsZoomingNow)
			{
				PlayerMovementState = EPlayerMovementState::CROUCHING_ZOOMING_MOVING;
				return;
			}
			PlayerMovementState = EPlayerMovementState::CROUCHING_MOVING;
		}
		else if (!IsCrouchingNow && IsZoomingNow)
		{
			PlayerMovementState = EPlayerMovementState::ZOOMING_MOVING;
		}
		else if (!IsCrouchingNow && !IsZoomingNow)
		{
			PlayerMovementState = EPlayerMovementState::JOGGING;
		}
	}
	else
	{
		if (IsCrouchingNow)
		{
			if (IsZoomingNow)
			{
				PlayerMovementState = EPlayerMovementState::CROUCHING_ZOOMING_IDLE;
				return;
			}
			PlayerMovementState = EPlayerMovementState::CROUCHING;
		}
		else if (!IsCrouchingNow && IsZoomingNow)
		{
			PlayerMovementState = EPlayerMovementState::ZOOMING_IDLE;
		}
		else if (!IsCrouchingNow && !IsZoomingNow)
		{
			PlayerMovementState = EPlayerMovementState::IDLE;
		}
	}
}

// Called to bind functionality to input
void ACSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACSCharacter::MoveForvard);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACSCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ACSCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ACSCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ACSCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ACSCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACSCharacter::Jump);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ACSCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ACSCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACSCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ACSCharacter::StopFire);

	PlayerInputComponent->BindAction("Fire_Shotgun", IE_Released, this, &ACSCharacter::FireShotgun);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ACSCharacter::BeginSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ACSCharacter::EndSprint);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ACSCharacter::ReloadMagazine);

	PlayerInputComponent->BindAction("First Weapon", IE_Pressed, this, &ACSCharacter::GetFirstWeaponSlot);
	PlayerInputComponent->BindAction("Second Weapon", IE_Pressed, this, &ACSCharacter::GetSecondWeaponSlot);
	PlayerInputComponent->BindAction("Third Weapon", IE_Pressed, this, &ACSCharacter::GetThirdWeaponSlot);

	PlayerInputComponent->BindAction("Drop Weapon", IE_Pressed, this, &ACSCharacter::DropWeapon);

	PlayerInputComponent->BindAction("Grab Item", IE_Pressed, this, &ACSCharacter::GrabItem);

	PlayerInputComponent->BindAction("On/Off Flashlight", IE_Pressed, this, &ACSCharacter::FlashlightPowerOnOff);

}

FVector ACSCharacter::GetPawnViewLocation() const
{
	if (CameraComponent)
	{
		return CameraComponent->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

TArray<ACSWeapon*> ACSCharacter::GetCharacterWeapons() const
{
	return Weapons;
}

FRotator ACSCharacter::GetAimOffset() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

void ACSCharacter::GetLifetimeReplicatedProps(TArray < class FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACSCharacter, CurrentWeapon);
	DOREPLIFETIME(ACSCharacter, StarterWeaponClasses);
	DOREPLIFETIME(ACSCharacter, Weapons);
	DOREPLIFETIME(ACSCharacter, ReloadingNow);
}