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
#include "UI/SurvivorHUD.h"
#include "World/ResourceNode.h"
#include "Wildlife/AnimalBase.h"
#include "Core/InteractableInterface.h"
#include "BiomeSurvivor.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DamageEvents.h"

// ---- Helper: create a transient InputAction with given ValueType ----
static UInputAction* CreateRuntimeInputAction(const FName& Name, EInputActionValueType ValueType)
{
	UInputAction* Action = NewObject<UInputAction>(GetTransientPackage(), Name);
	Action->ValueType = ValueType;
	return Action;
}

// ---- Helper: create runtime InputMappingContext with default bindings ----
static UInputMappingContext* CreateDefaultMappingContext(
	UInputAction* Move, UInputAction* Look, UInputAction* Jump, UInputAction* Sprint,
	UInputAction* Crouch, UInputAction* Interact, UInputAction* Attack, UInputAction* Block,
	UInputAction* Dodge, UInputAction* ToggleCamera, UInputAction* Pause, UInputAction* Inventory)
{
	UInputMappingContext* IMC = NewObject<UInputMappingContext>(GetTransientPackage(), TEXT("IMC_DefaultRuntime"));

	// WASD -> Move (Axis2D)
	{
		FEnhancedActionKeyMapping& W = IMC->MapKey(Move, EKeys::W);
		UInputModifierSwizzleAxis* SwizzleW = NewObject<UInputModifierSwizzleAxis>();
		SwizzleW->Order = EInputAxisSwizzle::YXZ;
		W.Modifiers.Add(SwizzleW);
	}
	{
		FEnhancedActionKeyMapping& S = IMC->MapKey(Move, EKeys::S);
		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>();
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		S.Modifiers.Add(Swizzle);
		UInputModifierNegate* Negate = NewObject<UInputModifierNegate>();
		S.Modifiers.Add(Negate);
	}
	{
		IMC->MapKey(Move, EKeys::D);
	}
	{
		FEnhancedActionKeyMapping& A = IMC->MapKey(Move, EKeys::A);
		UInputModifierNegate* Negate = NewObject<UInputModifierNegate>();
		A.Modifiers.Add(Negate);
	}

	// Mouse XY -> Look (Axis2D)
	IMC->MapKey(Look, EKeys::Mouse2D);

	// Space -> Jump
	IMC->MapKey(Jump, EKeys::SpaceBar);

	// Shift -> Sprint
	IMC->MapKey(Sprint, EKeys::LeftShift);

	// Ctrl -> Crouch
	IMC->MapKey(Crouch, EKeys::LeftControl);

	// E -> Interact
	IMC->MapKey(Interact, EKeys::E);

	// Left Mouse -> Attack
	IMC->MapKey(Attack, EKeys::LeftMouseButton);

	// Right Mouse -> Block
	IMC->MapKey(Block, EKeys::RightMouseButton);

	// Alt -> Dodge
	IMC->MapKey(Dodge, EKeys::LeftAlt);

	// V -> Toggle Camera
	IMC->MapKey(ToggleCamera, EKeys::V);

	// Escape -> Pause
	IMC->MapKey(Pause, EKeys::Escape);

	// Tab -> Inventory
	IMC->MapKey(Inventory, EKeys::Tab);

	return IMC;
}

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

	// ---- Visible Body (placeholder shapes) ----
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -15.0f));
	BodyMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.3f));
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->CastShadow = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderFinder.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderFinder.Object);
	}

	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(GetCapsuleComponent());
	HeadMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 72.0f));
	HeadMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.35f));
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadMesh->CastShadow = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereFinder.Succeeded())
	{
		HeadMesh->SetStaticMesh(SphereFinder.Object);
	}

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

	// Apply colored materials to body
	if (BodyMesh)
	{
		UMaterialInterface* BaseMat = BodyMesh->GetMaterial(0);
		if (BaseMat)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.15f, 0.35f, 0.15f)); // Dark green shirt
			BodyMesh->SetMaterial(0, DynMat);
		}
	}
	if (HeadMesh)
	{
		UMaterialInterface* BaseMat = HeadMesh->GetMaterial(0);
		if (BaseMat)
		{
			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.85f, 0.7f, 0.55f)); // Skin tone
			HeadMesh->SetMaterial(0, DynMat);
		}
	}

	// Auto-create input actions at runtime if none set via Blueprint
	if (!MoveAction)   MoveAction   = CreateRuntimeInputAction(TEXT("IA_Move"),   EInputActionValueType::Axis2D);
	if (!LookAction)   LookAction   = CreateRuntimeInputAction(TEXT("IA_Look"),   EInputActionValueType::Axis2D);
	if (!JumpAction)   JumpAction   = CreateRuntimeInputAction(TEXT("IA_Jump"),   EInputActionValueType::Boolean);
	if (!SprintAction) SprintAction = CreateRuntimeInputAction(TEXT("IA_Sprint"), EInputActionValueType::Boolean);
	if (!CrouchAction) CrouchAction = CreateRuntimeInputAction(TEXT("IA_Crouch"), EInputActionValueType::Boolean);
	if (!InteractAction) InteractAction = CreateRuntimeInputAction(TEXT("IA_Interact"), EInputActionValueType::Boolean);
	if (!AttackAction) AttackAction = CreateRuntimeInputAction(TEXT("IA_Attack"), EInputActionValueType::Boolean);
	if (!BlockAction)  BlockAction  = CreateRuntimeInputAction(TEXT("IA_Block"),  EInputActionValueType::Boolean);
	if (!DodgeAction)  DodgeAction  = CreateRuntimeInputAction(TEXT("IA_Dodge"),  EInputActionValueType::Boolean);
	if (!ToggleCameraAction) ToggleCameraAction = CreateRuntimeInputAction(TEXT("IA_ToggleCamera"), EInputActionValueType::Boolean);
	if (!PauseAction) PauseAction = CreateRuntimeInputAction(TEXT("IA_Pause"), EInputActionValueType::Boolean);
	if (!InventoryAction) InventoryAction = CreateRuntimeInputAction(TEXT("IA_Inventory"), EInputActionValueType::Boolean);

	// Create and register default input mapping context
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			UInputMappingContext* DefaultIMC = CreateDefaultMappingContext(
				MoveAction, LookAction, JumpAction, SprintAction, CrouchAction,
				InteractAction, AttackAction, BlockAction, DodgeAction, ToggleCameraAction,
				PauseAction, InventoryAction);
			Subsystem->AddMappingContext(DefaultIMC, 0);

			UE_LOG(LogBiomeSurvivor, Log, TEXT("Runtime input mapping context registered"));
		}
	}
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
		if (PauseAction)
			EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &ASurvivorCharacter::HandlePause);
		if (InventoryAction)
			EnhancedInput->BindAction(InventoryAction, ETriggerEvent::Started, this, &ASurvivorCharacter::HandleToggleInventory);
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
		if (!HitActor) return;

		// Get HUD for notifications
		ASurvivorHUD* HUD = nullptr;
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			HUD = Cast<ASurvivorHUD>(PC->GetHUD());
		}

		// 1. Try IInteractableInterface
		if (HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
		{
			IInteractableInterface::Execute_Interact(HitActor, this);
			OnInteract.Broadcast(HitActor);
			UE_LOG(LogBiomeSurvivor, Log, TEXT("Interacted via interface with: %s"), *HitActor->GetName());
			return;
		}

		// 2. Try resource node harvesting
		if (AResourceNode* ResourceNode = Cast<AResourceNode>(HitActor))
		{
			if (ResourceNode->IsAvailable())
			{
				// Bare hands = 10 damage, no tool type
				bool bHarvested = ResourceNode->HarvestNode(this, 10.0f, NAME_None);
				if (bHarvested && HUD)
				{
					FString ResourceStr = ResourceNode->ResourceName.IsEmpty()
						? UEnum::GetValueAsString(ResourceNode->ResourceType)
						: ResourceNode->ResourceName.ToString();
					HUD->ShowNotification(
						FText::FromString(FString::Printf(TEXT("Gathering %s..."), *ResourceStr)),
						1.5f);
				}
				OnInteract.Broadcast(HitActor);
			}
			else if (HUD)
			{
				HUD->ShowNotification(FText::FromString(TEXT("Resource depleted")), 1.5f);
			}
			return;
		}

		// 3. Try harvesting dead animal
		if (AAnimalBase* Animal = Cast<AAnimalBase>(HitActor))
		{
			if (Animal->IsDead() && Animal->bHarvestable)
			{
				Animal->Harvest(this);
				if (HUD)
				{
					HUD->ShowNotification(
						FText::FromString(FString::Printf(TEXT("Harvested %s"), *Animal->AnimalName.ToString())),
						2.0f);
				}
				OnInteract.Broadcast(HitActor);
			}
			else if (!Animal->IsDead() && HUD)
			{
				// Punch the animal (bare hands attack)
				Animal->OnDamaged(5.0f, this);
				HUD->ShowNotification(
					FText::FromString(FString::Printf(TEXT("Hit %s!"), *Animal->AnimalName.ToString())),
					1.0f);
				OnInteract.Broadcast(HitActor);
			}
			return;
		}

		// 4. Generic interaction fallback
		OnInteract.Broadcast(HitActor);
		UE_LOG(LogBiomeSurvivor, Verbose, TEXT("Interacted with: %s"), *HitActor->GetName());
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

// ============ MENU HANDLERS ============

void ASurvivorCharacter::HandlePause()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	AHUD* HUDBase = PC->GetHUD();
	if (!HUDBase) return;

	// Forward to SurvivorHUD via reflection-free cast
	if (ASurvivorHUD* HUD = Cast<ASurvivorHUD>(HUDBase))
	{
		HUD->TogglePauseMenu();
	}
}

void ASurvivorCharacter::HandleToggleInventory()
{
	if (bIsDead) return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	if (ASurvivorHUD* HUD = Cast<ASurvivorHUD>(PC->GetHUD()))
	{
		HUD->ShowNotification(FText::FromString(TEXT("Inventory coming soon...")), 2.0f);
	}
}
