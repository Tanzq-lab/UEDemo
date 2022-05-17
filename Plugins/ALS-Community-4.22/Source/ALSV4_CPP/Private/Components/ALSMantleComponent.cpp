// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Doğa Can Yanıkoğlu


#include "Components/ALSMantleComponent.h"
#include "Character/ALSCharacter.h"
#include "Character/Animation/ALSCharacterAnimInstance.h"
#include "Components/ALSDebugComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Library/ALSMathLibrary.h"


const FName NAME_MantleEnd(TEXT("MantleEnd"));
const FName NAME_MantleUpdate(TEXT("MantleUpdate"));
const FName NAME_MantleTimeline(TEXT("MantleTimeline"));
const FName NAME_ClimbCornerEnd(TEXT("ClimbCornerEnd"));
const FName NAME_ClimbCornerUpdate(TEXT("ClimbCornerUpdate"));
const FName NAME_ClimbCornerTimeline(TEXT("ClimbCornerTimeline"));
static const FName NAME_Hand_L(TEXT("Hand_L"));
static const FName NAME_Hand_R(TEXT("Hand_R"));
static const FName NAME_Foot_L(TEXT("Foot_L"));
static const FName NAME_Foot_R(TEXT("Foot_R"));
static const FName NAME_LocationAmount_X(TEXT("LocationAmount_X"));
static const FName NAME_LocationAmount_Y(TEXT("LocationAmount_Y"));
static const FName NAME_LocationAmount_Z(TEXT("LocationAmount_Z"));
static const FName NAME_LocationDistance_X(TEXT("LocationDistance_X"));
static const FName NAME_LocationDistance_Y(TEXT("LocationDistance_Y"));
static const FName NAME_LocationDistance_Z(TEXT("LocationDistance_Z"));

FName UALSMantleComponent::NAME_IgnoreOnlyPawn(TEXT("IgnoreOnlyPawn"));
ECollisionChannel UALSMantleComponent::ClimbCollisionChannel = ECC_GameTraceChannel1;


UALSMantleComponent::UALSMantleComponent()
{
	/* 两个是连着一起的，代表启动Tick并且之后会关闭。 */
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	/* 初始化时间轴组件 */
	MantleTimeline = CreateDefaultSubobject<UTimelineComponent>(NAME_MantleTimeline);
	ClimbCornerTimeline = CreateDefaultSubobject<UTimelineComponent>(NAME_ClimbCornerTimeline);
}

void UALSMantleComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		OwnerCharacter = Cast<AALSBaseCharacter>(GetOwner());
		if (OwnerCharacter)
		{
			ALSDebugComponent = OwnerCharacter->FindComponentByClass<UALSDebugComponent>();
			AddTickPrerequisiteActor(OwnerCharacter); // 每次都在角色 Tick 后 Tick，以保证每一次都是最新的值

			// 对时间轴进行绑定
			/** Mantle */
			FOnTimelineFloat Mantle_TimelineUpdated;
			FOnTimelineEvent Mantle_TimelineFinished;
			Mantle_TimelineUpdated.BindUFunction(this, NAME_MantleUpdate);
			Mantle_TimelineFinished.BindUFunction(this, NAME_MantleEnd);
			MantleTimeline->SetTimelineFinishedFunc(Mantle_TimelineFinished);
			MantleTimeline->SetLooping(false);
			// 设置时间在指定的时间后停止
			MantleTimeline->SetTimelineLengthMode(TL_TimelineLength);
			// 按照曲线传出的值进行更新
			MantleTimeline->AddInterpFloat(MantleTimelineCurve, Mantle_TimelineUpdated);

			/** ClimbCorner */
			FOnTimelineFloat ClimbCorner_TimelineUpdated;
			FOnTimelineEvent ClimbCorner_TimelineFinished;
			ClimbCorner_TimelineUpdated.BindUFunction(this, NAME_ClimbCornerUpdate);
			ClimbCorner_TimelineFinished.BindUFunction(this, NAME_ClimbCornerEnd);
			ClimbCornerTimeline->SetTimelineFinishedFunc(ClimbCorner_TimelineFinished);
			ClimbCornerTimeline->SetLooping(false);
			// 设置时间在指定的时间后停止
			ClimbCornerTimeline->SetTimelineLengthMode(TL_TimelineLength);
			// 按照曲线传出的值进行更新
			ClimbCornerTimeline->AddInterpFloat(ClimbCornerTimelineCurve, ClimbCorner_TimelineUpdated);

			// 将函数加入委托中
			OwnerCharacter->JumpPressedDelegate.AddUniqueDynamic(this, &UALSMantleComponent::OnOwnerJumpInput);
			OwnerCharacter->JumpReleaseDelegate.AddUniqueDynamic(this, &UALSMantleComponent::OnOwnerJumpRelease);
			OwnerCharacter->ClimbCornerAnimDelegate.AddUniqueDynamic(this, &UALSMantleComponent::ClimbCornerStart);
			OwnerCharacter->RagdollStateChangedDelegate.AddUniqueDynamic(
				this, &UALSMantleComponent::OnOwnerRagdollStateChanged);
		}
	}
}


void UALSMantleComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!OwnerCharacter) return;

	// 在角色处于空中状态并有输入值的时候进行攀爬检查
	if (OwnerCharacter->GetMovementState() == EALSMovementState::InAir)
	{
		if (OwnerCharacter->HasMovementInput())
		{
			if (!MantleCheck(FallingTraceSettings, EDrawDebugTrace::Type::ForOneFrame))
			{
				LadgeClimbCheck(LadgeTraceSettings, EDrawDebugTrace::Type::ForOneFrame);
			}
		}
	}

	// 如果在 LadderClimbing 状态就更新相关状态
	if (OwnerCharacter->GetMovementState() == EALSMovementState::Climbing)
	{
		if (OwnerCharacter->GetMovementAction() == EALSMovementAction::ClimbJumping)
		{
			ClimbJumpUpdate(DeltaTime);
		}
		else
		{
			UpdateLedgeClimb(DeltaTime);
		}
	}
}

/*
 * 开始攀爬
 */
