// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Doğa Can Yanıkoğlu
// Contributors:    Haziq Fadhil, Drakynfly, CanisHelix


#include "Character/ALSBaseCharacter.h"


#include "Character/Animation/ALSCharacterAnimInstance.h"
#include "Character/Animation/ALSPlayerCameraBehavior.h"
#include "Library/ALSMathLibrary.h"
#include "Components/ALSDebugComponent.h"

#include "Components/CapsuleComponent.h"
#include "Curves/CurveFloat.h"
#include "Character/ALSCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


const FName NAME_FP_Camera(TEXT("FP_Camera"));
const FName NAME_Pelvis(TEXT("Pelvis"));
const FName NAME_pelvis(TEXT("pelvis"));
const FName NAME_head(TEXT("head"));
const FName NAME_RagdollPose(TEXT("RagdollPose"));
const FName NAME_RotationAmount(TEXT("RotationAmount"));
const FName NAME_ClimbRotationAmount(TEXT("ClimbRotationAmount"));
const FName NAME_YawOffset(TEXT("YawOffset"));
const FName NAME_root(TEXT("root"));
const FName NAME_spine_03(TEXT("spine_03"));


AALSBaseCharacter::AALSBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UALSCharacterMovementComponent>(CharacterMovementComponentName))
/*  初始化运动组件 */
{
	PrimaryActorTick.bCanEverTick = true;
	/* 不跟随控制的旋转而旋转 */
	bUseControllerRotationYaw = 0;
	/* 打开属性复制， 复制移动相关的属性 */
	bReplicates = true;
	SetReplicatingMovement(true);
	
	// 初始化角色默认的几种状态
	OverlayStates.Add(EALSOverlayState::Default);
	OverlayStates.Add(EALSOverlayState::Masculine);
	OverlayStates.Add(EALSOverlayState::Feminine);
	OverlayStates.Add(EALSOverlayState::Injured);
	OverlayStates.Add(EALSOverlayState::HandsTied);
}

void AALSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward/Backwards", this, &AALSBaseCharacter::PlayerForwardMovementInput);
	PlayerInputComponent->BindAxis("MoveRight/Left", this, &AALSBaseCharacter::PlayerRightMovementInput);
	PlayerInputComponent->BindAxis("LookUp/Down", this, &AALSBaseCharacter::PlayerCameraUpInput);
	PlayerInputComponent->BindAxis("LookLeft/Right", this, &AALSBaseCharacter::PlayerCameraRightInput);
	PlayerInputComponent->BindAction("JumpAction", IE_Pressed, this, &AALSBaseCharacter::JumpPressedAction);
	PlayerInputComponent->BindAction("JumpAction", IE_Released, this, &AALSBaseCharacter::JumpReleasedAction);
	PlayerInputComponent->BindAction("StanceAction", IE_Pressed, this, &AALSBaseCharacter::StancePressedAction);
	PlayerInputComponent->BindAction("WalkAction", IE_Pressed, this, &AALSBaseCharacter::WalkPressedAction);
	PlayerInputComponent->BindAction("RagdollAction", IE_Pressed, this, &AALSBaseCharacter::RagdollPressedAction);
	PlayerInputComponent->BindAction("SelectRotationMode_1", IE_Pressed, this,
	                                 &AALSBaseCharacter::VelocityDirectionPressedAction);
	PlayerInputComponent->BindAction("SelectRotationMode_2", IE_Pressed, this,
	                                 &AALSBaseCharacter::LookingDirectionPressedAction);
	PlayerInputComponent->BindAction("SprintAction", IE_Pressed, this, &AALSBaseCharacter::SprintPressedAction);
	PlayerInputComponent->BindAction("SprintAction", IE_Released, this, &AALSBaseCharacter::SprintReleasedAction);
	PlayerInputComponent->BindAction("AimAction", IE_Pressed, this, &AALSBaseCharacter::AimPressedAction);
	PlayerInputComponent->BindAction("AimAction", IE_Released, this, &AALSBaseCharacter::AimReleasedAction);
	PlayerInputComponent->BindAction("CameraAction", IE_Pressed, this, &AALSBaseCharacter::CameraPressedAction);
	PlayerInputComponent->BindAction("CameraAction", IE_Released, this, &AALSBaseCharacter::CameraReleasedAction);
}

/*
 * 在组件初始化之后
 * 在这个函数中将父类的运动组件转化为自定义的运动组件
 */
void AALSBaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	MyCharacterMovementComponent = Cast<UALSCharacterMovementComponent>(Super::GetMovementComponent());
}

/*
 * 与服务器进行数据同步，通过条件对数据复制进行约束，节约网络带宽。
 */
void AALSBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AALSBaseCharacter, TargetRagdollLocation);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, ReplicatedCurrentAcceleration, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, ReplicatedControlRotation, COND_SkipOwner);

	DOREPLIFETIME(AALSBaseCharacter, DesiredGait);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, DesiredStance, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, DesiredRotationMode, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(AALSBaseCharacter, RotationMode, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, OverlayState, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, ViewMode, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, VisibleMesh, COND_SkipOwner);
}

void AALSBaseCharacter::OnBreakfall_Implementation()
{
	Replicated_PlayMontage(GetRollAnimation(), 1.35);
}

void AALSBaseCharacter::Replicated_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	// Roll: Simply play a Root Motion Montage.
	if (MainAnimInstance)
	{
		MainAnimInstance->Montage_Play(Montage, PlayRate);
	}
	Server_PlayMontage(Montage, PlayRate);
}

void AALSBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 如果我们在网络游戏中，禁用曲线运动。
	bEnableNetworkOptimizations = !IsNetMode(NM_Standalone);

	// 确保mesh和动画蓝图在CharacterBP之后更新，以确保它得到最新的值。
	GetMesh()->AddTickPrerequisiteActor(this);

	// 设置移动数据模型
	SetMovementModel();

	// 强制更新状态使用所需的初始值。
	ForceUpdateCharacterState();

	// 对当前状态做出相应的初始化
	if (Stance == EALSStance::Standing)
	{
		UnCrouch();
	}
	else if (Stance == EALSStance::Crouching)
	{
		Crouch();
	}

	// 设置默认的旋转值。
	TargetRotation = GetActorRotation();
	LastVelocityRotation = TargetRotation;
	LastMovementInputRotation = TargetRotation;

	/* 如果是由服务器代理的，就关闭根运动 */
	if (MainAnimInstance && GetLocalRole() == ROLE_SimulatedProxy)
	{
		MainAnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
	}

	/* 在运动组件中设置对应的运动数据 */
	MyCharacterMovementComponent->SetMovementSettings(GetTargetMovementSettings());

	/* 在蓝图中添加该组件，然后在基类中实现加载该组件的逻辑 */
	ALSDebugComponent = FindComponentByClass<UALSDebugComponent>();
	ALSClimbComponent = FindComponentByClass<UALSMantleComponent>();
}

/*
 * 在组件初始化之前
 */
void AALSBaseCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	if (GetMesh())
	{
		MainAnimInstance = Cast<UALSCharacterAnimInstance>(GetMesh()->GetAnimInstance());
	}
}

void AALSBaseCharacter::SetAimYawRate(float NewAimYawRate)
{
	AimYawRate = NewAimYawRate;
	if (MainAnimInstance)
	{
		MainAnimInstance->GetCharacterInformationMutable().AimYawRate = AimYawRate;
	}
}
bool AALSBaseCharacter::GetShowTraces() const
{
	return ALSDebugComponent && ALSDebugComponent->GetShowTraces();
}

void AALSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 设置必要移动数据
	SetEssentialValues(DeltaTime);

	switch (MovementState)
	{
	case EALSMovementState::None: break;
	case EALSMovementState::Grounded:
		UpdateCharacterMovement();
		UpdateGroundedRotation(DeltaTime);
		break;
	case EALSMovementState::InAir:
		UpdateInAirRotation(DeltaTime);

		break;
	case EALSMovementState::Mantling: break;
	case EALSMovementState::Climbing: break;
	case EALSMovementState::Ragdoll:
		RagdollUpdate(DeltaTime);

		break;
	default: ;
	}

	// Cache values
	PreviousVelocity = GetVelocity();
	PreviousAimYaw = AimingRotation.Yaw;
}

void AALSBaseCharacter::RagdollStart()
{
	if (RagdollStateChangedDelegate.IsBound())
	{
		/* 如果有绑定的函数， 就进行多播， 通知它们，已经进入洋娃娃状态了 */
		RagdollStateChangedDelegate.Broadcast(true);
	}

	// /** When Networked, disables replicate movement reset TargetRagdollLocation and ServerRagdollPull variable
	// and if the host is a dedicated server, change character mesh optimisation option to avoid z-location bug*/
	// MyCharacterMovementComponent->bIgnoreClientMovementErrorChecksAndCorrection = 1;
	//
	// if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	// {
	// 	DefVisBasedTickOp = GetMesh()->VisibilityBasedAnimTickOption;
	// 	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	// }

	TargetRagdollLocation = GetMesh()->GetSocketLocation(NAME_Pelvis);
	ServerRagdollPull = 0;

	/* 步骤1:清除角色移动模式，设置移动状态为 Ragdoll */
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	SetMovementState(EALSMovementState::Ragdoll);

	/* 步骤2:禁用胶囊碰撞，并从骨盆开始启用网格物理模拟。 */
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(NAME_Pelvis, true, true);

	/* 第三步:停止任何活动的蒙太奇。 */
	if (MainAnimInstance)
	{
		MainAnimInstance->Montage_Stop(0.2f);
	}

	if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		SetReplicateMovement(false);
	}
}

/* 洋娃娃状态结束
 * 重新启用移动复制
 * 并且如果主机是一个专用的服务器设置网格可见性基于 anim tick选项回到默认
 */
void AALSBaseCharacter::RagdollEnd()
{
	// if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	// {
	// 	GetMesh()->VisibilityBasedAnimTickOption = DefVisBasedTickOp;
	// }
	//
	// // Revert back to default settings
	// MyCharacterMovementComponent->bIgnoreClientMovementErrorChecksAndCorrection = 0;
	// GetMesh()->bOnlyAllowAutonomousTickPose = false;

	if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		SetReplicateMovement(true);
	}

	/* 步骤1:保存当前布娃娃姿势的快照，以便在AnimGraph中使用，以混合布娃娃 */
	if (MainAnimInstance)
	{
		MainAnimInstance->SavePoseSnapshot(NAME_RagdollPose);
	}

	/* 步骤2:如果布娃娃在地上，将移动模式设置为行走，并播放Get Up动画。
	 * 如果不是，设置移动模式为下落，并更新角色的移动速度以匹配最后一个布娃娃的速度。
	 */
	if (bRagdollOnGround)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		if (MainAnimInstance)
		{
			MainAnimInstance->Montage_Play(GetGetUpAnimation(bRagdollFaceUp), 1.0f,
			                               EMontagePlayReturnType::MontageLength, 0.0f, true);
		}
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		GetCharacterMovement()->Velocity = LastRagdollVelocity;
	}

	/* 第三步:重新启用胶囊碰撞，并禁用网格上的物理模拟。 */
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetAllBodiesSimulatePhysics(false);

	/* 告诉其他地方，当前角色的洋娃娃功能结束了 */
	if (RagdollStateChangedDelegate.IsBound())
	{
		RagdollStateChangedDelegate.Broadcast(false);
	}
}

void AALSBaseCharacter::Server_SetMeshLocationDuringRagdoll_Implementation(FVector MeshLocation)
{
	TargetRagdollLocation = MeshLocation;
}

void AALSBaseCharacter::SetMovementState(const EALSMovementState NewState, bool bForce)
{
	if (bForce || MovementState != NewState)
	{
		PrevMovementState = MovementState;
		MovementState = NewState;
		OnMovementStateChanged(PrevMovementState);
	}
}

void AALSBaseCharacter::SetMovementAction(const EALSMovementAction NewAction, bool bForce)
{
	if (bForce || MovementAction != NewAction)
	{
		const EALSMovementAction Prev = MovementAction;
		MovementAction = NewAction;
		OnMovementActionChanged(Prev);
	}
}

void AALSBaseCharacter::SetStance(const EALSStance NewStance, bool bForce)
{
	if (bForce || Stance != NewStance)
	{
		const EALSStance Prev = Stance;
		Stance = NewStance;
		OnStanceChanged(Prev);
	}
}

void AALSBaseCharacter::SetGait(const EALSGait NewGait, bool bForce)
{
	/* 如果是强制性的或者和之前状态不同 */
	if (bForce || Gait != NewGait)
	{
		const EALSGait Prev = Gait;
		Gait = NewGait;
		OnGaitChanged(Prev);
	}
}


void AALSBaseCharacter::SetDesiredStance(EALSStance NewStance)
{
	DesiredStance = NewStance;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredStance(NewStance);
	}
}

void AALSBaseCharacter::Server_SetDesiredStance_Implementation(EALSStance NewStance)
{
	SetDesiredStance(NewStance);
}

void AALSBaseCharacter::SetDesiredGait(const EALSGait NewGait)
{
	DesiredGait = NewGait;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredGait(NewGait);
	}
}

void AALSBaseCharacter::SetDesiredLaddering(bool NewState)
{
	bDesiredLaddering = NewState;

	if (MainAnimInstance)
	{
		MainAnimInstance->bDesiredLaddering = bDesiredLaddering;
	}

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredLaddering(NewState);
	}
}

void AALSBaseCharacter::SetRotateInClimbAngle(float Angle)
{
	RotateInClimbAngle = Angle;

	if (MainAnimInstance)
	{
		MainAnimInstance->RotateInClimbAngle = RotateInClimbAngle;
	}

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetTurnInClimbAngle(Angle);
	}
}

void AALSBaseCharacter::Server_SetTurnInClimbAngle_Implementation(float Angle)
{
	SetRotateInClimbAngle(Angle);
}

void AALSBaseCharacter::SetClimbingType(EALSClimbingType NewType)
{
	ClimbingType = NewType;

	if (MainAnimInstance)
	{
		MainAnimInstance->ClimbingType = ClimbingType;
	}
}

void AALSBaseCharacter::Server_SetClimbingType_Implementation(EALSClimbingType NewType)
{
	SetClimbingType(NewType);
}


void AALSBaseCharacter::SetAnimClimbCornerParam(float TimeLength, float StartTime, float PlayRate)
{
	if (ClimbCornerAnimDelegate.IsBound())
	{
		/* 多播攀爬旋转蒙太奇参数 */
		ClimbCornerAnimDelegate.Broadcast(TimeLength, StartTime, PlayRate);
	}
}

void AALSBaseCharacter::Server_SetDesiredLaddering_Implementation(bool NewState)
{
	SetDesiredLaddering(NewState);
}


void AALSBaseCharacter::Server_SetDesiredGait_Implementation(EALSGait NewGait)
{
	SetDesiredGait(NewGait);
}

void AALSBaseCharacter::SetDesiredRotationMode(EALSRotationMode NewRotMode)
{
	DesiredRotationMode = NewRotMode;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredRotationMode(NewRotMode);
	}
}

void AALSBaseCharacter::Server_SetDesiredRotationMode_Implementation(EALSRotationMode NewRotMode)
{
	SetDesiredRotationMode(NewRotMode);
}

