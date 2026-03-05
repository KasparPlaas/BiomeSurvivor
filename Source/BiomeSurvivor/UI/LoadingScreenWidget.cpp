// LoadingScreenWidget.cpp - Loading screen implementation
#include "UI/LoadingScreenWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Engine/Texture2D.h"
#include "Kismet/KismetMathLibrary.h"

void ULoadingScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (LoadingTips.Num() == 0)
    {
        InitializeDefaultTips();
    }

    if (LoadingProgressBar)
    {
        LoadingProgressBar->SetFillColorAndOpacity(ProgressBarColor);
        LoadingProgressBar->SetPercent(0.0f);
    }

    SetVisibility(ESlateVisibility::Collapsed);
}

void ULoadingScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIsLoading) return;

    UpdateProgressSmooth(InDeltaTime);

    // Auto-rotate tips
    TipTimer += InDeltaTime;
    if (TipTimer >= TipRotationInterval)
    {
        TipTimer = 0.0f;
        ShowNextTip();
    }

    // If loading is done and minimum display time has passed, fade out
    if (bReadyToFinish)
    {
        const float Elapsed = GetWorld()->GetRealTimeSeconds() - LoadingStartTime;
        if (Elapsed >= MinimumDisplayTime && DisplayedProgress >= 0.99f)
        {
            bIsLoading = false;

            if (FadeOutAnimation)
            {
                PlayAnimation(FadeOutAnimation);

                FTimerHandle FadeTimer;
                GetWorld()->GetTimerManager().SetTimer(FadeTimer, [this]()
                {
                    SetVisibility(ESlateVisibility::Collapsed);
                    OnLoadingFinished();
                }, FadeOutDuration, false);
            }
            else
            {
                SetVisibility(ESlateVisibility::Collapsed);
                OnLoadingFinished();
            }
        }
    }
}

void ULoadingScreenWidget::StartLoading(const FString& MapName, const FText& DisplayName)
{
    bIsLoading = true;
    bReadyToFinish = false;
    CurrentProgress = 0.0f;
    TargetProgress = 0.0f;
    DisplayedProgress = 0.0f;
    TipTimer = 0.0f;
    LoadingStartTime = GetWorld()->GetRealTimeSeconds();

    SetVisibility(ESlateVisibility::Visible);

    // Set background for this map
    if (BackgroundImage)
    {
        UTexture2D** MapBG = MapBackgrounds.Find(MapName);
        UTexture2D* BG = MapBG ? *MapBG : DefaultBackground;
        if (BG)
        {
            BackgroundImage->SetBrushFromTexture(BG);
        }
    }

    // Set map name
    if (MapNameText)
    {
        MapNameText->SetText(DisplayName);
    }

    // Reset progress bar
    if (LoadingProgressBar)
    {
        LoadingProgressBar->SetPercent(0.0f);
    }

    if (PercentageText)
    {
        PercentageText->SetText(FText::FromString("0%"));
    }

    if (LoadingStatusText)
    {
        LoadingStatusText->SetText(FText::FromString("Loading..."));
    }

    // Show first tip
    SelectRandomTip();

    // Play fade in
    if (FadeInAnimation)
    {
        PlayAnimation(FadeInAnimation);
    }

    OnLoadingStarted();
}

void ULoadingScreenWidget::SetProgress(float Progress)
{
    TargetProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
    OnProgressUpdated(TargetProgress);
}

void ULoadingScreenWidget::SetStatusText(const FText& Status)
{
    if (LoadingStatusText)
    {
        LoadingStatusText->SetText(Status);
    }
}

void ULoadingScreenWidget::FinishLoading()
{
    TargetProgress = 1.0f;
    bReadyToFinish = true;
}

