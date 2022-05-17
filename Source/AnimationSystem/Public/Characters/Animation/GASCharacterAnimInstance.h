// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/Animation/ALSCharacterAnimInstance.h"
#include "GASCharacterAnimInstance.generated.h"

class AGSHeroCharacter;
/**
 * 
 */
UCLASS()
class ANIMATIONSYSTEM_API UGASCharacterAnimInstance : public UALSCharacterAnimInstance
{
	GENERATED_BODY()

public:
	// UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	// bool bIsInteract = false;

	
	virtual void NativeInitializeAnimation() override;
	
	UFUNCTION(BlueprintCallable, Category="GAS|PickUp")
	void PickUpStart();
	
	UFUNCTION(BlueprintCallable, Category="GAS|PickUp")
	void PickUpEnd();


public:
	/** Read Only Data|Character Information */

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ALS|Torch")
	bool bStartEquipTorch = false;

	
protected:
	/** References */
	UPROPERTY(BlueprintReadOnly, Category = "Read Only Data|Character Information")
	AGSHeroCharacter* GASCharacter = nullptr;
};
