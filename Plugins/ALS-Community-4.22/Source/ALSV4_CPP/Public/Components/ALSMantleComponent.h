// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Doğa Can Yanıkoğlu

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Library/ALSCharacterStructLibrary.h"

#include "ALSMantleComponent.generated.h"

// forward declarations
class UTimelineComponent;
class AALSBaseCharacter;
class UALSDebugComponent;


UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API UALSMantleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UALSMantleComponent();

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
	bool MantleCheck(const FALSMantleTraceSettings& TraceSettings,
	                 EDrawDebugTrace::Type DebugType);

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
	void MantleStart(float MantleHeight, const FALSComponentAndTransform& MantleLedgeWS,
	                 EALSMantleType MantleType);

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
	void MantleUpdate(float BlendIn);

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
	void MantleEnd();

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
	void OnOwnerJumpInput();

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
	void OnOwnerJumpRelease();

	UFUNCTION(BlueprintCallable, Category = "ALS|Mantle System")
	void OnOwnerRagdollStateChanged(bool bRagdollState);

	/** 在蓝图中实现， 根据参数返回对应的攀爬资源 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "ALS|Mantle System")
	FALSMantleAsset GetMantleAsset(EALSMantleType MantleType, EALSOverlayState CurrentOverlayState);

public:

	/**
	 * @brief 检测并设置起始攀爬类型
	 */
	// UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System")
	// void SetStartLadderType();
	
	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System")
	bool LadgeClimbCheck(const FALSMantleTraceSettings& TraceSettings,
	                      EDrawDebugTrace::Type DebugType);

	/**
	 * @brief 检测手部旁边有无物体
	 */
	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System")
	bool ClimbingMovingDetection(FName BoneName, bool bIsRight, EDrawDebugTrace::Type DebugType);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System")
	void UpdateLedgeClimb(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System")
	void UpdateLedgeCharacter(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System")
	void UpdateLedgeInputInfo(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System")
	void UpdateLedgeAnimInfo(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System")
	bool IsTouchFloor(FName BoneName_L, FName BoneName_R, EDrawDebugTrace::Type DebugType);
	
	/** Climbing System - Corner */
	
	/**
	 * @brief 开始攀爬旋转
	 */
	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System - Corner")
	void ClimbCornerStart(float TimeLength, float StartTime, float PlayRate);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System - Corner")
	void ClimbCornerUpdate(float BlendIn);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System - Corner")
	void ClimbCornerEnd();

	UFUNCTION(BlueprintCallable, Category = "ALS|Climbing System - Corner")
	bool CornerCheck(bool bIsRight, bool bCanTraceOuter);
	
	UFUNCTION(BlueprintCallable, Category = "ALS|Climbing System - Corner")
	int32 CanCornerClimbing(bool bIsOuter, bool bIsRight, FTransform& CornerTarget, EDrawDebugTrace::Type DebugType);


	
	/** Climbing System - Jump */
	
	/**
	 * @brief 开始攀爬旋转
	 */
	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System - Jump")
	void ClimbJumpCheck(EDrawDebugTrace::Type DebugType);

	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System - Jump")
	void ClimbJumpUpdate(float DeltaTime);

	/** Climbing System - Exit */
	UFUNCTION(BlueprintCallable, Category = "ALS|Ladge System - Exit")
	void ExitClimbing();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// Called when the game starts
	virtual void BeginPlay() override;

	/** Mantling*/
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "ALS|Mantle System")
	void Server_MantleStart(float MantleHeight, const FALSComponentAndTransform& MantleLedgeWS,
	                        EALSMantleType MantleType);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "ALS|Mantle System")
	void Multicast_MantleStart(float MantleHeight, const FALSComponentAndTransform& MantleLedgeWS,
	                           EALSMantleType MantleType);

	/** Mantling*/
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "ALS|Ladge System")
	void Server_ExitClimbing();

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category = "ALS|Ladge System")
	void Multicast_ExitClimbing();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Mantle System")
	UTimelineComponent* MantleTimeline = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
	FALSMantleTraceSettings GroundedTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
	FALSMantleTraceSettings AutomaticTraceSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
	FALSMantleTraceSettings FallingTraceSettings;

	/* mantle更新曲线 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
	UCurveFloat* MantleTimelineCurve;

	static FName NAME_IgnoreOnlyPawn;

	/** 允许mantle的物体配置 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
	FName ClimbObjectDetectionProfile = NAME_IgnoreOnlyPawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
	TEnumAsByte<ECollisionChannel> WalkableSurfaceDetectionChannel = ECC_Visibility;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Mantle System")
	FALSMantleParams MantleParams;

	/* mantle组件 和 相对位置 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Mantle System")
	FALSComponentAndTransform MantleLedgeLS;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Mantle System")
	FVector TestPoint;

	/* 攀爬点 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Mantle System")
	FTransform MantleTarget = FTransform::Identity;

	/* 角色真实相差的变换距离 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Mantle System")
	FTransform MantleActualStartOffset = FTransform::Identity;

	/* 动画真实运动的时候和mantle点之间的偏差值 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Mantle System")
	FTransform MantleAnimatedStartOffset = FTransform::Identity;

	/** 可接受的mantle物体最大速度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Mantle System")
	float AcceptableVelocityWhileMantling = 10.0f;

protected:
	/** Ledge System */

	static ECollisionChannel ClimbCollisionChannel;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Ledge System")
	FALSMantleTraceSettings LadgeTraceSettings;
	
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System")
	FALSComponentAndTransform LedgeClimbLS;
	
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System")
	FALSComponentAndTransform LastLedgeClimbLS;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System")
	FVector2D MovingTransitionDelta;

	/**
	 * @brief 上一次位置
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System")
	FVector LastLocation;

	/**
	 * @brief 攀爬移动速度
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System")
	FVector Speed;
	
	/**
	 * @brief 是否有按键输入
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System")
	bool bCanMoving = false;

	/**
	 * @brief 移动动画播放速率
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System")
	float MovingAnimPlayRate;
	
	/**
	 * @brief 角色旋转值插值变化量
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|Ledge System - Config")
	float RotationInterpSpeed = 15.f;
	
	/**
	 * @brief 角色旋转值插值变化量
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|Ledge System - Config")
	float MoveDeltaInterpSpeed = 5.f;
	
	/**
	 * @brief 角色在移动的时候位置插值变化量
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|Ledge System - Config")
	FVector MovingLagSpeed = FVector(5.f, 5.f, 10.f);
	
	/**
	 * @brief 角色在未移动的时候插值变化量
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|Ledge System - Config")
	FVector NotMoveLagSpeed = FVector(15.f, 15.f, 20.f);
	
	/**
	 * @brief 攀爬目标点
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System")
	FTransform LedgeTargetWS;

	/** Ledge System - Corner */
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ALS|Ledge System - Corner")
	UTimelineComponent* ClimbCornerTimeline = nullptr;
	
	/* 攀爬角落旋转更新曲线 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Ledge System - Corner")
	UCurveFloat* ClimbCornerTimelineCurve;
	
	/* 角色真实相差的变换距离 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Corner")
	FTransform CornerActualStartOffset = FTransform::Identity;

	/* 动画真实运动的时候和  旋转点 之间的偏差值 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Corner")
	FTransform CornerAnimatedStartOffset = FTransform::Identity;

	/**
	 * @brief 攀爬旋转的相对物体以及相对变换
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALS|Ledge System - Corner")
	FALSCornerClimbValues CornerClimbValues;

	/** Ledge System - Jump */
	
	/**
	 * @brief 上一次跳跃键按下时间
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	float LastJumpInputTime;

	/**
	 * @brief 按下跳跃键的时间
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	float JumpInputTime;
	
	/**
	 * @brief 按下跳跃键那一刻按下向上的值
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	float JumpUpValue;
	
	/**
	 * @brief 按下跳跃键那一刻按下向右的值
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	float JumpRightValue;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	bool bJumpPressed = false;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	bool bCanJumping = false;
	
	/**
	 * @brief 代表本次射线检测没有检测到任何物体
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	bool bCanExit = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	bool bHasBlock = false;

	/**
	 * @brief 跳跃起点和终点之间的距离
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	float JumpLength;
	
	/**
	 * @brief 与跳跃点的差值
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	FVector JumpDistanceDiff;

	/**
	 * @brief 保存动画跳跃距离
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	FVector AnimJumpDistance;
	
	/**
	 * @brief 在跳跃状态是否可以进行移动了
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	bool bCanJumpMove;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	FALSComponentAndTransform JumpTargetInfo;

	// TODO  待删
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Ledge System - Jump")
	FVector JumpPoint;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALS|Ledge System - Jump")
	FALSClimbJumpParams JumpParams;

private:
	UPROPERTY()
	AALSBaseCharacter* OwnerCharacter;

	UPROPERTY()
	UALSDebugComponent* ALSDebugComponent = nullptr;
};

