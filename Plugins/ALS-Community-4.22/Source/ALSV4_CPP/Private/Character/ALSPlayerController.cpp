// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Doğa Can Yanıkoğlu
// Contributors:    Drakynfly


#include "Character/ALSPlayerController.h"
#include "Character/ALSCharacter.h"
#include "Character/ALSPlayerCameraManager.h"
#include "Components/ALSDebugComponent.h"
#include "Kismet/GameplayStatics.h"

void AALSPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	PossessedCharacter = Cast<AALSBaseCharacter>(NewPawn);
	// 服务器只在监听服务器中设置摄像头。
	if (!IsRunningDedicatedServer())
	{
		SetupCamera();
	}
	SetupDebugInputs();
}

/*
 * 当Pawn发生改变的时候调用
 */
void AALSPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	// 转化成引用
	PossessedCharacter = Cast<AALSBaseCharacter>(GetPawn());
	// 启用摄像头
	SetupCamera();
}

void AALSPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();
	// 只有在真人控制的时候才需要使用Debug， 减少不必要的函数构建
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		SetupDebugInputs();
	}
}

/*
 * 启用摄像机，并将角色引用传入
 */
void AALSPlayerController::SetupCamera()
{
	AALSPlayerCameraManager* CastedMgr = Cast<AALSPlayerCameraManager>(PlayerCameraManager);
	if (PossessedCharacter && CastedMgr)
	{
		CastedMgr->OnPossess(PossessedCharacter);
	}
}

/*
 * 绑定 DEBUG 输入按键
 */
void AALSPlayerController::SetupDebugInputs()
{
	if (PossessedCharacter)
	{
		UActorComponent* Comp = PossessedCharacter->GetComponentByClass(UALSDebugComponent::StaticClass());
		if (Comp)
		{
			UALSDebugComponent* DebugComp = Cast<UALSDebugComponent>(Comp);
			if (InputComponent && DebugComp)
			{
				InputComponent->BindKey(EKeys::Tab, EInputEvent::IE_Pressed, DebugComp, &UALSDebugComponent::ToggleHud);
				InputComponent->BindKey(EKeys::V, EInputEvent::IE_Pressed, DebugComp, &UALSDebugComponent::ToggleDebugView);
				InputComponent->BindKey(EKeys::T, EInputEvent::IE_Pressed, DebugComp, &UALSDebugComponent::ToggleTraces);
				InputComponent->BindKey(EKeys::Y, EInputEvent::IE_Pressed, DebugComp, &UALSDebugComponent::ToggleDebugShapes);
				InputComponent->BindKey(EKeys::U, EInputEvent::IE_Pressed, DebugComp, &UALSDebugComponent::ToggleLayerColors);
				InputComponent->BindKey(EKeys::I, EInputEvent::IE_Pressed, DebugComp, &UALSDebugComponent::ToggleCharacterInfo);
				InputComponent->BindKey(EKeys::Z, EInputEvent::IE_Pressed, DebugComp, &UALSDebugComponent::ToggleSlomo);
				InputComponent->BindKey(EKeys::Comma, EInputEvent::IE_Pressed, DebugComp, &UALSDebugComponent::PreviousFocusedDebugCharacter);
				InputComponent->BindKey(EKeys::Period, EInputEvent::IE_Pressed, DebugComp, &UALSDebugComponent::NextFocusedDebugCharacter);
				InputComponent->BindKey(EKeys::M, EInputEvent::IE_Pressed, DebugComp, &UALSDebugComponent::ToggleDebugMesh);
			}
		}
	}
}
