// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Doğa Can Yanıkoğlu
// Contributors:    Haziq Fadhil, Jens Bjarne Myhre


#include "Character/Animation/ALSCharacterAnimInstance.h"
#include "Character/ALSBaseCharacter.h"
#include "Library/ALSMathLibrary.h"
#include "Components/ALSDebugComponent.h"

#include "Curves/CurveVector.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


static const FName NAME_BasePose_CLF(TEXT("BasePose_CLF"));
static const FName NAME_BasePose_N(TEXT("BasePose_N"));
static const FName NAME_Enable_FootIK_R(TEXT("Enable_FootIK_R"));
static const FName NAME_Enable_FootIK_L(TEXT("Enable_FootIK_L"));
static const FName NAME_Enable_HandIK_L(TEXT("Enable_HandIK_L"));
static const FName NAME_Enable_HandIK_R(TEXT("Enable_HandIK_R"));
static const FName NAME_Enable_Climbing_HandIK_L(TEXT("Enable_Climbing_HandIK_L"));
static const FName NAME_Enable_Climbing_HandIK_R(TEXT("Enable_Climbing_HandIK_R"));
static const FName NAME_Enable_Transition(TEXT("Enable_Transition"));
static const FName NAME_FootLock_L(TEXT("FootLock_L"));
static const FName NAME_FootLock_R(TEXT("FootLock_R"));
static const FName NAME_Enable_HandLock_L(TEXT("Enable_HandLock_L"));
static const FName NAME_Enable_HandLock_R(TEXT("Enable_HandLock_R"));
static const FName NAME_ik_hand_l(TEXT("ik_hand_l"));
static const FName NAME_ik_hand_r(TEXT("ik_hand_r"));
static const FName NAME_Grounded___Slot(TEXT("Grounded Slot"));
static const FName NAME_Layering_Arm_L(TEXT("Layering_Arm_L"));
static const FName NAME_Layering_Arm_L_Add(TEXT("Layering_Arm_L_Add"));
static const FName NAME_Layering_Arm_L_LS(TEXT("Layering_Arm_L_LS"));
static const FName NAME_Layering_Arm_R(TEXT("Layering_Arm_R"));
static const FName NAME_Layering_Arm_R_Add(TEXT("Layering_Arm_R_Add"));
static const FName NAME_Layering_Arm_R_LS(TEXT("Layering_Arm_R_LS"));
static const FName NAME_Layering_Hand_L(TEXT("Layering_Hand_L"));
static const FName NAME_Layering_Hand_R(TEXT("Layering_Hand_R"));
static const FName NAME_Weight_InClimbing(TEXT("Weight_InClimbing"));
static const FName NAME_ball_l(TEXT("ball_l"));
static const FName NAME_ball_r(TEXT("ball_r"));
static const FName NAME_Layering_Head_Add(TEXT("Layering_Head_Add"));
static const FName NAME_Layering_Spine_Add(TEXT("Layering_Spine_Add"));
static const FName NAME_Mask_AimOffset(TEXT("Mask_AimOffset"));
static const FName NAME_Mask_LandPrediction(TEXT("Mask_LandPrediction"));
static const FName NAME__ALSCharacterAnimInstance__RotationAmount(TEXT("RotationAmount"));
static const FName NAME_VB___foot_target_l(TEXT("VB foot_target_l"));
static const FName NAME_VB___foot_target_r(TEXT("VB foot_target_r"));
static const FName NAME_VB___ik_foot_l_Offset(TEXT("VB ik_foot_l_Offset"));
static const FName NAME_VB___ik_foot_r_Offset(TEXT("VB ik_foot_r_Offset"));
static const FName NAME_W_Gait(TEXT("W_Gait"));
static const FName NAME__ALSCharacterAnimInstance__root(TEXT("root"));

ECollisionChannel UALSCharacterAnimInstance::ClimbCollisionChannel = ECC_GameTraceChannel1;

void UALSCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<AALSBaseCharacter>(TryGetPawnOwner());
}

void UALSCharacterAnimInstance::NativeBeginPlay()
{
	// 当调用NativeInitializeAnimation()发生时，似乎玩家pawn组件没有真正初始化。
	// 这就是为什么在这里尝试获取ALS调试组件的原因。
	if (APawn* Owner = TryGetPawnOwner())
	{
		ALSDebugComponent = Owner->FindComponentByClass<UALSDebugComponent>();
	}
}

void UALSCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (!Character)
	{
		RotationMode = EALSRotationMode::VelocityDirection;

		// 没有获取到角色就不能进行更新
		return;
	}

	if (DeltaSeconds == 0.0f)
	{
		// 防止第一个帧的更新 （排除除以 0 的可能）
		return;
	}

	// 更新角色信息。
	CharacterInformation.Velocity = Character->GetCharacterMovement()->Velocity;
	CharacterInformation.MovementInput = Character->GetMovementInput();
	CharacterInformation.AimingRotation = Character->GetAimingRotation();
	CharacterInformation.CharacterActorRotation = Character->GetActorRotation();

	UpdateAimingValues(DeltaSeconds);
	UpdateLayerValues();
	UpdateFootIK(DeltaSeconds);

	if (MovementState.Grounded())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Grounded"));
		// 检查是否移动，如果IsMoving和HasMovementInput，或者如果速度大于150，则启用移动动画。
		const bool bPrevShouldMove = Grounded.bShouldMove;
		Grounded.bShouldMove = ShouldMoveCheck();

		// 刚开始移动
		if (bPrevShouldMove == false && Grounded.bShouldMove)
		{
			// 初始化操作，将一些变量归零, 防止刚刚开始运动就开启旋转。
			TurnInPlaceValues.ElapsedDelayTime = 0.0f;
			Grounded.bRotateL = false;
			Grounded.bRotateR = false;
		}

		if (Grounded.bShouldMove)
		{
			// 正在移动状态
			UpdateMovementValues(DeltaSeconds);
			UpdateRotationValues();
		} // 移动状态的更新

		else
		{
			// 一直没有移动
			if (CanRotateInPlace())
			{
				/*
				 * 可以原地旋转就进行旋转并计算原地旋转动画播放速率,
				 * 只要是进入了这个状态，bRotateL / bRotateR 至少一个会有值，通过这个值可以确定是否继续循环播放动画，
				 * 原地旋转通过 (N) Rotate Left / Right 90 状态机传递 Grounded.RotateRate 给 Rotation Amount 曲线。
				 */
				RotateInPlaceCheck();
			}
			else
			{
				// 如果不能原地旋转就将置零
				Grounded.bRotateL = false;
				Grounded.bRotateR = false;
			}

			// 判断是否可以原地转身
			// 原地转身通过 (N) Not Moving 状态机传递 Grounded.RotationScale 给 Rotation Amount 曲线。
			if (CanTurnInPlace())
			{
				TurnInPlaceCheck(DeltaSeconds);
			}
			else
			{
				// 不可以就直接值为零
				TurnInPlaceValues.ElapsedDelayTime = 0.0f;
			}

			if (CanDynamicTransition())
			{
				DynamicTransitionCheck();
			}
		} // 没有移动状态的更新
	} // 在地面状态下的更新
	else if (MovementState.InAir())
	{
		// 做在空中状态的更新
		UpdateInAirValues(DeltaSeconds);
	}
	else if (MovementState.Ragdoll())
	{
		// 在洋娃娃状态下的更新
		UpdateRagdollValues();
	}
	else if (MovementState.Climbing())
	{
		UpdateClimbValues();
	}
}

void UALSCharacterAnimInstance::PlayTransition(const FALSDynamicMontageParams& Parameters)
{
	PlaySlotAnimationAsDynamicMontage(Parameters.Animation, NAME_Grounded___Slot,
	                                  Parameters.BlendInTime, Parameters.BlendOutTime, Parameters.PlayRate, 1,
	                                  0.0f, Parameters.StartTime);
}

void UALSCharacterAnimInstance::PlayTransitionChecked(const FALSDynamicMontageParams& Parameters)
{
	if (Stance.Standing() && !Grounded.bShouldMove)
	{
		PlayTransition(Parameters);
	}
}