void AALSBaseCharacter::SetRotationMode(const EALSRotationMode NewRotationMode, bool bForce)
{
	if (bForce || RotationMode != NewRotationMode)
	{
		const EALSRotationMode Prev = RotationMode;
		RotationMode = NewRotationMode;
		OnRotationModeChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetRotationMode(NewRotationMode, bForce);
		}
	}
}


void AALSBaseCharacter::Server_SetRotationMode_Implementation(EALSRotationMode NewRotationMode, bool bForce)
{
	SetRotationMode(NewRotationMode, bForce);
}

void AALSBaseCharacter::SetViewMode(const EALSViewMode NewViewMode, bool bForce)
{
	if (bForce || ViewMode != NewViewMode)
	{
		const EALSViewMode Prev = ViewMode;
		ViewMode = NewViewMode;
		OnViewModeChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetViewMode(NewViewMode, bForce);
		}
	}
}

void AALSBaseCharacter::SetCanInputMove(bool NewState, bool bForce)
{
	if (bForce || bCanInputMove != NewState)
	{
		bCanInputMove = NewState;

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetCanInputMove(NewState, bForce);
		}
	}
}

void AALSBaseCharacter::Server_SetCanInputMove_Implementation(bool NewState, bool bForce)
{
	SetCanInputMove(NewState, bForce);
}

void AALSBaseCharacter::Server_SetViewMode_Implementation(EALSViewMode NewViewMode, bool bForce)
{
	SetViewMode(NewViewMode, bForce);
}

void AALSBaseCharacter::SetOverlayState(const EALSOverlayState NewState, bool bForce)
{
	if (bForce || OverlayState != NewState)
	{
		const EALSOverlayState Prev = OverlayState;
		OverlayState = NewState;
		OnOverlayStateChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetOverlayState(NewState, bForce);
		}
	}
}


void AALSBaseCharacter::Server_SetOverlayState_Implementation(EALSOverlayState NewState, bool bForce)
{
	SetOverlayState(NewState, bForce);
}

void AALSBaseCharacter::EventOnLanded()
{
	/* 获得Z轴的速度 */
	const float VelZ = FMath::Abs(GetCharacterMovement()->Velocity.Z);

	// 如果是洋娃娃
	if (bRagdollOnLand && VelZ > RagdollOnLandVelocity)
	{
		ReplicatedRagdollStart();
	}
	else if (bBreakfallOnLand && bHasMovementInput && VelZ >= BreakfallOnLandVelocity)
	{
		// 默认是播放滚动动画
		OnBreakfall();
	}
	else
	{
		// 设置当前摩擦系数
		GetCharacterMovement()->BrakingFrictionFactor = bHasMovementInput ? 0.5f : 3.0f;

		// 0.5秒后，设置摩擦系数为零
		GetWorldTimerManager().SetTimer(OnLandedFrictionResetTimer, this,
		                                &AALSBaseCharacter::OnLandFrictionReset, 0.5f, false);
	}
}

void AALSBaseCharacter::Multicast_OnLanded_Implementation()
{
	if (!IsLocallyControlled())
	{
		EventOnLanded();
	}
}

/*
 * 刚开始跳跃的时候，在本地角色操作的情况下会触发该函数。
 */
void AALSBaseCharacter::EventOnJumped()
{
	/* 当速度超过100的时候，就用速度方向的旋转值，否则就用原来的旋转值 */
	InAirRotation = Speed > 100.0f ? LastVelocityRotation : GetActorRotation();

	if (MainAnimInstance)
	{
		/* 触发动画实例的跳跃函数 */
		MainAnimInstance->OnJumped();
	}
}

void AALSBaseCharacter::AddOverlayState(EALSOverlayState State) 
{
	if (OverlayStates.Find(State) < 0)
	{
		OverlayStates.Add(State);
	}
}

void AALSBaseCharacter::Server_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (MainAnimInstance)
	{
		MainAnimInstance->Montage_Play(Montage, PlayRate);
	}

	/* 强制更新数据 */
	ForceNetUpdate();
	Multicast_PlayMontage(Montage, PlayRate);
}

void AALSBaseCharacter::Multicast_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (MainAnimInstance && !IsLocallyControlled())
	{
		MainAnimInstance->Montage_Play(Montage, PlayRate);
	}
}

void AALSBaseCharacter::Multicast_OnJumped_Implementation()
{
	if (!IsLocallyControlled())
	{
		EventOnJumped();
	}
}

void AALSBaseCharacter::Server_RagdollStart_Implementation()
{
	Multicast_RagdollStart();
}

void AALSBaseCharacter::Multicast_RagdollStart_Implementation()
{
	RagdollStart();
}

void AALSBaseCharacter::Server_RagdollEnd_Implementation(FVector CharacterLocation)
{
	Multicast_RagdollEnd(CharacterLocation);
}

void AALSBaseCharacter::Multicast_RagdollEnd_Implementation(FVector CharacterLocation)
{
	RagdollEnd();
}

void AALSBaseCharacter::SetActorLocationAndTargetRotation(FVector NewLocation, FRotator NewRotation)
{
	SetActorLocationAndRotation(NewLocation, NewRotation);
	TargetRotation = NewRotation;
}

/*
 * 加载数据驱动表
 */
void AALSBaseCharacter::SetMovementModel()
{
	const FString ContextString = GetFullName();
	const FALSMovementStateSettings* OutRow =
		MovementModel.DataTable->FindRow<FALSMovementStateSettings>(MovementModel.RowName, ContextString);
	check(OutRow);
	MovementData = *OutRow;
}

/*
 * 强制更新状态使用所需的初始值。
 */
void AALSBaseCharacter::ForceUpdateCharacterState()
{
	SetGait(DesiredGait, true);
	SetStance(DesiredStance, true);
	SetRotationMode(DesiredRotationMode, true);
	SetViewMode(ViewMode, true);
	SetOverlayState(OverlayState, true);
	SetMovementState(MovementState, true);
	SetMovementAction(MovementAction, true);
}

void AALSBaseCharacter::SetHasMovementInput(bool bNewHasMovementInput)
{
	bHasMovementInput = bNewHasMovementInput;

	if (MainAnimInstance)
	{
		MainAnimInstance->GetCharacterInformationMutable().bHasMovementInput = bHasMovementInput;
	}
}

/*
 * 获取对应的运动数据
 */
FALSMovementSettings AALSBaseCharacter::GetTargetMovementSettings() const
{
	if (RotationMode == EALSRotationMode::VelocityDirection)
	{
		if (Stance == EALSStance::Standing)
		{
			return MovementData.VelocityDirection.Standing;
		}
		if (Stance == EALSStance::Crouching)
		{
			return MovementData.VelocityDirection.Crouching;
		}
	}
	else if (RotationMode == EALSRotationMode::LookingDirection)
	{
		if (Stance == EALSStance::Standing)
		{
			return MovementData.LookingDirection.Standing;
		}
		if (Stance == EALSStance::Crouching)
		{
			return MovementData.LookingDirection.Crouching;
		}
	}
	else if (RotationMode == EALSRotationMode::Aiming)
	{
		if (Stance == EALSStance::Standing)
		{
			return MovementData.Aiming.Standing;
		}
		if (Stance == EALSStance::Crouching)
		{
			return MovementData.Aiming.Crouching;
		}
	}

	// Default to velocity dir standing
	return MovementData.VelocityDirection.Standing;
}

