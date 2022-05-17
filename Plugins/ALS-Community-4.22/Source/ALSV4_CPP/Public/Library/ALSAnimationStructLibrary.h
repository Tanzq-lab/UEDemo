// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Doğa Can Yanıkoğlu
// Contributors:    


#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Animation/AnimSequenceBase.h"
#include "ALSCharacterEnumLibrary.h"


#include "ALSAnimationStructLibrary.generated.h"

USTRUCT(BlueprintType)
struct FALSDynamicMontageParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Dynamic Transition")
	UAnimSequenceBase* Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Dynamic Transition")
	float BlendInTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Dynamic Transition")
	float BlendOutTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Dynamic Transition")
	float PlayRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Dynamic Transition")
	float StartTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FALSLeanAmount
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float LR = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float FB = 0.0f;
};

USTRUCT(BlueprintType)
struct FALSVelocityBlend
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float F = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float B = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float L = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float R = 0.0f;
};

USTRUCT(BlueprintType)
struct FALSTurnInPlaceAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	UAnimSequenceBase* Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float AnimatedAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	bool ScaleTurnAngle = true;
};

USTRUCT(BlueprintType)
struct FALSTurnInClimbAsset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	UAnimSequenceBase* Animation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float AnimatedAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float PlayRate = 1.0f;

	float GetTimeLength() const
	{
		if (!Animation) return 0.f;
		return Animation->GetMaxCurrentTime();
	}
};


/*
 * 保存角色动画实例信息的结构体
 */
USTRUCT(BlueprintType)
struct FALSAnimCharacterInformation
{
	GENERATED_BODY()

	/* 由当前控制器旋转值插值得出的旋转值 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	FRotator AimingRotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	FRotator CharacterActorRotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	FVector Velocity;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	FVector RelativeVelocityDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	FVector Acceleration;

	/* 运动组件中目前的加速度 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	FVector MovementInput;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	bool bIsMoving = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	bool bHasMovementInput = false;

	/* 角色当前速度 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	float Speed = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	float MovementInputAmount = 0.0f;

	/* 摄像机移动旋转移动的速率 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	float AimYawRate = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	float ZoomAmount = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	EALSMovementState PrevMovementState = EALSMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Character Information")
	EALSViewMode ViewMode = EALSViewMode::ThirdPerson;
};

USTRUCT(BlueprintType)
struct FALSAnimGraphGrounded
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ALS|Anim Graph - Grounded")
	EALSHipsDirection TrackedHipsDirection = EALSHipsDirection::F;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	bool bShouldMove = false; // Should be false initially

	/* 可以向左旋转 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	bool bRotateL = false;

	/* 可以向右旋转 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	bool bRotateR = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ALS|Anim Graph - Grounded")
	bool bPivot = false;

	/* 旋转动画播放速率 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float RotateRate = 1.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float RotationScale = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float DiagonalScaleAmount = 0.0f;

	/* 角色跑步和行走混合值 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float WalkRunBlend = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float StandingPlayRate = 1.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float CrouchingPlayRate = 1.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float StrideBlend = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float FYaw = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float BYaw = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float LYaw = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Grounded")
	float RYaw = 0.0f;
};

USTRUCT(BlueprintType)
struct FALSAnimGraphInAir
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - In Air")
	bool bJumped = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - In Air")
	float JumpPlayRate = 1.2f;

	// 下落速度
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - In Air")
	float FallSpeed = 0.0f;

	/* 着陆强度 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - In Air")
	float LandPrediction = 1.0f;
};

USTRUCT(BlueprintType)
struct FALSAnimGraphInClimb
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - In Climb")
	float ClimbFixBlendValue = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - In Climb")
	float RotationScale = 0.0f;
};

USTRUCT(BlueprintType)
struct FALSAnimGraphAimingValues
{
	GENERATED_BODY()

	/* 光滑移动到目标旋转的值 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	FRotator SmoothedAimingRotation;

	/* 脊柱旋转方向，保证和摄像头同一个方向  */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	FRotator SpineRotation;

	/* 角色旋转值和目标旋转值之间的差值 X ： Yaw  Y : Pitch */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	FVector2D AimingAngle;

