// Fill out your copyright notice in the Description page of Project Settings.


#include "..\Public\CSCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "..\Public\CSWeapon.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "CoopGame.h"
#include "..\Public\CSHealthComponent.h"
#include "Net/UnrealNetwork.h"

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

	HealthComponent = CreateDefaultSubobject<UCSHealthComponent>(TEXT("Health Component"));

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	SprintFOV = 100.0f;
	ZoomedFOV = 65.0f;
	ZoomInterpSpeed = 20.0f;

	WeaponAttachSocketName = "weapon_socket";
	BackWeaponAttachSocketName = "back_weapon_socket";

	ReloadingNow = false;

	ChangingWeaponNow = false;
	EquipTime = 1.533f;

	IsFiringNow = false;

	SprintMultiplier = 1.2f;

	DefaultWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	BreakTime = 5.f;

	// Sprint Widget

	DefaultSprintProgress = 100.0f;
	CurrentSprintProgress = DefaultSprintProgress;

	SprintDecrease = 0.05f;
	SprintIncrease = 0.1f;

	ShowSprintWidget = false;
}

// Called when the game starts or when spawned
void ACSCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = CameraComponent->FieldOfView;

	HealthComponent->OnHealthChanged.AddDynamic(this, &ACSCharacter::OnHealthChanged);

	// Spawn weapons on the server ONLY
	if (Role == ROLE_Authority)
	{
		// Init defaults weapons
		InitAllWeapons();
	}
}

void ACSCharacter::InitAllWeapons()
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACSWeapon* FirstWeaponSlot = GetWorld()->SpawnActor<ACSWeapon>(StarterWeaponClasses[0], SpawnInfo);
	ACSWeapon* SecondWeaponSlot = GetWorld()->SpawnActor<ACSWeapon>(StarterWeaponClasses[1], SpawnInfo);

	AddWeapon(FirstWeaponSlot, SecondWeaponSlot);
}

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
		Crouch();
	}
}

