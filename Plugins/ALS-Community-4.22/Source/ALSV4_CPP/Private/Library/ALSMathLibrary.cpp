// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2021 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Doğa Can Yanıkoğlu
// Contributors:    Achim Turan


#include "Library/ALSMathLibrary.h"


#include "Library/ALSCharacterStructLibrary.h"
#include "Components/ALSDebugComponent.h"

#include "Components/CapsuleComponent.h"

/*
 * 将攀爬组件位置从局部坐标转化成世界坐标
 */
FTransform UALSMathLibrary::MantleComponentLocalToWorld(const FALSComponentAndTransform& CompAndTransform)
{
	/* 获取到物体的世界坐标变换的逆矩阵 */
	const FTransform& InverseTransform = CompAndTransform.Component->GetComponentToWorld().Inverse();
	/* 将局部坐标位置转化成世界坐标位置 */
	const FVector Location = InverseTransform.InverseTransformPosition(CompAndTransform.Transform.GetLocation());
	/* 将局部坐标旋转转化成世界坐标位置 */
	const FQuat Quat = InverseTransform.InverseTransformRotation(CompAndTransform.Transform.GetRotation());
	/* 将局部坐标缩放转化成世界坐标位置 */
	const FVector Scale = InverseTransform.InverseTransformPosition(CompAndTransform.Transform.GetScale3D());
	return {Quat, Location, Scale};
}

FTransform UALSMathLibrary::ALSComponentLocalToWorld(const FALSComponentAndTransform& CompAndTransform)
{
	
	return FTransform(CompAndTransform.Transform.ToMatrixWithScale()
		* CompAndTransform.Component->GetComponentTransform().ToMatrixWithScale().GetMatrixWithoutScale(0.f));
}

FTransform UALSMathLibrary::ALSComponentWorldToLocal(FALSComponentAndTransform& CompAndTransform)
{
	return FTransform(CompAndTransform.Transform.ToMatrixWithScale()
		* CompAndTransform.Component->GetComponentTransform().ToMatrixWithScale().GetMatrixWithoutScale(0.f).Inverse());
}


/*
 * 设X为 前后的输入值， Y为 左右的输入值，
 * 因为前后的与左右的输入值在都是非零值的情况下是不双方的值是不可能不受影响的，为了让输入的值更加细节，
 * 所以就将另一个的值映射之后添加到当前值上。
*/
TPair<float, float> UALSMathLibrary::FixDiagonalGamepadValues(const float X, const float Y)
{
	float ResultY = X * FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 0.6f),
	                                                      FVector2D(1.0f, 1.2f), FMath::Abs(Y));
	ResultY = FMath::Clamp(ResultY, -1.0f, 1.0f);
	float ResultX = Y * FMath::GetMappedRangeValueClamped(FVector2D(0.0f, 0.6f),
	                                                      FVector2D(1.0f, 1.2f), FMath::Abs(X));
	ResultX = FMath::Clamp(ResultX, -1.0f, 1.0f);
	return TPair<float, float>(ResultY, ResultX);
}

/*
 * 获得胶囊体下底的位置
 */
FVector UALSMathLibrary::GetCapsuleBaseLocation(const float ZOffset, UCapsuleComponent* Capsule)
{
	return Capsule->GetComponentLocation() -
		Capsule->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() + ZOffset);
}

float UALSMathLibrary::GetInterpSpeed(float Speed, float DeltaTime)
{
	return (Speed + 1) * FMath::GetMappedRangeValueClamped(FVector2D(15.f, 160.f), FVector2D(1.25f, 0.77f),
	                                                       1.f / DeltaTime);
}

/*
 * 从胶囊体的底部获取到胶囊体位置
 */
FVector UALSMathLibrary::GetCapsuleLocationFromBase(FVector BaseLocation, const float ZOffset,
                                                    UCapsuleComponent* Capsule)
{
	BaseLocation.Z += Capsule->GetScaledCapsuleHalfHeight() + ZOffset;
	return BaseLocation;
}

/*
 * 探测胶囊体是否在指定位置是否有空间
 */
