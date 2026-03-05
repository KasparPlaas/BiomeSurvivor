// Copyright Biome Survivor. All Rights Reserved.

#include "Combat/CombatComponent.h"
#include "Player/PlayerStatsComponent.h"
#include "Inventory/EquipmentComponent.h"
#include "Inventory/InventoryComponent.h"
#include "BiomeSurvivor.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Cooldown timers
	if (AttackCooldownTimer > 0.0f)
	{
		AttackCooldownTimer -= DeltaTime;
	}

	// Dodge i-frame timer
	if (bIsInvincible)
	{
		DodgeTimer -= DeltaTime;
		if (DodgeTimer <= 0.0f)
		{
			bIsInvincible = false;
			if (CurrentState == ECombatState::Dodging)
			{
				SetCombatState(ECombatState::Idle);
			}
		}
	}

	// Update lock-on
	if (LockedTarget)
	{
		float Dist = FVector::Dist(GetOwner()->GetActorLocation(), LockedTarget->GetActorLocation());
		if (Dist > LockOnRange || LockedTarget->IsPendingKillPending())
		{
			LockedTarget = nullptr;
		}
		else
		{
			// Face target
			ACharacter* Character = Cast<ACharacter>(GetOwner());
			if (Character)
			{
				FVector Dir = (LockedTarget->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal2D();
				FRotator TargetRot = Dir.Rotation();
				Character->SetActorRotation(FMath::RInterpTo(Character->GetActorRotation(), TargetRot, DeltaTime, 10.0f));
			}
		}
	}
}

void UCombatComponent::TryLightAttack()
{
	if (CurrentState != ECombatState::Idle && CurrentState != ECombatState::Blocking) return;
	if (AttackCooldownTimer > 0.0f) return;

	UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>();
	if (Stats && Stats->GetStamina() < LightAttackStaminaCost) return;

	if (Stats) Stats->ConsumeStamina(LightAttackStaminaCost);

	SetCombatState(ECombatState::Attacking);
	AttackCooldownTimer = AttackCooldown;

	// Perform melee sweep
	PerformMeleeSweep(1.0f);

	// Return to idle after animation
	// In a real implementation, this would be driven by anim notifies
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		if (CurrentState == ECombatState::Attacking)
		{
			SetCombatState(ECombatState::Idle);
		}
	}, 0.4f, false);
}

void UCombatComponent::TryHeavyAttack()
{
	if (CurrentState != ECombatState::Idle) return;
	if (AttackCooldownTimer > 0.0f) return;

	UPlayerStatsComponent* Stats = GetOwner()->FindComponentByClass<UPlayerStatsComponent>();
	if (Stats && Stats->GetStamina() < HeavyAttackStaminaCost) return;

	if (Stats) Stats->ConsumeStamina(HeavyAttackStaminaCost);

	SetCombatState(ECombatState::Attacking);
	AttackCooldownTimer = AttackCooldown * 1.5f;

	// Heavy attack does 2x damage
	PerformMeleeSweep(2.0f);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		if (CurrentState == ECombatState::Attacking)
		{
			SetCombatState(ECombatState::Idle);
		}
	}, 0.7f, false);
}

void UCombatComponent::TryRangedAttack()
{
	if (CurrentState != ECombatState::Idle) return;
	if (AttackCooldownTimer > 0.0f) return;

	// TODO: Check if ranged weapon is equipped
	// TODO: Check ammo
	// TODO: Spawn projectile

	UE_LOG(LogBiomeSurvivor, Log, TEXT("Ranged attack attempted"));
	AttackCooldownTimer = AttackCooldown;
}

void UCombatComponent::StartBlock()
{
	if (CurrentState != ECombatState::Idle) return;
	SetCombatState(ECombatState::Blocking);
}

void UCombatComponent::StopBlock()
{
	if (CurrentState == ECombatState::Blocking)
	{
		SetCombatState(ECombatState::Idle);
	}
}

void UCombatComponent::PerformDodge()
{
	if (CurrentState == ECombatState::Dodging || CurrentState == ECombatState::Staggered) return;

	SetCombatState(ECombatState::Dodging);
	bIsInvincible = true;
	DodgeTimer = DodgeIFrameDuration;

	// Apply dodge impulse
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character)
	{
		FVector DodgeDir = Character->GetLastMovementInputVector();
		if (DodgeDir.IsNearlyZero())
		{
			DodgeDir = -Character->GetActorForwardVector(); // Dodge backward by default
		}
		Character->LaunchCharacter(DodgeDir.GetSafeNormal() * 800.0f, true, false);
	}
}