void UALSMantleComponent::MantleStart(float MantleHeight, const FALSComponentAndTransform& MantleLedgeWS,
                                      EALSMantleType MantleType)
{
	// 检测指针是否正常
	if (OwnerCharacter == nullptr || !IsValid(MantleLedgeWS.Component) || !IsValid(MantleTimeline))
	{
		return;
	}

	// 如果是 ALSCharacter 角色类，并且处于爬高处阶段，就将手上的东西先清除
	if (MantleType != EALSMantleType::LowMantle && OwnerCharacter->IsA(AALSCharacter::StaticClass()))
	{
		Cast<AALSCharacter>(OwnerCharacter)->ClearHeldObject();
	}

	// 在攀爬的时候禁用 tick
	SetComponentTickEnabledAsync(false);

	// 步骤1:获取攀爬资源并使用它来设置新的盘攀爬参数。
	const FALSMantleAsset MantleAsset = GetMantleAsset(MantleType, OwnerCharacter->GetOverlayState());
	check(MantleAsset.PositionCorrectionCurve)

	MantleParams.AnimMontage = MantleAsset.AnimMontage;
	MantleParams.PositionCorrectionCurve = MantleAsset.PositionCorrectionCurve;
	MantleParams.StartingOffset = MantleAsset.StartingOffset;
	/* 通过当前攀爬高度来判断动画播放位置和播放速率 */
	MantleParams.StartingPosition = FMath::GetMappedRangeValueClamped({MantleAsset.LowHeight, MantleAsset.HighHeight},
	                                                                  {
		                                                                  MantleAsset.LowStartPosition,
		                                                                  MantleAsset.HighStartPosition
	                                                                  },
	                                                                  MantleHeight);
	MantleParams.PlayRate = FMath::GetMappedRangeValueClamped({MantleAsset.LowHeight, MantleAsset.HighHeight},
	                                                          {MantleAsset.LowPlayRate, MantleAsset.HighPlayRate},
	                                                          MantleHeight);

	// 步骤2:将世界空间目标转换为攀爬组件的局部空间，以用于移动对象。
	MantleLedgeLS.Component = MantleLedgeWS.Component;
	MantleLedgeLS.Transform = MantleLedgeWS.Transform * MantleLedgeWS.Component->GetComponentToWorld().Inverse();

	// 步骤3:设置攀爬目标并计算起始偏移量(角色变换和攀爬物体变换之间的差值)。
	MantleTarget = MantleLedgeWS.Transform;
	MantleActualStartOffset = UALSMathLibrary::TransfromSub(OwnerCharacter->GetActorTransform(), MantleTarget);

	// 步骤4:计算动画从目标位置开始的偏移量。这将是实际动画开始的位置相对于目标变换。
	FVector RotatedVector = MantleTarget.GetRotation().Vector() * MantleParams.StartingOffset.Y; /* 获得水平方向上的向量值 */
	RotatedVector.Z = MantleParams.StartingOffset.Z; /* 起始位置高度 */
	// 也就是起始点往后的一个点
	const FTransform StartOffset(MantleTarget.Rotator(), MantleTarget.GetLocation() - RotatedVector,
	                             FVector::OneVector);
	// 动画真实运动的时候和攀爬点之间的偏差值
	MantleAnimatedStartOffset = UALSMathLibrary::TransfromSub(StartOffset, MantleTarget);

	// 步骤5:清除角色的移动模式，并设置移动状态为攀爬
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);
	OwnerCharacter->SetMovementState(EALSMovementState::Mantling);

	/* 步骤6:
	 * 配置攀爬时间轴，让他和动画播放相同的长度，并与动画播放相同的速度。
	 * 然后开始时间线。
	 */

	// 获取曲线的时间范围
	float MinTime = 0.0f;
	float MaxTime = 0.0f;
	MantleParams.PositionCorrectionCurve->GetTimeRange(MinTime, MaxTime);
	// 让时间轴和动画播放样的时间轴长度
	MantleTimeline->SetTimelineLength(MaxTime - MantleParams.StartingPosition);
	// 同样的播放速率
	MantleTimeline->SetPlayRate(MantleParams.PlayRate);
	MantleTimeline->PlayFromStart();

	// 步骤7: 如果蒙太奇有效的话就播放蒙太奇。
	if (IsValid(MantleParams.AnimMontage))
	{
		OwnerCharacter->GetMainAnimInstance()->Montage_Play(MantleParams.AnimMontage, MantleParams.PlayRate,
		                                                    EMontagePlayReturnType::MontageLength,
		                                                    MantleParams.StartingPosition, false);
	}
}

bool UALSMantleComponent::MantleCheck(const FALSMantleTraceSettings& TraceSettings, EDrawDebugTrace::Type DebugType)
{
	if (!OwnerCharacter)
	{
		return false;
	}

	// 步骤1:向前追踪，找到角色不能行走的墙/物体。
	/* 找到方向， 如果有输入就对应上输入的向量，没有就对应角色向前的方向 */
	const FVector& TraceDirection = OwnerCharacter->HasMovementInput()
		                                ? OwnerCharacter->GetPlayerMovementInput()
		                                : OwnerCharacter->GetActorForwardVector();
	/* 获取角色脚底位置 */
	const FVector& CapsuleBaseLocation = UALSMathLibrary::GetCapsuleBaseLocation(
		2.0f, OwnerCharacter->GetCapsuleComponent());

	/*
	 * 第一次检测
	 * 向角色的上前方和上后方生成胶囊体检测
	 */
	FVector TraceStart = CapsuleBaseLocation + TraceDirection * -30.0f;
	TraceStart.Z += (TraceSettings.MaxLedgeHeight + TraceSettings.MinLedgeHeight) / 2.0f;
	const FVector TraceEnd = TraceStart + TraceDirection * TraceSettings.ReachDistance;
	const float HalfHeight = 1.0f + (TraceSettings.MaxLedgeHeight - TraceSettings.MinLedgeHeight) / 2.0f;

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	FHitResult HitResult;
	{
		const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(
			TraceSettings.ForwardTraceRadius, HalfHeight);
		const bool bHit = World->SweepSingleByProfile(HitResult, TraceStart, TraceEnd, FQuat::Identity,
		                                              ClimbObjectDetectionProfile,
		                                              CapsuleCollisionShape, Params);

		if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
		{
			UALSDebugComponent::DrawDebugCapsuleTraceSingle(World,
			                                                TraceStart,
			                                                TraceEnd,
			                                                CapsuleCollisionShape,
			                                                DebugType,
			                                                bHit,
			                                                HitResult,
			                                                FLinearColor::Black,
			                                                FLinearColor::Black,
			                                                1.0f);
		}
	}

	/*
	 * 在这三种情况下不能进行攀爬：
	 * 1. 没有探测到任何东西
	 * 2. 探测到东西了，但是起始点位置也探测到东西了，说明头上有物体，不能进行攀爬
	 * 3. 只有前面的一个球体探测到了物体，但是检测到可以直接行走，也就是比最小攀爬高度还低，也不能进行攀爬。
	 */
	if (!HitResult.IsValidBlockingHit() || OwnerCharacter->GetCharacterMovement()->IsWalkable(HitResult))
	{
		return false;
	}

	if (HitResult.GetComponent() != nullptr)
	{
		UPrimitiveComponent* PrimitiveComponent = HitResult.GetComponent();
		/* 如果将要攀爬的物体速度超过了设定的速度就不进行攀爬 */
		if (PrimitiveComponent && PrimitiveComponent->GetComponentVelocity().Size() > AcceptableVelocityWhileMantling)
		{
			return false;
		}
	}

	// 跳跃点的位置
	const FVector InitialTraceImpactPoint = HitResult.ImpactPoint;
	// 跳跃点的法线向量 如果是一个水平的台阶就是一个垂直于水平的向量
	const FVector InitialTraceNormal = HitResult.ImpactNormal;

	// 步骤2:从第一个轨迹的撞击点从上往下进行跟踪，并确定命中位置是否可步行。 第二次检测
	FVector DownwardTraceEnd = InitialTraceImpactPoint;
	DownwardTraceEnd.Z = CapsuleBaseLocation.Z;
	DownwardTraceEnd += InitialTraceNormal * -15.0f;
	FVector DownwardTraceStart = DownwardTraceEnd;
	DownwardTraceStart.Z += TraceSettings.MaxLedgeHeight + TraceSettings.DownwardTraceRadius + 1.0f;

	{
		const FCollisionShape SphereCollisionShape = FCollisionShape::MakeSphere(TraceSettings.DownwardTraceRadius);
		const bool bHit = World->SweepSingleByChannel(HitResult, DownwardTraceStart, DownwardTraceEnd, FQuat::Identity,
		                                              WalkableSurfaceDetectionChannel, SphereCollisionShape,
		                                              Params);

		if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
		{
			UALSDebugComponent::DrawDebugSphereTraceSingle(World,
			                                               TraceStart,
			                                               TraceEnd,
			                                               SphereCollisionShape,
			                                               DebugType,
			                                               bHit,
			                                               HitResult,
			                                               FLinearColor::Black,
			                                               FLinearColor::Black,
			                                               1.0f);
		}
	}


	// 不可行走
	if (!OwnerCharacter->GetCharacterMovement()->IsWalkable(HitResult))
	{
		return false;
	}

	// 获得攀爬点以及攀爬物体
	const FVector DownTraceLocation(HitResult.Location.X, HitResult.Location.Y, HitResult.ImpactPoint.Z);
	UPrimitiveComponent* HitComponent = HitResult.GetComponent();

	// 第三步:检查胶囊在向下轨迹的位置是否有站立的空间。
	// 如果有，将该位置设置为目标变换并计算攀爬高度。
	// 在攀爬位置上计算放置一个胶囊体后该胶囊体的位置
	const FVector& CapsuleLocationFBase = UALSMathLibrary::GetCapsuleLocationFromBase(
		DownTraceLocation, 2.0f, OwnerCharacter->GetCapsuleComponent());
	const bool bCapsuleHasRoom = UALSMathLibrary::CapsuleHasRoomCheck(OwnerCharacter->GetCapsuleComponent(),
	                                                                  CapsuleLocationFBase, 0.0f,
	                                                                  0.0f, DebugType,
	                                                                  ALSDebugComponent && ALSDebugComponent->
	                                                                  GetShowTraces());

	// 没有空间，停止检测
	if (!bCapsuleHasRoom)
	{
		return false;
	}

	/* 第四步:
	 * 获得攀爬点的变换信息以及攀爬高度
	 * 通过攀爬高度确定攀爬类型 
	 */
	const FTransform TargetTransform(
		(InitialTraceNormal * FVector(-1.0f, -1.0f, 0.0f)).ToOrientationRotator(), /* 取法线向量的水平旋转值 */
		CapsuleLocationFBase,
		FVector::OneVector);

	const float MantleHeight = (CapsuleLocationFBase - OwnerCharacter->GetActorLocation()).Z;

	EALSMantleType MantleType;
	if (OwnerCharacter->GetMovementState() == EALSMovementState::InAir)
	{
		MantleType = EALSMantleType::FallingCatch;
	}
	else
	{
		MantleType = MantleHeight > 125.0f ? EALSMantleType::HighMantle : EALSMantleType::LowMantle;
	}

	// 第五步 ： 一切准备就绪， 开始攀爬！
	FALSComponentAndTransform MantleWS;
	MantleWS.Component = HitComponent;
	MantleWS.Transform = TargetTransform;
	MantleStart(MantleHeight, MantleWS, MantleType);
	Server_MantleStart(MantleHeight, MantleWS, MantleType);

	return true;
}

