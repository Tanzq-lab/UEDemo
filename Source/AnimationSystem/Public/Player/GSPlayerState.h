// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GSPlayerState.generated.h"

UCLASS()
class ANIMATIONSYSTEM_API AGSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AGSPlayerState();

	// Implement IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY()
	class UGSAbilitySystemComponent* AbilitySystemComponent;
	
};
