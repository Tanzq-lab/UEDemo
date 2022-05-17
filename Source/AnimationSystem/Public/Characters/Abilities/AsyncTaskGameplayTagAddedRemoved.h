// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AsyncTaskGameplayTagAddedRemoved.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameplayTagAddedRemoved, FGameplayTag, Tag);

/**
 * 蓝图节点自动注册添加和删除FGameplayTags的监听器。
 * 在Blueprint/UMG中使用很有用。
 */
UCLASS()
class ANIMATIONSYSTEM_API UAsyncTaskGameplayTagAddedRemoved : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
		
public:
	UPROPERTY(BlueprintAssignable)
	FOnGameplayTagAddedRemoved OnTagAdded;

	UPROPERTY(BlueprintAssignable)
	FOnGameplayTagAddedRemoved OnTagRemoved;

	// 监听添加 和 删除 FGameplayTags
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UAsyncTaskGameplayTagAddedRemoved* ListenForGameplayTagAddedOrRemoved(UAbilitySystemComponent* AbilitySystemComponent, FGameplayTagContainer Tags);

	//当你想要结束AsyncTask时，你必须手动调用这个函数。
	//对于UMG Widgets，你可以在Widget的Destruct事件中调用它。
	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	UPROPERTY()
	UAbilitySystemComponent* ASC;

	FGameplayTagContainer Tags;

	virtual void TagChanged(const FGameplayTag Tag, int32 NewCount);
};
