// MapMarkerTypes.h - Shared types for the map/compass system
#pragma once

#include "CoreMinimal.h"
#include "MapMarkerTypes.generated.h"

UENUM(BlueprintType)
enum class EMapMarkerType : uint8
{
    PlayerBase       UMETA(DisplayName = "Player Base"),
    PointOfInterest  UMETA(DisplayName = "Point of Interest"),
    Resource         UMETA(DisplayName = "Resource"),
    Danger           UMETA(DisplayName = "Danger Zone"),
    Quest            UMETA(DisplayName = "Quest"),
    Death            UMETA(DisplayName = "Death Location"),
    Custom           UMETA(DisplayName = "Custom Marker"),
    TeamMember       UMETA(DisplayName = "Team Member"),
    Animal           UMETA(DisplayName = "Animal"),
    Shelter          UMETA(DisplayName = "Shelter"),
    WaterSource      UMETA(DisplayName = "Water Source"),
    CraftingStation  UMETA(DisplayName = "Crafting Station")
};

USTRUCT(BlueprintType)
struct FMapMarkerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGuid MarkerId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EMapMarkerType MarkerType = EMapMarkerType::Custom;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector WorldLocation = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Description;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor MarkerColor = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* MarkerIcon = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShowOnCompass = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bShowOnMap = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsVisible = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float VisibilityRange = -1.0f; // -1 = always visible
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsDynamic = false;       // Updates position each frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite) AActor* TrackedActor = nullptr; // For dynamic markers
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DiscoveryTimestamp = 0.0f;

    bool IsValid() const { return MarkerId.IsValid(); }
};

USTRUCT(BlueprintType)
struct FExploredRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D Center;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Radius = 500.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DiscoveryTime = 0.0f;
};
