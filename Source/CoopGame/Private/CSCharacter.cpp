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

	//CurrentWeapon = GetWorld()->SpawnActor<ACSWeapon>(StarterWeaponClasses[0], SpawnParams);
	//BackWeapon = GetWorld()->SpawnActor<ACSWeapon>(StarterWeaponClasses[1], SpawnParams);
	////Weapons.Add(GetWorld()->SpawnActor<ACSWeapon>(StarterWeaponClasses[0], SpawnParams));
	//BackWeapon->GetRootComponent()->SetActive(false);
	//BackWeapon->GetRootComponent()->SetHiddenInGame(true);
	////Weapons.Add(GetWorld()->SpawnActor<ACSWeapon>(StarterWeaponClasses[1], SpawnParams));

	//

	ACSWeapon* FirstWeaponSlot = GetWorld()->SpawnActor<ACSWeapon>(StarterWeaponClasses[0], SpawnInfo);
	//ACSWeapon* SecondWeaponSlot = GetWorld()->SpawnActor<ACSWeapon>(StarterWeaponClasses[1], SpawnInfo);

	AddWeapon(FirstWeaponSlot);
	//AddWeapon(SecondWeaponSlot);

	/*if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->SetOwningPawn(this);
		CurrentWeapon->AttachToComponent(Cast<USceneComponent>(GetMesh()), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		UE_LOG(LogTemp, Warning, TEXT("Current weapon: %s"), *CurrentWeapon->GetName());
	}*/

	/*if (BackWeapon)
	{
		BackWeapon->SetOwner(this);
		BackWeapon->AttachToComponent(Cast<USceneComponent>(GetMesh()), FAttachmentTransformRules::SnapToTargetNotIncludingScale, BackWeaponAttachSocketName);
		BackWeapon->GetRootComponent()->SetActive(false);
		
		UE_LOG(LogTemp, Warning, TEXT("Back weapon: %s"), *BackWeapon->GetName());
	}*/

	WeaponIndex = 0;
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