bool AALSBaseCharacter::CanSprint() const
{
	/*
	 * 根据旋转模式和当前加速度(输入)旋转来判断角色当前是否能够冲刺。
	 * 如果角色处于旋转模式，只允许在有完整的运动输入并且相对于摄像机正对或负50度的情况下进行冲刺。
	 */

	/* 没有输入值 或者 处于瞄准模式的时候不允许冲刺 */
	if (!bHasMovementInput || RotationMode == EALSRotationMode::Aiming)
	{
		return false;
	}

	const bool bValidInputAmount = MovementInputAmount > 0.9f;

	if (RotationMode == EALSRotationMode::VelocityDirection)
	{
		return bValidInputAmount;
	}

	if (RotationMode == EALSRotationMode::LookingDirection)
	{
		const FRotator AccRot = ReplicatedCurrentAcceleration.ToOrientationRotator();
		FRotator Delta = AccRot - AimingRotation;
		Delta.Normalize();

		return bValidInputAmount && FMath::Abs(Delta.Yaw) < 50.0f;
	}

	return false;
}

void AALSBaseCharacter::SetIsMoving(bool bNewIsMoving)
{
	bIsMoving = bNewIsMoving;

	if (MainAnimInstance)
	{
		MainAnimInstance->GetCharacterInformationMutable().bIsMoving = bIsMoving;
	}
}

FVector AALSBaseCharacter::GetMovementInput() const
{
	return ReplicatedCurrentAcceleration;
}

void AALSBaseCharacter::SetMovementInputAmount(float NewMovementInputAmount)
{
	MovementInputAmount = NewMovementInputAmount;

	if (MainAnimInstance)
	{
		MainAnimInstance->GetCharacterInformationMutable().MovementInputAmount = MovementInputAmount;
	}
}

/* 更新水平速度*/
void AALSBaseCharacter::SetSpeed(float NewSpeed)
{
	Speed = NewSpeed;

	/* 如果有动画实例，同时也要将角色的速度信息进行更新 */
	if (MainAnimInstance)
	{
		MainAnimInstance->GetCharacterInformationMutable().Speed = Speed;
	}
}

float AALSBaseCharacter::GetAnimCurveValue(FName CurveName) const
{
	if (MainAnimInstance)
	{
		return MainAnimInstance->GetCurveValue(CurveName);
	}

	return 0.0f;
}

void AALSBaseCharacter::SetVisibleMesh(USkeletalMesh* NewVisibleMesh)
{
	if (VisibleMesh != NewVisibleMesh)
	{
		const USkeletalMesh* Prev = VisibleMesh;
		VisibleMesh = NewVisibleMesh;
		OnVisibleMeshChanged(Prev);

		if (GetLocalRole() != ROLE_Authority)
		{
			Server_SetVisibleMesh(NewVisibleMesh);
		}
	}
}

void AALSBaseCharacter::Server_SetVisibleMesh_Implementation(USkeletalMesh* NewVisibleMesh)
{
	SetVisibleMesh(NewVisibleMesh);
}

void AALSBaseCharacter::SetRightShoulder(bool bNewRightShoulder)
{
	bRightShoulder = bNewRightShoulder;
	if (CameraBehavior)
	{
		CameraBehavior->bRightShoulder = bRightShoulder;
	}
}

ECollisionChannel AALSBaseCharacter::GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius)
{
	TraceOrigin = GetActorLocation();
	TraceRadius = 10.0f;
	return ECC_Visibility;
}

FTransform AALSBaseCharacter::GetThirdPersonPivotTarget()
{
	return GetActorTransform();
}

FVector AALSBaseCharacter::GetFirstPersonCameraTarget()
{
	return GetMesh()->GetSocketLocation(NAME_FP_Camera);
}

void AALSBaseCharacter::GetCameraParameters(float& TPFOVOut, float& FPFOVOut, bool& bRightShoulderOut) const
{
	TPFOVOut = ThirdPersonFOV;
	FPFOVOut = FirstPersonFOV;
	bRightShoulderOut = bRightShoulder;
}

void AALSBaseCharacter::SetAcceleration(const FVector& NewAcceleration)
{
	Acceleration = (NewAcceleration != FVector::ZeroVector || IsLocallyControlled())
		               ? NewAcceleration
		               : Acceleration / 2;

	if (MainAnimInstance)
	{
		MainAnimInstance->GetCharacterInformationMutable().Acceleration = Acceleration;
	}
}

/*
 * 洋娃娃状态的更新函数
 */
void AALSBaseCharacter::RagdollUpdate(float DeltaTime)
{
	/* 获得单个物体的线速度 */
	const FVector NewRagdollVel = GetMesh()->GetPhysicsLinearVelocity(NAME_root);
	// 设置 the Last Ragdoll Velocity.
	LastRagdollVelocity = NewRagdollVel != FVector::ZeroVector || IsLocallyControlled()
		                      ? NewRagdollVel
		                      : LastRagdollVelocity / 2;

	/* 使用Ragdoll Velocity来缩放Ragdoll的关节强度的物理动画。 */
	const float SpringValue = FMath::GetMappedRangeValueClamped({0.0f, 1000.0f}, {0.0f, 25000.0f},
	                                                            LastRagdollVelocity.Size());
	GetMesh()->SetAllMotorsAngularDriveParams(SpringValue, 0.0f, 0.0f, false);

	/*
	 * 如果下落速度超过-4000，则取消重力，防止持续加速。
	 * 这也可以防止布娃娃穿过地板。
	 */
	const bool bEnableGrav = LastRagdollVelocity.Z > -4000.0f;
	GetMesh()->SetEnableGravity(bEnableGrav);

	// 更新 Actor位置以跟踪 ragdoll。
	SetActorLocationDuringRagdoll(DeltaTime);
}

void AALSBaseCharacter::SetActorLocationDuringRagdoll(float DeltaTime)
{
	if (IsLocallyControlled())
	{
		/* 设置骨盆为目标位置。 */
		TargetRagdollLocation = GetMesh()->GetSocketLocation(NAME_Pelvis);
		if (!HasAuthority())
		{
			/* 如果不是服务器，就发送到服务器，执行更新actor位置*/
			Server_SetMeshLocationDuringRagdoll(TargetRagdollLocation);
		}
	}

	/* 确定布娃娃是朝上还是朝下，并相应地设置目标旋转。 */
	const FRotator PelvisRot = GetMesh()->GetSocketRotation(NAME_Pelvis);

	/* 判断盆骨是那一边， 如果翻转的盆骨计算脸朝什么方向就反着来计算 */
	if (bReversedPelvis)
	{
		bRagdollFaceUp = PelvisRot.Roll > 0.0f;
	}
	else
	{
		bRagdollFaceUp = PelvisRot.Roll < 0.0f;
	}

	// const FRotator TargetRagdollRotation = FRotator(0.f, bRagdollFaceUp ? PelvisRot.Yaw - 180.0f : PelvisRot.Yaw, 0.0f);
	FRotator TargetRagdollRotation(0.f, bRagdollFaceUp ? PelvisRot.Yaw - 180.0f : PelvisRot.Yaw, 0.f);

	/* 从目标位置向下跟踪以偏移目标位置，防止布娃娃躺在地上时胶囊的下半部分穿过地板。 */
	const FVector TraceVect(TargetRagdollLocation.X, TargetRagdollLocation.Y,
	                        TargetRagdollLocation.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(HitResult, TargetRagdollLocation, TraceVect,
	                                                  ECC_Visibility, Params);

	if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
	{
		UALSDebugComponent::DrawDebugLineTraceSingle(World,
		                                             TargetRagdollLocation,
		                                             TraceVect,
		                                             EDrawDebugTrace::Type::ForOneFrame,
		                                             bHit,
		                                             HitResult,
		                                             FLinearColor::Red,
		                                             FLinearColor::Green,
		                                             1.0f);
	}

	/* 判断角色是否处于地面 */
	bRagdollOnGround = HitResult.IsValidBlockingHit();
	FVector NewRagdollLoc = TargetRagdollLocation;

	if (bRagdollOnGround)
	{
		/* 如果处于地面 */

		/* 计算和地面相差距离 */
		const float ImpactDistZ = FMath::Abs(HitResult.ImpactPoint.Z - HitResult.TraceStart.Z);
		NewRagdollLoc.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - ImpactDistZ + 2.0f;
		// TargetRagdollRotation = UKismetMathLibrary::MakeRotFromZ(
		// 	GetMesh()->GetSocketLocation(NAME_head) - GetMesh()->GetSocketLocation(NAME_Pelvis));
	}

	/* 如果不是本地控制器 */
	if (!IsLocallyControlled())
	{
		/* 设置服务器洋娃娃拉力 */
		ServerRagdollPull = FMath::FInterpTo(ServerRagdollPull, 750.0f, DeltaTime, 0.6f);
		/* 水平速度大小 */
		float RagdollSpeed = FVector(LastRagdollVelocity.X, LastRagdollVelocity.Y, 0).Size();
		/* 确定拉动的骨骼名称 */
		FName RagdollSocketPullName = RagdollSpeed > 300 ? NAME_spine_03 : NAME_pelvis;
		/* 赋予 mesh 拉力 */
		GetMesh()->AddForce(
			(TargetRagdollLocation - GetMesh()->GetSocketLocation(RagdollSocketPullName)) * ServerRagdollPull,
			RagdollSocketPullName, true);
	}

	// 更新角色旋转以及对应位置
	SetActorLocationAndTargetRotation(bRagdollOnGround ? NewRagdollLoc : TargetRagdollLocation, TargetRagdollRotation);
}

void AALSBaseCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	/* 使用角色移动模式的改变来设置移动状态为正确的值。
	 * 这允许你拥有一组自定义的移动状态，但仍然使用默认角色移动组件的功能。
	 */
	if (GetCharacterMovement()->MovementMode == MOVE_Walking ||
		GetCharacterMovement()->MovementMode == MOVE_NavWalking)
	{
		SetMovementState(EALSMovementState::Grounded);
	}
	else if (GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		SetMovementState(EALSMovementState::InAir);
	}
}

