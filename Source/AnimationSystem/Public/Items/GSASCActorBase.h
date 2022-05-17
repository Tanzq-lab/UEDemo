// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "Characters/Abilities/GSInteractable.h"
#include "Library/ALSCharacterEnumLibrary.h"
#include "GSASCActorBase.generated.h"

class AGSHeroCharacter;
/**
 * @brief 可交互类
 */
UCLASS()
class ANIMATIONSYSTEM_API AGSASCActorBase : public AActor, public IAbilitySystemInterface, public IGSInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGSASCActorBase(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE class UCapsuleComponent* GetCollisionComp() const { return CollisionComp; }
	
	// virtual void NotifyActorBeginOverlap(AActor* Other) override;
	//
	// virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	
	// Implement IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// /**
	//  * @brief 设置该武器对应角色的覆盖类型
	//  */
	// UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GAS|Character")
	// EALSOverlayState OverlayState = EALSOverlayState::Default;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/*
	 * 如果该函数没有被重载，就会调用对应的蓝图函数，使用蓝图函数
	 * 如果重载了该函数，就可以实现C++和蓝图混合使用。
	 */

	/**
	 * @brief 当有角色触碰到了该物体时触发。
	 */
	UFUNCTION(BlueprintCallable, Category="GAS|Touch")
	virtual void OnTouch(AGSHeroCharacter* Pawn);

	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnTouchBP"))
	void K2_OnTouch(AGSHeroCharacter* Pawn);

	/**
	 * @brief 当角色离开了该物体时触发。
	 */
	UFUNCTION(BlueprintCallable, Category="GAS|Touch")
	virtual void TouchEnd(AGSHeroCharacter* Pawn);

	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "TouchEndBP"))
	void K2_TouchEnd(AGSHeroCharacter* Pawn);

protected:
	UPROPERTY()
	class UGSAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UCapsuleComponent* CollisionComp;
	
};
