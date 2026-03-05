// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MedicalComponent.generated.h"

/**
 * UMedicalComponent
 * Manages injuries (bleeding, fractures), bandaging, and healing items.
 * Values from plan.md Section 5.6.
 */

UENUM(BlueprintType)
enum class EInjuryType : uint8
{
	None		UMETA(DisplayName = "None"),
	Laceration	UMETA(DisplayName = "Laceration"),
	Fracture	UMETA(DisplayName = "Fracture"),
	Burn		UMETA(DisplayName = "Burn"),
	Bite		UMETA(DisplayName = "Animal Bite"),
	Sprain		UMETA(DisplayName = "Sprain")
};

USTRUCT(BlueprintType)
struct FInjury
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInjuryType Type = EInjuryType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Severity = 0.0f; // 0-100

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bTreated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealingProgress = 0.0f; // 0-100, reaches 100 = healed
};

UCLASS(ClassGroup=(BiomeSurvivor), meta=(BlueprintSpawnableComponent))
class BIOMESURVIOR_API UMedicalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMedicalComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Apply a new injury */
	UFUNCTION(BlueprintCallable, Category="Medical")
	void ApplyInjury(EInjuryType Type, float Severity);

	/** Treat an injury (bandage, splint, etc.) */
	UFUNCTION(BlueprintCallable, Category="Medical")
	bool TreatInjury(int32 InjuryIndex);

	/** Use a medical item */
	UFUNCTION(BlueprintCallable, Category="Medical")
	void UseMedicalItem(FName ItemID);

	/** Get all current injuries */
	UFUNCTION(BlueprintPure, Category="Medical")
	const TArray<FInjury>& GetInjuries() const { return ActiveInjuries; }

	/** Does the player have any injuries? */
	UFUNCTION(BlueprintPure, Category="Medical")
	bool HasInjuries() const { return ActiveInjuries.Num() > 0; }

	/** Has untreated bleeding? */
	UFUNCTION(BlueprintPure, Category="Medical")
	bool IsBleeding() const;

	/** Healing rate (per second, when treated) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Medical")
	float BaseHealingRate = 0.5f;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInjuryApplied, EInjuryType, Type);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInjuryHealed, EInjuryType, Type);

	UPROPERTY(BlueprintAssignable, Category="Medical")
	FOnInjuryApplied OnInjuryApplied;

	UPROPERTY(BlueprintAssignable, Category="Medical")
	FOnInjuryHealed OnInjuryHealed;

private:
	UPROPERTY()
	TArray<FInjury> ActiveInjuries;
};
