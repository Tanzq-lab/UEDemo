// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Engine/CollisionProfile.h"
#include "GSAT_WaitInteractableTarget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitInteractableTargetDelegate, const FGameplayAbilityTargetDataHandle&,
                                            Data);

/**
 * 对计时器执行行跟踪，寻找实现IGSInteractable的Actor，该Actor可用于交互。
 * StartLocations是硬编码的。
 * 如果只有一个起始位置，那么应该在AbilityTask节点上使用一个参数使其更通用。
 */
UCLASS()
class ANIMATIONSYSTEM_API UGSAT_WaitInteractableTarget : public UAbilityTask
{
	GENERATED_BODY()

public:
	UGSAT_WaitInteractableTarget(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(BlueprintAssignable)
	FWaitInteractableTargetDelegate FoundNewInteractableTarget;

	UPROPERTY(BlueprintAssignable)
	FWaitInteractableTargetDelegate LostInteractableTarget;

	/**
	* Traces a line on a timer looking for InteractableTargets.
	* @param MaxRange How far to trace.
	* @param TimerPeriod Period of trace timer.
	*/
	UFUNCTION(BlueprintCallable,
		meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true",
			HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static UGSAT_WaitInteractableTarget* WaitForInteractableTarget(UGameplayAbility* OwningAbility,
	                                                               FName TaskInstanceName,
	                                                               FCollisionProfileName TraceProfile,
	                                                               float MaxRange = 200.0f, float TimerPeriod = 0.1f);

	// 激活 Task
	virtual void Activate() override;

protected:
	FGameplayAbilityTargetingLocationInfo StartLocation;

	float MaxRange = 0.f;

	float TimerPeriod = 0.f;

	// bool bShowDebug = false;

	bool bTraceAffectsAimPitch = false;

	FCollisionProfileName TraceProfile;

	FGameplayAbilityTargetDataHandle TargetData;

	FTimerHandle TraceTimerHandle;

	virtual void OnDestroy(bool AbilityEnded) override;

	/** Traces as normal, but will manually filter all hit actors */
	void LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start, const FVector& End,
	               FName ProfileName, const FCollisionQueryParams Params, bool bLookForInteractableActor) const;

	void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart,
	                             FVector& OutTraceEnd, bool bIgnorePitch = false) const;

	// 将射线终点限制到一个合适的位置
	bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter,
	                                 float AbilityRange, FVector& ClippedPosition) const;

	UFUNCTION()
	void PerformTrace();

	FGameplayAbilityTargetDataHandle MakeTargetData(const FHitResult& HitResult) const;
private:
	
};
