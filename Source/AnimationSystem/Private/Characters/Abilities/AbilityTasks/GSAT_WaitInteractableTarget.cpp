// Copyright ©2022 Tanzq. All rights reserved.


#include "Characters/Abilities/AbilityTasks/GSAT_WaitInteractableTarget.h"
#include "Characters/Abilities/GSInteractable.h"
#include "Characters/Heroes/GSHeroCharacter.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Game/AnimationSystem.h"

const FName NAME_FP_Camera(TEXT("FP_Camera"));

UGSAT_WaitInteractableTarget::UGSAT_WaitInteractableTarget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTraceAffectsAimPitch = true;
}

UGSAT_WaitInteractableTarget* UGSAT_WaitInteractableTarget::WaitForInteractableTarget(
	UGameplayAbility* OwningAbility, FName TaskInstanceName, FCollisionProfileName TraceProfile, float MaxRange,
	float TimerPeriod)
{
	//在这里注册任务列表，提供一个给定的FName作为键
	UGSAT_WaitInteractableTarget* MyObj = NewAbilityTask<UGSAT_WaitInteractableTarget>(OwningAbility, TaskInstanceName);
	MyObj->TraceProfile = TraceProfile;
	MyObj->MaxRange = MaxRange;
	MyObj->TimerPeriod = TimerPeriod;
	// MyObj->bShowDebug = bShowDebug;

	const AGSHeroCharacter* Hero = Cast<AGSHeroCharacter>(OwningAbility->GetCurrentActorInfo()->AvatarActor);

	MyObj->StartLocation = FGameplayAbilityTargetingLocationInfo();
	MyObj->StartLocation.LocationType = EGameplayAbilityTargetingLocationType::SocketTransform;
	MyObj->StartLocation.SourceComponent = Hero->GetMesh();
	MyObj->StartLocation.SourceSocketName = NAME_FP_Camera;
	MyObj->StartLocation.SourceAbility = OwningAbility;

	return MyObj;
}

void UGSAT_WaitInteractableTarget::Activate()
{
	UWorld* World = GetWorld();

	// 激活之后进行射线检测
	World->GetTimerManager().SetTimer(TraceTimerHandle, this, &UGSAT_WaitInteractableTarget::PerformTrace, TimerPeriod,
	                                  true);
}

void UGSAT_WaitInteractableTarget::OnDestroy(bool AbilityEnded)
{
	UWorld* World = GetWorld();
	World->GetTimerManager().ClearTimer(TraceTimerHandle);

	Super::OnDestroy(AbilityEnded);
}

void UGSAT_WaitInteractableTarget::LineTrace(FHitResult& OutHitResult, const UWorld* World, const FVector& Start,
                                             const FVector& End, FName ProfileName, const FCollisionQueryParams Params,
                                             bool bLookForInteractableActor) const
{
	check(World);

	// 多射线检测
	TArray<FHitResult> HitResults;
	World->LineTraceMultiByProfile(HitResults, Start, End, ProfileName, Params);

	OutHitResult.TraceStart = Start;
	OutHitResult.TraceEnd = End;

	// 对射线结果进行遍历
	for (int32 HitIdx = 0; HitIdx < HitResults.Num(); ++HitIdx)
	{
		const FHitResult& Hit = HitResults[HitIdx];

		if (!Hit.Actor.IsValid() || Hit.Actor != Ability->GetCurrentActorInfo()->AvatarActor.Get())
		{
			// If bLookForInteractableActor is false, we're looking for an endpoint to trace to
			// 如果bLookForInteractableActor 是 false，那么就是正在寻找要跟踪的端点。
			if (bLookForInteractableActor && Hit.Actor.IsValid())
			{
				// 当bLookForInteractableActor为true时，命中组件必须重叠COLLISION_INTERACTABLE跟踪通道
				// 这样，一个很大的Actor就可以有一个小的交互按钮。
				if (Hit.Component.IsValid() && Hit.Component.Get()->GetCollisionResponseToChannel(
						COLLISION_INTERACTABLE)
					== ECollisionResponse::ECR_Overlap)
				{
					// Component/Actor must be available to interact
					bool bIsInteractable = Hit.Actor.Get()->Implements<UGSInteractable>();

					if (bIsInteractable && IGSInteractable::Execute_IsAvailableForInteraction(
						Hit.Actor.Get(), Hit.Component.Get()))
					{
						OutHitResult = Hit;
						OutHitResult.bBlockingHit = true; // treat it as a blocking hit
						return;
					}
				}

				OutHitResult.TraceEnd = Hit.Location;
				OutHitResult.bBlockingHit = false; // False means it isn't valid to interact with
				return;
			}

			// This is for the first line trace to get an end point to trace to
			// !Hit.Actor.IsValid() implies we didn't hit anything so return the endpoint as a blocking hit
			// Or if we hit something else
			OutHitResult = Hit;
			OutHitResult.bBlockingHit = true; // treat it as a blocking hit
			return;
		}
	}
}

