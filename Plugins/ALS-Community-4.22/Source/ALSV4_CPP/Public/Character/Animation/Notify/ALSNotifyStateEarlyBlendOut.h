// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Doğa Can Yanıkoğlu
// Contributors:    


#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Library/ALSCharacterEnumLibrary.h"

#include "ALSNotifyStateEarlyBlendOut.generated.h"

/**
 * 判断是否提前淡出动画
 */
UCLASS()
class ALSV4_CPP_API UALSNotifyStateEarlyBlendOut : public UAnimNotifyState
{
	GENERATED_BODY()

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                        float FrameDeltaTime) override;

	virtual FString GetNotifyName_Implementation() const override;

public:
	
	/**
	 * @brief 传递当前蒙太奇值
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	UAnimMontage* ThisMontage = nullptr;

	/**
	 * @brief 混合淡出时间
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	float BlendOutTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	bool bCheckMovementState = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	EALSMovementState MovementStateEquals = EALSMovementState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	bool bCheckStance = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	EALSStance StanceEquals = EALSStance::Standing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AnimNotify)
	bool bCheckMovementInput = false;
};
