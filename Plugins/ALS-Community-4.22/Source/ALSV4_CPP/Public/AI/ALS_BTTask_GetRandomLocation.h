// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Jens Bjarne Myhre
// Contributors:    Doğa Can Yanıkoğlu

#pragma once

#include "CoreMinimal.h"
#include "NavigationSystem.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "ALS_BTTask_GetRandomLocation.generated.h"

/** Picks a random location reachable through NavMesh within the Max Distance from the Owning Pawn's current location and assigns it to the specified Blackboard Key. */
UCLASS(Category=ALS, meta=(DisplayName = "Get Random Location"))
class ALSV4_CPP_API UALS_BTTask_GetRandomLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UALS_BTTask_GetRandomLocation();

	/** 选择的随机位置可能与 pawn 的最大距离。 */
	UPROPERTY(Category = Navigation, EditAnywhere, meta=(ClampMin = 1))
	float MaxDistance = 1000.0f;

	/** 选择位置时使用的导航查询过滤器，仅在分配时选择使用此过滤器可到达的位置。 */
	UPROPERTY(Category = Navigation, EditAnywhere)
	TSubclassOf<UNavigationQueryFilter> Filter = nullptr;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