void UGSAT_WaitInteractableTarget::AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params,
                                                           const FVector& TraceStart, FVector& OutTraceEnd,
                                                           bool bIgnorePitch) const
{
	if (!Ability) // Server and launching client only
	{
		return;
	}

	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();

	// Default to TraceStart if no PlayerController
	FVector ViewStart = TraceStart;
	FRotator ViewRot(0.0f);
	if (PC)
	{
		PC->GetPlayerViewPoint(ViewStart, ViewRot);
	}

	const FVector ViewDir = ViewRot.Vector();
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	ClipCameraRayToAbilityRange(ViewStart, ViewDir, TraceStart, MaxRange, ViewEnd);

	FHitResult HitResult;
	LineTrace(HitResult, InSourceActor->GetWorld(), ViewStart, ViewEnd, TraceProfile.Name, Params, false);

	const bool bUseTraceResult = HitResult.bBlockingHit && (FVector::DistSquared(TraceStart, HitResult.Location) <= (
		MaxRange * MaxRange));

	const FVector AdjustedEnd = (bUseTraceResult) ? HitResult.Location : ViewEnd;

	FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal();
	if (AdjustedAimDir.IsZero())
	{
		AdjustedAimDir = ViewDir;
	}

	if (!bTraceAffectsAimPitch && bUseTraceResult)
	{
		FVector OriginalAimDir = (ViewEnd - TraceStart).GetSafeNormal();

		if (!OriginalAimDir.IsZero())
		{
			// Convert to angles and use original pitch
			const FRotator OriginalAimRot = OriginalAimDir.Rotation();

			FRotator AdjustedAimRot = AdjustedAimDir.Rotation();
			AdjustedAimRot.Pitch = OriginalAimRot.Pitch;

			AdjustedAimDir = AdjustedAimRot.Vector();
		}
	}

	OutTraceEnd = TraceStart + (AdjustedAimDir * MaxRange);
}

bool UGSAT_WaitInteractableTarget::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection,
                                                               FVector AbilityCenter, float AbilityRange,
                                                               FVector& ClippedPosition) const
{
	const FVector CameraToCenter = AbilityCenter - CameraLocation;
	const float DotToCenter = FVector::DotProduct(CameraToCenter, CameraDirection);
	// 如果失败了，我们就会被指向远离中心的地方，但我们可能在球体内部，可以找到一个好的出口。
	if (DotToCenter >= 0)
	{
		const float DistanceSquared = CameraToCenter.SizeSquared() - (DotToCenter * DotToCenter);
		const float RadiusSquared = (AbilityRange * AbilityRange);
		if (DistanceSquared <= RadiusSquared)
		{
			const float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
			const float DistanceAlongRay = DotToCenter + DistanceFromCamera;
			//Subtracting instead of adding will get the other intersection point
			ClippedPosition = CameraLocation + (DistanceAlongRay * CameraDirection);
			//Cam aim point clipped to range sphere
			return true;
		}
	}
	return false;
}

