#include "HudWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "HudWidgetComponent.h"
#include "Blueprint/WidgetTree.h"

TSharedRef<SWidget> UHudWidget::RebuildWidget()
{
    if (!WidgetTree)
    {
        WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
    }

    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
    WidgetTree->RootWidget = RootCanvas;

    RebuildHud();
    return RootCanvas->TakeWidget();
}

void UHudWidget::NativeConstruct()
{
    Super::NativeConstruct();
    RebuildHud();
}

void UHudWidget::RebuildHud()
{
    if (!BackgroundLayer)
    {
        BackgroundLayer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("BackgroundLayer"));
        UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(BackgroundLayer);
        if (Slot)
        {
            Slot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
            Slot->SetAlignment(FVector2D(0.0f, 1.0f));
            Slot->SetPosition(FVector2D(0.0f, -0.0f));
            Slot->SetAutoSize(true);
        }
    }

    if (!Layer1Overlay)
    {
        Layer1Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Layer1Overlay"));
        UCanvasPanelSlot* Slot = RootCanvas->AddChildToCanvas(Layer1Overlay);
        if (Slot)
        {
            Slot->SetAnchors(FAnchors(0.0f, 1.0f, 0.0f, 1.0f));
            Slot->SetAlignment(FVector2D(0.0f, 1.0f));
            Slot->SetPosition(FVector2D(0.0f, -0.0f));
            Slot->SetAutoSize(true);
        }
    }

    if (!BackgroundLayer || !Layer1Overlay)
    {
        return;
    }

    BackgroundLayer->ClearChildren();
    Layer1Overlay->ClearChildren();

    for (UHudWidgetComponent* Component : Components)
    {
        if (!Component)
        {
            continue;
        }

        UOverlay* TargetLayer = (Component->Layer <= 0) ? BackgroundLayer : Layer1Overlay;
        Component->BuildWidget(this, WidgetTree, TargetLayer);
    }
}
