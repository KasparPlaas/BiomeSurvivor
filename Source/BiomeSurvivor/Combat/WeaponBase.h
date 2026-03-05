// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

/**
 * AWeaponBase
 * Base class for all weapons (melee and ranged).
 * Attached to the character's hand socket when equipped.
 * Values from plan.md Section 5.5.
 */

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Fist		UMETA(DisplayName = "Fist"),
	Knife		UMETA(DisplayName = "Knife"),
	Hatchet		UMETA(DisplayName = "Hatchet"),
	Axe			UMETA(DisplayName = "Axe"),
	Spear		UMETA(DisplayName = "Spear"),
	Sword		UMETA(DisplayName = "Sword"),
	Bow			UMETA(DisplayName = "Bow"),
	Crossbow	UMETA(DisplayName = "Crossbow"),
	Club		UMETA(DisplayName = "Club")
};

UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIOR_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	USkeletalMeshComponent* WeaponMesh;

	/** Weapon type */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	EWeaponType WeaponType = EWeaponType::Fist;

	/** Base damage of this weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float BaseDamage = 10.0f;

	/** Attack speed multiplier (1.0 = normal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float AttackSpeed = 1.0f;

	/** Range in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float WeaponRange = 150.0f;

	/** Stamina cost per swing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float StaminaCostPerSwing = 5.0f;

	/** Is this a ranged weapon? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	bool bIsRanged = false;

	/** For ranged: projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon", meta=(EditCondition="bIsRanged"))
	TSubclassOf<AActor> ProjectileClass;

	/** For ranged: projectile speed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon", meta=(EditCondition="bIsRanged"))
	float ProjectileSpeed = 5000.0f;

	/** Light attack animation montage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Animation")
	UAnimMontage* LightAttackMontage;

	/** Heavy attack animation montage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon|Animation")
	UAnimMontage* HeavyAttackMontage;

	// ---- API ----

	UFUNCTION(BlueprintCallable, Category="Weapon")
	void AttachToCharacter(ACharacter* Character, FName SocketName = TEXT("weapon_r"));

	UFUNCTION(BlueprintCallable, Category="Weapon")
	void DetachFromCharacter();

	UFUNCTION(BlueprintPure, Category="Weapon")
	bool IsRangedWeapon() const { return bIsRanged; }

protected:
	virtual void BeginPlay() override;
};