void UALSCharacterAnimInstance::PlayDynamicTransition(float ReTriggerDelay, FALSDynamicMontageParams Parameters)
{
	if (bCanPlayDynamicTransition)
	{
		bCanPlayDynamicTransition = false;

		// Play Dynamic Additive Transition Animation
		PlayTransition(Parameters);

		UWorld* World = GetWorld();
		check(World);
		World->GetTimerManager().SetTimer(PlayDynamicTransitionTimer, this,
		                                  &UALSCharacterAnimInstance::PlayDynamicTransitionDelay,
		                                  ReTriggerDelay, false);
	}
}

/*
 * 判断是否可以移动
 */
bool UALSCharacterAnimInstance::ShouldMoveCheck() const
{
	return CharacterInformation.bIsMoving && CharacterInformation.bHasMovementInput ||
		CharacterInformation.Speed > 150.0f;
}


/*
 * 判断是否可以原地旋转
 * 在瞄准或者第一人称模式返回 true
 */
bool UALSCharacterAnimInstance::CanRotateInPlace() const
{
	return RotationMode.Aiming() ||
		CharacterInformation.ViewMode == EALSViewMode::FirstPerson;
}

/* 只有当角色以第三人称视角看向摄像机，且“启用过渡”曲线具有完全权重时，才执行原地转弯检查。
 * Enable_Transition曲线在AnimBP的某些状态下被修改，
 * 所以角色只能在那些状态下转弯。
 */
bool UALSCharacterAnimInstance::CanTurnInPlace() const
{
	return RotationMode.LookingDirection() &&
		CharacterInformation.ViewMode == EALSViewMode::ThirdPerson &&
		GetCurveValue(NAME_Enable_Transition) >= 0.99f;
}

/*
 * 判断是否可以动态的过渡
 */
bool UALSCharacterAnimInstance::CanDynamicTransition() const
{
	return GetCurveValue(NAME_Enable_Transition) >= 0.99f;
}

void UALSCharacterAnimInstance::StartCornerClimb(bool bIsRight, float RotateAngle, float PlayRateScale, float StartTime,
                                                 bool OverrideCurrent)
{
	// 选择对应的旋转资源
	FALSTurnInClimbAsset TargetRotateAsset;

	if (ClimbingType == EALSClimbingType::Hanging)
	{
		if (bIsRight)
		{
			TargetRotateAsset = RotateInClimbValues.H_Rotate_90_R;
		}
		else
		{
			TargetRotateAsset = RotateInClimbValues.H_Rotate_90_L;
		}
	}
	else
	{
		if (bIsRight)
		{
			TargetRotateAsset = RotateInClimbValues.FH_Rotate_90_R;
		}
		else
		{
			TargetRotateAsset = RotateInClimbValues.FH_Rotate_90_L;
		}
	}

	// 如果没有在播放当前蒙太奇或者在有动画覆盖的状态下，就播放蒙太奇动画
	if (!OverrideCurrent && IsPlayingSlotAnimation(TargetRotateAsset.Animation, TargetRotateAsset.SlotName))
	{
		return;
	}

	// UE_LOG(LogTemp, Warning, TEXT("Climb Corner Animation Start !"));

	const float PlayRate = TargetRotateAsset.PlayRate * PlayRateScale;
	Character->SetAnimClimbCornerParam(TargetRotateAsset.GetTimeLength(), StartTime, PlayRate);

	PlaySlotAnimationAsDynamicMontage(TargetRotateAsset.Animation, TargetRotateAsset.SlotName, 0.2f, 0.2f,
	                                  PlayRate, 1, 0.0f, StartTime);

	// 获得旋转比例，在角色类中进行控制旋转。
	// ClimbingValues.RotationScale = RotateAngle / TargetRotateAsset.AnimatedAngle * TargetRotateAsset.PlayRate * PlayRateScale;
	// UE_LOG(LogTemp, Warning, TEXT("Rotation Scale : %f"), RotateAngle );
}

void UALSCharacterAnimInstance::PlayDynamicTransitionDelay()
{
	bCanPlayDynamicTransition = true;
}

void UALSCharacterAnimInstance::OnJumpedDelay()
{
	InAir.bJumped = false;
}

/*
 * 关闭 Pivot 状态
 */
void UALSCharacterAnimInstance::OnPivotDelay()
{
	Grounded.bPivot = false;
}

/*
 * 更新瞄准相关值
 */
void UALSCharacterAnimInstance::UpdateAimingValues(float DeltaSeconds)
{
	// Interp的瞄准旋转值，以实现平滑的瞄准旋转变化。
	//在计算角度之前插值旋转，确保数值不受actor旋转变化的影响，允许慢瞄准旋转变化与快速actor旋转变化。
	AimingValues.SmoothedAimingRotation = FMath::RInterpTo(AimingValues.SmoothedAimingRotation,
	                                                       CharacterInformation.AimingRotation, DeltaSeconds,
	                                                       Config.SmoothedAimingRotationInterpSpeed);

	//计算目标旋转值和角色旋转值之间的差值
	FRotator Delta = CharacterInformation.AimingRotation - CharacterInformation.CharacterActorRotation;
	Delta.Normalize();
	AimingValues.AimingAngle.X = Delta.Yaw;
	AimingValues.AimingAngle.Y = Delta.Pitch;

	// 计算光滑瞄准旋转值和绝当前旋转值之间的差值
	Delta = AimingValues.SmoothedAimingRotation - CharacterInformation.CharacterActorRotation;
	Delta.Normalize();
	SmoothedAimingAngle.X = Delta.Yaw;
	SmoothedAimingAngle.Y = Delta.Pitch;

	// 如果旋转模式不是速度旋转模式
	if (!RotationMode.VelocityDirection())
	{
		// 将向上向下的差值角度映射到 0 - 1
		AimingValues.AimSweepTime = FMath::GetMappedRangeValueClamped({-90.0f, 90.0f}, {1.0f, 0.0f},
		                                                              AimingValues.AimingAngle.Y);

		//使用瞄准 Yaw 角除以 脊柱数+骨盆骨，以获得脊柱旋转所需的数量，以保持和相机同一方向。
		AimingValues.SpineRotation.Roll = 0.0f;
		AimingValues.SpineRotation.Pitch = 0.0f;
		AimingValues.SpineRotation.Yaw = AimingValues.AimingAngle.X / 4.0f;
	}
	// 下面就是速度旋转模式
	else if (CharacterInformation.bHasMovementInput)
	{
		/*
		 * 如果有运动输入的话
		 * 获得运动输入旋转值和角色旋转值之间的差值，
		 * 并将水平旋转值映射到 0 - 1，
		 * 这个值的目的是让角色跟随着运动输入的方向旋转
		 */
		Delta = CharacterInformation.MovementInput.ToOrientationRotator() - CharacterInformation.CharacterActorRotation;
		Delta.Normalize();
		const float InterpTarget = FMath::GetMappedRangeValueClamped({-180.0f, 180.0f}, {0.0f, 1.0f}, Delta.Yaw);

		AimingValues.InputYawOffsetTime = FMath::FInterpTo(AimingValues.InputYawOffsetTime, InterpTarget,
		                                                   DeltaSeconds, Config.InputYawOffsetInterpSpeed);
	}

	//将瞄准偏航角分割成3个独立的偏航时间。这3个值用于目标偏移行为，以改善完全围绕角色旋转时目标偏移的混合。
	//这允许你保持瞄准响应，但仍然平稳地从左到右或从右到左混合。

	/*
	 * 将角色水平旋转时间分为三段，
	 * todo 为什么要将角色目标旋转分为三段
	 */
	AimingValues.LeftYawTime = FMath::GetMappedRangeValueClamped({0.0f, 180.0f}, {0.5f, 0.0f},
	                                                             FMath::Abs(SmoothedAimingAngle.X));
	AimingValues.RightYawTime = FMath::GetMappedRangeValueClamped({0.0f, 180.0f}, {0.5f, 1.0f},
	                                                              FMath::Abs(SmoothedAimingAngle.X));
	AimingValues.ForwardYawTime = FMath::GetMappedRangeValueClamped({-180.0f, 180.0f}, {0.0f, 1.0f},
	                                                                SmoothedAimingAngle.X);
}

/*
 * 更新动画层级相关变量
 * 实时获取曲线的值
 */
