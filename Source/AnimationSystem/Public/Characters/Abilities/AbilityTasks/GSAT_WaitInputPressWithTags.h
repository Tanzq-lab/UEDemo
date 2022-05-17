﻿// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GSAT_WaitInputPressWithTags.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInputPressWithTagsDelegate, float, TimeWaited);

/**
 * 等待，直到激活能力的输入被按下，ASC有需要的标记而不是被忽略的标记。
 * 如果这是真的，立即启动能力，因为按下键来激活它。
 * 我们希望服务器并行执行这个任务，并保持自己的时间。
 */
UCLASS()
class ANIMATIONSYSTEM_API UGSAT_WaitInputPressWithTags : public UAbilityTask	
{
	GENERATED_BODY()
public:
	UGSAT_WaitInputPressWithTags(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(BlueprintAssignable)
	FInputPressWithTagsDelegate OnPress;

	/**
	* Wait until the user presses the input button for this ability's activation. Returns time this node spent waiting for the press. Will return 0 if input was already down.
	* This is hardcoded for GA_InteractPassive to not fire when State.Interacting TagCount is > State.InteractingRemoval TagCount.
	* //TODO Ideally the RequiredTags, IgnoredTags, and State.Interacting TagCount would get moved into a subclass of FGameplayTagQuery and then we'd only expose that as one
	* parameter and rename the task to WaitInputPress_Query.
	*
	* @param RequiredTags Ability Owner must have all of these tags otherwise the input is ignored.
	* @param IgnoredTags Ability Owner cannot have any of these tags otherwise the input is ignored.
	*/
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSAT_WaitInputPressWithTags* WaitInputPressWithTags(UGameplayAbility* OwningAbility, FGameplayTagContainer RequiredTags, FGameplayTagContainer IgnoredTags, bool bTestAlreadyPressed = false);

	virtual void Activate() override;

	UFUNCTION()
	void OnPressCallback();

protected:
	float StartTime;

	bool bTestInitialState;

	FDelegateHandle DelegateHandle;

	FGameplayTagContainer RequiredTags;
	FGameplayTagContainer IgnoredTags;

	virtual void OnDestroy(bool AbilityEnded) override;

	/**
	* We can only listen for one input pressed event. I think it's because
	* UAbilitySystemComponent::InvokeReplicatedEvent sets ReplicatedData->GenericEvents[(uint8)EventType].bTriggered = true;
	* So if we want to keep listening for more input events, we just clear the delegate handle and bind again.
	*/
	virtual void Reset();
};