void AALSBaseCharacter::OnMovementStateChanged(const EALSMovementState PreviousState)
{
	if (MainAnimInstance)
	{
		FALSAnimCharacterInformation& AnimData = MainAnimInstance->GetCharacterInformationMutable();
		AnimData.PrevMovementState = PrevMovementState;
		MainAnimInstance->MovementState = MovementState;
	}
	
	if (MovementState == EALSMovementState::Grounded || MovementState == EALSMovementState::InAir)
	{
		bCanInputMove = true;
	}
	else
	{
		bCanInputMove = false;
	}
	
	if (MovementState == EALSMovementState::InAir)
	{
		if (MovementAction == EALSMovementAction::None)
		{
			// If the character enters the air, set the In Air Rotation and uncrouch if crouched.
			InAirRotation = GetActorRotation();
			if (Stance == EALSStance::Crouching)
			{
				UnCrouch();
			}
		}
		else if (MovementAction == EALSMovementAction::Rolling)
		{
			// If the character is currently rolling, enable the ragdoll.
			ReplicatedRagdollStart();
		}
	}

	if (CameraBehavior)
	{
		CameraBehavior->MovementState = MovementState;
	}
}

void AALSBaseCharacter::OnMovementActionChanged(const EALSMovementAction PreviousAction)
{
	if (MainAnimInstance)
	{
		MainAnimInstance->MovementAction = MovementAction;
	}


	switch (MovementAction)
	{

	case EALSMovementAction::None: break;
	case EALSMovementAction::LowMantle: break;
	case EALSMovementAction::HighMantle: break;
	case EALSMovementAction::Rolling:

		// Make the character crouch if performing a roll.
		Crouch();
		break;

	case EALSMovementAction::GettingUp: break;
	case EALSMovementAction::CornerClimbing:
		
		if (MainAnimInstance)
		{
			MainAnimInstance->StartCornerClimb(RightInputValue > .2f, RotateInClimbAngle, .8f, 0.f, false);
		}
		
		break;
	default: ;
	}
	
	if (PreviousAction == EALSMovementAction::Rolling)
	{
		if (DesiredStance == EALSStance::Standing)
		{
			UnCrouch();
		}
		else if (DesiredStance == EALSStance::Crouching)
		{
			Crouch();
		}
	}
	/*else if (PreviousAction == EALSMovementAction::ClimbJumping)
	{
		ALSClimbComponent->ClimbJumpEnd();
	}*/

	if (CameraBehavior)
	{
		CameraBehavior->MovementAction = MovementAction;
	}
}

void AALSBaseCharacter::OnStanceChanged(const EALSStance PreviousStance)
{
	if (MainAnimInstance)
	{
		MainAnimInstance->Stance = Stance;
	}

	if (CameraBehavior)
	{
		CameraBehavior->Stance = Stance;
	}

	MyCharacterMovementComponent->SetMovementSettings(GetTargetMovementSettings());
}

void AALSBaseCharacter::OnRotationModeChanged(EALSRotationMode PreviousRotationMode)
{
	if (MainAnimInstance)
	{
		MainAnimInstance->RotationMode = RotationMode;
	}

	//如果新的旋转模式是速度方向和角色在第一人称，设置viewmode为第三人称。
	if (RotationMode == EALSRotationMode::VelocityDirection && ViewMode == EALSViewMode::FirstPerson)
	{
		SetViewMode(EALSViewMode::ThirdPerson);
	}

	if (CameraBehavior)
	{
		CameraBehavior->SetRotationMode(RotationMode);
	}

	/* 运动模式发生改变的时候，对应的数据也要发生改变。 */
	MyCharacterMovementComponent->SetMovementSettings(GetTargetMovementSettings());
}

void AALSBaseCharacter::OnGaitChanged(const EALSGait PreviousGait)
{
	if (MainAnimInstance)
	{
		MainAnimInstance->Gait = Gait;
	}

	if (CameraBehavior)
	{
		CameraBehavior->Gait = Gait;
	}
}

void AALSBaseCharacter::OnViewModeChanged(const EALSViewMode PreviousViewMode)
{
	if (MainAnimInstance)
	{
		MainAnimInstance->GetCharacterInformationMutable().ViewMode = ViewMode;
	}

	if (ViewMode == EALSViewMode::ThirdPerson)
	{
		if (RotationMode == EALSRotationMode::VelocityDirection || RotationMode == EALSRotationMode::LookingDirection)
		{
			// If Third Person, set the rotation mode back to the desired mode.
			SetRotationMode(DesiredRotationMode);
		}
	}
	else if (ViewMode == EALSViewMode::FirstPerson && RotationMode == EALSRotationMode::VelocityDirection)
	{
		// If First Person, set the rotation mode to looking direction if currently in the velocity direction mode.
		SetRotationMode(EALSRotationMode::LookingDirection);
	}

	if (CameraBehavior)
	{
		CameraBehavior->ViewMode = ViewMode;
	}
}

void AALSBaseCharacter::OnOverlayStateChanged(const EALSOverlayState PreviousState)
{
	if (MainAnimInstance)
	{
		MainAnimInstance->OverlayState = OverlayState;
		MainAnimInstance->LastOverlayState = PreviousState;
	}
}

void AALSBaseCharacter::OnVisibleMeshChanged(const USkeletalMesh* PrevVisibleMesh)
{
	// 更新骨骼网格之前，我们更新材料和animbp变量
	GetMesh()->SetSkeletalMesh(VisibleMesh);

	// 将材质重置为新的网格默认值
	if (GetMesh() != nullptr)
	{
		for (int32 MaterialIndex = 0; MaterialIndex < GetMesh()->GetNumMaterials(); ++MaterialIndex)
		{
			GetMesh()->SetMaterial(MaterialIndex, nullptr);
		}
	}

	// 强制设置变量。这确保动画实例和角色在网格变化时保持同步
	ForceUpdateCharacterState();
}

void AALSBaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStance(EALSStance::Crouching);
}

void AALSBaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStance(EALSStance::Standing);
}

/*
 * 在刚开始跳跃的时候触发该函数
 */
void AALSBaseCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	if (IsLocallyControlled())
	{
		/* 如果是本地控制器控制的， 那就运行跳跃事件 */
		EventOnJumped();
	}
	if (HasAuthority())
	{
		/* 如果是服务器的跳跃， 那就进行多播 */
		Multicast_OnJumped();
	}
}

/*
 * 落地时，根据命中结果执行动作。
 * 触发EventOnLanded。
 * 注意在这个事件中移动模式仍然是“Falling”。
 * 当前速度值是着陆时的速度。
 * 也可以考虑OnMovementModeChanged()，因为它可以在移动模式改变为新模式时使用(最有可能是Walking)。
 */
void AALSBaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	// 当着陆的时候
	if (IsLocallyControlled())
	{
		EventOnLanded();
	}
	if (HasAuthority())
	{
		Multicast_OnLanded();
	}
}

void AALSBaseCharacter::OnLandFrictionReset()
{
	// 摩擦力复位
	GetCharacterMovement()->BrakingFrictionFactor = 0.0f;
}

/*
 * 设置角色运动需要的变量值
 */
void AALSBaseCharacter::SetEssentialValues(float DeltaTime)
{
	/* 当前角色不是由服务器模拟的 */
	if (GetLocalRole() != ROLE_SimulatedProxy)
	{
		/* 更新相关旋转值和加速度 */
		ReplicatedCurrentAcceleration = GetCharacterMovement()->GetCurrentAcceleration();
		ReplicatedControlRotation = GetControlRotation();
		EasedMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration();
	}

	/*
	 * 当前角色是由服务器模拟的，有些运动数据并不会传递过来，
	 * 所以当角色的最大加速度为零的时候就将当前加速度/2，形成一个缓冲，
	 * 让角色有一个光滑的过渡。
	 */
	else
	{
		EasedMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration() != 0
			                       ? GetCharacterMovement()->GetMaxAcceleration()
			                       : EasedMaxAcceleration / 2;
	}

	// 让当前值到目标值有一个光滑的过渡
	AimingRotation = FMath::RInterpTo(AimingRotation, ReplicatedControlRotation, DeltaTime, 30);

	/*
	 * 这些值表示胶囊如何移动以及它想要如何移动，
	 * 因此对于任何数据驱动的动画系统来说都是必不可少的。
	 * 它们也被用于整个系统的各种功能，
	 * 所以将它们都聚集在这一块，方便管理。
	 */

	const FVector CurrentVel = GetVelocity();

	// Set the amount of Acceleration.
	SetAcceleration((CurrentVel - PreviousVelocity) / DeltaTime);

	/* 更新的速度是水平方向上的速度 */
	SetSpeed(CurrentVel.Size2D());
	SetIsMoving(Speed > 1.0f);

	if (bIsMoving)
	{
		/* 将当前速度所指向的旋转值设置为上次速度的旋转值 */
		LastVelocityRotation = CurrentVel.ToOrientationRotator();
	}

	/*
	 * 通过获取角色的移动输入量来确定角色是否有移动输入。
	 * 移动输入量等于当前加速度除以最大加速度，
	 * 所以它的范围是0-1,1是可能的最大输入量，0是没有的。
	 * 如果角色有移动输入，更新最后移动输入旋转。
	 */
	SetMovementInputAmount(ReplicatedCurrentAcceleration.Size() / EasedMaxAcceleration);
	SetHasMovementInput(MovementInputAmount > 0.0f);
	if (bHasMovementInput)
	{
		LastMovementInputRotation = ReplicatedCurrentAcceleration.ToOrientationRotator();
	}

	/*
	 * 通过比较当前和之前的目标偏航值来设置目标偏航速率，除以DeltaTime。
	 * 这表示相机从左到右旋转的速度。
	 */
	SetAimYawRate(FMath::Abs((AimingRotation.Yaw - PreviousAimYaw) / DeltaTime));
}

void AALSBaseCharacter::UpdateCharacterMovement()
{
	// Set the Allowed Gait
	const EALSGait AllowedGait = GetAllowedGait();

	/* 确定实际步态。如果它不同于当前的步态，设置新的步态事件。 */
	const EALSGait ActualGait = GetActualGait(AllowedGait);

	if (ActualGait != Gait)
	{
		SetGait(ActualGait);
	}

	/* 根据当前允许的步态，更新角色最大行走速度到配置的速度。 */
	MyCharacterMovementComponent->SetAllowedGait(AllowedGait);
}

/*
 * 更新地面相关的值
 */
void AALSBaseCharacter::UpdateGroundedRotation(float DeltaTime)
{
	if (MovementAction == EALSMovementAction::None)
	{
		/* 判断是否 还在运动并且没有任何根运动 */
		if ((bIsMoving && bHasMovementInput || Speed > 150.0f) && !HasAnyRootMotion())
		{
			/* 获取地面旋转速率 */
			const float GroundedRotationRate = CalculateGroundedRotationRate();

			if (RotationMode == EALSRotationMode::VelocityDirection)
			{
				// 跟随速度方向的旋转就要过渡块一点
				SmoothCharacterRotation({0.0f, LastVelocityRotation.Yaw, 0.0f}, 800.0f, GroundedRotationRate,
				                        DeltaTime);
			}
			else if (RotationMode == EALSRotationMode::LookingDirection)
			{
				float YawValue;
				/* 如果是冲刺模式就过渡的快一些 */
				if (Gait == EALSGait::Sprinting)
				{
					YawValue = LastVelocityRotation.Yaw;
				}
				/* 其他情况就过渡的慢一些 */
				else
				{
					// Walking or Running..
					/* 获取每次旋转增加的差值 */
					const float YawOffsetCurveVal = MainAnimInstance
						                                ? MainAnimInstance->GetCurveValue(NAME_YawOffset)
						                                : 0.f;
					YawValue = AimingRotation.Yaw + YawOffsetCurveVal;
				}
				SmoothCharacterRotation({0.0f, YawValue, 0.0f}, 500.0f, GroundedRotationRate, DeltaTime);
			}
			else if (RotationMode == EALSRotationMode::Aiming)
			{
				/* 因为瞄准模式要跟随控制器的值，控制器旋转值要迅速跟上，才不会觉得冲突，所以就要旋转的更快了。 */
				SmoothCharacterRotation({0.0f, AimingRotation.Yaw, 0.0f}, 1000.0f, 20.0f, DeltaTime);
			}
		}

		/* 不在 运动或者处于根运动的状态 */
		else
		{
			/* 设置关于原地旋转的相关属性 */

			/* 如果处于第一人称， 或者第三人称的瞄准状态 就要确保当前旋转的值和目标旋转值之间的差值相差太多 */
			if (ViewMode == EALSViewMode::ThirdPerson && RotationMode == EALSRotationMode::Aiming ||
				ViewMode == EALSViewMode::FirstPerson)
			{
				LimitRotation(-100.0f, 100.0f, 20.0f, DeltaTime);
			}

			/*
			 * 应用原地旋转动画中的RotationAmount曲线。
			 * 旋转量曲线定义了每帧应该应用多少旋转，
			 * 这是为30fps动画计算的。
			 */
			const float RotAmountCurve = MainAnimInstance ? MainAnimInstance->GetCurveValue(NAME_RotationAmount) : 0.f;

			if (FMath::Abs(RotAmountCurve) > 0.001f)
			{
				if (GetLocalRole() == ROLE_AutonomousProxy)
				{
					/* 获得一个在 [-180, 180] 之间的值 */
					TargetRotation.Yaw = FRotator::NormalizeAxis(
						TargetRotation.Yaw + RotAmountCurve * (DeltaTime * 30.0f));
					SetActorRotation(TargetRotation);
				}
				else
				{
					AddActorWorldRotation({0, RotAmountCurve * (DeltaTime / (1.0f / 30.0f)), 0});
				}
				TargetRotation = GetActorRotation();
			}
		}
	} // 无动作 end 
	else if (MovementAction == EALSMovementAction::Rolling)
	{
		/* 如果是处于翻滚状态 */
		// 在处于单机游戏和有运动输入的时候更新角色旋转
		if (!bEnableNetworkOptimizations && bHasMovementInput)
		{
			SmoothCharacterRotation({0.0f, LastMovementInputRotation.Yaw, 0.0f}, 0.0f, 2.0f, DeltaTime);
		}
	}

	// Other actions are ignored...
}

