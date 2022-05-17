// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Items/GSASCActorBase.h"
#include "Library/ALSCharacterEnumLibrary.h"
#include "GSPickup.generated.h"

class AGSCharacterBase;
UCLASS()
class ANIMATIONSYSTEM_API AGSPickup : public AGSASCActorBase
{
	GENERATED_BODY()

public:	
	AGSPickup(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Check if pawn can use this pickup
	virtual bool CanBePickedUp(AGSCharacterBase* TestCharacter) const;

	UFUNCTION(BlueprintNativeEvent, Meta = (DisplayName = "CanBePickedUp"))
	bool K2_CanBePickedUp(AGSCharacterBase* TestCharacter) const;
	virtual bool K2_CanBePickedUp_Implementation(AGSCharacterBase* TestCharacter) const;

	
	UFUNCTION(BlueprintCallable, Category="GAS|PickUp")
	void PickupOnTouch(AGSCharacterBase* Pawn);
	
	UFUNCTION(BlueprintCallable, Category="GAS|PickUp")
	virtual UStaticMesh* GetPickUpStaticMesh() const;
	
	UFUNCTION(BlueprintCallable, Category="GAS|PickUp")
	virtual USkeletalMesh* GetPickUpSkeletalMesh() const;

	
	// 启用对应的能力
	UFUNCTION(BlueprintCallable, Category="GAS|PickUp")
	virtual void GivePickupTo(AGSCharacterBase* Pawn);
	
	UFUNCTION(BlueprintCallable, Category = "GAS|PickUp")
	UAnimMontage* GetPickUpMontage(EALSStance Stance) const;

	// /*
	//  * IGSInteractable
	//  */
	// virtual bool IsAnimationFinished_Implementation() const override;
	

protected:

	// Show effects when pickup disappears
	virtual void OnPickedUp();

	// Blueprint implementable effects
	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnPickedUp"))
	void K2_OnPickedUp();

	virtual void RespawnPickup();

	// Show effects when pickup appears
	virtual void OnRespawned();

	// Blueprint implementable effects
	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnRespawned"))
	void K2_OnRespawned();

	UFUNCTION()
	virtual void OnRep_IsActive();


public:
	// UPROPERTY(BlueprintReadOnly, Category = "GAS|PickUp|Animation")
	// bool bIsPlayFinished = false;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|PickUp|Animation")
	// bool bPickLeftHand = true;
	FName PickBoneName;
protected:	
	/**
	 * @brief 蹲伏拾取物体时对应的蒙太奇动画 
	 */
	UPROPERTY(BlueprintReadonly, EditAnywhere, Category = "GAS|PickUp|Animation")
	UAnimMontage* PickUpMontage_Crouch;
	
	/**
	 * @brief 站立拾取物体时对应的蒙太奇动画 
	 */
	UPROPERTY(BlueprintReadonly, EditAnywhere, Category = "GAS|PickUp|Animation")
	UAnimMontage* PickUpMontage_Standing;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsActive, Category = "GAS|PickUp")
	bool bIsActive;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|PickUp")
	bool bCanRespawn;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|PickUp")
	float RespawnTime;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|PickUp")
	FGameplayTagContainer RestrictedPickupTags;

	// Sound played when player picks it up
	UPROPERTY(EditDefaultsOnly, Category = "GAS|PickUp")
	class USoundCue* PickupSound;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|PickUp")
	TArray<TSubclassOf<class UGSGameplayAbility>> AbilityClasses;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|PickUp")
	TArray<TSubclassOf<class UGameplayEffect>> EffectClasses;

	// The character who has picked up this pickup
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GAS|PickUp")
	AGSCharacterBase* PickedUpBy;

	// The character who want to pick up this pickup
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "GAS|PickUp")
	AGSCharacterBase* WantToPickUpBy = nullptr;

	FTimerHandle TimerHandle_RespawnPickup;
};
