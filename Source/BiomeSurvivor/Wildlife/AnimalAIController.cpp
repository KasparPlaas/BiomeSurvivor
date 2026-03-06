// Copyright Biome Survivor. All Rights Reserved.

#include "Wildlife/AnimalAIController.h"
#include "Wildlife/AnimalBase.h"
#include "BiomeSurvivor.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"

const FName AAnimalAIController::BB_TargetActor = TEXT("TargetActor");
const FName AAnimalAIController::BB_HomeLocation = TEXT("HomeLocation");
const FName AAnimalAIController::BB_PatrolLocation = TEXT("PatrolLocation");
const FName AAnimalAIController::BB_AnimalState = TEXT("AnimalState");

AAnimalAIController::AAnimalAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// Setup AI Perception
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerceptionComp);

	// Sight configuration
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = SightAngle;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	AIPerceptionComp->ConfigureSense(*SightConfig);

	// Hearing configuration
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = HearingRange;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
	AIPerceptionComp->ConfigureSense(*HearingConfig);

	AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

void AAnimalAIController::BeginPlay()
{
	Super::BeginPlay();

	AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AAnimalAIController::OnPerceptionUpdated);
}

void AAnimalAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AAnimalBase* Animal = Cast<AAnimalBase>(InPawn);
	if (!Animal) return;

	// Update perception based on animal's detection radius
	SightConfig->SightRadius = Animal->DetectionRadius;
	SightConfig->LoseSightRadius = Animal->DetectionRadius * 1.25f;
	AIPerceptionComp->ConfigureSense(*SightConfig);

	// Start in Wandering state
	Animal->SetState(EAnimalState::Wandering);
	WanderPauseTimer = FMath::FRandRange(1.0f, 3.0f);

	UE_LOG(LogBiomeSurvivor, Log, TEXT("AI Controller possessed: %s (%s behavior)"),
		*Animal->AnimalName.ToString(), *UEnum::GetValueAsString(Animal->BehaviorType));
}

void AAnimalAIController::SetTargetActor(AActor* Target)
{
	CurrentTarget = Target;
}

void AAnimalAIController::ClearTarget()
{
	CurrentTarget = nullptr;
}

FVector AAnimalAIController::GetRandomPatrolPoint() const
{
	AAnimalBase* Animal = Cast<AAnimalBase>(GetPawn());
	if (!Animal) return FVector::ZeroVector;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return Animal->GetActorLocation();

	FNavLocation NavLoc;
	bool bFound = NavSys->GetRandomReachablePointInRadius(
		Animal->GetActorLocation(),
		Animal->WanderRadius,
		NavLoc
	);

	return bFound ? NavLoc.Location : Animal->GetActorLocation();
}

void AAnimalAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	AAnimalBase* Animal = Cast<AAnimalBase>(GetPawn());
	if (!Animal || Animal->IsDead()) return;

	for (AActor* Actor : UpdatedActors)
	{
		// Check if it's a player character
		if (!Actor || !Actor->IsA(ACharacter::StaticClass())) continue;

		FActorPerceptionBlueprintInfo Info;
		AIPerceptionComp->GetActorsPerception(Actor, Info);

		bool bSensed = false;
		for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
		{
			if (Stimulus.WasSuccessfullySensed())
			{
				bSensed = true;
				break;
			}
		}

		if (bSensed)
		{
			switch (Animal->BehaviorType)
			{
			case EAnimalBehavior::Passive:
				SetTargetActor(Actor);
				Animal->SetState(EAnimalState::Fleeing);
				break;

			case EAnimalBehavior::Aggressive:
				SetTargetActor(Actor);
				Animal->SetState(EAnimalState::Stalking);
				break;

			case EAnimalBehavior::Territorial:
				{
					float Distance = FVector::Dist(Animal->GetActorLocation(), Actor->GetActorLocation());
					if (Distance < Animal->DetectionRadius * 0.5f)
					{
						SetTargetActor(Actor);
						Animal->SetState(EAnimalState::Attacking);
					}
				}
				break;

			case EAnimalBehavior::Neutral:
				// Only react if already damaged
				break;
			}
		}
		else
		{
			// Lost sight of target
			if (Animal->CurrentState == EAnimalState::Fleeing ||
				Animal->CurrentState == EAnimalState::Stalking)
			{
				ClearTarget();
				Animal->SetState(EAnimalState::Wandering);
			}
		}
	}
}

// ============ TICK-BASED AI STATE MACHINE ============

void AAnimalAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AAnimalBase* Animal = Cast<AAnimalBase>(GetPawn());
	if (!Animal || Animal->IsDead()) return;

	switch (Animal->CurrentState)
	{
	case EAnimalState::Idle:
		ProcessIdle(DeltaTime, Animal);
		break;
	case EAnimalState::Wandering:
		ProcessWander(DeltaTime, Animal);
		break;
	case EAnimalState::Fleeing:
		ProcessFlee(DeltaTime, Animal);
		break;
	case EAnimalState::Attacking:
		ProcessAttack(DeltaTime, Animal);
		break;
	case EAnimalState::Stalking:
		ProcessStalking(DeltaTime, Animal);
		break;
	case EAnimalState::Feeding:
	case EAnimalState::Sleeping:
		// Passive states - just wait
		WanderPauseTimer -= DeltaTime;
		if (WanderPauseTimer <= 0.0f)
		{
			Animal->SetState(EAnimalState::Wandering);
		}
		break;
	default:
		break;
	}

	// Cooldown attack timer
	if (AIAttackTimer > 0.0f)
	{
		AIAttackTimer -= DeltaTime;
	}
}

void AAnimalAIController::ProcessIdle(float DeltaTime, AAnimalBase* Animal)
{
	WanderPauseTimer -= DeltaTime;
	if (WanderPauseTimer <= 0.0f)
	{
		Animal->SetState(EAnimalState::Wandering);
		bIsMovingToPoint = false;
	}
}

void AAnimalAIController::ProcessWander(float DeltaTime, AAnimalBase* Animal)
{
	if (!bIsMovingToPoint)
	{
		// Pick a new random patrol point
		FVector PatrolPoint = GetRandomPatrolPoint();
		if (!PatrolPoint.IsZero())
		{
			MoveToLocation(PatrolPoint, 50.0f);
			bIsMovingToPoint = true;
		}
	}
	else
	{
		// Check if we've reached our destination
		EPathFollowingStatus::Type Status = GetMoveStatus();
		if (Status != EPathFollowingStatus::Moving)
		{
			bIsMovingToPoint = false;
			// Pause before next wander
			Animal->SetState(EAnimalState::Idle);
			WanderPauseTimer = FMath::FRandRange(2.0f, 6.0f);
		}
	}
}

void AAnimalAIController::ProcessFlee(float DeltaTime, AAnimalBase* Animal)
{
	if (!CurrentTarget)
	{
		Animal->SetState(EAnimalState::Wandering);
		return;
	}

	float DistToTarget = FVector::Dist(Animal->GetActorLocation(), CurrentTarget->GetActorLocation());

	// If far enough away, stop fleeing
	if (DistToTarget > Animal->FleeDistance)
	{
		ClearTarget();
		StopMovement();
		Animal->SetState(EAnimalState::Wandering);
		return;
	}

	// Move away from target
	FVector FleeDir = (Animal->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
	FVector FleeTarget = Animal->GetActorLocation() + FleeDir * 800.0f;
	MoveToLocation(FleeTarget, 50.0f);
}

void AAnimalAIController::ProcessAttack(float DeltaTime, AAnimalBase* Animal)
{
	if (!CurrentTarget)
	{
		Animal->SetState(EAnimalState::Wandering);
		return;
	}

	float DistToTarget = FVector::Dist(Animal->GetActorLocation(), CurrentTarget->GetActorLocation());

	// If target is out of detection range, give up
	if (DistToTarget > Animal->DetectionRadius * 1.5f)
	{
		ClearTarget();
		StopMovement();
		Animal->SetState(EAnimalState::Wandering);
		return;
	}

	if (DistToTarget <= Animal->AttackRange)
	{
		// In attack range - deal damage
		StopMovement();

		if (AIAttackTimer <= 0.0f)
		{
			FDamageEvent DamageEvent;
			CurrentTarget->TakeDamage(Animal->AttackDamage, DamageEvent, this, Animal);
			AIAttackTimer = Animal->AttackCooldown;

			UE_LOG(LogBiomeSurvivor, Verbose, TEXT("%s attacked for %.1f damage"),
				*Animal->AnimalName.ToString(), Animal->AttackDamage);
		}
	}
	else
	{
		// Chase target
		MoveToActor(CurrentTarget, Animal->AttackRange * 0.8f);
	}
}

void AAnimalAIController::ProcessStalking(float DeltaTime, AAnimalBase* Animal)
{
	if (!CurrentTarget)
	{
		Animal->SetState(EAnimalState::Wandering);
		return;
	}

	float DistToTarget = FVector::Dist(Animal->GetActorLocation(), CurrentTarget->GetActorLocation());

	// If close enough, switch to attacking
	if (DistToTarget < Animal->AttackRange * 2.0f)
	{
		Animal->SetState(EAnimalState::Attacking);
		return;
	}

	// Slowly approach
	MoveToActor(CurrentTarget, Animal->AttackRange * 1.5f);
}
