// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Doğa Can Yanıkoğlu
// Contributors:    


#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Library/ALSAnimationStructLibrary.h"
#include "Library/ALSCharacterStructLibrary.h"
#include "Library/ALSStructEnumLibrary.h"

#include "ALSCharacterAnimInstance.generated.h"

struct FALSComponentAndTransform;
// forward declarations
class UALSDebugComponent;
class AALSBaseCharacter;
class UCurveFloat;
class UAnimSequence;
class UCurveVector;

/**
 * Main anim instance class for character
 */
UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API UALSCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	static ECollisionChannel ClimbCollisionChannel;
	
	virtual void NativeInitializeAnimation() override;

	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "ALS|Animation")
	void PlayTransition(const FALSDynamicMontageParams& Parameters);

	UFUNCTION(BlueprintCallable, Category = "ALS|Animation")
	void PlayTransitionChecked(const FALSDynamicMontageParams& Parameters);

	UFUNCTION(BlueprintCallable, Category = "ALS|Animation")
	void PlayDynamicTransition(float ReTriggerDelay, FALSDynamicMontageParams Parameters);

	UFUNCTION(BlueprintCallable, Category = "ALS|Event")
	void OnJumped();

	UFUNCTION(BlueprintCallable, Category = "ALS|Event")
	void OnPivot();

	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	void SetGroundedEntryState(EALSGroundedEntryState NewGroundedEntryState)
	{
		GroundedEntryState = NewGroundedEntryState;
	}

	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	void SetOverlayOverrideState(int32 OverlayOverrideState)
	{
		LayerBlendingValues.OverlayOverrideState = OverlayOverrideState;
	}

	/* 由动画通知来调用该函数 */
	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	void SetTrackedHipsDirection(EALSHipsDirection HipsDirection)
	{
		Grounded.TrackedHipsDirection = HipsDirection;
	}

	/** Enable Movement Animations if IsMoving and HasMovementInput, or if the Speed is greater than 150. */
	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	bool ShouldMoveCheck() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	bool CanRotateInPlace() const;

	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	bool CanTurnInPlace() const;

	/**
	 * Only perform a Dynamic Transition check if the "Enable Transition" curve is fully weighted.
	 * The Enable_Transition curve is modified within certain states of the AnimBP so
	 * that the character can only transition while in those states.
	 */
	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	bool CanDynamicTransition() const;

	/** 返回字符信息的可变引用，以便在字符类中轻松编辑它们 */
	FALSAnimCharacterInformation& GetCharacterInformationMutable()
	{
		return CharacterInformation;
	}


	/** Climbing System */

	UFUNCTION(BlueprintCallable, Category = "ALS|Climbing System")
	void StartCornerClimb(bool bIsRight, float RotateAngle, float PlayRateScale, float StartTime, bool OverrideCurrent);

private:
	void PlayDynamicTransitionDelay();

	void OnJumpedDelay();

	void OnPivotDelay();

	/** Update Values */

	void UpdateAimingValues(float DeltaSeconds);

	void UpdateLayerValues();

	void UpdateFootIK(float DeltaSeconds);

	void UpdateMovementValues(float DeltaSeconds);

	void UpdateRotationValues();

	void UpdateInAirValues(float DeltaSeconds);

	void UpdateRagdollValues();

	void UpdateClimbValues();

	/** Foot IK */

	void SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone,
	                    float& CurFootLockAlpha, bool& UseFootLockCurve,
	                    FVector& CurFootLockLoc, FRotator& CurFootLockRot) const;

	void SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot) const;

	void SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget, FVector FootOffsetRTarget);

	void ResetIKOffsets(float DeltaSeconds);

	void SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone, FName RootBone,
	                    FVector& CurLocationTarget, FVector& CurLocationOffset, FRotator& CurRotationOffset) const;

	bool SetClimbFootIK(FName FootBone, FName EnableFootIKCurve, bool bIsRight, FVector& FootOffset) const;

	void FixClimbingBody(FVector& FixBoneValue, float& BlendValue, float Distance) const;

	/** Climb Hand IK */
	int32 SetClimbHandIK(FName EnableFootLockCurve, FName HandBone, bool bIsRight, float& InterpSpeed, FVector& TargetHandLocation, UPrimitiveComponent
	                     *& Component) const;

	/** Grounded */

	void RotateInPlaceCheck();

	void TurnInPlaceCheck(float DeltaSeconds);

	void DynamicTransitionCheck();

	FALSVelocityBlend CalculateVelocityBlend() const;

	void TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent);

	/** Movement */

	FVector CalculateRelativeAccelerationAmount() const;

	float CalculateStrideBlend() const;

	float CalculateWalkRunBlend() const;

	float CalculateStandingPlayRate() const;

	float CalculateDiagonalScaleAmount() const;

	float CalculateCrouchingPlayRate() const;

	float CalculateLandPrediction() const;

	FALSLeanAmount CalculateAirLeanAmount() const;

	EALSMovementDirection CalculateMovementDirection() const;

	/** Util */

	float GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin, float ClampMax) const;

