#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerHudController.generated.h"

class UHudWidget;
class ULifeHudWidgetComponent;

UCLASS()
class CARIBBEANUNTITLED_API APlayerHudController : public APlayerController
{
    GENERATED_BODY()

public:
    APlayerHudController();

protected:
    UPROPERTY(EditDefaultsOnly, Category="HUD")
    TSubclassOf<UHudWidget> HudWidgetClass;

    UPROPERTY(EditDefaultsOnly, Instanced, Category="HUD")
    TObjectPtr<ULifeHudWidgetComponent> LifeComponent;

    UPROPERTY(Transient)
    TObjectPtr<UHudWidget> ActiveHudWidget;

    virtual void BeginPlay() override;

private:
    void CreateHud();
};
