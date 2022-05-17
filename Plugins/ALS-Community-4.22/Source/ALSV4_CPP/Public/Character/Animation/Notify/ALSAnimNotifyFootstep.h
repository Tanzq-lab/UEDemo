// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Doğa Can Yanıkoğlu
// Contributors:    


#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Library/ALSCharacterEnumLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "ALSAnimNotifyFootstep.generated.h"

class UDataTable;

/**
 * 角色脚部声音通知
 */
UCLASS()
class ALSV4_CPP_API UALSAnimNotifyFootstep : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	virtual FString GetNotifyName_Implementation() const override;

public:
	/**
	 * @brief 存储声音数据的表
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	UDataTable* HitDataTable;

	static FName NAME_Foot_R;
	/**
	 * @brief 当前通知所处脚的骨骼
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	FName FootSocketName = NAME_Foot_R;

	/**
	 * @brief 射线通道类型
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	TEnumAsByte<ETraceTypeQuery> TraceChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType;

	/**
	 * @brief 射线检测的长度
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float TraceLength = 50.0f;

	/**
	 * @brief 是否可以在地面上留下痕迹
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	bool bSpawnDecal = false;

	// 是否启用镜面数据，主要用来配置左右脚
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	bool bMirrorDecalX = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	bool bMirrorDecalY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	bool bMirrorDecalZ = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	bool bSpawnSound = true;

	static FName NAME_FootstepType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FName SoundParameterName = NAME_FootstepType;

	/**
	 * @brief 脚步声的类型
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	EALSFootstepType FootstepType = EALSFootstepType::Step;

	
	/**
	 * @brief 表示将声音屏蔽曲线添加进来。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	bool bOverrideMaskCurve = false;
	
	/**
	 * @brief 一个线性标量乘以音量，以使声音更响亮或更柔和。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float VolumeMultiplier = 1.0f;

	/**
	 * @brief 与音高相乘的线性标量。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float PitchMultiplier = 1.0f;

	/**
	 * @brief 是否使用的是 Niagara 特效技术
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	bool bSpawnNiagara = false;
};