/**
 * @brief 攀爬检测，判断是否可以进入攀爬状态
 */
bool UALSMantleComponent::LadgeClimbCheck(const FALSMantleTraceSettings& TraceSettings,
                                          EDrawDebugTrace::Type DebugType)
{
	if (!OwnerCharacter)
	{
		return false;
	}

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	int MaxIndex = (TraceSettings.MaxLedgeHeight - TraceSettings.MinLedgeHeight) / TraceSettings.ForwardTraceRadius + 1;
	for (int i = 0; i < MaxIndex; ++i)
	{
		// 步骤1:向前发射一条胶囊体，检测是否符合LadgeClimb通道
		const FVector& TraceDirection = OwnerCharacter->HasMovementInput()
			                                ? OwnerCharacter->GetPlayerMovementInput()
			                                : OwnerCharacter->GetActorForwardVector();
		/* 获取角色脚底位置 */
		const FVector& CapsuleBaseLocation = UALSMathLibrary::GetCapsuleBaseLocation(
			2.0f, OwnerCharacter->GetCapsuleComponent());

		FVector TraceStart = OwnerCharacter->GetActorLocation();
		TraceStart.Z = CapsuleBaseLocation.Z + (CapsuleBaseLocation.Z + TraceSettings.MaxLedgeHeight - TraceStart.Z) /
			2.f;
		FVector TraceEnd = TraceStart + TraceDirection * TraceSettings.ReachDistance;
		TraceStart.Z += TraceSettings.ForwardTraceRadius * i;
		TraceEnd.Z += TraceSettings.ForwardTraceRadius * i;

		FHitResult HitResult;
		{
			const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(
				TraceSettings.ForwardTraceRadius, 10.f);
			const bool bHit = World->SweepSingleByProfile(HitResult, TraceStart, TraceEnd, FQuat::Identity,
			                                              ClimbObjectDetectionProfile,
			                                              CapsuleCollisionShape, Params);

			if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
			{
				UALSDebugComponent::DrawDebugCapsuleTraceSingle(World,
				                                                TraceStart,
				                                                TraceEnd,
				                                                CapsuleCollisionShape,
				                                                DebugType,
				                                                bHit,
				                                                HitResult,
				                                                FLinearColor::Red,
				                                                FLinearColor::Blue,
				                                                1.0f);
			}

			if (!bHit) continue;
		}

		// 设置攀爬物体
		LedgeClimbLS.Component = HitResult.GetComponent();
		const FVector ImpactPoint = HitResult.ImpactPoint;
		const FVector ImpactNormal = HitResult.ImpactNormal;

		// 从上往下进行检测， 得到物体的高度
		TraceStart = ImpactPoint - ImpactNormal * TraceSettings.ForwardTraceRadius;
		TraceStart.Z += 20.f;
		TraceEnd = TraceStart;
		TraceEnd.Z += 20.f;

		{
			const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(
				TraceSettings.ForwardTraceRadius, 10.f);
			const bool bHit = World->SweepSingleByProfile(HitResult, TraceStart, TraceEnd, FQuat::Identity,
			                                              ClimbObjectDetectionProfile,
			                                              CapsuleCollisionShape, Params);

			if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
			{
				UALSDebugComponent::DrawDebugCapsuleTraceSingle(World,
				                                                TraceStart,
				                                                TraceEnd,
				                                                CapsuleCollisionShape,
				                                                DebugType,
				                                                bHit,
				                                                HitResult,
				                                                FLinearColor::Red,
				                                                FLinearColor::Black,
				                                                1.0f);
			}

			if (bHit) continue;
		}

		FRotator TargetRotation = FRotator(0.f, UKismetMathLibrary::MakeRotFromX(ImpactNormal).Yaw - 180.f, 0.f);
		FVector TargetLocation = ImpactPoint;
		TargetLocation.Z -= 40.f;
		TargetLocation += ImpactNormal * 35.f;

		LedgeTargetWS = FTransform(TargetRotation, TargetLocation, FVector::OneVector);
		// 设置对应攀爬点
		LedgeClimbLS.Transform = LedgeTargetWS;
		LedgeClimbLS.Transform = UALSMathLibrary::ALSComponentWorldToLocal(LedgeClimbLS);

		LedgeTargetWS = UALSMathLibrary::ALSComponentLocalToWorld(LedgeClimbLS);

		// 更新角色相关属性
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		OwnerCharacter->SetMovementState(EALSMovementState::Climbing);
		OwnerCharacter->SetRotationMode(EALSRotationMode::VelocityDirection);
		OwnerCharacter->SetDesiredRotationMode(EALSRotationMode::VelocityDirection);
		OwnerCharacter->SetDesiredLaddering(true);
		return true;
	}

	return false;
}

/**
 * @brief 攀爬移动检测
 */
