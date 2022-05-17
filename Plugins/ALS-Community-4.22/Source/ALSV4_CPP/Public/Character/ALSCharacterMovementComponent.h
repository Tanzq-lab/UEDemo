// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Haziq Fadhil
// Contributors:    Doğa Can Yanıkoğlu

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Library/ALSCharacterStructLibrary.h"

#include "ALSCharacterMovementComponent.generated.h"

/**
 * Authoritative networked Character Movement
 * 继承自运动组件
 */
UCLASS()
class ALSV4_CPP_API UALSCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

	/*
	 * 保存着角色运动数据
	 * 主要是保存着运动状态是否发生变化以及当前运动步态，
	 * 然后重载一些相关的函数配合变量的更新
	 */
	class ALSV4_CPP_API FSavedMove_My : public FSavedMove_Character
	{
	public:

		typedef FSavedMove_Character Super;

		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
		                        class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(class ACharacter* Character) override;

		// Walk Speed Update
		// 储存运动状态是否发生变化变量
		uint8 bSavedRequestMovementSettingsChange : 1;
		EALSGait SavedAllowedGait = EALSGait::Walking;
	};

	/*
	 * 重写其中的 AllocateNewMove 函数， 给自定义的保存运动结构体分配空间，和上面的结构体一起的。
	 */
	class ALSV4_CPP_API FNetworkPredictionData_Client_My : public FNetworkPredictionData_Client_Character
	{
	public:
		/* 重载构造函数， 将当前组件传入 */
		FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	/* 根据上面自定义结构体做出的修改 */
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity) override;

	// Movement Settings Override
	/*  根据运动曲线的值更新运动所需要的值 */
	
	virtual void PhysWalking(float deltaTime, int32 Iterations) override;
	// virtual void PhysFlying(float deltaTime, int32 Iterations) override;
	
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;

	// Movement Settings Variables
	// 重要的状态是否发生了改变
	UPROPERTY()
	uint8 bRequestMovementSettingsChange = 1;

	/* 保存将要达到的步态 */
	UPROPERTY()
	EALSGait AllowedGait = EALSGait::Walking;

	/* 目前状态下，运动状态配置 */
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Movement System")
	FALSMovementSettings CurrentMovementSettings;
	
	float GetMappedSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetMovementSettings(FALSMovementSettings NewMovementSettings);

	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetAllowedGait(EALSGait NewAllowedGait);

	UFUNCTION(Reliable, Server, Category = "Movement Settings")
	void Server_SetAllowedGait(EALSGait NewAllowedGait);
};
