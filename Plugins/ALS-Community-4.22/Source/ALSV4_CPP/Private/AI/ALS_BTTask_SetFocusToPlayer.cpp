// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Jens Bjarne Myhre
// Contributors:    Doğa Can Yanıkoğlu

#include "AI/ALS_BTTask_SetFocusToPlayer.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "AIController.h"

UALS_BTTask_SetFocusToPlayer::UALS_BTTask_SetFocusToPlayer()
{
	// 设置接节点名字
	NodeName = "Focus On Player";
}

EBTNodeResult::Type UALS_BTTask_SetFocusToPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 让AI一直盯着角色看
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Pawn)
	{
		OwnerComp.GetAIOwner()->SetFocus(Pawn);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

FString UALS_BTTask_SetFocusToPlayer::GetStaticDescription() const
{
	// 对行为的描述信息
	return "Set Focus to player's pawn";
}