void UALSCharacterAnimInstance::UpdateLayerValues()
{
	// 通过获得与Aim Offset掩模相反的Aim Offset权重。
	LayerBlendingValues.EnableAimOffset = FMath::Lerp(1.0f, 0.0f, GetCurveValue(NAME_Mask_AimOffset));

	// 设置基础姿势的权重
	LayerBlendingValues.BasePose_N = GetCurveValue(NAME_BasePose_N);
	LayerBlendingValues.BasePose_CLF = GetCurveValue(NAME_BasePose_CLF);

	// 设置每个身体部位的加量重量
	LayerBlendingValues.Spine_Add = GetCurveValue(NAME_Layering_Spine_Add);
	LayerBlendingValues.Head_Add = GetCurveValue(NAME_Layering_Head_Add);
	LayerBlendingValues.Arm_L_Add = GetCurveValue(NAME_Layering_Arm_L_Add);
	LayerBlendingValues.Arm_R_Add = GetCurveValue(NAME_Layering_Arm_R_Add);

	// 设置手动覆盖权重
	LayerBlendingValues.Hand_R = GetCurveValue(NAME_Layering_Hand_R);
	LayerBlendingValues.Hand_L = GetCurveValue(NAME_Layering_Hand_L);

	// 混合并设置手部IK权重，以确保只有在手臂图层允许的情况下，它们才被加权。
	LayerBlendingValues.EnableHandIK_L = FMath::Lerp(0.0f, GetCurveValue(NAME_Enable_HandIK_L),
	                                                 GetCurveValue(NAME_Layering_Arm_L));
	LayerBlendingValues.EnableHandIK_R = FMath::Lerp(0.0f, GetCurveValue(NAME_Enable_HandIK_R),
	                                                 GetCurveValue(NAME_Layering_Arm_R));

	//设置手臂是否应该混合在网格空间或局部空间。
	//网格空间的权重总是1，除非局部空间(LS)曲线是完全加权的。
	LayerBlendingValues.Arm_L_LS = GetCurveValue(NAME_Layering_Arm_L_LS);
	LayerBlendingValues.Arm_L_MS = static_cast<float>(1 - FMath::FloorToInt(LayerBlendingValues.Arm_L_LS));
	LayerBlendingValues.Arm_R_LS = GetCurveValue(NAME_Layering_Arm_R_LS);
	LayerBlendingValues.Arm_R_MS = static_cast<float>(1 - FMath::FloorToInt(LayerBlendingValues.Arm_R_LS));

	if (GetCurveValue(NAME_Weight_InClimbing) >= 0.99f)
	{
		LayerBlendingValues.Mask_Secondary_Motion = 0.f;
	}
	else
	{
		LayerBlendingValues.Mask_Secondary_Motion = 0.75f;
	}
}

/*
 * 更新脚部IK值
 */
void UALSCharacterAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	// 左右脚插值的目标位置
	FVector FootOffsetLTarget = FVector::ZeroVector;
	FVector FootOffsetRTarget = FVector::ZeroVector;

	// 更新脚部锁脚值
	SetFootLocking(DeltaSeconds, NAME_Enable_FootIK_L, NAME_FootLock_L,
	               IkFootL_BoneName, FootIKValues.FootLock_L_Alpha, FootIKValues.UseFootLockCurve_L,
	               FootIKValues.FootLock_L_Location, FootIKValues.FootLock_L_Rotation);
	SetFootLocking(DeltaSeconds, NAME_Enable_FootIK_R, NAME_FootLock_R,
	               IkFootR_BoneName, FootIKValues.FootLock_R_Alpha, FootIKValues.UseFootLockCurve_R,
	               FootIKValues.FootLock_R_Location, FootIKValues.FootLock_R_Rotation);

	if (MovementState.InAir())
	{
		// 如果角色在空中就将IK重置
		SetPelvisIKOffset(DeltaSeconds, FVector::ZeroVector, FVector::ZeroVector);
		ResetIKOffsets(DeltaSeconds);
	}


	// 在攀爬状态并且不在跳跃的时候才更新攀爬类型
	else if (MovementState.Climbing() && !MovementAction.ClimbJumping() && !MovementAction.CornerClimbing())
	{
		float FixBlendValue = 0.f;
		if (SetClimbFootIK(IkFootL_BoneName, NAME_Enable_FootIK_L, false, FootIKValues.FootOffset_L_Location) &&
			SetClimbFootIK(IkFootR_BoneName, NAME_Enable_FootIK_R, true, FootIKValues.FootOffset_R_Location))
		{
			Character->SetClimbingType(EALSClimbingType::Hanging);
		}
		else
		{
			if (!MovementAction.CornerClimbing())
			{
				FVector FixBoneValue;
				FixClimbingBody(FixBoneValue, FixBlendValue, 40.f);
				ClimbFixBoneOffset = FMath::VInterpTo(ClimbFixBoneOffset, FixBoneValue, DeltaSeconds, 3.f);
			}
			Character->SetClimbingType(EALSClimbingType::FreeHanging);
		}
		
		ClimbingValues.ClimbFixBlendValue = FMath::FInterpTo(ClimbingValues.ClimbFixBlendValue, FixBlendValue, DeltaSeconds, 5.f);
	}

	else if (!MovementState.Ragdoll())
	{
		// 当不在空中并且不是洋娃娃状态的时候，更新所有脚锁定和脚偏移值
		SetFootOffsets(DeltaSeconds, NAME_Enable_FootIK_L, IkFootL_BoneName, NAME__ALSCharacterAnimInstance__root,
		               FootOffsetLTarget,
		               FootIKValues.FootOffset_L_Location, FootIKValues.FootOffset_L_Rotation);
		SetFootOffsets(DeltaSeconds, NAME_Enable_FootIK_R, IkFootR_BoneName, NAME__ALSCharacterAnimInstance__root,
		               FootOffsetRTarget,
		               FootIKValues.FootOffset_R_Location, FootIKValues.FootOffset_R_Rotation);
		SetPelvisIKOffset(DeltaSeconds, FootOffsetLTarget, FootOffsetRTarget);
	}
}

/**
 * @brief 设置脚部锁定相关曲线
 * @param DeltaSeconds 经过的时间差
 * @param EnableFootIKCurve 是否启用了脚部IK曲线
 * @param FootLockCurve 脚部锁定曲线名字
 * @param IKFootBone 锁定IK脚的骨骼名字
 * @param CurFootLockAlpha 当前脚部锁定曲线值
 * @param UseFootLockCurve 是否使用脚部锁定曲线
 * @param CurFootLockLoc 当前脚部锁定剩余的向量值
 * @param CurFootLockRot 当前脚部锁定剩余的旋转值
 */
void UALSCharacterAnimInstance::SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve,
                                               FName IKFootBone, float& CurFootLockAlpha, bool& UseFootLockCurve,
                                               FVector& CurFootLockLoc, FRotator& CurFootLockRot) const
{
	// 判断是否启用了IK曲线
	if (GetCurveValue(EnableFootIKCurve) <= 0.0f)
	{
		return;
	}

	// 步骤1:设置“本地FootLock曲线”值
	float FootLockCurveVal;

	if (UseFootLockCurve)
	{
		// 在没有旋转或者不是服务器代理的时候才使用 锁脚曲线
		UseFootLockCurve = FMath::Abs(GetCurveValue(NAME__ALSCharacterAnimInstance__RotationAmount)) <= 0.001f ||
			Character->GetLocalRole() != ROLE_AutonomousProxy;
		FootLockCurveVal = GetCurveValue(FootLockCurve) * (1.f / GetSkelMeshComponent()->AnimUpdateRateParams->
			UpdateRate);
	}
	else
	{
		// 如果之前没有使用锁脚曲线， 但是当前锁脚曲线 等于1， 就启用锁脚曲线。
		UseFootLockCurve = GetCurveValue(FootLockCurve) >= 0.99f;
		FootLockCurveVal = 0.0f;
	}

	// 步骤2:只有当新值小于当前值或等于1时，才更新FootLock Alpha。
	// 这使得它使脚只能混合出锁定位置或锁定到一个新的位置，而永远不会混合。 持续属于脚部锁定状态。
	if (FootLockCurveVal >= 0.99f || FootLockCurveVal < CurFootLockAlpha)
	{
		CurFootLockAlpha = FootLockCurveVal;
	}

	// 步骤3:如果脚锁曲线等于1，则将新的锁位置和组件空间内的旋转保存为目标。
	if (CurFootLockAlpha >= 0.99f)
	{
		const FTransform& OwnerTransform =
			GetOwningComponent()->GetSocketTransform(IKFootBone, RTS_Component);
		CurFootLockLoc = OwnerTransform.GetLocation();
		CurFootLockRot = OwnerTransform.Rotator();
	}

	//步骤4:如果脚锁曲线还有值，那就更新脚锁偏移量，以保持脚固定在位置，而胶囊移动。
	if (CurFootLockAlpha > 0.0f)
	{
		SetFootLockOffsets(DeltaSeconds, CurFootLockLoc, CurFootLockRot);
	}
}