void ULoadingScreenWidget::ShowNextTip()
{
    if (LoadingTips.Num() == 0) return;

    int32 NewIndex = CurrentTipIndex;
    if (LoadingTips.Num() > 1)
    {
        while (NewIndex == CurrentTipIndex)
        {
            NewIndex = FMath::RandRange(0, LoadingTips.Num() - 1);
        }
    }
    else
    {
        NewIndex = 0;
    }

    CurrentTipIndex = NewIndex;
    const FLoadingTip& Tip = LoadingTips[CurrentTipIndex];

    if (TipTransitionAnimation)
    {
        PlayAnimation(TipTransitionAnimation);
    }

    if (TipText)
    {
        TipText->SetText(Tip.TipText);
    }

    if (TipCategoryText)
    {
        TipCategoryText->SetText(Tip.Category);
    }

    if (TipImage)
    {
        if (Tip.AssociatedImage)
        {
            TipImage->SetBrushFromTexture(Tip.AssociatedImage);
            TipImage->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            TipImage->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void ULoadingScreenWidget::UpdateProgressSmooth(float DeltaTime)
{
    // Smooth progress interpolation
    const float InterpSpeed = 3.0f;
    DisplayedProgress = FMath::FInterpTo(DisplayedProgress, TargetProgress, DeltaTime, InterpSpeed);

    if (LoadingProgressBar)
    {
        LoadingProgressBar->SetPercent(DisplayedProgress);
    }

    if (PercentageText)
    {
        const int32 Pct = FMath::RoundToInt(DisplayedProgress * 100.0f);
        PercentageText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Pct)));
    }
}

void ULoadingScreenWidget::SelectRandomTip()
{
    if (LoadingTips.Num() == 0) return;
    CurrentTipIndex = FMath::RandRange(0, LoadingTips.Num() - 1);

    const FLoadingTip& Tip = LoadingTips[CurrentTipIndex];
    if (TipText) TipText->SetText(Tip.TipText);
    if (TipCategoryText) TipCategoryText->SetText(Tip.Category);
}

void ULoadingScreenWidget::InitializeDefaultTips()
{
    auto AddTip = [this](const FString& Text, const FString& Category)
    {
        FLoadingTip Tip;
        Tip.TipText = FText::FromString(Text);
        Tip.Category = FText::FromString(Category);
        LoadingTips.Add(Tip);
    };

    // Survival tips
    AddTip("Keep your temperature above freezing. Hypothermia is the #1 killer in the wilderness.", "Survival");
    AddTip("Campfires provide warmth, light, and allow you to cook food. Always carry tinder.", "Survival");
    AddTip("Raw meat can make you sick. Cook all meat before eating.", "Survival");
    AddTip("Stay hydrated! Dehydration reduces stamina and can be fatal.", "Survival");
    AddTip("Sleep restores health and energy, but leaves you vulnerable to predators.", "Survival");
    AddTip("Blizzards can drop visibility to near zero. Find shelter immediately.", "Survival");
    AddTip("Wet clothing dramatically reduces insulation. Dry your clothes by a fire.", "Survival");

    // Crafting tips
    AddTip("Combine sticks and stone to craft basic tools at any time.", "Crafting");
    AddTip("Better materials produce better tools. Metal tools last much longer than stone.", "Crafting");
    AddTip("Crafting stations unlock advanced recipes not available in the field.", "Crafting");
    AddTip("Repair tools before they break completely. Broken tools cannot be repaired.", "Crafting");

    // Exploration tips
    AddTip("Different biomes have unique resources. Explore to find rare materials.", "Exploration");
    AddTip("Mark important locations on your map. You won't remember them all.", "Exploration");
    AddTip("The compass always points North. Use it with landmarks to navigate.", "Exploration");
    AddTip("Supply caches can be found in abandoned structures. Search carefully.", "Exploration");
    AddTip("Caves provide excellent shelter from storms and cold winds.", "Exploration");

    // Combat tips
    AddTip("Wolves hunt in packs. If you see one, others are nearby.", "Combat");
    AddTip("Bears are territorial but can be avoided by keeping your distance.", "Combat");
    AddTip("Crouching reduces detection. Move slowly through dangerous areas.", "Combat");
    AddTip("Bleeding must be treated quickly. Bandages stop blood loss.", "Combat");
    AddTip("Arrows can be recovered from downed animals. Don't waste them.", "Combat");

    // Building tips
    AddTip("Build your base near water and resources, but away from predator spawns.", "Building");
    AddTip("Walls and roofs protect from wind chill and rain.", "Building");
    AddTip("Upgrade wooden structures to stone for better durability.", "Building");
    AddTip("Storage containers keep your items safe while you explore.", "Building");

    // Multiplayer tips
    AddTip("Team members share body heat when close together.", "Multiplayer");
    AddTip("Coordinate tasks: one gathers, one hunts, one builds.", "Multiplayer");
    AddTip("Mark your base on each player's map so everyone can find it.", "Multiplayer");
}
