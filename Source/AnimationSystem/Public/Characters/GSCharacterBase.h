// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/ALSCharacter.h"
#include "AbilitySystemInterface.h"
#include "Animation/GASCharacterAnimInstance.h"
#include "Game/AnimationSystem.h"
#include "GameFramework/Character.h"
#include "GSCharacterBase.generated.h"

UCLASS()
class ANIMATIONSYSTEM_API AGSCharacterBase : public AALSCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGSCharacterBase(const class FObjectInitializer& ObjectInitializer);

	// Implement IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;


	UFUNCTION(BlueprintCallable, Category = "GAS|GSCharacter")
	virtual bool IsAlive() const;

	// Switch on AbilityID to return individual ability levels.
	UFUNCTION(BlueprintCallable, Category = "GASShooter|GSCharacter")
	virtual int32 GetAbilityLevel(EGSAbilityInputID AbilityID) const;

	// TODO 还没实现！
	UFUNCTION(BlueprintCallable, Category = "GAS|GSCharacter|Attributes")
	float GetHealth() const;

	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// // Called to bind functionality to input
	// virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PreInitializeComponents() override;

	/**
	* Getters for attributes from GSAttributeSetBase
	**/

	UFUNCTION(BlueprintCallable, Category = "GAS|GSCharacter|Attributes")
	int32 GetCharacterLevel() const;

	/** Utility */

	UFUNCTION(BlueprintCallable, Category = "GAS|Utility")
	UGASCharacterAnimInstance* GetGASAnimInstance() { return GASAnimInstance; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Grant abilities on the Server. The Ability Specs will be replicated to the owning client.
	virtual void AddCharacterAbilities();
	

public:

protected:
	// 对ASC的引用。它会在PlayerState上，如果角色没有PlayerState，它会在这里。
	UPROPERTY()
	class UGSAbilitySystemComponent* AbilitySystemComponent;

	// Default abilities for this Character. These will be removed on Character death and regiven if Character respawns.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|Abilities")
	TArray<TSubclassOf<class UGSGameplayAbility>> CharacterAbilities;

	

	UPROPERTY(BlueprintReadOnly, Category = "GAS|Utility")
	UGASCharacterAnimInstance* GASAnimInstance = nullptr;
};