void ACSCharacter::StartFire()
{
	if (CurrentWeapon && !IsSprintingNow && !ChangingWeaponNow)
	{
		CurrentWeapon->StartFire();
		IsFiringNow = true;
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
	if (Role < ROLE_Authority)
	{
		ServerBeginSprint();
	}
	MulticastBeginSprint();
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
	if (Role < ROLE_Authority)
	{
		ServerEndSprint();
	}
	MulticastEndSprint();
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

void ACSCharacter::EquipWeapon(ACSWeapon* NewWeapon)
{
	if (NewWeapon)
	{
		if (CurrentWeapon == NewWeapon)
		{
			return;
		}

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

void ACSCharacter::ServerEquipWeapon_Implementation(ACSWeapon* NewWeapon)
{
	EquipWeapon(NewWeapon);
}

bool ACSCharacter::ServerEquipWeapon_Validate(ACSWeapon* NewWeapon)
{
	return true;
}

void ACSCharacter::SetCurrentWeapon(ACSWeapon* NewWeapon)
{
	CurrentWeapon = NewWeapon;
	if (NewWeapon)
	{
		CurrentWeapon->SetOwningPawn(this);
		CurrentWeapon->OnEquip();
	}
}

void ACSCharacter::OnRep_CurrentWeapon(ACSWeapon* NewWeapon)
{
	//SetCurrentWeapon(NewWeapon);
}

void ACSCharacter::GetFirstWeaponSlot()
{
	/*if (Weapons[0])
	{
		EquipWeapon(Weapons[0]);
	}*/
}

void ACSCharacter::GetSecondWeaponSlot()
{
	/*if (Weapons[1])
	{
		EquipWeapon(Weapons[1]);
	}*/
}

void ACSCharacter::AddWeapon(ACSWeapon* NewWeapon)
{
	if (NewWeapon)
	{
		NewWeapon->OnEnterInventory(this);
		Weapons.AddUnique(NewWeapon);

		if (Weapons.Num() > 0 && CurrentWeapon == nullptr)
		{
			if (NewWeapon == Weapons[0])
			{
				EquipWeapon(Weapons[0]);
			}
		}
	}
}


//void ACSCharacter::TakeFirstWeapon()
//{
//	if (Role < ROLE_Authority)
//	{
//		ServerTakeFirstWeapon();
//	}
//
//	if (CurrentWeapon && !IsSprintingNow)
//	{
//		if (CurrentWeapon->GetClass() != Weapons[0]->GetClass())
//		{
//			//Weapons[WeaponIndex] = CurrentWeapon;
//			StopFire();
//			WeaponIndex = 0;
//
//			//CurrentWeapon = Weapons[WeaponIndex];
//
//			MulticastAttachWeapon();
//		}
//	}
//}
//
//void ACSCharacter::ServerTakeFirstWeapon_Implementation()
//{
//	TakeFirstWeapon();
//}
//
//bool ACSCharacter::ServerTakeFirstWeapon_Validate()
//{
//	return true;
//}
//
//void ACSCharacter::MulticastTakeFirstWeapon_Implementation()
//{
//	
//}
//
//void ACSCharacter::TakeSecondWeapon()
//{
//	if (Role < ROLE_Authority)
//	{
//		ServerTakeSecondWeapon();
//	}
//
//	if (CurrentWeapon && !IsSprintingNow)
//	{
//		if (CurrentWeapon->GetClass() != Weapons[1]->GetClass())
//		{
//			//Weapons[WeaponIndex] = CurrentWeapon;
//			StopFire();
//			WeaponIndex = 1;
//
//			//CurrentWeapon = Weapons[WeaponIndex];
//
//			MulticastAttachWeapon();
//		}
//	}
//	
//}
//
//void ACSCharacter::ServerTakeSecondWeapon_Implementation()
//{
//	TakeSecondWeapon();
//}
//
//bool ACSCharacter::ServerTakeSecondWeapon_Validate()
//{
//	return true;
//}
//
//void ACSCharacter::MulticastTakeSecondWeapon_Implementation()
//{
//}

void ACSCharacter::AttachWeapon()
{
	MulticastDetachWeapon();

	ChangingWeaponNow = true;

	CurrentWeapon = GetWorld()->SpawnActor<ACSWeapon>(StarterWeaponClasses[WeaponIndex], FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (CurrentWeapon)
	{
		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(Cast<USceneComponent>(GetMesh()), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		CurrentWeapon->GetRootComponent()->SetHiddenInGame(false);
		CurrentWeapon->GetRootComponent()->SetActive(true);
	}
	GetWorldTimerManager().SetTimer(TimerHandle_RemoveWeaponTime, this, &ACSCharacter::RemoveWeapon, EquipTime, true);
}

void ACSCharacter::ServerAttachWeapon_Implementation()
{
	AttachWeapon();
}

bool ACSCharacter::ServerAttachWeapon_Validate()
{
	return true;
}

void ACSCharacter::MulticastAttachWeapon_Implementation()
{
	AttachWeapon();
}

void ACSCharacter::DetachWeapon()
{	
	CurrentWeapon->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	CurrentWeapon->GetRootComponent()->SetHiddenInGame(true);
}

void ACSCharacter::ServerDetachWeapon_Implementation()
{
	DetachWeapon();
}

bool ACSCharacter::ServerDetachWeapon_Validate()
{
	return true;
}

void ACSCharacter::MulticastDetachWeapon_Implementation()
{
	DetachWeapon();
}

void ACSCharacter::SaveWeaponInfoAfterDetach(uint8 WeaponIndex)
{
	Weapons[WeaponIndex] = CurrentWeapon;
}

void ACSCharacter::RemoveWeapon()
{
	//UE_LOG(LogTemp, Warning, TEXT("Remove Weapon"));
	//ChangingWeaponNow = false;
	//GetWorldTimerManager().ClearTimer(TimerHandle_RemoveWeaponTime);
}

void ACSCharacter::TakeWeapon()
{
	//UE_LOG(LogTemp, Warning, TEXT("Take Weapon"));
	////AttachWeapon();
	//GetWorldTimerManager().SetTimer(TimerHandle_EquipWeaponTime, this, &ACSCharacter::AttachWeapon, EquipTime / 2.f, true);
	//ChangingWeaponNow = false;
	//GetWorldTimerManager().ClearTimer(TimerHandle_EquipWeaponTime);
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

	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CameraComponent->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

	CameraComponent->SetFieldOfView(NewFOV);
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

void ACSCharacter::OnHealthChanged(UCSHealthComponent* HealthComp, float Health, float HealthDelta, const UDamageType* DamageType, 
									AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied)
	{
		// DIE
		bDied = true;
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

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
}


