// Copyright Biome Survivor. All Rights Reserved.

#include "Player/SurvivorCharacter.h"
#include "Player/PlayerStatsComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/EquipmentComponent.h"
#include "Combat/CombatComponent.h"
#include "Combat/StealthComponent.h"
#include "Building/BuildingComponent.h"
#include "Crafting/CraftingComponent.h"
#include "Survival/TemperatureComponent.h"
#include "Survival/NutritionComponent.h"
#include "Survival/MedicalComponent.h"
#include "Survival/SleepComponent.h"
#include "BiomeSurvivor.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"

ASurvivorCharacter::ASurvivorCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// ---- Capsule ----
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// ---- Movement defaults ----
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->MaxWalkSpeed = 250.0f;       // Walk speed (cm/s)
	MoveComp->MaxWalkSpeedCrouched = 150.0f;
	MoveComp->JumpZVelocity = 500.0f;
	MoveComp->AirControl = 0.2f;
	MoveComp->GravityScale = 1.0f;
	MoveComp->bCanWalkOffLedges = true;
	MoveComp->NavAgentProps.bCanCrouch = true;
	MoveComp->NavAgentProps.bCanSwim = true;
	MoveComp->GetNavAgentPropertiesRef().bCanJump = true;

	// ---- Spring Arm (Third-Person Camera) ----
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 250.0f;
	SpringArm->SocketOffset = FVector(0.0f, 60.0f, 60.0f); // Over right shoulder
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 10.0f;

	// ---- Camera ----
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Don't rotate character with camera (strafe movement)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	// ---- Gameplay Components ----
	StatsComponent = CreateDefaultSubobject<UPlayerStatsComponent>(TEXT("PlayerStats"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("Equipment"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	BuildingComponent = CreateDefaultSubobject<UBuildingComponent>(TEXT("Building"));
	CraftingComponent = CreateDefaultSubobject<UCraftingComponent>(TEXT("Crafting"));
	TemperatureComponent = CreateDefaultSubobject<UTemperatureComponent>(TEXT("Temperature"));
	NutritionComponent = CreateDefaultSubobject<UNutritionComponent>(TEXT("Nutrition"));
	MedicalComponent = CreateDefaultSubobject<UMedicalComponent>(TEXT("Medical"));
	SleepComponent = CreateDefaultSubobject<USleepComponent>(TEXT("Sleep"));
	StealthComponent = CreateDefaultSubobject<UStealthComponent>(TEXT("Stealth"));

	// Replication
	SetReplicates(true);
	SetReplicateMovement(true);
}

void ASurvivorCharacter::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogBiomeSurvivor, Log, TEXT("SurvivorCharacter spawned: %s"), *GetName());
}

void ASurvivorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

	// Sprint stamina drain
	if (bIsSprinting && StatsComponent)
	{
		if (StatsComponent->GetStamina() > 0.0f)
		{
			GetCharacterMovement()->MaxWalkSpeed = 650.0f; // Sprint speed
		}
		else
		{
			// Out of stamina, stop sprinting
			HandleSprintStop();
		}
	}
}

void ASurvivorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASurvivorCharacter::HandleMove);
		if (LookAction)
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASurvivorCharacter::HandleLook);
		if (JumpAction)
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ASurvivorCharacter::HandleJump);
		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ASurvivorCharacter::HandleSprintStart);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASurvivorCharacter::HandleSprintStop);
		}
		if (CrouchAction)
			EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Started, this, &ASurvivorCharacter::HandleCrouchToggle);
		if (InteractAction)
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ASurvivorCharacter::HandleInteract);
		if (AttackAction)
			EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &ASurvivorCharacter::HandleAttack);
		if (BlockAction)
			EnhancedInput->BindAction(BlockAction, ETriggerEvent::Triggered, this, &ASurvivorCharacter::HandleBlock);
		if (DodgeAction)
			EnhancedInput->BindAction(DodgeAction, ETriggerEvent::Started, this, &ASurvivorCharacter::HandleDodge);
		if (ToggleCameraAction)
			EnhancedInput->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &ASurvivorCharacter::ToggleCameraView);
	}
}

void ASurvivorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASurvivorCharacter, bIsDead);
}

// ============ INPUT HANDLERS ============

void ASurvivorCharacter::HandleMove(const FInputActionValue& Value)
{
	if (bIsDead) return;

	const FVector2D MoveValue = Value.Get<FVector2D>();
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, MoveValue.Y);
	AddMovementInput(RightDir, MoveValue.X);
}

void ASurvivorCharacter::HandleLook(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();
	AddControllerYawInput(LookValue.X);
	AddControllerPitchInput(LookValue.Y);
}

void ASurvivorCharacter::HandleJump()
{
	if (bIsDead) return;

	if (StatsComponent && StatsComponent->GetStamina() >= 10.0f)
	{
		Jump();
		StatsComponent->ConsumeStamina(10.0f);
	}
}

void ASurvivorCharacter::HandleSprintStart()
{
	if (bIsDead) return;
	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = 650.0f;
}

void ASurvivorCharacter::HandleSprintStop()
{
	bIsSprinting = false;
	// Return to walk or run speed
	GetCharacterMovement()->MaxWalkSpeed = 450.0f; // Run speed (default non-sprint)
}

void ASurvivorCharacter::HandleCrouchToggle()
{
	if (bIsDead) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
		bIsSprinting = false;
	}
}

void ASurvivorCharacter::HandleInteract()
{
	if (bIsDead) return;
	TryInteract();
}

void ASurvivorCharacter::HandleAttack()
{
	if (bIsDead) return;
	if (CombatComponent)
	{
		CombatComponent->TryLightAttack();
	}
}

void ASurvivorCharacter::HandleBlock()
{
	if (bIsDead) return;
	if (CombatComponent)
	{
		CombatComponent->StartBlock();
	}
}

void ASurvivorCharacter::HandleDodge()
{
	if (bIsDead) return;
	if (CombatComponent && StatsComponent && StatsComponent->GetStamina() >= 20.0f)
	{
		CombatComponent->PerformDodge();
		StatsComponent->ConsumeStamina(20.0f);
	}
}

// ============ CAMERA ============

void ASurvivorCharacter::ToggleCameraView()
{
	bIsFirstPerson = !bIsFirstPerson;

	if (bIsFirstPerson)
	{
		SpringArm->TargetArmLength = 0.0f;
		SpringArm->SocketOffset = FVector(0.0f, 0.0f, 0.0f);
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		SpringArm->TargetArmLength = 250.0f;
		SpringArm->SocketOffset = FVector(0.0f, 60.0f, 60.0f);
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

// ============ INTERACTION ============

void ASurvivorCharacter::TryInteract()
{
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + FollowCamera->GetForwardVector() * InteractDistance;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			// Try to call the Interact interface on the hit actor
			OnInteract.Broadcast(HitActor);
			UE_LOG(LogBiomeSurvivor, Verbose, TEXT("Interacted with: %s"), *HitActor->GetName());
		}
	}
}

// ============ DAMAGE & DEATH ============

float ASurvivorCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.0f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (StatsComponent)
	{
		StatsComponent->ApplyDamage(ActualDamage);

		if (StatsComponent->GetHealth() <= 0.0f)
		{
			Die();
		}
	}

	return ActualDamage;
}

void ASurvivorCharacter::Die()
{
	if (bIsDead) return;

	bIsDead = true;
	OnPlayerDied.Broadcast();

	// Disable movement and input
	GetCharacterMovement()->DisableMovement();
	DisableInput(Cast<APlayerController>(GetController()));

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Player died: %s"), *GetName());

	// TODO: Drop inventory items on ground
	// TODO: Play death animation
	// TODO: Trigger respawn via GameMode
}