/**
 * @brief 更新当前脚部锁定变换的偏差值
 */
void UALSCharacterAnimInstance::SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot) const
{
	FRotator RotationDifference = FRotator::ZeroRotator;
	// 使用当前和最后一次更新的旋转之间的delta，来找出脚应该旋转多少才能保持在地面上。
	if (Character->GetCharacterMovement()->IsMovingOnGround())
	{
		RotationDifference = CharacterInformation.CharacterActorRotation - Character->GetCharacterMovement()->
			GetLastUpdateRotation();
		RotationDifference.Normalize();
	}

	//获得相对于网格旋转帧之间的距离（速度 * 时间 = 位移），以找到应该偏移多少才能保持脚锁在地面上。
	const FVector& LocationDifference = GetOwningComponent()->GetComponentRotation().UnrotateVector(
		CharacterInformation.Velocity * DeltaSeconds);

	//从当前局部位置减去位置差，并旋转该位置差，以保持脚处于组件空间中。
	LocalLoc = (LocalLoc - LocationDifference).RotateAngleAxis(RotationDifference.Yaw, FVector::DownVector);

	// 从当前的局部旋转减去旋转差，得到新的局部旋转。
	FRotator Delta = LocalRot - RotationDifference;
	Delta.Normalize();
	LocalRot = Delta;
}

/**
 * @brief 设置盆骨偏差值
 * 先将角色下降到对应位置，然后再让角色的脚慢慢平移
 */
void UALSCharacterAnimInstance::SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget,
                                                  FVector FootOffsetRTarget)
{
	// 通过计算平均脚部IK曲线值来计算骨盆Alpha值。如果alpha值为0，清除偏移量。
	FootIKValues.PelvisAlpha =
		(GetCurveValue(NAME_Enable_FootIK_L) + GetCurveValue(NAME_Enable_FootIK_R)) / 2.0f;

	if (FootIKValues.PelvisAlpha > 0.0f)
	{
		// 步骤1: 设置新的盆骨目标为最低的脚偏移
		const FVector PelvisTarget = FootOffsetLTarget.Z < FootOffsetRTarget.Z ? FootOffsetLTarget : FootOffsetRTarget;

		// 步骤2: 当前骨盆偏移量插值到新的目标值。
		// 根据新目标是高于还是低于当前目标，以不同的速度进行插值。
		const float InterpSpeed = PelvisTarget.Z > FootIKValues.PelvisOffset.Z ? 10.0f : 15.0f;
		FootIKValues.PelvisOffset =
			FMath::VInterpTo(FootIKValues.PelvisOffset, PelvisTarget, DeltaSeconds, InterpSpeed);
	}
	else
	{
		FootIKValues.PelvisOffset = FVector::ZeroVector;
	}
}

/**
 * @brief 重置IK偏差值
 */
void UALSCharacterAnimInstance::ResetIKOffsets(float DeltaSeconds)
{
	// 将脚部IK慢慢的差值到0
	FootIKValues.FootOffset_L_Location = FMath::VInterpTo(FootIKValues.FootOffset_L_Location,
	                                                      FVector::ZeroVector, DeltaSeconds, 15.0f);
	FootIKValues.FootOffset_R_Location = FMath::VInterpTo(FootIKValues.FootOffset_R_Location,
	                                                      FVector::ZeroVector, DeltaSeconds, 15.0f);
	FootIKValues.FootOffset_L_Rotation = FMath::RInterpTo(FootIKValues.FootOffset_L_Rotation,
	                                                      FRotator::ZeroRotator, DeltaSeconds, 15.0f);
	FootIKValues.FootOffset_R_Rotation = FMath::RInterpTo(FootIKValues.FootOffset_R_Rotation,
	                                                      FRotator::ZeroRotator, DeltaSeconds, 15.0f);
}

/**
 * @brief 
 * @param DeltaSeconds 相差时间
 * @param EnableFootIKCurve 启用的脚部IK曲线的名称
 * @param IKFootBone 脚部IK骨骼名称
 * @param RootBone 根骨骼的名称
 * @param CurLocationTarget 目前目标位置
 * @param CurLocationOffset 目前位置偏差值
 * @param CurRotationOffset 目前旋转偏差值
 */
void UALSCharacterAnimInstance::SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone,
                                               FName RootBone, FVector& CurLocationTarget, FVector& CurLocationOffset,
                                               FRotator& CurRotationOffset) const
{
	// 只有当Foot IK曲线有一个权值时，才更新Foot IK偏移值。如果它等于0，清除偏移值。
	if (GetCurveValue(EnableFootIKCurve) <= 0)
	{
		CurLocationOffset = FVector::ZeroVector;
		CurRotationOffset = FRotator::ZeroRotator;
		return;
	}

	//步骤1: 从脚的位置向下跟踪，找到几何体。如果表面是可行走的，保存受击点和法线。
	USkeletalMeshComponent* OwnerComp = GetOwningComponent();
	FVector IKFootFloorLoc = OwnerComp->GetSocketLocation(IKFootBone);
	IKFootFloorLoc.Z = OwnerComp->GetSocketLocation(RootBone).Z;

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	const FVector TraceStart = IKFootFloorLoc + FVector(0.0, 0.0, Config.IK_TraceDistanceAboveFoot);
	const FVector TraceEnd = IKFootFloorLoc - FVector(0.0, 0.0, Config.IK_TraceDistanceBelowFoot);

	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(HitResult,
	                                                  TraceStart,
	                                                  TraceEnd,
	                                                  ECC_Visibility, Params);

	if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
	{
		UALSDebugComponent::DrawDebugLineTraceSingle(
			World,
			TraceStart,
			TraceEnd,
			EDrawDebugTrace::Type::ForOneFrame,
			bHit,
			HitResult,
			FLinearColor::Red,
			FLinearColor::Green,
			5.0f);
	}

	FRotator TargetRotOffset = FRotator::ZeroRotator;
	if (Character->GetCharacterMovement()->IsWalkable(HitResult))
	{
		FVector ImpactPoint = HitResult.ImpactPoint;
		FVector ImpactNormal = HitResult.ImpactNormal;

		//步骤1.1: 找出撞击点和预期的(平坦的)地板位置的差异。
		//这些值被法线乘以脚的高度抵消，以在有角度的表面上获得更好的表现。（在面对有角度的坡的时候会目标位置会向前移动一点）
		CurLocationTarget = ImpactPoint + ImpactNormal * Config.FootHeight -
			(IKFootFloorLoc + FVector(0, 0, Config.FootHeight));

		// 步骤1.2:计算旋转偏移，通过获取ImpactNormal的Atan2。
		TargetRotOffset.Pitch = -FMath::RadiansToDegrees(FMath::Atan2(ImpactNormal.X, ImpactNormal.Z));
		TargetRotOffset.Roll = FMath::RadiansToDegrees(FMath::Atan2(ImpactNormal.Y, ImpactNormal.Z));
	}

	// 步骤2: 对当前位置进行插值偏移到新的目标值。
	// 根据新目标是高于还是低于当前目标，以不同的速度进行插值。
	// 当前位置高就插值大一些，低就慢一些
	const float InterpSpeed = CurLocationOffset.Z > CurLocationTarget.Z ? 30.f : 15.0f;
	CurLocationOffset = FMath::VInterpTo(CurLocationOffset, CurLocationTarget, DeltaSeconds, InterpSpeed);

	//步骤3: 当前旋转偏移量插值到新的目标值。
	CurRotationOffset = FMath::RInterpTo(CurRotationOffset, TargetRotOffset, DeltaSeconds, 30.0f);
}

/**
 * @return  0 代表当前在锁手，不更新手部位置， -1 代表没有检测到物体  1： 代表检测到了东西，需要进行更新
 */
