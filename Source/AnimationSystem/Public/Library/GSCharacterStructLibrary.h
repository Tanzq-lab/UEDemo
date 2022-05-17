// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GSCharacterStructLibrary.generated.h"


/**
 * @brief 步枪数据表类型
 */
USTRUCT(BlueprintType) 
struct FRifleAssetsInfo : public FTableRowBase {
	GENERATED_BODY();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Rifle Assets")
	FName Name;

	// UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	// EGSRifleModel EnumName = EGSRifleModel::M4A1;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Rifle Assets")
	USkeletalMesh* SkeletalMesh;

	// UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	// UPhysicsAsset* PhysicAssets;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Rifle Assets")
	UStaticMesh* MagazineMesh;

	UPROPERTY( EditAnywhere, Category = "Rifle Assets")
	FName MagazineSocketName = FName("magazineSocket");

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	FName MuzzleSocketName = FName("BulletStart");

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	class USoundCue* ShootingSound;

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	bool HeadShotDeath = false;

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	float DamageStrength = 1.f;

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	float AutomaticShootingSpeed = 0.03f;

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	FVector2D DispersionScale = FVector2D(190.f, 150.f);

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	bool bIsSniperRifle = false;

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	int32 MaxAmmoCountPerMagazine = 30;

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	FRotator MagazineRelativeRot;
};

/**
 * @brief 手枪数据表类型
 */
USTRUCT(BlueprintType) 
struct FPistolsAssetsInfo : public FTableRowBase {
	GENERATED_BODY();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Rifle Assets")
	FName Name;

	// UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	// EGSPistolModel EnumName = EGSPistolModel::M9;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Rifle Assets")
	USkeletalMesh* SkeletalMesh;

	// UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	// UPhysicsAsset* PhysicAssets;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Rifle Assets")
	UStaticMesh* MagazineMesh;

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	FName MagazineSocketName = FName("magazineSocket");

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	FName MuzzleSocketName = FName("BulletStart");

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	USoundCue* ShootingSound;

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	bool bWithSilencer = false;

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	float DamageStrength = 1.f;

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	float AutomaticShootingSpeed = 0.03f;

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	int32 MaxAmmoCountPerMagazine = 15;

	UPROPERTY(EditAnywhere, Category = "Rifle Assets")
	TSubclassOf<UAnimInstance> AnimBlueprint;
};