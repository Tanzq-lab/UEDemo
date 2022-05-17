// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GSCharacterEnumLibrary.generated.h"

/**
 * @brief 物件装配在身上的类型
 */
UENUM(BlueprintType)
enum class EGSEquipType : uint8
{
	None,
	Rifle,
	Pistol
};

/**
 * @brief 步枪名字
 */
UENUM(BlueprintType)
enum class EGSRifleModel : uint8
{
	None,
	M4A1 UMETA(DisplayName = "M4A1"),
	AK_47 UMETA(DisplayName = "AK-47"),
	Famas UMETA(DisplayName = "Famas"),
	AS_50 UMETA(DisplayName = "AS-50"),
	aa_50_beowulf UMETA(DisplayName = "aa-50-beowulf"),
	perun_x16 UMETA(DisplayName = "perun-x16"),
	C8 UMETA(DisplayName = "C8")
};

/**
 * @brief 手枪名字
 */
UENUM(BlueprintType)
enum class EGSPistolModel : uint8
{
	None,
	M9 UMETA(DisplayName = "M9"),
	P_018 UMETA(DisplayName = "P-018"),
	S_W_Model_39 UMETA(DisplayName = "S&W Model 39"),
	Desert_Deagle UMETA(DisplayName = "Desert Deagle")
};