int32 UALSCharacterAnimInstance::SetClimbHandIK(FName EnableFootLockCurve, FName HandBone, bool bIsRight,
                                                float& InterpSpeed, FVector& TargetHandLocation,
                                                UPrimitiveComponent* &Component) const
{
	if (GetCurveValue(EnableFootLockCurve) > 0.1f)
	{
		return 0;
	}

	float TraceDistance = TraceDistance = bIsRight ? 13.f : -13.f;

	float InputValue = Character->RightInputValue;
	if (bCanClimbMove)
	{
		if (ClimbingType == EALSClimbingType::FreeHanging)
		{
			TraceDistance = bIsRight ? 5.f : -5.f;
		}

		if (bIsRight && InputValue >= 0.2f || !bIsRight && InputValue <= -0.2f)
		{
			TraceDistance += InputValue * (ClimbingType == EALSClimbingType::Hanging ? 15.f : 25.f);
		}
	}

	FVector TraceStart = Character->GetActorRotation().UnrotateVector(Character->GetActorLocation());
	TraceStart += FVector(0.f, TraceDistance, 40.f);
	TraceStart = Character->GetActorRotation().RotateVector(TraceStart);

	FVector TraceEnd = TraceStart + Character->GetActorForwardVector() * 60.f;

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	// 步骤一 ： 向前进行射线检测，获取x.y轴的值
	FHitResult HitResult;
	{
		const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(
			10.f, 25.f);
		const bool bHit = World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity,
		                                              ClimbCollisionChannel,
		                                              CapsuleCollisionShape, Params);

		if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
		{
			UALSDebugComponent::DrawDebugCapsuleTraceSingle(World,
			                                                TraceStart,
			                                                TraceEnd,
			                                                CapsuleCollisionShape,
			                                                EDrawDebugTrace::Type::ForOneFrame,
			                                                bHit,
			                                                HitResult,
			                                                FLinearColor::Red,
			                                                FLinearColor::Blue,
			                                                10.0f);
		}

		if (!bHit) return -1;
	}


	TargetHandLocation = HitResult.ImpactPoint;
	TargetHandLocation += HitResult.ImpactNormal * 5.f;

	// 步骤二： 从上向下进行射线检测，获取高度
	TraceStart = HitResult.ImpactPoint;
	TraceStart.Z = HitResult.Location.Z;
	TraceEnd = TraceStart;
	TraceEnd.Z -= 15.f;
	TraceStart.Z += 15.f;

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
			                                               EDrawDebugTrace::Type::ForOneFrame,
			                                               bHit,
			                                               HitResult,
			                                               FLinearColor::Red,
			                                               FLinearColor::Blue,
			                                               10.0f);
		}

		if (!bHit) return -1;
	}


	if (HitResult.ImpactNormal.Z <= 0.1f) return -1;

	TargetHandLocation.Z = HitResult.ImpactPoint.Z - 8.f;

	InterpSpeed = FMath::Clamp(
		FMath::Abs(GetOwningComponent()->GetSocketTransform(HandBone).GetLocation().Z - TargetHandLocation.Z) - 2.f,
		5.f,
		12.f);
	if (!bCanClimbMove) InterpSpeed = 20.f;

	Component = HitResult.GetComponent();
	return 1;
}

/**
 * @brief 返回 false 代表没有检测到物体 使用长攀爬， 返回 true 代表检测到了物体，使用短攀爬。
 */
bool UALSCharacterAnimInstance::SetClimbFootIK(FName FootBone, FName EnableFootIKCurve, bool bIsRight,
                                               FVector& FootOffset) const
{
	FHitResult HitResult;
	UWorld* World = GetWorld();
	check(World);

	for (int height = 70.f; height < 101.f; height += 10.f)
	{
		FVector TraceStart = Character->GetActorRotation().UnrotateVector(Character->GetActorLocation());
		TraceStart += FVector(10.f, bIsRight ? 8.f : -8.f, -height);
		TraceStart = Character->GetActorRotation().RotateVector(TraceStart);

		FVector TraceEnd = TraceStart + Character->GetActorForwardVector() * 20.f;


		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Character);

		const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(
			15.f, 10.f);
		const bool bHit = World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity,
		                                              ECC_Visibility,
		                                              CapsuleCollisionShape, Params);

		if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
		{
			UALSDebugComponent::DrawDebugCapsuleTraceSingle(World,
			                                                TraceStart,
			                                                TraceEnd,
			                                                CapsuleCollisionShape,
			                                                EDrawDebugTrace::Type::ForOneFrame,
			                                                bHit,
			                                                HitResult,
			                                                FLinearColor::Red,
			                                                FLinearColor::Black,
			                                                10.0f);
		}
	}

	if (!HitResult.bBlockingHit) return false;

	FootOffset = HitResult.ImpactPoint - Character->GetMesh()->GetSocketLocation(FootBone);
	FootOffset.Z += Config.FootHeight - 5.f;
	FootOffset += HitResult.ImpactNormal * Config.FootLength;

	return true;
}

void UALSCharacterAnimInstance::FixClimbingBody(FVector& FixBoneValue, float& BlendValue, float Distance) const
{
	FHitResult HitResult;
	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	const FVector TraceStart = Character->GetActorLocation() - Character->GetActorUpVector() * 30.f;
	const FVector TraceEnd = TraceStart + Character->GetActorForwardVector() * Distance;


	const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(
		Character->GetCapsuleComponent()->GetScaledCapsuleRadius() * 0.75f,
		Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2.f);
	const bool bHit = World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility,
	                                              CapsuleCollisionShape, Params);

	if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
	{
		UALSDebugComponent::DrawDebugCapsuleTraceSingle(World,
		                                                TraceStart,
		                                                TraceEnd,
		                                                CapsuleCollisionShape,
		                                                EDrawDebugTrace::ForOneFrame,
		                                                bHit,
		                                                HitResult,
		                                                FLinearColor::Black,
		                                                FLinearColor::Green,
		                                                1.0f);
	}

	if (!bHit)
	{
		FixBoneValue = FVector(0.f, -5.f, 0.f);
		return;
	}

	BlendValue = 1.f;
	FixBoneValue = FVector(0.f, (HitResult.Time - 1) * Distance / 2.5f, 0.f);
}

/*
 * 原地旋转检查
 */
void UALSCharacterAnimInstance::RotateInPlaceCheck()
{
	// 步骤1:通过检查瞄准角度是否超过阈值来检查角色是否应该向左或向右旋转。
	Grounded.bRotateL = AimingValues.AimingAngle.X < RotateInPlace.RotateMinThreshold;
	Grounded.bRotateR = AimingValues.AimingAngle.X > RotateInPlace.RotateMaxThreshold;

	// 步骤2:如果角色应该旋转，设置旋转速率与目标偏航速率比例。
	// 这使得角色在移动摄像机更快时旋转更快。
	if (Grounded.bRotateL || Grounded.bRotateR)
	{
		/* 通过摄像机旋转速率映射输出对应的动画旋转播放速率 */
		Grounded.RotateRate = FMath::GetMappedRangeValueClamped(
			{RotateInPlace.AimYawRateMinRange, RotateInPlace.AimYawRateMaxRange},
			{RotateInPlace.MinPlayRate, RotateInPlace.MaxPlayRate},
			CharacterInformation.AimYawRate);
	}
}

/*
 * 原地转身检查
 * 如果检查通过了就执行原地转身操作
 */
