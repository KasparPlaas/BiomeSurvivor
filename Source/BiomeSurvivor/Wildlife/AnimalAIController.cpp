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

const FName AAnimalAIController::BB_TargetActor = TEXT("TargetActor");
const FName AAnimalAIController::BB_HomeLocation = TEXT("HomeLocation");
const FName AAnimalAIController::BB_PatrolLocation = TEXT("PatrolLocation");
const FName AAnimalAIController::BB_AnimalState = TEXT("AnimalState");

AAnimalAIController::AAnimalAIController()
{
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

	// Run behavior tree
	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);

		// Set home location in blackboard
		UBlackboardComponent* BB = GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsVector(BB_HomeLocation, Animal->GetActorLocation());
		}
	}

	UE_LOG(LogBiomeSurvivor, Log, TEXT("AI Controller possessed: %s (%s behavior)"),
		*Animal->AnimalName.ToString(), *UEnum::GetValueAsString(Animal->BehaviorType));
}

void AAnimalAIController::SetTargetActor(AActor* Target)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsObject(BB_TargetActor, Target);
	}
}

void AAnimalAIController::ClearTarget()
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->ClearValue(BB_TargetActor);
	}
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
