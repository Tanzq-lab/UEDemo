// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Jens Bjarne Myhre
// Contributors:    

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ALS_BTTask_SetFocusToPlayer.generated.h"

/** 将 AIController 的 Focus 设置为 Player 的 Pawn Actor。 */
UCLASS(Category = ALS, meta = (DisplayName = "Set Focus to Player"))
class ALSV4_CPP_API UALS_BTTask_SetFocusToPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UALS_BTTask_SetFocusToPlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
