// Copyright Biome Survivor. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

/**
 * AProjectileBase
 * Base class for all projectiles (arrows, bolts, thrown weapons).
 * Uses UProjectileMovementComponent for physics simulation.
 */

UCLASS(BlueprintType, Blueprintable)
class BIOMESURVIOR_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Projectile")
	class USphereComponent* CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile")
	float Damage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile")
	float Speed = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile")
	float GravityScale = 1.0f;

	/** Time before projectile auto-destroys */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile")
	float LifeSpan = 10.0f;

	/** Can this projectile be picked up after it lands? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Projectile")
	bool bRecoverable = true;

	/** Initialize projectile direction and speed */
	UFUNCTION(BlueprintCallable, Category="Projectile")
	void FireInDirection(const FVector& Direction);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
			   UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
