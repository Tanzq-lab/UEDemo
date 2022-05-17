// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Haziq Fadhil
// Contributors:    Doğa Can Yanıkoğlu


#include "Character/ALSCharacterMovementComponent.h"
#include "Character/ALSBaseCharacter.h"

#include "Curves/CurveVector.h"

UALSCharacterMovementComponent::UALSCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/*
 * 移动更新后触发的事件
 */
void UALSCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation,
                                                       const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

	if (!CharacterOwner)
	{
		return;
	}

	// Set Movement Settings
	// 如果移动信息有发生改变的话，就更新信息，然后重置该布尔值。
	if (bRequestMovementSettingsChange)
	{
		/* 最大行走速度在蹲伏状态下也是和正常状态一样的。 */
		const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
		MaxWalkSpeed = UpdateMaxWalkSpeed;
		MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;

		bRequestMovementSettingsChange = false;
	}
}

/*
 * 模拟走路的物理属性
 * 主要是更新摩擦力，让运动控制的更加细致。
 */
void UALSCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	if (CurrentMovementSettings.MovementCurve)
	{
		// 根据移动曲线更新地面摩擦。
		// 这允许在每个速度下精细地控制移动行为。
		GroundFriction = CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Z;
	}
	Super::PhysWalking(deltaTime, Iterations);
}

/*
 * 根据曲线更新加速度
 */
float UALSCharacterMovementComponent::GetMaxAcceleration() const
{
	/* 如果没有在地面上移动或者没有加载运动曲线，就使用原来的加速度 */
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxAcceleration();
	}

	/* 否则就使用曲线的加速度 */
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).X;
}

/*
 * 根据运动曲线更新减速度
 */
float UALSCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxBrakingDeceleration();
	}
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Y;
}

/*
 * 更新状态压缩标志
 */
void UALSCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags) // Client only
{
	Super::UpdateFromCompressedFlags(Flags);

	/* 如果存在自定的掩码，就表明状态变量已经改变，同时改变当前变量 */
	bRequestMovementSettingsChange = Flags & FSavedMove_Character::FLAG_Custom_0;
}

/*
 * 将自定义的结构体内容传入其中
 */
class FNetworkPredictionData_Client* UALSCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (!ClientPredictionData)
	{
		/* 转换为父类 */
		UALSCharacterMovementComponent* MutableThis = const_cast<UALSCharacterMovementComponent*>(this);

		/* 更新客户端预测数据，将自定义的结构体加入其中 */
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_My(*this);
		/* 当在更新之间插入时，允许最大距离字符滞后于服务器位置。 */
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		/* 字符被传送到新服务器位置的最大距离，没有任何平滑。 */
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

/*
 * 数据清空函数，提高重用性
 */
void UALSCharacterMovementComponent::FSavedMove_My::Clear()
{
	Super::Clear();

	bSavedRequestMovementSettingsChange = false;
	SavedAllowedGait = EALSGait::Walking;
}

/*
 * 将判断运动状态是否发生变化变量加入到CompressedFlags中。
 */
uint8 UALSCharacterMovementComponent::FSavedMove_My::GetCompressedFlags() const
{
	// 压缩信息中包含着 跳跃键是否按下 、 是否有蹲伏想法
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedRequestMovementSettingsChange)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

/*
 * 设置存储的运动变量，将角色的运动数据传入，更新结构体中的变量。
 */
void UALSCharacterMovementComponent::FSavedMove_My::SetMoveFor(ACharacter* Character, float InDeltaTime,
                                                               FVector const& NewAccel,
                                                               class FNetworkPredictionData_Client_Character&
                                                               ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UALSCharacterMovementComponent* CharacterMovement = Cast<UALSCharacterMovementComponent>(
		Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		bSavedRequestMovementSettingsChange = CharacterMovement->bRequestMovementSettingsChange;
		SavedAllowedGait = CharacterMovement->AllowedGait;
	}
}

/*
 * 对角色预测状态进行更新
 */
void UALSCharacterMovementComponent::FSavedMove_My::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UALSCharacterMovementComponent* CharacterMovement = Cast<UALSCharacterMovementComponent>(
		Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->AllowedGait = SavedAllowedGait;
	}
}

UALSCharacterMovementComponent::FNetworkPredictionData_Client_My::FNetworkPredictionData_Client_My(
	const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UALSCharacterMovementComponent::FNetworkPredictionData_Client_My::AllocateNewMove()
{
	return MakeShared<FSavedMove_My>();
}

/*
 * 服务器更新运动步态
 */
void UALSCharacterMovementComponent::Server_SetAllowedGait_Implementation(const EALSGait NewAllowedGait)
{
	AllowedGait = NewAllowedGait;
}

/*
 * 根据当前速度获取当前在曲线中的位置。
 */
float UALSCharacterMovementComponent::GetMappedSpeed() const
{
	/*
	 * 将角色的当前速度映射到配置的移动速度，范围为0-3,
	 * 0 = 停止，1 = 行走速度，2 = 奔跑速度，3 = 冲刺速度。
	 * 这让我们能够改变移动速度，但仍然在计算中使用映射范围以获得一致的结果。
	 */

	const float Speed = Velocity.Size2D();
	const float LocWalkSpeed = CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = CurrentMovementSettings.RunSpeed;
	const float LocSprintSpeed = CurrentMovementSettings.SprintSpeed;

	if (Speed > LocRunSpeed)
	{
		return FMath::GetMappedRangeValueClamped({LocRunSpeed, LocSprintSpeed}, {2.0f, 3.0f}, Speed);
	}

	if (Speed > LocWalkSpeed)
	{
		return FMath::GetMappedRangeValueClamped({LocWalkSpeed, LocRunSpeed}, {1.0f, 2.0f}, Speed);
	}

	return FMath::GetMappedRangeValueClamped({0.0f, LocWalkSpeed}, {0.0f, 1.0f}, Speed);
}

/*
 * 从拥有者中获取移动数据
 */
void UALSCharacterMovementComponent::SetMovementSettings(FALSMovementSettings NewMovementSettings)
{
	CurrentMovementSettings = NewMovementSettings;
	/* 设置已更新数据 */
	bRequestMovementSettingsChange = true;
}

/*
 * 根据新设置步态设置最大步行速度(从所属客户端调用)
 */
void UALSCharacterMovementComponent::SetAllowedGait(EALSGait NewAllowedGait)
{
	if (AllowedGait == NewAllowedGait) return;

	/* 判断是否是由本地操作者操作 */
	if (PawnOwner->IsLocallyControlled())
	{
		AllowedGait = NewAllowedGait;
		/* 如果是联机状态，并且是真人操作，就将当前的操作传递给服务器进行更新 */
		if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
		{
			// client -> server 
			Server_SetAllowedGait(NewAllowedGait);
		}
		/* 状态已经更新 */
		bRequestMovementSettingsChange = true;
		
		return;
	}

	/* 如果当前不是服务器 */
	if (!PawnOwner->HasAuthority())
	{
		/* 更新最大行走速度 */
		const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
		MaxWalkSpeed = UpdateMaxWalkSpeed;
		MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;
	}
}