bool UALSMantleComponent::ClimbingMovingDetection(FName BoneName, bool bIsRight, EDrawDebugTrace::Type DebugType)
{
	
	// 步骤一 ： 手部 从前往后进行谁射线检测， 计算出 手部放置的水平位置和身体对应的旋转值。
	FVector TraceStart = OwnerCharacter->GetActorRotation().UnrotateVector(OwnerCharacter->GetActorLocation());
	// FVector TraceStart = UALSMathLibrary::ALSComponentLocalToWorld(LastLedgeClimbLS).GetLocation();
	TraceStart += FVector(20.f, bIsRight ? 10.f : -10.f, 40.f);
	TraceStart = OwnerCharacter->GetActorRotation().RotateVector(TraceStart);
	FVector TraceEnd = TraceStart + OwnerCharacter->GetActorForwardVector() * 50.f;

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	FHitResult HitResult;
	{
		const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(
			5.f, 20.f);
		const bool bHit = World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity,
		                                              ClimbCollisionChannel,
		                                              CapsuleCollisionShape, Params);

		if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
		{
			UALSDebugComponent::DrawDebugCapsuleTraceSingle(World,
			                                                TraceStart,
			                                                TraceEnd,
			                                                CapsuleCollisionShape,
			                                                DebugType,
			                                                bHit,
			                                                HitResult,
			                                                FLinearColor::Yellow,
			                                                FLinearColor::Blue,
			                                                10.0f);
		}

		if (!bHit)
		{
			return false;
		}
	}

	const FVector ImpactPoint = HitResult.ImpactPoint;
	FRotator NormalRotator = FRotator(0.f, UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal).Yaw - 180.f, 0.f);
	if (UKismetMathLibrary::DegAcos(
		FVector::DotProduct(OwnerCharacter->GetActorForwardVector(), HitResult.ImpactNormal)) < 120.f)
		return false;

	// 第二步： 从探测点的位置进行射线检测，得到手部放置的高度
	TraceStart = HitResult.ImpactPoint;
	TraceStart.Z = HitResult.Location.Z;
	TraceEnd = TraceStart;
	TraceEnd.Z -= 20.f;
	TraceStart.Z += 10.f;

	{
		const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeSphere(10.f);
		const bool bHit = World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity,
		                                              ClimbCollisionChannel,
		                                              CapsuleCollisionShape, Params);

		if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
		{
			UALSDebugComponent::DrawDebugSphereTraceSingle(World,
			                                               TraceStart,
			                                               TraceEnd,
			                                               CapsuleCollisionShape,
			                                               DebugType,
			                                               bHit,
			                                               HitResult,
			                                               FLinearColor::White,
			                                               FLinearColor::Blue,
			                                               10.0f);
		}

		if (!bHit)
		{
			return false;
		}
	}

	if (HitResult.ImpactNormal.Z <= 0.1f)
	{
		return false;
	}

	// 第三步 ： 更新对应值
	FVector TargetLocation = ImpactPoint - NormalRotator.Vector() * 35.f;
	TargetLocation.Z = HitResult.ImpactPoint.Z - 40.f;

	LedgeClimbLS.Component = HitResult.GetComponent();
	LedgeClimbLS.Transform = FTransform(
		NormalRotator,
		TargetLocation,
		FVector::OneVector);
	LedgeClimbLS.Transform = UALSMathLibrary::ALSComponentWorldToLocal(LedgeClimbLS);

	return true;
}

void UALSMantleComponent::UpdateLedgeClimb(float DeltaTime)
{
	//  更新角色相关信息
	UpdateLedgeCharacter(DeltaTime);

	// 更新输入相关信息
	UpdateLedgeInputInfo(DeltaTime);

	// 更新动画相关信息
	UpdateLedgeAnimInfo(DeltaTime);
}


void UALSMantleComponent::UpdateLedgeCharacter(float DeltaTime)
{
	// 插值计算 将角色当前位置变换到目标位置  应对动态变化的物体
	LedgeTargetWS = UALSMathLibrary::ALSComponentLocalToWorld(LedgeClimbLS);

	// 根据角色的不同状态 设置不同的插值速率
	const FVector LagSpeed = bCanMoving ? MovingLagSpeed : NotMoveLagSpeed;

	const FVector InterpLocation = UALSMathLibrary::CalculateAxisIndependentLag(
		OwnerCharacter->GetActorLocation(), LedgeTargetWS.GetLocation(), FRotator(LedgeTargetWS.GetRotation()),
		LagSpeed, DeltaTime / 2.f);

	const FRotator InterpRotation = FMath::RInterpTo(OwnerCharacter->GetActorRotation(),
	                                                 FRotator(LedgeTargetWS.GetRotation()), DeltaTime,
	                                                 RotationInterpSpeed);

	const bool bCapsuleHasRoom = UALSMathLibrary::CapsuleHasRoomCheck(OwnerCharacter->GetCapsuleComponent(),
	                                                                  InterpLocation, 0.0f,
	                                                                  -5.f, EDrawDebugTrace::Type::ForOneFrame,
	                                                                  ALSDebugComponent && ALSDebugComponent->
	                                                                  GetShowTraces());
	if (bCapsuleHasRoom)
	{
		OwnerCharacter->SetActorLocationAndRotation(InterpLocation, InterpRotation);
	}

	const FVector LocationDelta = LedgeTargetWS.InverseTransformVectorNoScale(
		OwnerCharacter->GetActorLocation() - LastLocation);

	const float DirectionX = FMath::GetMappedRangeValueClamped(FVector2D(-.4f, .4f), FVector2D(-2.3f, 2.3f),
	                                                           LocationDelta.Y);
	const float DirectionY = FMath::GetMappedRangeValueClamped(FVector2D(-10.f, 10.f), FVector2D(-5.f, 5.f),
	                                                           Speed.Z);

	const FVector2D TargetDelta = FVector2D(DirectionX, DirectionY) * FMath::GetMappedRangeValueClamped(
		FVector2D(0.f, 80.f), FVector2D(0.f, 1.f), FVector2D(Speed.X, Speed.Y).Size());

	MovingTransitionDelta = FMath::Vector2DInterpConstantTo(MovingTransitionDelta, TargetDelta, DeltaTime,
	                                                        MoveDeltaInterpSpeed);

	const float TargetPlayRate = FMath::GetMappedRangeValueClamped(
			FVector2D(0.f, 1.6f), FVector2D(.6f, 2.2f), Speed.Size())
		* bCanMoving * FMath::Clamp(FMath::Sqrt(1.f / (DeltaTime * 10.f)) * .4f, 0.f, 2.f);

	MovingAnimPlayRate = FMath::Clamp(FMath::FInterpTo(MovingAnimPlayRate, TargetPlayRate, DeltaTime,
	                                                   UALSMathLibrary::GetInterpSpeed(5.f, DeltaTime)), 0.f, 1.f);

	Speed = (OwnerCharacter->GetActorLocation() - LastLocation) / DeltaTime;
	LastLocation = OwnerCharacter->GetActorLocation();

	// 储存上一次位置的局部坐标
	LastLedgeClimbLS.Component = LedgeClimbLS.Component;
	LastLedgeClimbLS.Transform = OwnerCharacter->GetActorTransform();
	LastLedgeClimbLS.Transform = UALSMathLibrary::ALSComponentWorldToLocal(LastLedgeClimbLS);

	if (IsTouchFloor(NAME_Foot_L, NAME_Foot_R, EDrawDebugTrace::Type::ForOneFrame))
	{
		ExitClimbing();
	}
}


/**
 * @brief 更新动画相关属性
 */
void UALSMantleComponent::UpdateLedgeAnimInfo(float DeltaTime)
{
	check(OwnerCharacter && OwnerCharacter->GetMainAnimInstance());
	const auto& MainAnim = OwnerCharacter->GetMainAnimInstance();

	MainAnim->bCanClimbMove = bCanMoving;
	MainAnim->ClimbMovingDelta = MovingTransitionDelta;
	MainAnim->ClimbMovingPlayRate = MovingAnimPlayRate;
}

void UALSMantleComponent::UpdateLedgeInputInfo(float DeltaTime)
{
	// 如果是跳跃蓄力状态，执行跳跃相关检测
	if (bJumpPressed)
	{
		// 跳跃蓄力状态不进行移动
		bCanMoving = false;

		// 进行跳跃检测
		if (!bHasBlock)
		{
			ClimbJumpCheck(EDrawDebugTrace::Type::ForOneFrame);
		}
		return;
	}

	// 如果有移动输入，执行移动检测相关逻辑
	const float RightInputValue = OwnerCharacter->RightInputValue;
	if (RightInputValue > .1f)
	{
		if (ClimbingMovingDetection(NAME_Hand_R, true, EDrawDebugTrace::Type::ForOneFrame))
		{
			if (Speed.Size() >= 10.f || !CornerCheck(true, false))
			{
				bCanMoving = true;
			}
			return;
		}
		CornerCheck(true, true);
	}
	else if (RightInputValue < -.1f)
	{
		if (ClimbingMovingDetection(NAME_Hand_L, false, EDrawDebugTrace::Type::ForOneFrame))
		{
			if (Speed.Size() >= 10.f || !CornerCheck(false, false))
			{
				bCanMoving = true;
			}
			return;
		}

		CornerCheck(false, true);
	}
	bCanMoving = false;
}

/**
 * @brief 判断角色的脚是否触碰到了地面。
 */