void UCombatComponent::ToggleLockOn()
{
	if (LockedTarget)
	{
		LockedTarget = nullptr;
		UE_LOG(LogBiomeSurvivor, Log, TEXT("Lock-on disengaged"));
	}
	else
	{
		LockedTarget = FindLockOnTarget();
		if (LockedTarget)
		{
			UE_LOG(LogBiomeSurvivor, Log, TEXT("Locked on to: %s"), *LockedTarget->GetName());
		}
	}
}

float UCombatComponent::CalculateDamage(float BaseDamage, AActor* Target) const
{
	float FinalDamage = BaseDamage;

	// Get weapon bonus from equipped weapon
	UEquipmentComponent* Equipment = GetOwner()->FindComponentByClass<UEquipmentComponent>();
	if (Equipment)
	{
		FItemInstance Weapon = Equipment->GetMainHandWeapon();
		if (!Weapon.IsEmpty())
		{
			const UItemDefinition* WeaponDef = UInventoryComponent::GetItemDefinition(Weapon.ItemID);
			if (WeaponDef)
			{
				FinalDamage += WeaponDef->CombatStats.BaseDamage;
			}
		}
	}

	// Target armor reduction
	if (Target)
	{
		UEquipmentComponent* TargetEquip = Target->FindComponentByClass<UEquipmentComponent>();
		if (TargetEquip)
		{
			float Armor = TargetEquip->GetTotalArmor();
			// Armor formula: damage = base * (100 / (100 + armor))
			FinalDamage = FinalDamage * (100.0f / (100.0f + Armor));
		}
	}

	return FMath::Max(1.0f, FinalDamage); // Minimum 1 damage
}

void UCombatComponent::ApplyDamageToTarget(AActor* Target, float BaseDamage, TSubclassOf<UDamageType> DamageType)
{
	if (!Target) return;

	float FinalDamage = CalculateDamage(BaseDamage, Target);

	UGameplayStatics::ApplyDamage(
		Target,
		FinalDamage,
		Cast<ACharacter>(GetOwner())->GetController(),
		GetOwner(),
		DamageType ? DamageType : UDamageType::StaticClass()
	);

	OnDamageDealt.Broadcast(Target, FinalDamage);
}

void UCombatComponent::SetCombatState(ECombatState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnCombatStateChanged.Broadcast(NewState);
	}
}

void UCombatComponent::PerformMeleeSweep(float DamageMultiplier)
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return;

	FVector Start = Character->GetActorLocation() + FVector(0, 0, 60);
	FVector End = Start + Character->GetActorForwardVector() * MeleeRange;

	TArray<FHitResult> HitResults;
	FCollisionShape Shape = FCollisionShape::MakeSphere(MeleeSweepRadius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	if (GetWorld()->SweepMultiByChannel(HitResults, Start, End, FQuat::Identity, ECC_Pawn, Shape, Params))
	{
		TSet<AActor*> HitActors; // Prevent double-hits
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && !HitActors.Contains(HitActor))
			{
				HitActors.Add(HitActor);
				float BaseDamage = 10.0f * DamageMultiplier; // Fist damage
				ApplyDamageToTarget(HitActor, BaseDamage);
			}
		}
	}

	// Degrade weapon durability
	UEquipmentComponent* Equipment = GetOwner()->FindComponentByClass<UEquipmentComponent>();
	if (Equipment)
	{
		Equipment->DegradeItem(EEquipSlot::MainHand, 1.0f);
	}
}

AActor* UCombatComponent::FindLockOnTarget() const
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character) return nullptr;

	TArray<AActor*> NearbyActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	UKismetSystemLibrary::SphereOverlapActors(
		GetOwner(),
		Character->GetActorLocation(),
		LockOnRange,
		ObjectTypes,
		ACharacter::StaticClass(),
		TArray<AActor*>{GetOwner()},
		NearbyActors
	);

	// Find closest target in front of player
	AActor* BestTarget = nullptr;
	float BestScore = -1.0f;
	FVector Forward = Character->GetActorForwardVector();

	for (AActor* Actor : NearbyActors)
	{
		FVector ToTarget = (Actor->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
		float Dot = FVector::DotProduct(Forward, ToTarget);
		if (Dot > 0.5f && Dot > BestScore) // Must be in front (within ~60 degrees)
		{
			BestScore = Dot;
			BestTarget = Actor;
		}
	}

	return BestTarget;
}

// ---- Server RPCs ----

void UCombatComponent::ServerLightAttack_Implementation()
{
	TryLightAttack();
}

void UCombatComponent::ServerHeavyAttack_Implementation()
{
	TryHeavyAttack();
}