void UGSAT_WaitInteractableTarget::PerformTrace()
{
	bool bTraceComplex = false;
	TArray<AActor*> ActorsToIgnore;

	AActor* SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!SourceActor)
	{
		// Hero is dead
		UE_LOG(LogHints, Error, TEXT("%s SourceActor was null"), *FString(__FUNCTION__));
		return;
	}

	ActorsToIgnore.Add(SourceActor);

	// Check player's perspective, could be 1P or 3P
	AGSHeroCharacter* Hero = Cast<AGSHeroCharacter>(SourceActor);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AGameplayAbilityTargetActor_SingleLineTrace), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);

	// Calculate TraceEnd
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;
	// 仅对服务器和启动客户端有效
	AimWithPlayerController(SourceActor, Params, TraceStart, TraceEnd);

	// ------------------------------------------------------

	FHitResult ReturnHitResult;
	LineTrace(ReturnHitResult, GetWorld(), TraceStart, TraceEnd, TraceProfile.Name, Params, true);


	// bBlockingHit = valid, available Interactable Actor
	if (!ReturnHitResult.bBlockingHit) // 没有找到物体
	{
		// No valid, available Interactable Actor

		// 如果没有找到有效的、可用的交互式Actor，则默认为跟踪行的终点
		ReturnHitResult.Location = TraceEnd;

		if (TargetData.Num() > 0) // 但是保存了之前的数据
		{
			AActor* Item = TargetData.Get(0)->GetHitResult()->Actor.Get();
			if (Item)
			{
				// Previous trace had a valid Interactable Actor, now we don't have one
				// Broadcast last valid target
				LostInteractableTarget.Broadcast(TargetData);
				AGSASCActorBase* InteractActor = Cast<AGSASCActorBase>(Item);
				if (InteractActor)
				{
					InteractActor->TouchEnd(Hero);
				}
			}
		}

		TargetData = MakeTargetData(ReturnHitResult);
	}
	else // 找到了物体
	{
		// Valid, available Interactable Actor

		bool bBroadcastNewTarget = true;

		if (TargetData.Num() > 0) // 数据种还保存着之前的数据
		{
			AActor* OldTarget = TargetData.Get(0)->GetHitResult()->Actor.Get();

			if (OldTarget == ReturnHitResult.Actor.Get())
			{
				// 如果和之前的物体一样，就不用进行广播了。
				bBroadcastNewTarget = false;
			}
			else if (OldTarget) // 找到了新物体
			{
				// 找到了物体，并且这个新物体和之前的不一样。
				// 将之前的物体传递给 Lost 
				LostInteractableTarget.Broadcast(TargetData);

				AGSASCActorBase* InteractActor = Cast<AGSASCActorBase>(OldTarget);
				if (InteractActor)
				{
					InteractActor->TouchEnd(Hero);
				}
			}
		}

		if (bBroadcastNewTarget)
		{
			// 到这里就说明新的物体有效
			// 将新的物体广播给 Found
			TargetData = MakeTargetData(ReturnHitResult);
			FoundNewInteractableTarget.Broadcast(TargetData);

			AGSASCActorBase* NowInteractActor = Cast<AGSASCActorBase>(ReturnHitResult.Actor.Get());
			if (NowInteractActor)
			{
				NowInteractActor->OnTouch(Hero);
			}
		}
	}

#if ENABLE_DRAW_DEBUG

	if (Hero->GetShowTraces())
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, TimerPeriod);

		if (ReturnHitResult.bBlockingHit)
		{
			DrawDebugSphere(GetWorld(), ReturnHitResult.Location, 20.0f, 16, FColor::Red, false, TimerPeriod);
		}
		else
		{
			DrawDebugSphere(GetWorld(), ReturnHitResult.TraceEnd, 20.0f, 16, FColor::Green, false, TimerPeriod);
		}
	}
#endif // ENABLE_DRAW_DEBUG
}

FGameplayAbilityTargetDataHandle UGSAT_WaitInteractableTarget::MakeTargetData(const FHitResult& HitResult) const
{
	/** Note: This will be cleaned up by the FGameplayAbilityTargetDataHandle (via an internal TSharedPtr) */
	return StartLocation.MakeTargetDataHandleFromHitResult(Ability, HitResult);
}