bool UALSMantleComponent::IsTouchFloor(FName BoneName_L, FName BoneName_R, EDrawDebugTrace::Type DebugType)
{
	FVector TraceStart = OwnerCharacter->GetMesh()->GetSocketLocation(BoneName_L);
	TraceStart.Z -= 10.f;
	FVector TraceEnd = OwnerCharacter->GetMesh()->GetSocketLocation(BoneName_R);
	TraceEnd.Z -= 10.f;

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	FHitResult HitResult;
	{
		const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeSphere(10.f);
		const bool bHit = World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity,
		                                              ECC_Visibility,
		                                              CapsuleCollisionShape, Params);

		if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
		{
			UALSDebugComponent::DrawDebugSphereTraceSingle(World,
			                                               TraceStart,
			                                               TraceEnd,
			                                               CapsuleCollisionShape,
			                                               DebugType,
			                                               bHit,
			                                               HitResult,
			                                               FLinearColor::Green,
			                                               FLinearColor::Red,
			                                               10.0f);
		}
		if (!bHit) return false;
	}

	if (!OwnerCharacter->GetCharacterMovement()->IsWalkable(HitResult))
	{
		return false;
	}

	// 判断是否有空间站立
	const FVector& CapsuleLocationFBase = UALSMathLibrary::GetCapsuleLocationFromBase(
		HitResult.ImpactPoint, 2.0f, OwnerCharacter->GetCapsuleComponent());
	return UALSMathLibrary::CapsuleHasRoomCheck(OwnerCharacter->GetCapsuleComponent(),
	                                            CapsuleLocationFBase, 0.0f,
	                                            0.0f, DebugType,
	                                            ALSDebugComponent && ALSDebugComponent->
	                                            GetShowTraces());
}

/**
 * @brief 让蒙太奇和时间轴保一致的运行时间。
 */
void UALSMantleComponent::ClimbCornerStart(float TimeLength, float StartTime, float PlayRate)
{
	LedgeClimbLS = CornerClimbValues.CAT_TargetLS;

	ClimbCornerTimeline->SetTimelineLength(TimeLength - StartTime);
	ClimbCornerTimeline->SetPlayRate(PlayRate);
	ClimbCornerTimeline->PlayFromStart();
}

/**
 * @brief 更新角色的相对旋转值和相对移动。
 */
void UALSMantleComponent::ClimbCornerUpdate(float BlendIn)
{
	if (!OwnerCharacter)
	{
		return;
	}

	// 步骤1:从存储的局部变换中不断更新攀爬目标，以跟随移动的对象
	FTransform CornerTarget = UALSMathLibrary::ALSComponentLocalToWorld(CornerClimbValues.CAT_TargetLS);

	// 步骤2:获取当前时间对应的曲线数据
	// 曲线是没有去掉开始位置的长度，所以在获取时间数据时候需要加上起始点
	const FVector CurveVec = CornerClimbValues.GetMoveCurve()->GetVectorValue(
		ClimbCornerTimeline->GetPlaybackPosition());

	// 曲线X对应的是运动高度插值Alpha
	const float PositionAlpha = CurveVec.X;
	// 曲线Y对应的是XZ轴距离插值Alpha
	const float XZCorrectionAlpha = CurveVec.Y;
	// 曲线Z对应的是前后距离插值Alpha
	const float YCorrectionAlpha = CurveVec.Z;

	/* 步骤3:多个变换组合在一起进行差值，独立控制水平和垂直混合到动画 开始位置和目标位置。 */

	// 通过水平运动数据对 X 、 Z 轴 变换进行插值
	const FTransform TargetHzTransform(CornerAnimatedStartOffset.GetRotation(),
	                                   {
		                                   CornerAnimatedStartOffset.GetLocation().X,
		                                   CornerActualStartOffset.GetLocation().Y,
		                                   CornerAnimatedStartOffset.GetLocation().Z
	                                   },
	                                   FVector::OneVector);
	const FTransform& HzLerpResult =
		UKismetMathLibrary::TLerp(CornerActualStartOffset, TargetHzTransform, XZCorrectionAlpha);

	// 通过运动数据对 Y 轴 变换进行插值
	const FTransform TargetVtTransform(CornerActualStartOffset.GetRotation(),
	                                   {
		                                   CornerActualStartOffset.GetLocation().X,
		                                   CornerAnimatedStartOffset.GetLocation().Y,
		                                   CornerActualStartOffset.GetLocation().Z
	                                   },
	                                   FVector::OneVector);
	const FTransform& VtLerpResult =
		UKismetMathLibrary::TLerp(CornerActualStartOffset, TargetVtTransform, YCorrectionAlpha);

	// 最终距离差混合变换结果
	const FTransform ResultTransform(HzLerpResult.GetRotation(),
	                                 {
		                                 HzLerpResult.GetLocation().X, VtLerpResult.GetLocation().Y,
		                                 HzLerpResult.GetLocation().Z
	                                 },
	                                 FVector::OneVector);

	// 对当前混合位置和目标位置进行插值计算，得出最终在的位置
	const FTransform& ResultLerp = UKismetMathLibrary::TLerp(
		UALSMathLibrary::TransfromAdd(CornerTarget, ResultTransform), CornerTarget,
		PositionAlpha);

	// 为了防止角色一下子就到了混合的插值位置，所以添加了一个曲线进行插值，让角色和目标之间有一个过渡
	const FTransform& LerpedTarget =
		UKismetMathLibrary::TLerp(UALSMathLibrary::TransfromAdd(CornerTarget, CornerActualStartOffset), ResultLerp,
		                          BlendIn);

	// 步骤4: 更新角色位置和旋转
	OwnerCharacter->SetActorLocationAndTargetRotation(LerpedTarget.GetLocation(), LerpedTarget.GetRotation().Rotator());
}

void UALSMantleComponent::ClimbCornerEnd()
{
	if (OwnerCharacter)
	{
		OwnerCharacter->SetMovementAction(EALSMovementAction::None);
	}

	// 组件启用 tick 
	SetComponentTickEnabledAsync(true);
}

bool UALSMantleComponent::CornerCheck(bool bIsRight, bool bCanTraceOuter)
{
	int32 OuterRet = -1;
	FTransform CornerTarget;
	const int32 InnerRet = CanCornerClimbing(false, bIsRight, CornerTarget, EDrawDebugTrace::ForOneFrame);
	if (bCanTraceOuter && !InnerRet)
	{
		OuterRet = CanCornerClimbing(true, bIsRight, CornerTarget, EDrawDebugTrace::ForOneFrame);
	}

	/*
	 * 只有两种情况可以继续执行：
	 * 1. 可以向内转向攀爬
	 * 2. 可以向外转向攀爬
	 */
	if (InnerRet == 1)
	{
		CornerClimbValues.CornerType = EALSCornerClimbType::Inner;
	}
	else if (!InnerRet && OuterRet == 1)
	{
		CornerClimbValues.CornerType = EALSCornerClimbType::Outer;
	}
	else
	{
		return false;
	}

	// 禁用 组件tick 函数
	SetComponentTickEnabledAsync(false);

	// 设置旋转角度
	const auto& ActorTrans = OwnerCharacter->GetActorTransform();
	OwnerCharacter->SetRotateInClimbAngle(CornerClimbValues.TurnAngle);

	CornerActualStartOffset = UALSMathLibrary::TransfromSub(ActorTrans, CornerTarget);

	// 步骤4:计算动画从目标位置开始的偏移量。这将是实际动画开始的位置相对于目标变换。
	const FVector RotatedVector = -OwnerCharacter->GetActorForwardVector() * CornerClimbValues.StartingOffset.Y +
		OwnerCharacter->GetActorRightVector() * CornerClimbValues.DirectionValue * (
			CornerClimbValues.StartingOffset.X + CornerClimbValues.TurnAngle > 90.f
				? (CornerClimbValues.TurnAngle - 90.f) * 0.5f
				: 0.f) +
		OwnerCharacter->GetActorUpVector() * (CornerTarget.GetLocation().Z - OwnerCharacter->GetActorLocation().Z +
			20.f);

	const FTransform StartOffset(ActorTrans.GetRotation(), ActorTrans.GetLocation() + RotatedVector,
	                             FVector::OneVector);
	// 动画真实运动的时候和攀爬点之间的偏差值
	CornerAnimatedStartOffset = UALSMathLibrary::TransfromSub(StartOffset, CornerTarget);

	// 设置当前运动动作，通知动画实例
	OwnerCharacter->SetMovementAction(EALSMovementAction::CornerClimbing);

	return true;
}

