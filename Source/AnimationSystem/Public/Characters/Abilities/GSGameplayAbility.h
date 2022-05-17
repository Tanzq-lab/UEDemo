// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Game/AnimationSystem.h"
#include "GSGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class ANIMATIONSYSTEM_API UGSGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGSGameplayAbility();

	// 当按下输入时，具有此设置的能力将自动激活
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EGSAbilityInputID AbilityInputID = EGSAbilityInputID::None;

	// 将能力与槽相关联而不绑定到自动激活的输入的值。
	// 被动技能不会与输入绑定，所以我们需要一种将技能与插槽关联起来的方法。
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EGSAbilityInputID AbilityID = EGSAbilityInputID::None;
	
	// If true, this ability will activate when its bound input is pressed. Disable if you want to bind an ability to an
	// input but not have it activate when pressed.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bActivateOnInput = true;
	
	// 表示技能被授予后立即激活。用于被动能力和他人被强制拥有的能力。
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bActivateAbilityOnGranted = false;

	//如果一个能力被标记为'ActivateAbilityOnGranted'，在这里给出时立即激活它们
	// Epic的评论:项目可能想要启动被动或做其他“BeginPlay”类型的逻辑。
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
};
