#include "CaribbeanGameMode.h"

#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ACaribbeanGameMode::ACaribbeanGameMode()
{
    DefaultPawnClass = APlayableActor::StaticClass();
}

UClass* ACaribbeanGameMode::GetDefaultPawnClassForController_Implementation(
    AController* InController
)
{
    UClass* PawnClass =
        Super::GetDefaultPawnClassForController_Implementation(InController);

    return PawnClass ? PawnClass : APlayableActor::StaticClass();
}

APawn* ACaribbeanGameMode::SpawnDefaultPawnFor_Implementation(
    AController* NewPlayer,
    AActor* StartSpot
)
{
    APlayerController* PlayerController = Cast<APlayerController>(NewPlayer);

    if (!PlayerController)
    {
        return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
    }

    if (bPossessPlacedPlayableActor)
    {
        if (APlayableActor* PlacedPlayable = FindAvailablePlacedPlayableActor())
        {
            UE_LOG(
                LogTemp,
                Log,
                TEXT("CaribbeanGameMode: Reusing placed playable actor %s for %s"),
                *PlacedPlayable->GetName(),
                *PlayerController->GetName()
            );

            return PlacedPlayable;
        }
    }

    return SpawnPlayerForController(PlayerController, StartSpot);
}

// ============================================================
// PLAYER LOGIN
// ============================================================

void ACaribbeanGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
}

void ACaribbeanGameMode::HandleStartingNewPlayer_Implementation(
    APlayerController* NewPlayer
)
{
    Super::HandleStartingNewPlayer_Implementation(NewPlayer);

    if (!HasAuthority() || !NewPlayer)
    {
        return;
    }

    if (Cast<APlayableActor>(NewPlayer->GetPawn()))
    {
        return;
    }

    if (APawn* ExistingPawn = NewPlayer->GetPawn())
    {
        NewPlayer->UnPossess();
        ExistingPawn->Destroy();
    }

    RestartPlayer(NewPlayer);
    EnsurePlayableActorForController(NewPlayer);
}

// ============================================================
// SPAWN PLAYER
// ============================================================

APlayableActor* ACaribbeanGameMode::SpawnPlayerForController(
    APlayerController* PlayerController,
    AActor* StartSpot
)
{
    if (!PlayerController)
    {
        return nullptr;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return nullptr;
    }

    // ============================================================
    // FIND PLAYER START
    // ============================================================

    if (!StartSpot)
    {
        StartSpot = FindPlayerStart(PlayerController);
    }

    FVector SpawnLocation = FVector::ZeroVector;
    FRotator SpawnRotation = FRotator::ZeroRotator;

    if (StartSpot)
    {
        SpawnLocation = StartSpot->GetActorLocation();
        SpawnRotation = StartSpot->GetActorRotation();
    }
    else
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("CaribbeanGameMode: No PlayerStart found for %s. Using world origin."),
            *PlayerController->GetName()
        );
    }

    // ============================================================
    // SPAWN PARAMETERS
    // ============================================================

    FActorSpawnParameters SpawnParameters;

    SpawnParameters.Owner = PlayerController;

    SpawnParameters.Instigator = nullptr;

    SpawnParameters.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Spawn the configured playable pawn class so the GameMode can
    // keep using APlayableActor or any derived Blueprint override.
    TSubclassOf<APawn> PawnClass = GetDefaultPawnClassForController(PlayerController);
    APawn* SpawnedPawn = World->SpawnActor<APawn>(
        PawnClass ? PawnClass.Get() : APlayableActor::StaticClass(),
        SpawnLocation,
        SpawnRotation,
        SpawnParameters
    );

    APlayableActor* Player = Cast<APlayableActor>(SpawnedPawn);

    if (!Player)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("CaribbeanGameMode: Failed to spawn playable pawn for %s"),
            *PlayerController->GetName()
        );

        return nullptr;
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("CaribbeanGameMode: Spawned %s for %s"),
        *Player->GetName(),
        *PlayerController->GetName()
    );

    return Player;
}

APlayableActor* ACaribbeanGameMode::EnsurePlayableActorForController(
    APlayerController* PlayerController
)
{
    if (!PlayerController)
    {
        return nullptr;
    }

    if (APlayableActor* ExistingPlayable =
        Cast<APlayableActor>(PlayerController->GetPawn()))
    {
        return ExistingPlayable;
    }

    if (bPossessPlacedPlayableActor)
    {
        if (APlayableActor* PlacedPlayable = FindAvailablePlacedPlayableActor())
        {
            if (PlayerController->GetPawn() != PlacedPlayable)
            {
                PlayerController->Possess(PlacedPlayable);
            }

            return PlacedPlayable;
        }
    }

    if (APawn* ExistingPawn = PlayerController->GetPawn())
    {
        PlayerController->UnPossess();

        if (HasAuthority())
        {
            ExistingPawn->Destroy();
        }
    }

    if (APlayableActor* SpawnedPlayable = SpawnPlayerForController(PlayerController))
    {
        if (PlayerController->GetPawn() != SpawnedPlayable)
        {
            PlayerController->Possess(SpawnedPlayable);
        }

        return SpawnedPlayable;
    }

    return nullptr;
}

APlayableActor* ACaribbeanGameMode::FindAvailablePlacedPlayableActor() const
{
    UWorld* World = GetWorld();

    if (!World)
    {
        return nullptr;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(
        World,
        APlayableActor::StaticClass(),
        FoundActors
    );

    for (AActor* Actor : FoundActors)
    {
        APlayableActor* PlayableActor = Cast<APlayableActor>(Actor);

        if (!PlayableActor)
        {
            continue;
        }

        if (PlayableActor->GetController())
        {
            continue;
        }

        return PlayableActor;
    }

    return nullptr;
}