bool UALSMathLibrary::CapsuleHasRoomCheck(UCapsuleComponent* Capsule, FVector TargetLocation, float HeightOffset,
                                          float RadiusOffset, EDrawDebugTrace::Type DebugType, bool DrawDebugTrace)
{
	/*
	 * 在目标位置生成一个胶囊体，检测是是否有空间
	 */
	const float ZTarget = Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere() - RadiusOffset + HeightOffset;
	FVector TraceStart = TargetLocation;
	TraceStart.Z += ZTarget;
	FVector TraceEnd = TargetLocation;
	TraceEnd.Z -= ZTarget;
	const float Radius = Capsule->GetUnscaledCapsuleRadius() + RadiusOffset;

	const UWorld* World = Capsule->GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Capsule->GetOwner());

	FHitResult HitResult;
	const FCollisionShape SphereCollisionShape = FCollisionShape::MakeSphere(Radius);
	const bool bHit = World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity,
	                                              ECC_Visibility, FCollisionShape::MakeSphere(Radius), Params);

	if (DrawDebugTrace)
	{
		UALSDebugComponent::DrawDebugSphereTraceSingle(World,
		                                               TraceStart,
		                                               TraceEnd,
		                                               SphereCollisionShape,
		                                               DebugType,
		                                               bHit,
		                                               HitResult,
		                                               FLinearColor(0.130706f, 0.896269f, 0.144582f, 1.0f),
		                                               // light green
		                                               FLinearColor(0.932733f, 0.29136f, 1.0f, 1.0f), // light purple
		                                               1.0f);
	}

	// 没有检测到任何东西代表有空间， 否则代表没有空间。
	return !HitResult.bBlockingHit && !HitResult.bStartPenetrating;
}

/*
 * IncreaseBuffer = true ,范围变小， = false 范围变大
 */
bool UALSMathLibrary::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool IncreaseBuffer)
{
	if (IncreaseBuffer)
	{
		return Angle >= MinAngle - Buffer && Angle <= MaxAngle + Buffer;
	}
	return Angle >= MinAngle + Buffer && Angle <= MaxAngle - Buffer;
}

/*
 * 取输入角度并确定其象限(方向)。
 * 使用当前的移动方向增加或减少每个象限的角度范围上的缓冲器。
 */
EALSMovementDirection UALSMathLibrary::CalculateQuadrant(EALSMovementDirection Current, float FRThreshold,
                                                         float FLThreshold,
                                                         float BRThreshold, float BLThreshold, float Buffer,
                                                         float Angle)
{
	// 目前方向与自己的方向平行的话，就缩小范围，没有平行就扩大范围

	// Angle 代表的是角色和摄像头之间相差的角度。
	// 注意，我这里和源码不一样，我将这个函数和AngleInRange函数进行了修改，个人感觉更加清晰了，没有重复的角度了。
	// 改完之后的意思：比如目前在向前状态，这次需要判断还是在向前/向后状态的话，判断区域会缩小，而其他的区域变大。
	if (AngleInRange(Angle, FLThreshold, FRThreshold, Buffer,
	                 Current == EALSMovementDirection::Forward || Current == EALSMovementDirection::Backward))
	{
		return EALSMovementDirection::Forward;
	}

	if (AngleInRange(Angle, FRThreshold, BRThreshold, Buffer,
	                 Current == EALSMovementDirection::Right || Current == EALSMovementDirection::Left))
	{
		return EALSMovementDirection::Right;
	}

	if (AngleInRange(Angle, BLThreshold, FLThreshold, Buffer,
	                 Current == EALSMovementDirection::Right || Current == EALSMovementDirection::Left))
	{
		return EALSMovementDirection::Left;
	}

	return EALSMovementDirection::Backward;
}


/*
 * 分三个位置进行更新，让插值最大化
 */
FVector UALSMathLibrary::CalculateAxisIndependentLag(FVector CurrentLocation, FVector TargetLocation,
                                                     FRotator CenterRotation, FVector LagSpeeds, float DeltaTime)
{
	// 让旋转只受到水平旋转的影响，而不受到其他方向的旋转影响。如果开启了其他方向的旋转的话，在进行插值的时候是会有倾斜，而不是平滑的移动到轴位置。
	CenterRotation.Roll = 0.0f;
	CenterRotation.Pitch = 0.0f;
	// 转换成局部坐标
	const FVector UnrotatedCurLoc = CenterRotation.UnrotateVector(CurrentLocation);
	const FVector UnrotatedTargetLoc = CenterRotation.UnrotateVector(TargetLocation);

	// 进行插值
	const FVector ResultVector(
		FMath::FInterpTo(UnrotatedCurLoc.X, UnrotatedTargetLoc.X, DeltaTime, LagSpeeds.X),
		FMath::FInterpTo(UnrotatedCurLoc.Y, UnrotatedTargetLoc.Y, DeltaTime, LagSpeeds.Y),
		FMath::FInterpTo(UnrotatedCurLoc.Z, UnrotatedTargetLoc.Z, DeltaTime, LagSpeeds.Z));

	// 还原成世界坐标
	return CenterRotation.RotateVector(ResultVector);
}