	/* 对应向上向下摆头的动画时间 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	float AimSweepTime = 0.5f;

	/* 对应向左向右移动的动画时间 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	float InputYawOffsetTime = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	float ForwardYawTime = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	float LeftYawTime = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Aiming Values")
	float RightYawTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FALSAnimGraphLayerBlending
{
	GENERATED_BODY()

	
	/**
	 * @brief 0 : 正常状态， 1 ： 攀爬状态， 2 ： 翻滚状态， 3 ： 起身状态
	 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	int32 OverlayOverrideState = 0;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float EnableAimOffset = 1.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float BasePose_N = 1.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float BasePose_CLF = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_L = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_L_Add = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_L_LS = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_L_MS = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_R = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_R_Add = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_R_LS = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Arm_R_MS = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Hand_L = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Hand_R = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Legs = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Legs_Add = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Pelvis = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Pelvis_Add = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Spine = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Spine_Add = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Head = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Head_Add = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float EnableHandIK_L = 1.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float EnableHandIK_R = 1.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Layer Blending")
	float Mask_Secondary_Motion = 0.75f;
};

USTRUCT(BlueprintType)
struct FALSAnimGraphFootIK
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	float FootLock_L_Alpha = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	float FootLock_R_Alpha = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	bool UseFootLockCurve_L;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	bool UseFootLockCurve_R;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FVector FootLock_L_Location;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FVector TargetFootLock_R_Location;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FVector FootLock_R_Location;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FRotator TargetFootLock_L_Rotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FRotator FootLock_L_Rotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FRotator TargetFootLock_R_Rotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FRotator FootLock_R_Rotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FVector FootOffset_L_Location;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FVector FootOffset_R_Location;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FRotator FootOffset_L_Rotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FRotator FootOffset_R_Rotation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	FVector PelvisOffset;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ALS|Anim Graph - Foot IK")
	float PelvisAlpha = 0.0f;
};

USTRUCT(BlueprintType)
struct FALSAnimTurnInPlace
{
	GENERATED_BODY()

	/* 原地旋转最小角度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float TurnCheckMinAngle = 45.0f;

	/* 播放转身180°动作的阈值 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float Turn180Threshold = 130.0f;

	/* 最大摄像头旋转速率 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float AimYawRateLimit = 50.0f;

	/* 摄像头位置等待时间 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float ElapsedDelayTime = 0.0f;

	/* 原地转身最小旋转角度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float MinAngleDelay = 0.f;

	/* 原地转身最大旋转角度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	float MaxAngleDelay = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FALSTurnInPlaceAsset N_TurnIP_L90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FALSTurnInPlaceAsset N_TurnIP_R90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FALSTurnInPlaceAsset N_TurnIP_L180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FALSTurnInPlaceAsset N_TurnIP_R180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FALSTurnInPlaceAsset CLF_TurnIP_L90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FALSTurnInPlaceAsset CLF_TurnIP_R90;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FALSTurnInPlaceAsset CLF_TurnIP_L180;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Turn In Place")
	FALSTurnInPlaceAsset CLF_TurnIP_R180;
};


USTRUCT(BlueprintType)
struct FALSAnimRotateInClimb
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Climb")
	FALSTurnInClimbAsset FH_Rotate_90_L;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Climb")
	FALSTurnInClimbAsset FH_Rotate_90_R;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Climb")
	FALSTurnInClimbAsset H_Rotate_90_L;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Climb")
	FALSTurnInClimbAsset H_Rotate_90_R;
};

USTRUCT(BlueprintType)
struct FALSAnimRotateInPlace
{
	GENERATED_BODY()

	// 最小旋转阈值
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Place")
	float RotateMinThreshold = -50.0f;

	// 最大旋转阈值
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Place")
	float RotateMaxThreshold = 50.0f;

	/* 原地旋转最小水平角度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Place")
	float AimYawRateMinRange = 90.0f;

	/* 原地旋转最大水平角度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Place")
	float AimYawRateMaxRange = 270.0f;

	/* 原地旋转动画最小播放速率 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Place")
	float MinPlayRate = 1.15f;

	/* 原地旋转动画最大播放速率 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Rotate In Place")
	float MaxPlayRate = 3.0f;
};

USTRUCT(BlueprintType)
struct FALSAnimConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float AnimatedWalkSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float AnimatedRunSpeed = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float AnimatedSprintSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float AnimatedCrouchSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float VelocityBlendInterpSpeed = 12.0f;

	/* 角色在地面倾斜插值速度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float GroundedLeanInterpSpeed = 4.0f;

	/* 角色在空中倾斜插值速度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float InAirLeanInterpSpeed = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float SmoothedAimingRotationInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float InputYawOffsetInterpSpeed = 8.0f;

	/* 触发Pivot状态最大不能超过的速度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float TriggerPivotSpeedLimit = 200.0f;

	/** 脚骨骼距离地面的高度（脚踝骨骼位置并不是贴地的，它距离地面还有一些高度） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float FootHeight = 13.5f;

	/** 脚骨骼距离脚趾的水平宽度 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float FootLength = 20.f;

	/** Threshold value for activating dynamic transition on various animations */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float DynamicTransitionThreshold = 8.0f;

	/* 脚部IK向上射线的距离 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float IK_TraceDistanceAboveFoot = 50.0f;

	/* 脚部IK向下射线的距离 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Main Configuration")
	float IK_TraceDistanceBelowFoot = 45.0f;
};

