// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/ALSPlayerController.h"
#include "Library/ALSCharacterEnumLibrary.h"
#include "GASPlayerController.generated.h"

UCLASS()
class ANIMATIONSYSTEM_API AGASPlayerController : public AALSPlayerController
{
	GENERATED_BODY()

protected:
	
	// UFUNCTION()
	// virtual void OnRep_HasWeapon(bool OldState);

public:
	// virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "ALS|Weapon")
	void OnOverlayStateChanged(const EALSOverlayState PreviousState);
	
protected:

	// UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HasWeapon, Category="GAS|Weapon")
	// bool bHasWeapon = false;
	
};
