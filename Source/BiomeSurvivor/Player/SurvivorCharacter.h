// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SurvivorCharacter.generated.h"

class UPlayerStatsComponent;
class UInventoryComponent;
class UEquipmentComponent;
class UCombatComponent;
class UBuildingComponent;
class UCraftingComponent;
class UTemperatureComponent;
class UNutritionComponent;
class UMedicalComponent;
class USleepComponent;
class UStealthComponent;
class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UInputMappingContext;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, AActor*, InteractedActor);

/**
 * ASurvivorCharacter
 *
 * The main player character. Component-based architecture:
 * Every gameplay system is a UActorComponent attached to this character.
 * The character itself handles movement, camera, and input routing.
 */
UCLASS(Blueprintable)
class BIOMESURVIVOR_API ASurvivorCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASurvivorCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// ============ COMPONENTS ============

	/** Third-person spring arm. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	/** Camera component. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	/** Player survival stats (health, hunger, thirst, stamina, comfort). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPlayerStatsComponent> StatsComponent;

	/** Inventory system. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UInventoryComponent> InventoryComponent;

	/** Equipment slots. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEquipmentComponent> EquipmentComponent;

	/** Combat system. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCombatComponent> CombatComponent;

	/** Building system. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBuildingComponent> BuildingComponent;

	/** Crafting system. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCraftingComponent> CraftingComponent;

	/** Body temperature. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTemperatureComponent> TemperatureComponent;

	/** Nutrition / food system. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNutritionComponent> NutritionComponent;

	/** Medical / injury system. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UMedicalComponent> MedicalComponent;

	/** Sleep / fatigue system. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USleepComponent> SleepComponent;

	/** Stealth detection. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStealthComponent> StealthComponent;

	// ============ CAMERA ============

	/** Toggle between first-person and third-person view. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ToggleCameraView();

	/** Is the player currently in first-person mode? */
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	bool bIsFirstPerson = false;

	// ============ INTERACTION ============

	/** Perform a line trace and interact with the hit actor. */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();

	/** Max interaction distance in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractDistance = 500.0f;

	/** Fires when the player interacts with something. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInteract OnInteract;

	/** Fires when the player dies. */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDied OnPlayerDied;

	// ============ DEATH ============

	/** Handle player death. */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	void Die();

	/** Is the player currently dead? */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Survival")
	bool bIsDead = false;

protected:
	// ---- Enhanced Input Actions (set in Blueprint or DataAsset) ----
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> BlockAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleCameraAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InventoryAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> QuickBar1Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> QuickBar2Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> QuickBar3Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> QuickBar4Action;

	// ---- Input Handlers ----
	void HandleMove(const struct FInputActionValue& Value);
	void HandleLook(const struct FInputActionValue& Value);
	void HandleJump();
	void HandleSprintStart();
	void HandleSprintStop();
	void HandleCrouchToggle();
	void HandleInteract();
	void HandleAttack();
	void HandleBlock();
	void HandleBlockRelease();
	void HandleDodge();
	void HandlePause();
	void HandleToggleInventory();
	void HandleQuickBar1();
	void HandleQuickBar2();
	void HandleQuickBar3();
	void HandleQuickBar4();

	/** Whether the player is currently sprinting. */
	bool bIsSprinting = false;

public:
	// ---- Visible Body Mesh (placeholder) ----

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> HeadMesh;
};
