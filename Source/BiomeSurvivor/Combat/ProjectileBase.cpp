// Copyright Biome Survivor. All Rights Reserved.

#include "Combat/ProjectileBase.h"
#include "BiomeSurvivor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->InitSphereRadius(10.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(CollisionComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = GravityScale;

	SetReplicates(true);
	InitialLifeSpan = LifeSpan;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);

	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->ProjectileGravityScale = GravityScale;

	SetLifeSpan(LifeSpan);
}

void AProjectileBase::FireInDirection(const FVector& Direction)
{
	ProjectileMovement->Velocity = Direction.GetSafeNormal() * Speed;
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetInstigator()) return;

	if (HasAuthority())
	{
		UGameplayStatics::ApplyDamage(
			OtherActor,
			Damage,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass()
		);

		UE_LOG(LogBiomeSurvivor, Log, TEXT("Projectile hit: %s for %.1f damage"), *OtherActor->GetName(), Damage);
	}

	// Stop movement
	ProjectileMovement->StopMovementImmediately();

	if (bRecoverable)
	{
		// Stick to surface, become pickupable
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AttachToComponent(OtherComp, FAttachmentTransformRules::KeepWorldTransform);
		SetLifeSpan(60.0f); // Persist for 1 minute
	}
	else
	{
		Destroy();
	}
}