void UALSCharacterAnimInstance::TurnInPlaceCheck(float DeltaSeconds)
{
	/* 步骤1:
	 * 检查瞄准角度是否在转弯检查最小角度之外，以及是否瞄准偏航速率低于瞄准偏航速率限制 （摄像机移动太快了就不进行旋转了） 。
	 * 如果是，开始计算经过的延迟时间。
	 * 如果不是，重置延迟时间。
	 * 这确保了在转向到位之前，条件在一段持续的时间内保持真实。
	 */
	if (FMath::Abs(AimingValues.AimingAngle.X) <= TurnInPlaceValues.TurnCheckMinAngle ||
		CharacterInformation.AimYawRate >= TurnInPlaceValues.AimYawRateLimit)
	{
		TurnInPlaceValues.ElapsedDelayTime = 0.0f;
		return;
	}


	/* 步骤2:
	 * 检查“经过延迟时间”是否超过设置的延迟时间(映射到转角范围)。
	 * 如果是，触发原地转弯。
	 */
	TurnInPlaceValues.ElapsedDelayTime += DeltaSeconds;
	const float ClampedAimAngle = FMath::GetMappedRangeValueClamped({TurnInPlaceValues.TurnCheckMinAngle, 180.0f},
	                                                                {
		                                                                TurnInPlaceValues.MinAngleDelay,
		                                                                TurnInPlaceValues.MaxAngleDelay
	                                                                },
	                                                                AimingValues.AimingAngle.X);

	if (TurnInPlaceValues.ElapsedDelayTime > ClampedAimAngle)
	{
		FRotator TurnInPlaceYawRot = CharacterInformation.AimingRotation;
		TurnInPlaceYawRot.Roll = 0.0f;
		TurnInPlaceYawRot.Pitch = 0.0f;
		TurnInPlace(TurnInPlaceYawRot, 1.0f, 0.0f, false);
	}
}


void UALSCharacterAnimInstance::DynamicTransitionCheck()
{
	// Check each foot to see if the location difference between the IK_Foot bone and its desired / target location
	// (determined via a virtual bone) exceeds a threshold. If it does, play an additive transition animation on that foot.
	// The currently set transition plays the second half of a 2 foot transition animation, so that only a single foot moves.
	// Because only the IK_Foot bone can be locked, the separate virtual bone allows the system to know its desired location when locked.
	FTransform SocketTransformA = GetOwningComponent()->GetSocketTransform(IkFootL_BoneName, RTS_Component);
	FTransform SocketTransformB = GetOwningComponent()->GetSocketTransform(
		NAME_VB___foot_target_l, RTS_Component);
	float Distance = (SocketTransformB.GetLocation() - SocketTransformA.GetLocation()).Size();
	if (Distance > Config.DynamicTransitionThreshold)
	{
		FALSDynamicMontageParams Params;
		Params.Animation = TransitionAnim_R;
		Params.BlendInTime = 0.2f;
		Params.BlendOutTime = 0.2f;
		Params.PlayRate = 1.5f;
		Params.StartTime = 0.8f;
		PlayDynamicTransition(0.1f, Params);
	}

	SocketTransformA = GetOwningComponent()->GetSocketTransform(IkFootR_BoneName, RTS_Component);
	SocketTransformB = GetOwningComponent()->GetSocketTransform(NAME_VB___foot_target_r, RTS_Component);
	Distance = (SocketTransformB.GetLocation() - SocketTransformA.GetLocation()).Size();
	if (Distance > Config.DynamicTransitionThreshold)
	{
		FALSDynamicMontageParams Params;
		Params.Animation = TransitionAnim_L;
		Params.BlendInTime = 0.2f;
		Params.BlendOutTime = 0.2f;
		Params.PlayRate = 1.5f;
		Params.StartTime = 0.8f;
		PlayDynamicTransition(0.1f, Params);
	}
}

void UALSCharacterAnimInstance::UpdateMovementValues(float DeltaSeconds)
{
	// 插值并且设置速度混合值
	const FALSVelocityBlend& TargetBlend = CalculateVelocityBlend();
	VelocityBlend.F = FMath::FInterpTo(VelocityBlend.F, TargetBlend.F, DeltaSeconds, Config.VelocityBlendInterpSpeed);
	VelocityBlend.B = FMath::FInterpTo(VelocityBlend.B, TargetBlend.B, DeltaSeconds, Config.VelocityBlendInterpSpeed);
	VelocityBlend.L = FMath::FInterpTo(VelocityBlend.L, TargetBlend.L, DeltaSeconds, Config.VelocityBlendInterpSpeed);
	VelocityBlend.R = FMath::FInterpTo(VelocityBlend.R, TargetBlend.R, DeltaSeconds, Config.VelocityBlendInterpSpeed);

	// todo IK的时候需要设置
	// Set the Diagonal Scale Amount.
	Grounded.DiagonalScaleAmount = CalculateDiagonalScaleAmount();

	// 设置相对加速度和设置身体倾斜值
	RelativeAccelerationAmount = CalculateRelativeAccelerationAmount();
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, RelativeAccelerationAmount.Y, DeltaSeconds,
	                                 Config.GroundedLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, RelativeAccelerationAmount.X, DeltaSeconds,
	                                 Config.GroundedLeanInterpSpeed);

	// 设置角色跑步和行走混合值
	Grounded.WalkRunBlend = CalculateWalkRunBlend();

	// 设置步幅混合值
	Grounded.StrideBlend = CalculateStrideBlend();

	// 设置站立和蹲伏播放速率
	Grounded.StandingPlayRate = CalculateStandingPlayRate();
	Grounded.CrouchingPlayRate = CalculateCrouchingPlayRate();
}

void UALSCharacterAnimInstance::UpdateRotationValues()
{
	// Set the Movement Direction
	MovementDirection = CalculateMovementDirection();

	// Set the Yaw Offsets. 
	/*
	 * 这些值影响动画中的“YawOffset”曲线，并用于偏移角色旋转以实现更自然的运动。
	 * 这些曲线允许对每个移动方向的偏移量进行精细的控制。
	 * 根据角色速度方向和目标旋转方向的之间的差值来确定对应曲线的取值（也就是倾斜角度）。
	 */
	FRotator Delta = CharacterInformation.Velocity.ToOrientationRotator() - CharacterInformation.AimingRotation;
	Delta.Normalize();
	const FVector& FBOffset = YawOffset_FB->GetVectorValue(Delta.Yaw);
	Grounded.FYaw = FBOffset.X;
	Grounded.BYaw = FBOffset.Y;
	const FVector& LROffset = YawOffset_LR->GetVectorValue(Delta.Yaw);
	Grounded.LYaw = LROffset.X;
	Grounded.RYaw = LROffset.Y;
}

/*
 * 更新在空中相关状态
 */
void UALSCharacterAnimInstance::UpdateInAirValues(float DeltaSeconds)
{
	/* 更新下落速度。
	 * 只需要在空中设置这个值，允许你在AnimGraph中使用它作为着陆强度。
	 * 如果不是，Z速度会在着陆时回到0。
	 */
	InAir.FallSpeed = CharacterInformation.Velocity.Z;

	// Set the Land Prediction weight.
	InAir.LandPrediction = CalculateLandPrediction();

	// 插值得到对应倾斜量
	const FALSLeanAmount& InAirLeanAmount = CalculateAirLeanAmount();
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, InAirLeanAmount.LR, DeltaSeconds, Config.InAirLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, InAirLeanAmount.FB, DeltaSeconds, Config.InAirLeanInterpSpeed);
}

/*
 * 更新洋娃娃相关的值
 */
void UALSCharacterAnimInstance::UpdateRagdollValues()
{
	// 按速度值大小缩放乱抓。布娃娃移动得越快，角色就乱抓得越快。
	const float VelocityLength = GetOwningComponent()->GetPhysicsLinearVelocity(NAME__ALSCharacterAnimInstance__root).
	                                                   Size();
	FlailRate = FMath::GetMappedRangeValueClamped({0.0f, 1000.0f}, {0.0f, 1.0f}, VelocityLength);
}

/**
 * @brief 更新攀爬相关的值
 */