/**
 * @return 1 : 代表可以转身， 0 ： 代表没有检测到物体，  -1 ： 代表检测到了物体，但是太高了。
 */
int32 UALSMantleComponent::CanCornerClimbing(bool bIsOuter, bool bIsRight, FTransform& CornerTarget,
                                             EDrawDebugTrace::Type DebugType)
{
	UWorld* World = GetWorld();
	check(World);

	/* 步骤一 ： 设置攀爬旋转的起始终止位置 --- 分为外部检测和内部检测 */
	FVector TraceStart, TraceEnd;
	if (bIsOuter)
	{
		TraceEnd = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 60.f + OwnerCharacter
			->GetActorUpVector() * 40.f;
		TraceStart = TraceEnd + OwnerCharacter->GetActorRightVector() * (bIsRight ? 50.f : -50.f);
		TraceEnd += OwnerCharacter->GetActorRightVector() * (bIsRight ? -80.f : 80.f);
	}
	else
	{
		TraceStart = OwnerCharacter->GetActorLocation() - OwnerCharacter->GetActorForwardVector() * 25.f +
			OwnerCharacter->GetActorUpVector() * 40.f;
		TraceEnd = TraceStart + OwnerCharacter->GetActorRightVector() * (bIsRight ? 80.f : -80.f);
	}

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	/*
	 * 步骤二 ： 两次射线检测，得到角色最终要到达的点。
	 */
	FHitResult HitResult;
	{
		const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(
			10.f, 25.f);
		const bool bHit = World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity,
		                                              ClimbCollisionChannel, CapsuleCollisionShape, Params);

		if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
		{
			UALSDebugComponent::DrawDebugCapsuleTraceSingle(World,
			                                                TraceStart,
			                                                TraceEnd,
			                                                CapsuleCollisionShape,
			                                                DebugType,
			                                                bHit,
			                                                HitResult,
			                                                bIsOuter ? FLinearColor::Green : FLinearColor::Yellow,
			                                                bIsOuter ? FLinearColor::Red : FLinearColor::Blue,
			                                                1.0f);
		}

		if (!bHit) return 0;
	}

	FVector TargetLocation = HitResult.ImpactPoint;
	FVector ImpactNormal = HitResult.ImpactNormal;

	TraceStart = HitResult.ImpactPoint;
	TraceEnd = TraceStart;
	TraceStart.Z += 30.f;
	TraceEnd.Z -= 10.f;

	{
		const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeSphere(10.f);
		const bool bHit = World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity,
		                                              ClimbCollisionChannel, CapsuleCollisionShape, Params);

		if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
		{
			UALSDebugComponent::DrawDebugSphereTraceSingle(World,
			                                               TraceStart,
			                                               TraceEnd,
			                                               CapsuleCollisionShape,
			                                               DebugType,
			                                               bHit,
			                                               HitResult,
			                                               FLinearColor::Green,
			                                               FLinearColor::Black,
			                                               1.0f);
		}

		// 如果起始位置就检测到了物体，就说明要转身的这个物体太高了。
		if (HitResult.bBlockingHit && HitResult.bStartPenetrating) return -1;
	}

	// 计算旋转角度
	float TurnAngle = 0.f;
	if (bIsOuter)
	{
		TurnAngle = 180 -
			UKismetMathLibrary::DegAcos(FVector::DotProduct(OwnerCharacter->GetActorForwardVector(), ImpactNormal));
	}
	else
	{
		TurnAngle = FMath::Abs(
			UKismetMathLibrary::DegAcos(FVector::DotProduct(OwnerCharacter->GetActorForwardVector(), ImpactNormal)));
	}
	if (TurnAngle < 10.f) return -1;

	TargetLocation.Z = HitResult.ImpactPoint.Z;
	TargetLocation.Z -= 40.f;
	TargetLocation += ImpactNormal * 35.f;

	/*
	 * 步骤三 ： 判断角色是否能够在检测点上。
	 */
	const bool bCapsuleHasRoom = UALSMathLibrary::CapsuleHasRoomCheck(OwnerCharacter->GetCapsuleComponent(),
	                                                                  TargetLocation,
	                                                                  0.0f,
	                                                                  -10.f, DebugType,
	                                                                  ALSDebugComponent && ALSDebugComponent->
	                                                                  GetShowTraces());

	// 没有空间
	if (!bCapsuleHasRoom)
	{
		return -1;
	}

	/* 步骤四 ： 更新角色攀爬旋转对应信息。 */
	// 获得目标信息
	CornerClimbValues.CAT_TargetLS.Component = HitResult.GetComponent();

	FRotator TargetRotation = FRotator(0.f, UKismetMathLibrary::MakeRotFromX(ImpactNormal).Yaw - 180.f, 0.f);

	CornerTarget = FTransform(TargetRotation, TargetLocation, FVector::OneVector);
	CornerClimbValues.CAT_TargetLS.Transform = CornerTarget;
	CornerClimbValues.CAT_TargetLS.Transform =
		UALSMathLibrary::ALSComponentWorldToLocal(CornerClimbValues.CAT_TargetLS);

	CornerClimbValues.TurnAngle = TurnAngle;
	CornerClimbValues.DirectionValue = bIsRight ? 1.f : -1.f;

	return 1;
}

