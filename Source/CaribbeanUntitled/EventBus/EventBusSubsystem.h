//
// EventBusSubsystem.h
// CaribbeanUntitled
//

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EventBusSubsystem.generated.h"


/**
 * Generic typed event bus.
 *
 * Events are defined by their payload type.
 *
 * Example:
 *
 *     FAttackPayload
 *     FMovementPayload
 *     FDeathPayload
 *
 * Systems can publish and subscribe to any payload type
 * without the EventBus needing to know about that type.
 *
 * Example:
 *
 *     EventBus->Publish(FAttackPayload(...));
 *
 *     EventBus->Subscribe<FAttackPayload>(
 *         ...
 *     );
 */
UCLASS()
class CARIBBEANUNTITLED_API UEventBusSubsystem
    : public UWorldSubsystem
{
    GENERATED_BODY()

public:

    //=========================================================================
    // Publish
    //=========================================================================

    /**
     * Publishes a typed event to every listener registered for PayloadType.
     */
    template<typename PayloadType>
    void Publish(const PayloadType& Payload);


    /**
     * Publishes a typed event by moving the payload.
     */
    template<typename PayloadType>
    void Publish(PayloadType&& Payload);


    //=========================================================================
    // Subscribe
    //=========================================================================

    /**
     * Subscribes an object member function to a payload type.
     *
     * Example:
     *
     * EventBus->Subscribe<FAttackPayload>(
     *     this,
     *     &UAnimationSystem::OnAttack
     * );
     */
    template<typename PayloadType, typename UserClass>
    void Subscribe(
        UserClass* Object,
        void (UserClass::*Function)(const PayloadType&)
    );


    //=========================================================================
    // Unsubscribe
    //=========================================================================

    /**
     * Removes all subscriptions belonging to an object.
     */
    template<typename UserClass>
    void Unsubscribe(UserClass* Object);


private:

    /**
     * Base interface allowing the EventBus to store different
     * typed event channels in one container.
     */
    class IEventChannel
    {
    public:

        virtual ~IEventChannel() = default;

        virtual void RemoveObject(void* Object) = 0;
    };


    /**
     * Typed event channel.
     *
     * Every payload type gets its own channel.
     */
    template<typename PayloadType>
    class TEventChannel : public IEventChannel
    {
    public:

        DECLARE_MULTICAST_DELEGATE_OneParam(
            FDelegate,
            const PayloadType&
        );

        FDelegate Delegate;


        virtual void RemoveObject(void* Object) override
        {
            Delegate.RemoveAll(Object);
        }
    };


    /**
     * Event channels indexed by their payload type.
     */
    TMap<
        uint32,
        TUniquePtr<IEventChannel>
    > EventChannels;


    /**
     * Returns the channel for a payload type.
     *
     * Creates it if it does not already exist.
     */
    template<typename PayloadType>
    TEventChannel<PayloadType>& GetOrCreateChannel();


    /**
     * Gets the unique runtime ID associated with a C++ type.
     */
    template<typename PayloadType>
    static uint32 GetEventTypeId();
};

//=============================================================================
// Template Implementation
//=============================================================================

template<typename PayloadType>
uint32 UEventBusSubsystem::GetEventTypeId()
{
    using DecayedType = typename TDecay<PayloadType>::Type;
    static const uint32 TypeId = GetTypeHash(DecayedType::StaticStruct());
    return TypeId;
}

template<typename PayloadType>
void UEventBusSubsystem::Publish(const PayloadType& Payload)
{
    using DecayedType = typename TDecay<PayloadType>::Type;

    const uint32 TypeId = GetEventTypeId<DecayedType>();

    TUniquePtr<IEventChannel>* ExistingChannel = EventChannels.Find(TypeId);
    if (!ExistingChannel)
    {
        return;
    }

    TEventChannel<DecayedType>& Channel =
        static_cast<TEventChannel<DecayedType>&>(**ExistingChannel);

    Channel.Delegate.Broadcast(Payload);
}

template<typename PayloadType>
void UEventBusSubsystem::Publish(PayloadType&& Payload)
{
    using DecayedType = typename TDecay<PayloadType>::Type;
    Publish<DecayedType>(static_cast<const DecayedType&>(Payload));
}

template<typename PayloadType>
UEventBusSubsystem::TEventChannel<PayloadType>&
UEventBusSubsystem::GetOrCreateChannel()
{
    using DecayedType = typename TDecay<PayloadType>::Type;

    const uint32 TypeId = GetEventTypeId<DecayedType>();

    TUniquePtr<IEventChannel>* ExistingChannel = EventChannels.Find(TypeId);
    if (ExistingChannel)
    {
        return static_cast<TEventChannel<DecayedType>&>(**ExistingChannel);
    }

    TUniquePtr<TEventChannel<DecayedType>> NewChannel = MakeUnique<TEventChannel<DecayedType>>();
    TEventChannel<DecayedType>* ChannelPtr = NewChannel.Get();

    EventChannels.Add(TypeId, MoveTemp(NewChannel));

    return *ChannelPtr;
}


template<typename PayloadType, typename UserClass>
void UEventBusSubsystem::Subscribe(
    UserClass* Object,
    void (UserClass::*Function)(const PayloadType&)
)
{
    if (!Object || !Function)
    {
        return;
    }

    TEventChannel<PayloadType>& Channel =
        GetOrCreateChannel<PayloadType>();

    Channel.Delegate.AddUObject(
        Object,
        Function
    );
}


template<typename UserClass>
void UEventBusSubsystem::Unsubscribe(
    UserClass* Object
)
{
    if (!Object)
    {
        return;
    }

    for (TPair<uint32, TUniquePtr<IEventChannel>>& Pair : EventChannels)
    {
        if (Pair.Value)
        {
            Pair.Value->RemoveObject(Object);
        }
    }
}
