#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "../Actors/PlayableActor.h"
#include "../UI/PlayerHudController.h"
#include "CaribbeanGameMode.generated.h"

UCLASS()
class CARIBBEANUNTITLED_API ACaribbeanGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:

    ACaribbeanGameMode();

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Player Spawning")
    bool bPossessPlacedPlayableActor = true;

    virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
    virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
    virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    APlayableActor* SpawnPlayerForController(
        APlayerController* PlayerController,
        AActor* StartSpot = nullptr
    );
    APlayableActor* EnsurePlayableActorForController(
        APlayerController* PlayerController
    );
    APlayableActor* FindAvailablePlacedPlayableActor() const;
};