/*
 * 更新在空中的旋转值
 */
void AALSBaseCharacter::UpdateInAirRotation(float DeltaTime)
{
	if (RotationMode == EALSRotationMode::VelocityDirection || RotationMode == EALSRotationMode::LookingDirection)
	{
		// Velocity / Looking Direction Rotation
		SmoothCharacterRotation({0.0f, InAirRotation.Yaw, 0.0f}, 0.0f, 5.0f, DeltaTime);
	}
	else if (RotationMode == EALSRotationMode::Aiming)
	{
		// Aiming Rotation
		SmoothCharacterRotation({0.0f, AimingRotation.Yaw, 0.0f}, 0.0f, 15.0f, DeltaTime);
		InAirRotation = GetActorRotation();
	}
}

/*
 * 计算允许步态。
 * 这代表角色当前允许的最大步态，可以由所需的步态、旋转模式、姿态等决定。
 * 如果你想强迫角色在室内行走，这可以在这里完成。
 */
EALSGait AALSBaseCharacter::GetAllowedGait() const
{
	if (Stance == EALSStance::Standing)
	{
		if (RotationMode != EALSRotationMode::Aiming)
		{
			if (DesiredGait == EALSGait::Sprinting)
			{
				return CanSprint() ? EALSGait::Sprinting : EALSGait::Running;
			}
			return DesiredGait;
		}
	}

	// Crouching stance & Aiming rot mode has same behaviour

	if (DesiredGait == EALSGait::Sprinting)
	{
		return EALSGait::Running;
	}

	return DesiredGait;
}

EALSGait AALSBaseCharacter::GetActualGait(EALSGait AllowedGait) const
{
	/*
	 * 得到实际的步态。
	 * 这是由角色的实际移动计算出来的， 所以它可以不同于期望的步态或允许的步态。
	 * 例如，如果允许的步态变成了行走，那么实际的步态将仍然是奔跑，直到角色减速到行走的速度。
	 *
	 * 向可允许的步态慢慢过渡
	 */

	const float LocWalkSpeed = MyCharacterMovementComponent->CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = MyCharacterMovementComponent->CurrentMovementSettings.RunSpeed;

	if (Speed > LocRunSpeed + 10.0f)
	{
		if (AllowedGait == EALSGait::Sprinting)
		{
			return EALSGait::Sprinting;
		}
		return EALSGait::Running;
	}

	if (Speed >= LocWalkSpeed + 10.0f)
	{
		return EALSGait::Running;
	}

	return EALSGait::Walking;
}

/*
 * 让角角色光滑的移动到指定的旋转值
 */
void AALSBaseCharacter::SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed,
                                                float DeltaTime)
{
	TargetRotation =
		FMath::RInterpConstantTo(TargetRotation, Target, DeltaTime, TargetInterpSpeed);
	SetActorRotation(
		FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, ActorInterpSpeed));
}

float AALSBaseCharacter::CalculateGroundedRotationRate() const
{
	/*
	 * 使用移动设置中的当前旋转速率曲线计算旋转速率。
	 * 将曲线与映射的速度结合使用，可以对每个速度的旋转速率进行高水平的控制。
	 * 增加速度，如果相机是快速旋转更有反应的旋转。
	 */
	const float MappedSpeedVal = MyCharacterMovementComponent->GetMappedSpeed();
	const float CurveVal =
		MyCharacterMovementComponent->CurrentMovementSettings.RotationRateCurve->GetFloatValue(MappedSpeedVal);
	const float ClampedAimYawRate = FMath::GetMappedRangeValueClamped({0.0f, 300.0f}, {1.0f, 3.0f}, AimYawRate);
	return CurveVal * ClampedAimYawRate;
}

/*
 * 确保角色当前旋转值和目标旋转值不超过一定的范围
 */
void AALSBaseCharacter::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime)
{
	FRotator Delta = AimingRotation - GetActorRotation();
	Delta.Normalize();
	const float RangeVal = Delta.Yaw;

	if (RangeVal < AimYawMin || RangeVal > AimYawMax)
	{
		const float ControlRotYaw = AimingRotation.Yaw;
		const float TargetYaw = ControlRotYaw + (RangeVal > 0.0f ? AimYawMin : AimYawMax);
		SmoothCharacterRotation({0.0f, TargetYaw, 0.0f}, 0.0f, InterpSpeed, DeltaTime);
	}
}

/*
 * 根据输入和控制器方向获得控制器向前、向后的向量
 */
void AALSBaseCharacter::GetControlForwardRightVector(FVector& Forward, FVector& Right) const
{
	const FRotator ControlRot(0.0f, AimingRotation.Yaw, 0.0f);
	Forward = GetInputAxisValue("MoveForward/Backwards") * UKismetMathLibrary::GetForwardVector(ControlRot);
	Right = GetInputAxisValue("MoveRight/Left") * UKismetMathLibrary::GetRightVector(ControlRot);
}

/*
 * 获得总的控制向量
 */
FVector AALSBaseCharacter::GetPlayerMovementInput() const
{
	FVector Forward = FVector::ZeroVector;
	FVector Right = FVector::ZeroVector;
	GetControlForwardRightVector(Forward, Right);
	return (Forward + Right).GetSafeNormal();
}

/*
 * 读取玩家向前向后的输入量
 */
void AALSBaseCharacter::PlayerForwardMovementInput(float Value)
{
	ForwardInputValue = UALSMathLibrary::FixDiagonalGamepadValues(Value, GetInputAxisValue("MoveRight/Left")).Key;
	
	// if (MovementState == EALSMovementState::Grounded || MovementState == EALSMovementState::InAir)
	if (bCanInputMove)
	{
		// 如果是在陆地状态或者在空中， 默认的相机相对移动行为
		const FRotator DirRotator(0.0f, AimingRotation.Yaw, 0.0f);
		AddMovementInput(UKismetMathLibrary::GetForwardVector(DirRotator), ForwardInputValue);
	}
}

/*
 * 读取玩家向左向右的输入量
 */
void AALSBaseCharacter::PlayerRightMovementInput(float Value)
{
	RightInputValue = UALSMathLibrary::FixDiagonalGamepadValues(GetInputAxisValue("MoveForward/Backwards"), Value).
		Value;
	
	// if (MovementState == EALSMovementState::Grounded || MovementState == EALSMovementState::InAir)
	if (bCanInputMove)
	{
		const FRotator DirRotator(0.0f, AimingRotation.Yaw, 0.0f);
		AddMovementInput(UKismetMathLibrary::GetRightVector(DirRotator), RightInputValue);
	}
}

