#include "LifeHudWidgetComponent.h"

#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"
#include "HudWidget.h"
#include "../Components/StatsComponent.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

ULifeHudWidgetComponent::ULifeHudWidgetComponent()
{
    ConstructorHelpers::FObjectFinderOptional<UTexture2D> LifeTextureAsset(
        TEXT("/Game/Assets/Hud/life.life")
    );

    if (LifeTextureAsset.Succeeded())
    {
        LifeTexture = LifeTextureAsset.Get();
    }
}

namespace
{
    UStatsComponent* FindStatsComponentFromWidget(UUserWidget* OwnerWidget)
    {
        if (!OwnerWidget)
        {
            return nullptr;
        }

        APlayerController* PC = OwnerWidget->GetOwningPlayer();
        APawn* Pawn = PC ? PC->GetPawn() : nullptr;
        return Pawn ? Pawn->FindComponentByClass<UStatsComponent>() : nullptr;
    }
}

void ULifeHudWidgetComponent::BuildWidget(UUserWidget* OwnerWidget, UWidgetTree* WidgetTree, UOverlay* LayerContainer)
{
    UHudWidget* HudWidget = Cast<UHudWidget>(OwnerWidget);
    if (!HudWidget || !WidgetTree || !LayerContainer)
    {
        return;
    }

    UOverlay* LifeContainer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
    LayerContainer->AddChildToOverlay(LifeContainer);

    if (UOverlaySlot* LayerSlot = Cast<UOverlaySlot>(LifeContainer->Slot))
    {
        LayerSlot->SetPadding(FMargin(ScreenOffset.X, ScreenOffset.Y, 0.0f, 0.0f));
        LayerSlot->SetHorizontalAlignment(HAlign_Left);
        LayerSlot->SetVerticalAlignment(VAlign_Bottom);
    }

    USizeBox* BarBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    BarBox->SetWidthOverride(BarSize.X);
    BarBox->SetHeightOverride(BarSize.Y);

    UImage* BarImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
    BarImage->SetColorAndOpacity(BarColor);
    BarImage->SetBrushSize(BarSize);
    BarBox->AddChild(BarImage);

    UOverlaySlot* BarSlot = LifeContainer->AddChildToOverlay(BarBox);
    if (BarSlot)
    {
        BarSlot->SetPadding(FMargin(BarOffset.X, BarOffset.Y, 0.0f, 0.0f));
        BarSlot->SetHorizontalAlignment(HAlign_Left);
        BarSlot->SetVerticalAlignment(VAlign_Top);
    }

    if (UStatsComponent* Stats = FindStatsComponentFromWidget(OwnerWidget))
    {
        const float MaxHealth = FMath::Max(Stats->GetMaxHealth(), 1.0f);
        const float Fill = FMath::Clamp(Stats->GetHealth() / MaxHealth, 0.0f, 1.0f);
        BarBox->SetWidthOverride(BarSize.X * Fill);
    }

    if (bDrawBackgroundTexture)
    {
        USizeBox* TextureBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
        TextureBox->SetWidthOverride(WidgetSize.X);
        TextureBox->SetHeightOverride(WidgetSize.Y);

        UImage* LifeImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
        if (LifeTexture)
        {
            const FVector2D SourceSize(
                FMath::Max(1.0f, static_cast<float>(LifeTexture->GetSizeX())),
                FMath::Max(1.0f, static_cast<float>(LifeTexture->GetSizeY()))
            );
            const FVector2D FinalSize = SourceSize * FMath::Max(TextureScale, 0.01f);
            LifeImage->SetBrushFromTexture(LifeTexture, true);
            LifeImage->SetBrushSize(FinalSize);
        }

        TextureBox->AddChild(LifeImage);

        UOverlaySlot* TextureSlot = LifeContainer->AddChildToOverlay(TextureBox);
        if (TextureSlot)
        {
            TextureSlot->SetHorizontalAlignment(HAlign_Left);
            TextureSlot->SetVerticalAlignment(VAlign_Top);
        }
    }
}
