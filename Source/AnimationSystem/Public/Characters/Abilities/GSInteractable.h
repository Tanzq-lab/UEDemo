// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "GSInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UGSInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 通过 GAS 与物体进行交互的接口
 */
class ANIMATIONSYSTEM_API IGSInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	* 该物体可以和玩家交互？
	*
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	bool IsAvailableForInteraction(UPrimitiveComponent* InteractionComponent) const;
	virtual bool IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent) const;
	

	/**
	* 玩家需要按下多久才可以与该物体交互？
	*
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	float GetInteractionDuration(UPrimitiveComponent* InteractionComponent) const;
	virtual float GetInteractionDuration_Implementation(UPrimitiveComponent* InteractionComponent) const;

	/**
	* 在调用PreInteract()之前，我们是否同步？怎样同步? 默认为 false 和 OnlyServerWait。
	* OnlyServerWait - client 预测地调用 PreInteract()。
	* OnlyClientWait - 客户端等待服务器调用 PreInteract()。
	* 如果我们正在激活另一个ASC(玩家)的能力，并想要与我们的交互持续时间计时器同步动作或动画，这是很有用的。
	* 客户端和服务器在调用PreInteract()之前相互等待。
	* 玩家复活使用 OnlyClientWait 以便玩家复活与服务器同步，因为我们不能本地预测一个能力运行在另一个玩家。
	* 倒下玩家的复活动画将与本地玩家的互动持续时间计时器同步。
	*
	* 
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void GetPreInteractSyncType(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent) const;
	virtual void GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent) const;

	/**
	* 我们是否同步？在调用PostInteract()之前怎样同步? 默认为 false 和 OnlyServerWait。
	* OnlyServerWait - client预测地调用PostInteract()。
	* OnlyClientWait -客户端等待服务器调用PostInteract()。
	* 客户端和服务器在调用PostInteract()之前相互等待。
	* 玩家复活使用 OnlyServerWait，使客户端不被卡在交互持续时间结束后等待服务器。
	* Revive的PostInteract()只会在伺服器上运行代码，所以客户端可以比伺服器先“完成”。
	*
	* 
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void GetPostInteractSyncType(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent) const;
	void GetPostInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent) const;

	/**
	* 与物体进行互动。
	* 这将会在启动交互持续时间计时器之前调用。
	* 这可能起到一些作用，应用(可预测性或不可预测性)GameplayEffects，触发(可预测性或不可预测性)gameplayability等等。
	* 您可以使用此函数授予将在PostInteract()上被预先激活的能力，以隐藏AbilitySpec复制时间。
	* 如果你想做一些预测，你可以从InteractingActor获得ASC，并使用它的ScopedPredictionKey。
	* 玩家复活使用PreInteract()触发一个能力，播放与互动持续时间相同的动画。如果这个能力结束，它将在PostInteract()中复活玩家。
	*
	*
	* 
	* @param InteractingActor The Actor interacting with this Actor. It will be the AvatarActor from a GameplayAbility.
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void PreInteract(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent);
	virtual void PreInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent) {};

	/**
	* 与这个角色互动。这将在交互持续时间计时器完成后调用。这可能会起到一些作用，应用(可预测性或不可预测性)GameplayEffects，触发(可预测性或不可预测性)gameplayability等等。
	* 如果你想做一些预测，你可以从InteractingActor获得ASC，并使用它的ScopedPredictionKey。
	* 如果你需要预先触发GameplayAbility，玩家的ASC需要提前获得该能力。
	* 如果你不想在游戏开始时授予每个可能的预测能力，你可以通过在PreInteract()中授予它来隐藏复制AbilitySpec所需的时间，而不是交互所需的时间。
	* 
	*
	* @param InteractingActor The Actor interacting with this Actor. It will be the AvatarActor from a GameplayAbility.
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void PostInteract(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent);
	virtual void PostInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent) {};

	/**
	* 取消正在进行的交互，通常是在等待交互持续时间定时器时发生在PreInteract()中的任何事情。
	* 如果玩家提前释放输入，这个函数将被调用。
	*
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void CancelInteraction(UPrimitiveComponent* InteractionComponent);
	virtual void CancelInteraction_Implementation(UPrimitiveComponent* InteractionComponent) {};

	/**
	* 当这个Actor取消交互(例如死亡)时，为GA_Interact返回一个委托来绑定到该委托。
	*
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	virtual FSimpleMulticastDelegate* GetTargetCancelInteractionDelegate(UPrimitiveComponent* InteractionComponent);

	/**
	* 注册一个与此可交互表交互的参与者。
	* 当这个交互对象想要提前取消交互时(例如，一个正在恢复的玩家在恢复过程中死亡)，发送一个GameplayEvent给他们。
	* 不意味着被覆盖。
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable|Do Not Override")
	void RegisterInteracter(UPrimitiveComponent* InteractionComponent, AActor* InteractingActor);
	void RegisterInteracter_Implementation(UPrimitiveComponent* InteractionComponent, AActor* InteractingActor);

	/**
	* 从这个可交互表中注销一个交互 actor。
	* 不意味着被覆盖。
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable|Do Not Override")
	void UnregisterInteracter(UPrimitiveComponent* InteractionComponent, AActor* InteractingActor);
	void UnregisterInteracter_Implementation(UPrimitiveComponent* InteractionComponent, AActor* InteractingActor);

	/**
	* 可交互(可以是外部的Actor，但不是 Interacter )想要取消交互(例如，正在恢复的玩家在恢复过程中死亡)。
	* 不意味着被覆盖。
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable|Do Not Override")
	void InteractableCancelInteraction(UPrimitiveComponent* InteractionComponent);
	void InteractableCancelInteraction_Implementation(UPrimitiveComponent* InteractionComponent);

protected:
	TMap<UPrimitiveComponent*, TArray<AActor*>> Interacters;
};