void ACSCharacter::EndCrouch()
{
	if (!IsSprintingNow)
	{
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
		bWantsToZoom = true;
		IsZoomingNow = true;
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
	bWantsToZoom = false;
	IsZoomingNow = false;
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

	else
	{
		TargetFOV = DefaultFOV;
	}

	float NewFOV = FMath::FInterpTo(CameraComponent->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComponent->SetFieldOfView(NewFOV);
}

void ACSCharacter::StartFire()
{
	if (CurrentWeapon && !IsSprintingNow && !ChangingWeaponNow)
	{
		IsFiringNow = true;
		CurrentWeapon->StartFire();
	}
	
}

void ACSCharacter::StopFire()
{
	if (CurrentWeapon && !IsSprintingNow)
	{
		CurrentWeapon->StopFire();
		IsFiringNow = false;
	}
}

void ACSCharacter::BeginSprint()
{
	if (isMoving() && CanSprint() && !bIsCrouched)
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
	/*if (Role < ROLE_Authority)
	{
		ServerReloadMagazine();
	}*/
	if (CurrentWeapon && CurrentWeapon->CanReload() && !IsSprintingNow)
	{
		StopFire();
		CurrentWeapon->Reload();
		ReloadingNow = true;
	}
}


void ACSCharacter::EndReloading()
{
	if (Role < ROLE_Authority)
	{
		ServerEndReloading();
	}
	ReloadingNow = false;
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

void ACSCharacter::EquipWeapon(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon)
{
	if (NewWeapon)
	{
		if (NewWeapon->GetClass() == ShotgunWeapon.Get() && !IsShotgunEquiped)
		{
			IsShotgunEquiped = true;
		}

		if (NewWeapon->GetClass() != ShotgunWeapon.Get() && IsShotgunEquiped)
		{
			IsShotgunEquiped = false;
		}

		if (CurrentWeapon == NewWeapon)
		{
			return;
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
	}
}

void ACSCharacter::ServerEquipWeapon_Implementation(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon)
{
	EquipWeapon(NewWeapon, PrevWeapon);
}

bool ACSCharacter::ServerEquipWeapon_Validate(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon)
{
	return true;
}

void ACSCharacter::SetCurrentWeapon(ACSWeapon* NewWeapon, ACSWeapon* PrevWeapon)
{
	CurrentWeapon = NewWeapon;
	if (NewWeapon)
	{
		CurrentWeapon->SetOwningPawn(this);
		CurrentWeapon->OnEquip();

		if (PrevWeapon)
		{
			PrevWeapon->SetOwningPawn(this);
			PrevWeapon->AttachWeaponToCharacter(BackWeaponAttachSocketName);
		}
	}
}

void ACSCharacter::OnRep_CurrentWeapon(ACSWeapon* NewWeapon)
{
	//SetCurrentWeapon(NewWeapon);
}


// Equip weapon from first slot
void ACSCharacter::GetFirstWeaponSlot()
{
	if (Weapons[0])
	{
		EquipWeapon(Weapons[0], Weapons[1]);
	}
}

void ACSCharacter::GetSecondWeaponSlot()
{
	if (Weapons[1])
	{
		EquipWeapon(Weapons[1], Weapons[0]);
	}
}

void ACSCharacter::AddWeapon(ACSWeapon* NewWeapon, ACSWeapon* SecondWeapon)
{
	if (NewWeapon)
	{
		NewWeapon->OnEnterInventory(this);
		Weapons.AddUnique(NewWeapon);
		Weapons.AddUnique(SecondWeapon);

		if (Weapons.Num() > 0 && CurrentWeapon == nullptr)
		{
			if (NewWeapon == Weapons[0])
			{
				EquipWeapon(NewWeapon, SecondWeapon);
			}
		}
	}
}

bool ACSCharacter::isMoving()
{
	if (GetMovementComponent()->Velocity.Size() < 10.f)
	{
		return false;
	}
	return true;
}

// Called every frame
void ACSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetFOVCameraView(DeltaTime);

	HandleSprintWidget(DeltaTime);
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

	if (CurrentWeapon && ShotgunWeapon && (CurrentWeapon->GetClass() == ShotgunWeapon.Get()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Shotgun"));
		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACSCharacter::StartFire);
	}
	else
	{
		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACSCharacter::StartFire);
		PlayerInputComponent->BindAction("Fire", IE_Released, this, &ACSCharacter::StopFire);
	}

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ACSCharacter::BeginSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ACSCharacter::EndSprint);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ACSCharacter::ReloadMagazine);

	PlayerInputComponent->BindAction("First Weapon", IE_Pressed, this, &ACSCharacter::GetFirstWeaponSlot);
	PlayerInputComponent->BindAction("Second Weapon", IE_Pressed, this, &ACSCharacter::GetSecondWeaponSlot);
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

void ACSCharacter::OnHealthChanged(UCSHealthComponent* HealthComp, float Health, float HealthDelta, const UDamageType* DamageType, 
									AController* InstigatedBy, AActor* DamageCauser)
{

	if (Health <= 0.0f && !bDied)
	{

		UE_LOG(LogTemp, Warning, TEXT("Dying.."));
		// DIE
		bDied = true;
		
		DetachFromControllerPendingDestroy();

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		SetActorEnableCollision(true);

		if (!bIsRagdoll)
		{
		// Ragdoll
		
			GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetMesh()->SetSimulatePhysics(true);
			GetMesh()->WakeAllRigidBodies();
			GetMesh()->bBlendPhysics = true;

			UCharacterMovementComponent* CharacterComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
			if (CharacterComp)
			{
				CharacterComp->StopMovementImmediately();
				CharacterComp->DisableMovement();
				CharacterComp->SetComponentTickEnabled(false);
			}

			SetLifeSpan(10.0f);
			bIsRagdoll = true;
		}

		/*PlayDeathEffects();*/

		SetLifeSpan(10.0f);
	}
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
	DOREPLIFETIME(ACSCharacter, BackWeapon);
	DOREPLIFETIME(ACSCharacter, StarterWeaponClasses);
	DOREPLIFETIME(ACSCharacter, Weapons);
	DOREPLIFETIME(ACSCharacter, WeaponIndex);
	DOREPLIFETIME(ACSCharacter, ReloadingNow);
	DOREPLIFETIME(ACSCharacter, bDied);
	DOREPLIFETIME(ACSCharacter, bIsRagdoll);
}