void AALSBaseCharacter::PlayerCameraUpInput(float Value)
{
	AddControllerPitchInput(LookUpDownRate * Value);
}

void AALSBaseCharacter::PlayerCameraRightInput(float Value)
{
	AddControllerYawInput(LookLeftRightRate * Value);
}

/* 跳跃动作 ：
 * 当处于布娃娃状态的时候，按下取消布娃娃
 * 当处于蹲伏状态的时候，站起来。
 * 当处于站立状态的时候，进行跳跃操作。
 */
void AALSBaseCharacter::JumpPressedAction()
{
	/* 调用绑定委托的函数 */
	if (JumpPressedDelegate.IsBound())
	{
		JumpPressedDelegate.Broadcast();
	}

	if (MovementAction == EALSMovementAction::None)
	{
		if (MovementState == EALSMovementState::Grounded)
		{
			if (Stance == EALSStance::Standing)
			{
				Jump();
			}
			else if (Stance == EALSStance::Crouching)
			{
				UnCrouch();
			}
		}
		else if (MovementState == EALSMovementState::Ragdoll)
		{
			ReplicatedRagdollEnd();
		}
	}
}

void AALSBaseCharacter::JumpReleasedAction()
{
	/* 调用绑定委托的函数 */
	if (JumpReleaseDelegate.IsBound())
	{
		JumpReleaseDelegate.Broadcast();
	}

	StopJumping();
}

void AALSBaseCharacter::SprintPressedAction()
{
	SetDesiredGait(EALSGait::Sprinting);
}

void AALSBaseCharacter::SprintReleasedAction()
{
	SetDesiredGait(EALSGait::Running);
}

/*
 * AimAction:按住“AimAction”进入瞄准模式，释放回到之前的旋转模式。
 */
void AALSBaseCharacter::AimPressedAction()
{
	SetRotationMode(EALSRotationMode::Aiming);
}

void AALSBaseCharacter::AimReleasedAction()
{
	switch (ViewMode)
	{
	case EALSViewMode::ThirdPerson:
		SetRotationMode(DesiredRotationMode);
		break;

	case EALSViewMode::FirstPerson:
		SetRotationMode(EALSRotationMode::LookingDirection);
		break;

	default: ;
	}
}

/*
 * 长按切换人称视角
 * 按一下切换摄像头在左边还是右边
 */
void AALSBaseCharacter::CameraPressedAction()
{
	UWorld* World = GetWorld();
	check(World);
	CameraActionPressedTime = World->GetTimeSeconds();

	/*
	 * 如果在指定时间内没有松开按键，就切换视角
	 */
	GetWorldTimerManager().SetTimer(OnCameraModeSwapTimer, this,
	                                &AALSBaseCharacter::OnSwitchCameraMode, ViewModeSwitchHoldTime, false);
}

void AALSBaseCharacter::CameraReleasedAction()
{
	if (ViewMode == EALSViewMode::FirstPerson)
	{
		// Don't swap shoulders on first person mode
		return;
	}

	UWorld* World = GetWorld();
	check(World);
	/* 在规定时间内松开了按键，切换左右肩视角并清空时间，防止进入第一人称模式 */
	if (World->GetTimeSeconds() - CameraActionPressedTime < ViewModeSwitchHoldTime)
	{
		// Switch shoulders
		SetRightShoulder(!bRightShoulder);
		GetWorldTimerManager().ClearTimer(OnCameraModeSwapTimer); // Prevent mode change
	}
}

void AALSBaseCharacter::OnSwitchCameraMode()
{
	// Switch camera mode
	if (ViewMode == EALSViewMode::FirstPerson)
	{
		SetViewMode(EALSViewMode::ThirdPerson);
	}
	else if (ViewMode == EALSViewMode::ThirdPerson)
	{
		SetViewMode(EALSViewMode::FirstPerson);
	}
}

/*
 * Stance Action: 单独按一次， 切换站立蹲伏动作   双击开启翻滚动作
 */
void AALSBaseCharacter::StancePressedAction()
{
	if (MovementAction != EALSMovementAction::None)
	{
		return;
	}

	UWorld* World = GetWorld();
	check(World);

	const float PrevStanceInputTime = LastStanceInputTime;

	LastStanceInputTime = World->GetTimeSeconds();

	if (LastStanceInputTime - PrevStanceInputTime <= RollDoubleTapTimeout)
	{
		// 开启翻滚动作动画
		Replicated_PlayMontage(GetRollAnimation(), 1.15f);

		// 因为按下第一次的时候将状态切换了，所以这一步要将第一次按下的状态进行还原。
		if (Stance == EALSStance::Standing)
		{
			SetDesiredStance(EALSStance::Crouching);
		}
		else if (Stance == EALSStance::Crouching)
		{
			SetDesiredStance(EALSStance::Standing);
		}
		return;
	}

	/* 单独点击 */
	if (MovementState == EALSMovementState::Grounded)
	{
		if (Stance == EALSStance::Standing)
		{
			SetDesiredStance(EALSStance::Crouching);
			Crouch();
		}
		else if (Stance == EALSStance::Crouching)
		{
			SetDesiredStance(EALSStance::Standing);
			UnCrouch();
		}
	}

	// Notice: MovementState == EALSMovementState::InAir case is removed
}

/*
 * 行走和奔跑状态切换
 */
void AALSBaseCharacter::WalkPressedAction()
{
	if (DesiredGait == EALSGait::Walking)
	{
		SetDesiredGait(EALSGait::Running);
	}
	else if (DesiredGait == EALSGait::Running)
	{
		SetDesiredGait(EALSGait::Walking);
	}
}

void AALSBaseCharacter::RagdollPressedAction()
{
	/* 按下一次就是开， 再按一次就是关闭了 */

	if (GetMovementState() == EALSMovementState::Ragdoll)
	{
		ReplicatedRagdollEnd();
	}
	else
	{
		ReplicatedRagdollStart();
	}
}

/*
 * 切换成速度旋转模式
 */
void AALSBaseCharacter::VelocityDirectionPressedAction()
{
	// Select Rotation Mode: Switch the desired (default) rotation mode to Velocity or Looking Direction.
	// This will be the mode the character reverts back to when un-aiming
	SetDesiredRotationMode(EALSRotationMode::VelocityDirection);
	SetRotationMode(EALSRotationMode::VelocityDirection);
}

/*
 * 切换成观察模式
 */
void AALSBaseCharacter::LookingDirectionPressedAction()
{
	SetDesiredRotationMode(EALSRotationMode::LookingDirection);
	SetRotationMode(EALSRotationMode::LookingDirection);
}

void AALSBaseCharacter::ReplicatedRagdollStart()
{
	if (HasAuthority())
	{
		Multicast_RagdollStart();
	}
	else
	{
		Server_RagdollStart();
	}
}

void AALSBaseCharacter::ReplicatedRagdollEnd()
{
	if (HasAuthority())
	{
		Multicast_RagdollEnd(GetActorLocation());
	}
	else
	{
		Server_RagdollEnd(GetActorLocation());
	}
}

void AALSBaseCharacter::OnRep_RotationMode(EALSRotationMode PrevRotMode)
{
	OnRotationModeChanged(PrevRotMode);
}

void AALSBaseCharacter::OnRep_ViewMode(EALSViewMode PrevViewMode)
{
	OnViewModeChanged(PrevViewMode);
}

/* 当 @OverlayState 变量发生改变的时候调用 */
void AALSBaseCharacter::OnRep_OverlayState(EALSOverlayState PrevOverlayState)
{
	OnOverlayStateChanged(PrevOverlayState);
}

void AALSBaseCharacter::OnRep_VisibleMesh(USkeletalMesh* NewVisibleMesh)
{
	OnVisibleMeshChanged(NewVisibleMesh);
}
