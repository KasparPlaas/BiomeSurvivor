// LoadingScreenWidget.h - Async level loading screen with progress, tips, and biome art
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class UWidgetAnimation;
class UOverlay;
class UBorder;

USTRUCT(BlueprintType)
struct FLoadingTip
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText TipText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Category;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* AssociatedImage = nullptr;
};

UCLASS()
class BIOMESURVIVOR_API ULoadingScreenWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Loading")
    void StartLoading(const FString& MapName, const FText& DisplayName);

    UFUNCTION(BlueprintCallable, Category = "Loading")
    void SetProgress(float Progress);

    UFUNCTION(BlueprintCallable, Category = "Loading")
    void SetStatusText(const FText& Status);

    UFUNCTION(BlueprintCallable, Category = "Loading")
    void FinishLoading();

    UFUNCTION(BlueprintCallable, Category = "Loading")
    void ShowNextTip();

protected:
    // ─── Bound Widgets ───

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UImage* BackgroundImage;

    UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
    UProgressBar* LoadingProgressBar;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* LoadingStatusText;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* PercentageText;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* MapNameText;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* TipCategoryText;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UTextBlock* TipText;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UImage* TipImage;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    UBorder* FadeOverlay;

    // ─── Animations ───

    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    UWidgetAnimation* FadeInAnimation;

    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    UWidgetAnimation* FadeOutAnimation;

    UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
    UWidgetAnimation* TipTransitionAnimation;

    // ─── Configuration ───

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    TArray<FLoadingTip> LoadingTips;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    float TipRotationInterval = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    float MinimumDisplayTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    float FadeOutDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    TMap<FString, UTexture2D*> MapBackgrounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    UTexture2D* DefaultBackground;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    FLinearColor ProgressBarColor = FLinearColor(0.9f, 0.7f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    FLinearColor ProgressBarBackgroundColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.8f);

    // ─── Blueprint Events ───

    UFUNCTION(BlueprintImplementableEvent, Category = "Loading")
    void OnLoadingStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Loading")
    void OnLoadingFinished();

    UFUNCTION(BlueprintImplementableEvent, Category = "Loading")
    void OnProgressUpdated(float Progress);

private:
    float CurrentProgress = 0.0f;
    float TargetProgress = 0.0f;
    float DisplayedProgress = 0.0f;
    float TipTimer = 0.0f;
    float LoadingStartTime = 0.0f;
    int32 CurrentTipIndex = -1;
    bool bIsLoading = false;
    bool bReadyToFinish = false;

    void InitializeDefaultTips();
    void UpdateProgressSmooth(float DeltaTime);
    void SelectRandomTip();
};