void UALSMantleComponent::ClimbJumpCheck(EDrawDebugTrace::Type DebugType)
{
	UWorld* World = GetWorld();
	check(World);

	if (FMath::Abs(JumpRightValue) <= 0.4f && FMath::Abs(JumpUpValue) <= 0.4f) return;

	// 步骤一 ： 向移动跳跃方向发射射线检测，检测是否有物体并且可以攀爬，然后得到攀爬角色要移动到的物体高度。
	float JumpHeight = FMath::Clamp((GetWorld()->GetTimeSeconds() - LastJumpInputTime) * JumpParams.JumpGrowRate, 0.f,
	                                JumpParams.JumpLengthMax);


	FVector TraceEnd = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 20.f
		+ OwnerCharacter->GetActorUpVector() * 50.f;
	FVector TraceStart = TraceEnd + (OwnerCharacter->GetActorUpVector() * JumpUpValue + OwnerCharacter->
		GetActorRightVector() * JumpRightValue) * JumpHeight;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	// 向左向右有值，向上向下没有值时，不添加当前攀爬的物体。
	if (FMath::Abs(JumpRightValue) <= 0.1f || FMath::Abs(JumpUpValue) >= 0.1f)
	{
		Params.AddIgnoredActor(LedgeClimbLS.Component->GetOwner());
	}

	FHitResult HitResult;
	{
		const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeSphere(30.f);
		const bool bHit = World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility,
		                                              CapsuleCollisionShape, Params);

		if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
		{
			UALSDebugComponent::DrawDebugSphereTraceSingle(World,
			                                               TraceStart,
			                                               TraceEnd,
			                                               CapsuleCollisionShape,
			                                               DebugType,
			                                               bHit,
			                                               HitResult,
			                                               FLinearColor::Green,
			                                               FLinearColor::Red,
			                                               1.0f);
		}

		if (!bHit) return;
	}

	// 只要能运行到这里，就表明检测到了物体。
	bCanExit = false;
	// 起始位置如果检测到了物体就代表当前这个攀爬点没有手放的地方
	if (HitResult.ImpactNormal.Z <= 0.1f) return;

	const float Height = HitResult.ImpactPoint.Z - 40.f;

	// 步骤二 ： 向前射线检测，得到检测点 x、y轴的值
	TraceStart = HitResult.ImpactPoint;
	TraceStart.Z -= 5.f;
	TraceEnd = TraceStart + OwnerCharacter->GetActorForwardVector() * 20.f;
	TraceStart -= OwnerCharacter->GetActorForwardVector() * 20.f;

	{
		const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeSphere(10.f);
		const bool bHit = World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility,
		                                              CapsuleCollisionShape, Params);

		if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
		{
			UALSDebugComponent::DrawDebugSphereTraceSingle(World,
			                                               TraceStart,
			                                               TraceEnd,
			                                               CapsuleCollisionShape,
			                                               DebugType,
			                                               bHit,
			                                               HitResult,
			                                               FLinearColor::Yellow,
			                                               FLinearColor::Blue,
			                                               1.0f);
		}

		if (!bHit) return;
	}

	// 步骤三 ： 检测跳跃点是否有空间
	FVector TargetLocation = HitResult.ImpactPoint + HitResult.ImpactNormal * 35.f;
	TargetLocation.Z = Height;

	// 计算手放置的位置，做为跳跃的辅助绘制点。
	JumpPoint = HitResult.ImpactPoint;
	JumpPoint.Z = Height + 40.f;

	const bool bCapsuleHasRoom = UALSMathLibrary::CapsuleHasRoomCheck(OwnerCharacter->GetCapsuleComponent(),
	                                                                  TargetLocation + HitResult.ImpactNormal * 20.f,
	                                                                  0.0f,
	                                                                  -10.f, DebugType,
	                                                                  ALSDebugComponent && ALSDebugComponent->
	                                                                  GetShowTraces());

	// 没有空间，停止检测
	if (!bCapsuleHasRoom)
	{
		bHasBlock = true;
		return;
	}

	// 步骤四： 计算到达目标点所需的位置信息
	JumpTargetInfo.Component = HitResult.GetComponent();
	FRotator TargetRotation = FRotator(0.f, UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal).Yaw - 180.f, 0.f);
	JumpTargetInfo.Transform = FTransform(TargetRotation, TargetLocation, FVector::OneVector);
	JumpTargetInfo.Transform = UALSMathLibrary::ALSComponentWorldToLocal(JumpTargetInfo);

	JumpLength = (OwnerCharacter->GetActorLocation() - TargetLocation).Size();

	bCanJumping = true;
}

void UALSMantleComponent::ClimbJumpUpdate(float DeltaTime)
{
	if (!OwnerCharacter)
	{
		return;
	}

	/*
	 * 步骤一 ： 判断是否可以更新角色移动位置
	 */

	// 只有当曲线完全融合的时候才移动角色。
	if (!bCanJumpMove)
	{
		FVector Distance;
		Distance.X = OwnerCharacter->GetMainAnimInstance()->GetCurveValue(NAME_LocationDistance_X);
		Distance.Y = OwnerCharacter->GetMainAnimInstance()->GetCurveValue(NAME_LocationDistance_Y);
		Distance.Z = OwnerCharacter->GetMainAnimInstance()->GetCurveValue(NAME_LocationDistance_Z);

		if (!Distance.Equals(FVector::ZeroVector, 0.1f) && Distance.Equals(AnimJumpDistance, 0.1f))
		{
			bCanJumpMove = true;
		}
		else
		{
			// 曲线没有完全融合，记录当前值，停止更新。
			AnimJumpDistance = Distance;
			return;
		}
	}

	/*
	 * 步骤二 ： 判断是否要结束跳跃
	 */
	LedgeTargetWS = UALSMathLibrary::ALSComponentLocalToWorld(JumpTargetInfo);

	FVector LocationDelta;
	LocationDelta.X = OwnerCharacter->GetMainAnimInstance()->GetCurveValue(NAME_LocationAmount_X);
	LocationDelta.Y = OwnerCharacter->GetMainAnimInstance()->GetCurveValue(NAME_LocationAmount_Y);
	LocationDelta.Z = OwnerCharacter->GetMainAnimInstance()->GetCurveValue(NAME_LocationAmount_Z);

	// 如果曲线值传递过来动画相差距离都为零并且已经移动到了终点的话，就退出跳跃状态。
	if (LocationDelta.Equals(FVector::ZeroVector, 0.1f) && LedgeTargetWS.Equals(LedgeTargetWS, 0.1f))
	{
		OwnerCharacter->SetMovementAction(EALSMovementAction::None);
		return;
	}


	/*
	 * 步骤三 ： 更新角色位置
	 */
	FVector Loc = OwnerCharacter->GetActorRotation().UnrotateVector(LedgeTargetWS.GetLocation());
	if (FMath::Abs(LocationDelta.X) > 0.f && FMath::Abs(AnimJumpDistance.X) > 0.f && FMath::Abs(JumpDistanceDiff.X) >
		0.f)
	{
		Loc.X -= LocationDelta.X * JumpDistanceDiff.X / AnimJumpDistance.X;
	}
	if (FMath::Abs(LocationDelta.Y) > 0.f && FMath::Abs(AnimJumpDistance.Y) > 0.f && FMath::Abs(JumpDistanceDiff.Y) >
		0.f)
	{
		Loc.Y -= LocationDelta.Y * JumpDistanceDiff.Y / AnimJumpDistance.Y;
	}
	if (FMath::Abs(LocationDelta.Z) > 0.f && FMath::Abs(AnimJumpDistance.Z) > 0.f && FMath::Abs(JumpDistanceDiff.Z) >
		0.f)
	{
		Loc.Z += LocationDelta.Z * FMath::Abs(JumpDistanceDiff.Z) / AnimJumpDistance.Z;
	}

	const FVector InterpLoc = UKismetMathLibrary::VInterpTo(OwnerCharacter->GetActorLocation(),
	                                                        OwnerCharacter->GetActorRotation().RotateVector(Loc),
	                                                        DeltaTime,
	                                                        UALSMathLibrary::GetInterpSpeed(20.f, DeltaTime));

	// TestPoint = InterpLoc;
	OwnerCharacter->SetActorLocation(InterpLoc);
}

void UALSMantleComponent::ExitClimbing()
{
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	OwnerCharacter->SetMovementState(EALSMovementState::InAir);
	OwnerCharacter->SetMovementAction(EALSMovementAction::None);
	OwnerCharacter->SetDesiredRotationMode(EALSRotationMode::LookingDirection);
}

void UALSMantleComponent::Server_ExitClimbing_Implementation()
{
	Multicast_ExitClimbing();
}

void UALSMantleComponent::Multicast_ExitClimbing_Implementation()
{
	if (OwnerCharacter && !OwnerCharacter->IsLocallyControlled())
	{
		ExitClimbing();
	}
}

void UALSMantleComponent::Server_MantleStart_Implementation(float MantleHeight,
                                                            const FALSComponentAndTransform& MantleLedgeWS,
                                                            EALSMantleType MantleType)
{
	Multicast_MantleStart(MantleHeight, MantleLedgeWS, MantleType);
}

void UALSMantleComponent::Multicast_MantleStart_Implementation(float MantleHeight,
                                                               const FALSComponentAndTransform& MantleLedgeWS,
                                                               EALSMantleType MantleType)
{
	if (OwnerCharacter && !OwnerCharacter->IsLocallyControlled())
	{
		MantleStart(MantleHeight, MantleLedgeWS, MantleType);
	}
}