protected:
	/** References */
	UPROPERTY(BlueprintReadOnly, Category = "Read Only Data|Character Information")
	AALSBaseCharacter* Character = nullptr;

	/** Character Information */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimCharacterInformation CharacterInformation;

public:
	/** Read Only Data|Character Information */

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSMovementState MovementState = EALSMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSMovementAction MovementAction = EALSMovementAction::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSRotationMode RotationMode = EALSRotationMode::LookingDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSGait Gait = EALSGait::Walking;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSStance Stance = EALSStance::Standing;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSOverlayState OverlayState = EALSOverlayState::Default;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSOverlayState LastOverlayState = EALSOverlayState::Default;

	/** Climbing System */

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Climbing System")
	bool bDesiredLaddering = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Climbing System")
	float JumpInClimbL_R;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Climbing System")
	float JumpInClimbU_D;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Climbing System")
	bool bCanClimbJump;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Climbing System")
	float RotateInClimbAngle = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Climbing System")
	bool bCanClimbMove = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Climbing System")
	FVector2D ClimbMovingDelta;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Climbing System")
	EALSClimbingType ClimbingType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Climbing System")
	float ClimbMovingPlayRate;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Climbing System")
	FVector ClimbFixBoneOffset;


protected:
	/** Anim Graph - Grounded */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimGraphGrounded Grounded;


	/** Anim Graph - Grounded */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimGraphInClimb ClimbingValues;

public:
	/* 插值得到的速度各个方向的速度混合值 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FALSVelocityBlend VelocityBlend;

	/* 身体倾斜值 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FALSLeanAmount LeanAmount;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FVector RelativeAccelerationAmount = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FALSGroundedEntryState GroundedEntryState = EALSGroundedEntryState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FALSMovementDirection MovementDirection = EALSMovementDirection::Forward;

protected:
	/** Anim Graph - In Air */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - In Air", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimGraphInAir InAir;

	/** Anim Graph - Aiming Values */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Aiming Values", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimGraphAimingValues AimingValues;

public:
	/* 光滑瞄准旋转值和绝当前旋转值之间的差值 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Aiming Values")
	FVector2D SmoothedAimingAngle = FVector2D::ZeroVector;

protected:
	/** Anim Graph - Ragdoll */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Ragdoll")
	float FlailRate = 0.0f;

	/** Anim Graph - Layer Blending */

	/** 保存曲线的值 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Layer Blending", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimGraphLayerBlending LayerBlendingValues;

	/** Anim Graph - Foot IK */
	/** 关于脚部IK的一些值，会在动画蓝图的脚部IK中进行使用 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Foot IK", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimGraphFootIK FootIKValues;


	/** Anim Graph - Hand IK */

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Hand IK", Meta = (
		ShowOnlyInnerProperties))
	FVector HandIKTarget_L;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Hand IK", Meta = (
		ShowOnlyInnerProperties))
	FVector HandIKTarget_R;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Hand IK", Meta = (
		ShowOnlyInnerProperties))
	FALSComponentAndTransform HandIK_LS_L;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Hand IK", Meta = (
		ShowOnlyInnerProperties)) 
	FALSComponentAndTransform HandIK_LS_R;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Hand IK", Meta = (
		ShowOnlyInnerProperties))
	FVector LastHandLocation_L;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Hand IK", Meta = (
		ShowOnlyInnerProperties))
	FVector LastHandLocation_R;

	// UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Hand IK", Meta = (
	// 	ShowOnlyInnerProperties))
	// FVector FootIKTarget_L;
	//
	// UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Hand IK", Meta = (
	// 	ShowOnlyInnerProperties))
	// FVector FootIKTarget_R;

	/** Turn In Place */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Turn In Place", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimTurnInPlace TurnInPlaceValues;

	/** Rotate In Place */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Rotate In Place", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimRotateInPlace RotateInPlace;

	/** Rotate In Climb */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Rotate In Climb", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimRotateInClimb RotateInClimbValues;

	/** Configuration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Main Configuration", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimConfiguration Config;

	/** Blend Curves */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	UCurveFloat* DiagonalScaleAmountCurve = nullptr;

	/* 走路曲线 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	UCurveFloat* StrideBlend_N_Walk = nullptr;

	/* 跑步曲线 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	UCurveFloat* StrideBlend_N_Run = nullptr;

	/* 蹲伏走路曲线 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	UCurveFloat* StrideBlend_C_Walk = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	UCurveFloat* LandPredictionCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	UCurveFloat* LeanInAirCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	UCurveVector* YawOffset_FB = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	UCurveVector* YawOffset_LR = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	UAnimSequenceBase* TransitionAnim_R = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	UAnimSequenceBase* TransitionAnim_L = nullptr;

	/** IK Bone Names */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Anim Graph - Foot IK")
	FName IkFootL_BoneName = FName(TEXT("ik_foot_l"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Anim Graph - Foot IK")
	FName IkFootR_BoneName = FName(TEXT("ik_foot_r"));

private:
	FTimerHandle OnPivotTimer;

	FTimerHandle PlayDynamicTransitionTimer;

	FTimerHandle OnJumpedTimer;

	bool bCanPlayDynamicTransition = true;

	UPROPERTY()
	UALSDebugComponent* ALSDebugComponent = nullptr;
};
