// MapComponent.h - Player component managing map/compass state, markers, and fog of war
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/MapMarkerTypes.h"
#include "MapComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMarkerAdded, const FMapMarkerData&, Marker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMarkerRemoved, FGuid, MarkerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMarkerUpdated, const FMapMarkerData&, Marker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRegionExplored, const FExploredRegion&, Region);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BIOMESURVIOR_API UMapComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMapComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Marker Management ───

    UFUNCTION(BlueprintCallable, Category = "Map")
    FGuid AddMarker(const FMapMarkerData& MarkerData);

    UFUNCTION(BlueprintCallable, Category = "Map")
    bool RemoveMarker(FGuid MarkerId);

    UFUNCTION(BlueprintCallable, Category = "Map")
    bool UpdateMarkerLocation(FGuid MarkerId, FVector NewLocation);

    UFUNCTION(BlueprintCallable, Category = "Map")
    bool SetMarkerVisibility(FGuid MarkerId, bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Map")
    void AddPlayerMarker(FVector Location, FText Name, EMapMarkerType Type = EMapMarkerType::Custom);

    UFUNCTION(BlueprintCallable, Category = "Map")
    void AddDeathMarker(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Map")
    void ClearAllMarkers();

    UFUNCTION(BlueprintCallable, Category = "Map")
    void ClearMarkersOfType(EMapMarkerType Type);

    // ─── Queries ───

    UFUNCTION(BlueprintPure, Category = "Map")
    TArray<FMapMarkerData> GetAllMarkers() const { return Markers; }

    UFUNCTION(BlueprintPure, Category = "Map")
    TArray<FMapMarkerData> GetVisibleMarkers() const;

    UFUNCTION(BlueprintPure, Category = "Map")
    TArray<FMapMarkerData> GetMarkersOfType(EMapMarkerType Type) const;

    UFUNCTION(BlueprintPure, Category = "Map")
    TArray<FMapMarkerData> GetCompassMarkers() const;

    UFUNCTION(BlueprintPure, Category = "Map")
    FMapMarkerData GetNearestMarker(FVector Location, EMapMarkerType Type = EMapMarkerType::Custom) const;

    UFUNCTION(BlueprintPure, Category = "Map")
    float GetCompassBearing() const;

    UFUNCTION(BlueprintPure, Category = "Map")
    FVector2D GetPlayerMapPosition() const;

    UFUNCTION(BlueprintPure, Category = "Map")
    float GetDistanceToMarker(FGuid MarkerId) const;

    UFUNCTION(BlueprintPure, Category = "Map")
    float GetBearingToMarker(FGuid MarkerId) const;

    // ─── Fog of War ───

    UFUNCTION(BlueprintPure, Category = "Map")
    bool IsRegionExplored(FVector2D Location) const;

    UFUNCTION(BlueprintPure, Category = "Map")
    float GetExplorationPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Map")
    TArray<FExploredRegion> GetExploredRegions() const { return ExploredRegions; }

    // ─── Events ───

    UPROPERTY(BlueprintAssignable) FOnMarkerAdded OnMarkerAdded;
    UPROPERTY(BlueprintAssignable) FOnMarkerRemoved OnMarkerRemoved;
    UPROPERTY(BlueprintAssignable) FOnMarkerUpdated OnMarkerUpdated;
    UPROPERTY(BlueprintAssignable) FOnRegionExplored OnRegionExplored;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    float ExplorationRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    float ExplorationUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    float CompassVisibilityRange = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    FVector2D MapWorldMin = FVector2D(-100000, -100000);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    FVector2D MapWorldMax = FVector2D(100000, 100000);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    int32 MaxMarkers = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    int32 MaxDeathMarkers = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    bool bAutoDiscoverLocations = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Settings")
    float AutoDiscoverRange = 5000.0f;

private:
    UPROPERTY() TArray<FMapMarkerData> Markers;
    UPROPERTY() TArray<FExploredRegion> ExploredRegions;

    float ExplorationTimer = 0.0f;
    FVector LastExplorationPosition = FVector::ZeroVector;

    void UpdateExploration();
    void UpdateDynamicMarkers();
    FMapMarkerData* FindMarker(FGuid MarkerId);
    FLinearColor GetDefaultColorForType(EMapMarkerType Type) const;
};
