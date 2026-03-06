// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AnimalAIController.generated.h"

/**
 * AAnimalAIController
 * AI Controller for all wildlife. Uses behavior trees and environment queries.
 * Supports perception (sight, hearing), patrol, flee, attack, stalk behaviors.
 */

UCLASS()
class BIOMESURVIVOR_API AAnimalAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAnimalAIController();

	/** Behavior tree asset to run */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	UBehaviorTree* BehaviorTreeAsset;

	/** Sight perception configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Perception")
	float SightRadius = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Perception")
	float LoseSightRadius = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Perception")
	float SightAngle = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Perception")
	float HearingRange = 1500.0f;

	/** Blackboard keys */
	static const FName BB_TargetActor;
	static const FName BB_HomeLocation;
	static const FName BB_PatrolLocation;
	static const FName BB_AnimalState;

	// ---- API ----

	/** Set target to pursue/flee from */
	UFUNCTION(BlueprintCallable, Category="AI")
	void SetTargetActor(AActor* Target);

	/** Clear current target */
	UFUNCTION(BlueprintCallable, Category="AI")
	void ClearTarget();

	/** Get a random patrol point within wander radius */
	UFUNCTION(BlueprintCallable, Category="AI")
	FVector GetRandomPatrolPoint() const;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	/** AI Perception setup */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
	class UAIPerceptionComponent* AIPerceptionComp;

	UPROPERTY()
	class UAISenseConfig_Sight* SightConfig;

	UPROPERTY()
	class UAISenseConfig_Hearing* HearingConfig;

	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	virtual void Tick(float DeltaTime) override;

	/** Current target tracked by perception */
	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;

	/** Timer for wander pauses */
	float WanderPauseTimer = 0.0f;

	/** Timer for attack cooldown */
	float AIAttackTimer = 0.0f;

	/** Whether currently moving to a location */
	bool bIsMovingToPoint = false;

	/** Handle AI movement for each state */
	void ProcessIdle(float DeltaTime, class AAnimalBase* Animal);
	void ProcessWander(float DeltaTime, class AAnimalBase* Animal);
	void ProcessFlee(float DeltaTime, class AAnimalBase* Animal);
	void ProcessAttack(float DeltaTime, class AAnimalBase* Animal);
	void ProcessStalking(float DeltaTime, class AAnimalBase* Animal);
};
