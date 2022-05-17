// Copyright ©2022 Tanzq. All rights reserved.


#include "Characters/Animation/GASCharacterAnimInstance.h"

#include "Characters/Heroes/GSHeroCharacter.h"
#include "Items/Weapons/GSWeapon.h"

void UGASCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	GASCharacter = Cast<AGSHeroCharacter>(Character);
}

void UGASCharacterAnimInstance::PickUpStart()
{
	AGSPickup* Item  = GASCharacter->LastTouchItem;
	if (Item)
	{
		Item->SetActorHiddenInGame(true);
		const AGSWeapon* m_Weapon = Cast<AGSWeapon>(Item);
		m_Weapon->GetWeaponMesh()->SetAllBodiesSimulatePhysics(false);
		m_Weapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GASCharacter->AttachToHand(Item->GetPickUpStaticMesh(), Item->GetPickUpSkeletalMesh(), nullptr, Item->PickBoneName,
								   FVector(0.f, 0.f, 0.f));
	}
	else
	{
		UE_LOG(LogHints, Error, TEXT("%s %s PickUpItem is nullptr"), *FString(__FUNCTION__), *GetName());
	}
}

void UGASCharacterAnimInstance::PickUpEnd()
{
	AGSPickup* Item  = GASCharacter->LastTouchItem;
	GASCharacter->ClearHeldObject();
	const AGSWeapon* PickUpWeapon = Cast<AGSWeapon>(Item);
	if (PickUpWeapon)
	{
		if (PickUpWeapon->WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Pistol")))
		{
			GASCharacter->ShowPistolMesh(true);
		}
		else if (PickUpWeapon->WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Rifle")))
		{
			GASCharacter->ShowRifleMesh(true);
		}
		else if (PickUpWeapon->WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Bow")))
		{
			GASCharacter->ShowBowMesh(true);
		}
	}

	// 动画播放完毕，取消正在交互标签并且接受输入移动。
	// Item->bIsPlayFinished = true;
	GASCharacter->SetCanInputMove(true);
}
