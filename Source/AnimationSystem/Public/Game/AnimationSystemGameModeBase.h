// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AnimationSystemGameModeBase.generated.h"

class UDataTable;
/**
 * 
 */
UCLASS()
class ANIMATIONSYSTEM_API AAnimationSystemGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:	
	/*
	 * Weapons
	 */
	
	/**
	 * @brief 获取步枪数据表
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS|Weapons")
	UDataTable* GetRifleDB() const { return RifleDB; }
	
	/**
	 * @brief 获取手枪数据表
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS|Weapons")
	UDataTable* GetPistolDB() const { return PistolDB; }

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS|Config")
	UDataTable* RifleDB;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS|Config")
	UDataTable* PistolDB;
};