void UALSCharacterAnimInstance::UpdateClimbValues()
{
	/*
	 *  设置手部对应位置
	 *  采用记录局部坐标的方式，更新的时候转换成全局变量，应对动态变化物体的情况。
	 */
	float InterpSpeed_L, InterpSpeed_R;
	FVector TargetHandLocation_L, TargetHandLocation_R;
	UPrimitiveComponent *Component_L = nullptr, *Component_R = nullptr;
	const int32 LeftValue = SetClimbHandIK(NAME_Enable_HandLock_L, NAME_ik_hand_l, false, InterpSpeed_L,
	                                       TargetHandLocation_L, Component_L);
	const int32 RightValue = SetClimbHandIK(NAME_Enable_HandLock_R, NAME_ik_hand_r, true, InterpSpeed_R,
	                                        TargetHandLocation_R, Component_R);
	
	if (RightValue >= 0 && LeftValue >= 0)
	{
		if (LeftValue && Component_L)
		{
			HandIK_LS_L.Component = Component_L;
			HandIK_LS_L.Transform = FTransform(GetOwningComponent()->GetSocketTransform(NAME_ik_hand_l).GetRotation(),
			                                   FMath::VInterpTo(HandIKTarget_L, TargetHandLocation_L,
			                                                    GetWorld()->GetDeltaSeconds(),
			                                                    InterpSpeed_L), FVector::OneVector);
			HandIK_LS_L.Transform = UALSMathLibrary::ALSComponentWorldToLocal(HandIK_LS_L);
		}

		if (RightValue && Component_R)
		{
			HandIK_LS_R.Component = Component_R;
			HandIK_LS_R.Transform = FTransform(GetOwningComponent()->GetSocketTransform(NAME_ik_hand_r).GetRotation(),
			                                   FMath::VInterpTo(HandIKTarget_R, TargetHandLocation_R,
			                                                    GetWorld()->GetDeltaSeconds(), InterpSpeed_R),
			                                   FVector::OneVector);
			HandIK_LS_R.Transform = UALSMathLibrary::ALSComponentWorldToLocal(HandIK_LS_R);
		}
	}

	if (HandIK_LS_L.Component)
	{
		HandIKTarget_L = UALSMathLibrary::ALSComponentLocalToWorld(HandIK_LS_L).GetLocation();
	}

	if (HandIK_LS_R.Component)
	{
		HandIKTarget_R = UALSMathLibrary::ALSComponentLocalToWorld(HandIK_LS_R).GetLocation();
	}

}

/*
 * 获得动画曲线约束后的值
 */
float UALSCharacterAnimInstance::GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin,
                                                     float ClampMax) const
{
	return FMath::Clamp(GetCurveValue(Name) + Bias, ClampMin, ClampMax);
}

/*
 * 计算角色在前后左右的速度混合值
 */
FALSVelocityBlend UALSCharacterAnimInstance::CalculateVelocityBlend() const
{
	/*
	 * 计算速度混合。
	 * 这个值表示角色在每个方向上的速度量 (标准化，使对角线在每个方向上等于 .5)，
	 * 并在BlendMulti节点中使用，以产生比标准混合空间更好的方向混合。
	 */
	// 计算局部角色速度方向
	const FVector LocRelativeVelocityDir =
		CharacterInformation.CharacterActorRotation.UnrotateVector(CharacterInformation.Velocity.GetSafeNormal(0.1f));
	// 局部角色速度各方向值的总和
	const float Sum = FMath::Abs(LocRelativeVelocityDir.X) + FMath::Abs(LocRelativeVelocityDir.Y) +
		FMath::Abs(LocRelativeVelocityDir.Z);
	// 计算出一个速度平均值
	const FVector RelativeDir = LocRelativeVelocityDir / Sum;

	FALSVelocityBlend Result;
	Result.F = FMath::Clamp(RelativeDir.X, 0.0f, 1.0f);
	Result.B = FMath::Abs(FMath::Clamp(RelativeDir.X, -1.0f, 0.0f));
	Result.L = FMath::Abs(FMath::Clamp(RelativeDir.Y, -1.0f, 0.0f));
	Result.R = FMath::Clamp(RelativeDir.Y, 0.0f, 1.0f);
	return Result;
}

/*
* 计算相对加速度量。
* 此值表示相对于操作者旋转的当前加速/减速量。
* 它被归一化为 -1 - 1 的范围，所以 -1等于最大制动减速，1等于角色移动组件的最大加速度。
*/
FVector UALSCharacterAnimInstance::CalculateRelativeAccelerationAmount() const
{
	// 加速度和速度处于同一个方向， 说明是加速
	if (FVector::DotProduct(CharacterInformation.Acceleration, CharacterInformation.Velocity) > 0.0f)
	{
		const float MaxAcc = Character->GetCharacterMovement()->GetMaxAcceleration();
		return CharacterInformation.CharacterActorRotation.UnrotateVector(
			CharacterInformation.Acceleration.GetClampedToMaxSize(MaxAcc) / MaxAcc);
	}

	// 减速状态
	const float MaxBrakingDec = Character->GetCharacterMovement()->GetMaxBrakingDeceleration();
	return
		CharacterInformation.CharacterActorRotation.UnrotateVector(
			CharacterInformation.Acceleration.GetClampedToMaxSize(MaxBrakingDec) / MaxBrakingDec);
}

/*
* 计算Stride Blend。
* 这个值在混合空间中被用来缩放步幅(双脚移动的距离)，这样角色就可以以不同的移动速度行走或奔跑。
* 它还允许行走或奔跑的步态动画独立混合，同时仍然匹配动画速度与运动速度，防止角色需要进行半走+半跑混合。
* 这些曲线被用来映射步幅到最大的控制速度。
*/
float UALSCharacterAnimInstance::CalculateStrideBlend() const
{
	const float CurveTime = CharacterInformation.Speed / GetOwningComponent()->GetComponentScale().Z;
	// 区分跑步和走路
	const float ClampedGait = GetAnimCurveClamped(NAME_W_Gait, -1.0, 0.0f, 1.0f);
	const float LerpedStrideBlend =
		FMath::Lerp(StrideBlend_N_Walk->GetFloatValue(CurveTime), StrideBlend_N_Run->GetFloatValue(CurveTime),
		            ClampedGait);
	return FMath::Lerp(LerpedStrideBlend, StrideBlend_C_Walk->GetFloatValue(CharacterInformation.Speed),
	                   GetCurveValue(NAME_BasePose_CLF));
}

/*
 * 计算步行-跑步混合。
 * 此值在Blendspaces中用于在步行和跑步之间进行混合。
 */
float UALSCharacterAnimInstance::CalculateWalkRunBlend() const
{
	return Gait.Walking() ? 0.0f : 1.0;
}

/*
* 计算站立播放速率
* 通过将角色的速度除以每个步态的动画速度来计算游戏速率。
* 插值由存在于每个运动周期的“w_Gait”动画曲线决定，以便播放速率始终与当前的混合动画同步。
* 该值还被 混合步幅 和 网格比例 分割，以便播放速率随着步幅或尺度的减小而增加。
*/
float UALSCharacterAnimInstance::CalculateStandingPlayRate() const
{
	const float LerpedSpeed = FMath::Lerp(CharacterInformation.Speed / Config.AnimatedWalkSpeed,
	                                      CharacterInformation.Speed / Config.AnimatedRunSpeed,
	                                      GetAnimCurveClamped(NAME_W_Gait, -1.0f, 0.0f, 1.0f));

	const float SprintAffectedSpeed = FMath::Lerp(LerpedSpeed, CharacterInformation.Speed / Config.AnimatedSprintSpeed,
	                                              GetAnimCurveClamped(NAME_W_Gait, -2.0f, 0.0f, 1.0f));

	return FMath::Clamp((SprintAffectedSpeed / Grounded.StrideBlend) / GetOwningComponent()->GetComponentScale().Z,
	                    0.0f, 3.0f);
}

/*
 * 计算斜比例尺金额。
 * 此值用于缩放足IK根骨，以使足IK骨在对角线混合物上覆盖更多的距离。
 * 如果没有缩放，由于IK骨骼的线性平移混合，脚将不能在对角线方向上移动足够远。
 * 这条曲线用来很容易地映射值。
 */
float UALSCharacterAnimInstance::CalculateDiagonalScaleAmount() const
{
	return DiagonalScaleAmountCurve->GetFloatValue(FMath::Abs(VelocityBlend.F + VelocityBlend.B));
}

/*
 * 计算蹲伏播放速率
 */
float UALSCharacterAnimInstance::CalculateCrouchingPlayRate() const
{
	return FMath::Clamp(
		CharacterInformation.Speed / Config.AnimatedCrouchSpeed / Grounded.StrideBlend / GetOwningComponent()->
		GetComponentScale().Z,
		0.0f, 2.0f);
}

