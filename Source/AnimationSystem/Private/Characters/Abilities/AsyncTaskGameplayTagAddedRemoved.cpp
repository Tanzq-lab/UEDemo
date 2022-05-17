// Copyright ©2022 Tanzq. All rights reserved.

#include "Characters/Abilities/AsyncTaskGameplayTagAddedRemoved.h"

#include "Game/AnimationSystem.h"

UAsyncTaskGameplayTagAddedRemoved* UAsyncTaskGameplayTagAddedRemoved::ListenForGameplayTagAddedOrRemoved(UAbilitySystemComponent * AbilitySystemComponent, FGameplayTagContainer InTags)
{
	UAsyncTaskGameplayTagAddedRemoved* ListenForGameplayTagAddedRemoved = NewObject<UAsyncTaskGameplayTagAddedRemoved>();
	ListenForGameplayTagAddedRemoved->ASC = AbilitySystemComponent;
	ListenForGameplayTagAddedRemoved->Tags = InTags;

	// 如果没有需要添加的Tags 或者 没有ASC 就直接结束。
	if (!IsValid(AbilitySystemComponent) || InTags.Num() < 1)
	{
		ListenForGameplayTagAddedRemoved->EndTask();
		return nullptr;
	}

	TArray<FGameplayTag> TagArray;
	InTags.GetGameplayTagArray(TagArray);
	
	for (const FGameplayTag Tag : TagArray)
	{
		AbilitySystemComponent->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).AddUObject(ListenForGameplayTagAddedRemoved, &UAsyncTaskGameplayTagAddedRemoved::TagChanged);
	}

	return ListenForGameplayTagAddedRemoved;
}

void UAsyncTaskGameplayTagAddedRemoved::EndTask()
{
	if (IsValid(ASC))
	{
		TArray<FGameplayTag> TagArray;
		Tags.GetGameplayTagArray(TagArray);

		for (FGameplayTag Tag : TagArray)
		{
			ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
	}

	SetReadyToDestroy();
	MarkPendingKill();
}

void UAsyncTaskGameplayTagAddedRemoved::TagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		OnTagAdded.Broadcast(Tag);
	}
	else
	{
		OnTagRemoved.Broadcast(Tag);
	}
}
