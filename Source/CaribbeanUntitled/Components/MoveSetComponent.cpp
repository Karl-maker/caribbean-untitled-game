// MoveSetComponent.cpp
#include "MoveSetComponent.h"

UMoveSetComponent::UMoveSetComponent()
{
    // Set this component to be initialized when the game starts
    PrimaryComponentTick.bCanEverTick = false;
}

FGameplayCombatMove UMoveSetComponent::GetMove(EMoveSetTypeKey Key) const
{
    if (!IsKeyValid(Key))
    {
        UE_LOG(LogTemp, Warning, TEXT("[MoveSetComponent]: GetMove - Invalid key provided"));
        return FGameplayCombatMove(); // Return empty move
    }

    const FGameplayCombatMove* FoundMove = Moves.Find(Key);
    if (FoundMove)
    {
        return *FoundMove;
    }

    UE_LOG(LogTemp, Warning, TEXT("[MoveSetComponent]: GetMove - No move found for key: %d"), static_cast<int32>(Key));
    return FGameplayCombatMove();
}

void UMoveSetComponent::AddMove(EMoveSetTypeKey Key, const FGameplayCombatMove& Move)
{
    if (!IsKeyValid(Key))
    {
        UE_LOG(LogTemp, Warning, TEXT("[MoveSetComponent]: AddMove - Invalid key provided"));
        return;
    }

    if (Key == EMoveSetTypeKey::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MoveSetComponent]: AddMove - Cannot add move with 'None' key"));
        return;
    }

    // Check if move already exists and log warning if overwriting
    if (Moves.Contains(Key))
    {
        UE_LOG(LogTemp, Warning, TEXT("[MoveSetComponent]: AddMove - Overwriting existing move for key: %d"), static_cast<int32>(Key));
    }

    Moves.Add(Key, Move);
}

bool UMoveSetComponent::RemoveMove(EMoveSetTypeKey Key)
{
    if (!IsKeyValid(Key) || Key == EMoveSetTypeKey::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MoveSetComponent]: RemoveMove - Invalid key provided"));
        return false;
    }

    int32 RemovedCount = Moves.Remove(Key);
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("[MoveSetComponent]: RemoveMove - Successfully removed move for key: %d"), static_cast<int32>(Key));
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("[MoveSetComponent]: RemoveMove - No move found for key: %d"), static_cast<int32>(Key));
    return false;
}

void UMoveSetComponent::ClearAllMoves()
{
    Moves.Empty();
    UE_LOG(LogTemp, Log, TEXT("[MoveSetComponent]: ClearAllMoves - All moves cleared"));
}

bool UMoveSetComponent::HasMove(EMoveSetTypeKey Key) const
{
    if (!IsKeyValid(Key))
    {
        return false;
    }
    return Moves.Contains(Key);
}

TArray<EMoveSetTypeKey> UMoveSetComponent::GetAvailableMoveKeys() const
{
    TArray<EMoveSetTypeKey> Keys;
    Moves.GetKeys(Keys);
    return Keys;
}

bool UMoveSetComponent::IsKeyValid(EMoveSetTypeKey Key) const
{
    // Check if the key is within the enum range
    return Key > EMoveSetTypeKey::None && Key <= EMoveSetTypeKey::CounterAttack;
}