float UALSCharacterAnimInstance::CalculateLandPrediction() const
{
	/*
	 * 通过在速度方向上追踪，找到一个可行走的表面，
	 * 并得到“时间”(范围为0-1,1为最大值，0即将着陆)，计算出计算出预测权重。
	 * 着陆预测曲线用于控制时间如何影响一个平滑混合的最终权重。
	 */
	if (InAir.FallSpeed >= -200.0f)
	{
		// 速度太小了就直接返回0.f
		return 0.0f;
	}

	const UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent();
	const FVector& CapsuleWorldLoc = CapsuleComp->GetComponentLocation();
	const float VelocityZ = CharacterInformation.Velocity.Z;
	FVector VelocityClamped = CharacterInformation.Velocity;
	// 将角色Z轴的速度限制在 [-4000.0f, -200.0f] 之间
	VelocityClamped.Z = FMath::Clamp(VelocityZ, -4000.0f, -200.0f);
	VelocityClamped.Normalize();

	// 根据下落速度映射成检测长度向量
	const FVector TraceLength = VelocityClamped * FMath::GetMappedRangeValueClamped(
		{0.0f, -4000.0f}, {50.0f, 2000.0f}, VelocityZ);

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	FHitResult HitResult;
	const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(CapsuleComp->GetUnscaledCapsuleRadius(),
	                                                                           CapsuleComp->
	                                                                           GetUnscaledCapsuleHalfHeight());

	// 起始点是当前胶囊体的位置，终止点起始点加上了速度方向后的值。  生成的是一个胶囊体检测。
	const bool bHit = World->SweepSingleByChannel(HitResult, CapsuleWorldLoc, CapsuleWorldLoc + TraceLength,
	                                              FQuat::Identity,
	                                              ECC_Visibility, CapsuleCollisionShape, Params);

	if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
	{
		UALSDebugComponent::DrawDebugCapsuleTraceSingle(World,
		                                                CapsuleWorldLoc,
		                                                CapsuleWorldLoc + TraceLength,
		                                                CapsuleCollisionShape,
		                                                EDrawDebugTrace::Type::ForOneFrame,
		                                                bHit,
		                                                HitResult,
		                                                FLinearColor::Red,
		                                                FLinearColor::Green,
		                                                5.0f);
	}

	if (Character->GetCharacterMovement()->IsWalkable(HitResult))
	{
		// 如果地面可行走，就返回对应的着陆强度，根据射线未进入地面的长度比例对应着陆曲线上的值，进行插值
		// 如果动画播放还没有处于落地状态，就不启用落点检测点，也就是 NAME_Mask_LandPrediction 等于1的时候， 返回值为零。
		return FMath::Lerp(LandPredictionCurve->GetFloatValue(HitResult.Time), 0.0f,
		                   GetCurveValue(NAME_Mask_LandPrediction));
	}

	return 0.0f;
}

/*
 * 计算空中倾斜量
 */
FALSLeanAmount UALSCharacterAnimInstance::CalculateAirLeanAmount() const
{
	/*
	 * 使用相对速度方向和数量来确定角色在空中时应该倾斜多少。
	 * 倾斜在空气曲线得到下降速度，并被用作乘数平稳地逆转倾斜方向时，从移动向上过渡到移动向下。
	 */
	FALSLeanAmount CalcLeanAmount;
	// 获得相对速度量
	const FVector& UnrotatedVel = CharacterInformation.CharacterActorRotation.UnrotateVector(
		CharacterInformation.Velocity) / 350.0f;
	FVector2D InversedVect(UnrotatedVel.Y, UnrotatedVel.X);
	// 根据下路速度获得对应的值 然后乘以相对速度量 获得角色倾斜值
	InversedVect *= LeanInAirCurve->GetFloatValue(InAir.FallSpeed);
	CalcLeanAmount.LR = InversedVect.X;
	CalcLeanAmount.FB = InversedVect.Y;
	return CalcLeanAmount;
}

/*
 * 计算移动方向。
 * 这个值代表角色在观察方向/瞄准旋转模式中相对于摄像机移动的方向，
 * 并在循环混合animm图层中使用，
 * 以混合到适当的方向状态。
 */
EALSMovementDirection UALSCharacterAnimInstance::CalculateMovementDirection() const
{
	// 冲刺模式和速度方向模式就是向前运动
	if (Gait.Sprinting() || RotationMode.VelocityDirection())
	{
		return EALSMovementDirection::Forward;
	}

	// 计算角色速度方向和角色目标旋转方向的差值，然后返回对应的方向
	FRotator Delta = CharacterInformation.Velocity.ToOrientationRotator() - CharacterInformation.AimingRotation;
	Delta.Normalize();
	return UALSMathLibrary::CalculateQuadrant(MovementDirection, 70.0f, -70.0f, 110.0f, -110.0f, 5.0f, Delta.Yaw);
}

/*
 * 进行原地转身操作
 */
void UALSCharacterAnimInstance::TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime,
                                            bool OverrideCurrent)
{
	// 步骤1 ： 设置转身角度，转身是水平旋转，所以只取水平方向
	FRotator Delta = TargetRotation - CharacterInformation.CharacterActorRotation;
	Delta.Normalize();
	const float TurnAngle = Delta.Yaw;

	// 步骤2 ： 选择对应的旋转资源
	FALSTurnInPlaceAsset TargetTurnAsset;

	if (Stance.Standing())
	{
		// 处于站立状态
		if (FMath::Abs(TurnAngle) < TurnInPlaceValues.Turn180Threshold)
		{
			// 没有超过180°阈值，根据转身角度选择对应的转身动画
			TargetTurnAsset = TurnAngle < 0.0f
				                  ? TurnInPlaceValues.N_TurnIP_L90
				                  : TurnInPlaceValues.N_TurnIP_R90;
		}
		else
		{
			TargetTurnAsset = TurnAngle < 0.0f
				                  ? TurnInPlaceValues.N_TurnIP_L180
				                  : TurnInPlaceValues.N_TurnIP_R180;
		}
	}
	else
	{
		// 蹲伏状态
		if (FMath::Abs(TurnAngle) < TurnInPlaceValues.Turn180Threshold)
		{
			TargetTurnAsset = TurnAngle < 0.0f
				                  ? TurnInPlaceValues.CLF_TurnIP_L90
				                  : TurnInPlaceValues.CLF_TurnIP_R90;
		}
		else
		{
			TargetTurnAsset = TurnAngle < 0.0f
				                  ? TurnInPlaceValues.CLF_TurnIP_L180
				                  : TurnInPlaceValues.CLF_TurnIP_R180;
		}
	}

	// 步骤3 ： 如果没有在播放当前蒙太奇或者在有动画覆盖的状态下，就播放蒙太奇动画
	if (!OverrideCurrent && IsPlayingSlotAnimation(TargetTurnAsset.Animation, TargetTurnAsset.SlotName))
	{
		return;
	}
	PlaySlotAnimationAsDynamicMontage(TargetTurnAsset.Animation, TargetTurnAsset.SlotName, 0.2f, 0.2f,
	                                  TargetTurnAsset.PlayRate * PlayRateScale, 1, 0.0f, StartTime);

	// 步骤4:获得旋转比例，在角色类中进行控制旋转。
	if (TargetTurnAsset.ScaleTurnAngle)
	{
		Grounded.RotationScale = TurnAngle / TargetTurnAsset.AnimatedAngle * TargetTurnAsset.PlayRate * PlayRateScale;
	}
	else
	{
		Grounded.RotationScale = TargetTurnAsset.PlayRate * PlayRateScale;
	}
}

/*
 * 在刚开始跳跃的时候，由拥有者触发
 */
void UALSCharacterAnimInstance::OnJumped()
{
	/* 更新在跳跃状态下的属性 */
	InAir.bJumped = true;
	InAir.JumpPlayRate = FMath::GetMappedRangeValueClamped({0.0f, 600.0f}, {1.2f, 1.5f}, CharacterInformation.Speed);

	/* 过一会儿之后将处跳跃键按下操作值设置为 false*/
	UWorld* World = GetWorld();
	check(World);
	World->GetTimerManager().SetTimer(OnJumpedTimer, this,
	                                  &UALSCharacterAnimInstance::OnJumpedDelay, 0.1f, false);
}

void UALSCharacterAnimInstance::OnPivot()
{
	// 如果速度没有超过限制的速度，就可以启用 Pivot 状态。 然后再过段时间再将它关闭。
	Grounded.bPivot = CharacterInformation.Speed < Config.TriggerPivotSpeedLimit;
	UWorld* World = GetWorld();
	check(World);
	World->GetTimerManager().SetTimer(OnPivotTimer, this,
	                                  &UALSCharacterAnimInstance::OnPivotDelay, 0.1f, false);
}
