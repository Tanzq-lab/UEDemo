// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "UObject/Object.h"
#include "GSAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class ANIMATIONSYSTEM_API UGSAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UGSAbilitySystemComponent();
	
	bool bCharacterAbilitiesGiven = false;

	virtual void AbilityLocalInputPressed(int32 InputID) override;
	
};