// 攀爬动作的更新
void UALSMantleComponent::MantleUpdate(float BlendIn)
{
	if (!OwnerCharacter)
	{
		return;
	}

	// 步骤1:从存储的局部变换中不断更新攀爬目标，以跟随移动的对象
	MantleTarget = UALSMathLibrary::MantleComponentLocalToWorld(MantleLedgeLS);

	// 步骤2:获取当前时间对应的曲线数据
	// 曲线是没有去掉开始位置的长度，所以在获取时间数据时候需要加上起始点
	const FVector CurveVec = MantleParams.PositionCorrectionCurve
	                                     ->GetVectorValue(
		                                     MantleParams.StartingPosition + MantleTimeline->GetPlaybackPosition());
	// 曲线X对应的是运动高度插值Alpha
	const float PositionAlpha = CurveVec.X;
	// 曲线Y对应的是水平距离插值Alpha
	const float XYCorrectionAlpha = CurveVec.Y;
	// 曲线Z对应的是垂直距离插值Alpha
	const float ZCorrectionAlpha = CurveVec.Z;

	/* 步骤3:多个变换组合在一起进行差值，独立控制水平和垂直混合到动画 开始位置和目标位置。 */

	// 通过水平运动数据对水平变换进行插值
	const FTransform TargetHzTransform(MantleAnimatedStartOffset.GetRotation(),
	                                   {
		                                   MantleAnimatedStartOffset.GetLocation().X,
		                                   MantleAnimatedStartOffset.GetLocation().Y,
		                                   MantleActualStartOffset.GetLocation().Z
	                                   },
	                                   FVector::OneVector);
	const FTransform& HzLerpResult =
		UKismetMathLibrary::TLerp(MantleActualStartOffset, TargetHzTransform, XYCorrectionAlpha);

	// 通过垂直运动数据对垂直变换进行插值
	const FTransform TargetVtTransform(MantleActualStartOffset.GetRotation(),
	                                   {
		                                   MantleActualStartOffset.GetLocation().X,
		                                   MantleActualStartOffset.GetLocation().Y,
		                                   MantleAnimatedStartOffset.GetLocation().Z
	                                   },
	                                   FVector::OneVector);
	const FTransform& VtLerpResult =
		UKismetMathLibrary::TLerp(MantleActualStartOffset, TargetVtTransform, ZCorrectionAlpha);

	// 最终距离差混合变换结果
	const FTransform ResultTransform(HzLerpResult.GetRotation(),
	                                 {
		                                 HzLerpResult.GetLocation().X, HzLerpResult.GetLocation().Y,
		                                 VtLerpResult.GetLocation().Z
	                                 },
	                                 FVector::OneVector);

	// 对当前混合位置和目标位置进行插值计算，得出最终在的位置
	const FTransform& ResultLerp = UKismetMathLibrary::TLerp(
		UALSMathLibrary::TransfromAdd(MantleTarget, ResultTransform), MantleTarget,
		PositionAlpha);

	// 为了防止角色一下子就到了混合的插值位置，所以添加了一个曲线进行插值，让角色和目标之间有一个过渡
	const FTransform& LerpedTarget =
		UKismetMathLibrary::TLerp(UALSMathLibrary::TransfromAdd(MantleTarget, MantleActualStartOffset), ResultLerp,
		                          BlendIn);

	// 步骤4: 更新角色位置和旋转
	OwnerCharacter->SetActorLocationAndTargetRotation(LerpedTarget.GetLocation(), LerpedTarget.GetRotation().Rotator());
}

void UALSMantleComponent::MantleEnd()
{
	// 设置角色移动模式为行走模式
	if (OwnerCharacter)
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		// 如果手上拿了东西就将手上的东西进行还原
		if (OwnerCharacter->IsA(AALSCharacter::StaticClass()))
		{
			Cast<AALSCharacter>(OwnerCharacter)->UpdateHeldObject();
		}
	}

	// 组件启用 tick 
	SetComponentTickEnabledAsync(true);
}

/*
 * 当跳跃键按下的时候，进行跳跃检查， 在攀爬状态执行攀爬跳跃操作
 * 比如说在空中的状态、蹲伏的状态等，按下跳跃键，就会调用到该函数，进行攀爬检测。
 * 先进行 mantle 检测 在进行 ladge 检测
 *
 */
void UALSMantleComponent::OnOwnerJumpInput()
{
	bCanExit = false;
	if (OwnerCharacter && OwnerCharacter->GetMovementAction() == EALSMovementAction::None)
	{
		if (OwnerCharacter->GetMovementState() == EALSMovementState::Grounded)
		{
			if (OwnerCharacter->HasMovementInput())
			{
				if (!MantleCheck(GroundedTraceSettings, EDrawDebugTrace::Type::ForDuration))
				{
					LadgeClimbCheck(LadgeTraceSettings, EDrawDebugTrace::Type::ForDuration);
				}
			}
		}
		else if (OwnerCharacter->GetMovementState() == EALSMovementState::InAir)
		{
			if (!MantleCheck(FallingTraceSettings, EDrawDebugTrace::Type::ForDuration))
			{
				LadgeClimbCheck(LadgeTraceSettings, EDrawDebugTrace::Type::ForDuration);
			}
		}
	}

	/*
	 * 如果是攀爬状态，开始执行攀爬跳跃的一些操作。
	 */
	if (OwnerCharacter->GetMovementState() == EALSMovementState::Climbing)
	{
		bCanExit = true;
		bHasBlock = false;
		LastJumpInputTime = GetWorld()->GetTimeSeconds();
		bJumpPressed = true;

		/* 跳跃方向以起始按下的值为基准。 */
		JumpUpValue = OwnerCharacter->ForwardInputValue;
		JumpRightValue = OwnerCharacter->RightInputValue;
	}
}

/**
 * @brief 跳跃按键松开之后，执行登陆或者跳跃攀爬动画。
 */
void UALSMantleComponent::OnOwnerJumpRelease()
{
	check(OwnerCharacter);

	/*UE_LOG(LogTemp, Warning, TEXT("sdfsdf"));
	UE_LOG(LogLinker, Warning, TEXT("sdfsdf"));*/
	if (OwnerCharacter->GetMovementState() == EALSMovementState::Climbing)
	{
		bJumpPressed = false;

		/* 判断是否可以退出 */
		if (bCanExit)
		{
			/*如果跳跃方向是向上，但是没有检测到物体，就执行攀爬，否则就执行退出攀爬动画。*/
			if (JumpUpValue > 0.1f)
			{
				MantleCheck(FallingTraceSettings, EDrawDebugTrace::Type::ForOneFrame);
			}

			else
			{
				ExitClimbing();
			}
			return;
		}

		/* 判断是否可以跳跃 */
		if (bCanJumping)
		{
			if (!IsValid(JumpTargetInfo.Component))
			{
				return;
			}

			bCanJumping = false;

			// 步骤一 ： 更新动画播放相关数据
			const float Angle = FMath::Atan2(JumpUpValue, JumpRightValue);
			const float RightValue = FMath::GetMappedRangeValueClamped(
				{-JumpParams.JumpLengthMax, JumpParams.JumpLengthMax}, {-1.f, 1.f},
				JumpLength * FMath::Cos(Angle));
			const float UpValue = FMath::GetMappedRangeValueClamped(
				{-JumpParams.JumpLengthMax, JumpParams.JumpLengthMax}, {-1.f, 1.f},
				JumpLength * FMath::Sin(Angle));


			// 步骤二 ： 计算运动曲线相关数据
			JumpDistanceDiff = OwnerCharacter->GetActorRotation().UnrotateVector(OwnerCharacter->GetActorLocation())
				- OwnerCharacter->GetActorRotation().UnrotateVector(
					UALSMathLibrary::ALSComponentLocalToWorld(JumpTargetInfo).GetLocation());

			LedgeClimbLS = JumpTargetInfo;

			// 因为有曲线融合，所以刚开始的时候不进行移动。
			bCanJumpMove = false;
			if (OwnerCharacter->GetMainAnimInstance())
			{
				OwnerCharacter->GetMainAnimInstance()->JumpInClimbL_R = RightValue;
				OwnerCharacter->GetMainAnimInstance()->JumpInClimbU_D = UpValue;
			}
			OwnerCharacter->SetMovementAction(EALSMovementAction::ClimbJumping);
		}
	}
}

void UALSMantleComponent::OnOwnerRagdollStateChanged(bool bRagdollState)
{
	/* 如果拥有者进入了洋娃娃状态，就停止攀爬 */
	if (bRagdollState)
	{
		MantleTimeline->Stop();
	}
}
