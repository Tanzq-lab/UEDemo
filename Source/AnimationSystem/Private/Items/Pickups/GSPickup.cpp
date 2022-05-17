// Copyright ©2022 Tanzq. All rights reserved.


#include "Items/Pickups/GSPickup.h"

#include "AbilitySystemComponent.h"
#include "Characters/GSCharacterBase.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/GSGameplayAbility.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"


// Sets default values
AGSPickup::AGSPickup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bIsActive = true;
	bCanRespawn = true;
	RespawnTime = 5.0f;

	// RestrictedPickupTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));
	// RestrictedPickupTags.AddTag(FGameplayTag::RequestGameplayTag("State.KnockedDown"));
}

void AGSPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGSPickup, bIsActive);
	DOREPLIFETIME(AGSPickup, PickedUpBy);
}

bool AGSPickup::CanBePickedUp(AGSCharacterBase* TestCharacter) const
{
	return bIsActive && TestCharacter && TestCharacter->IsAlive() && !IsPendingKill() && !TestCharacter->GetAbilitySystemComponent()->HasAnyMatchingGameplayTags(RestrictedPickupTags) && K2_CanBePickedUp(TestCharacter);
}

bool AGSPickup::K2_CanBePickedUp_Implementation(AGSCharacterBase* TestCharacter) const
{
	return true;
}
//
// bool AGSPickup::IsAnimationFinished_Implementation() const
// {
// 	return bIsPlayFinished;
// }

void AGSPickup::PickupOnTouch(AGSCharacterBase* Pawn)
{
	if (CanBePickedUp(Pawn))
	{
		PickedUpBy = Pawn;
		GivePickupTo(Pawn);
		bIsActive = false;
		OnPickedUp();

		// 这个可以当拓展子类拓展功能参考代码
		// if (bCanRespawn && RespawnTime > 0.0f)
		// {
		// 	GetWorldTimerManager().SetTimer(TimerHandle_RespawnPickup, this, &AGSPickup::RespawnPickup, RespawnTime, false);
		// }
	}
}

UStaticMesh* AGSPickup::GetPickUpStaticMesh() const 
{
	return nullptr;
}

USkeletalMesh* AGSPickup::GetPickUpSkeletalMesh() const
{
	return nullptr;
}

void AGSPickup::GivePickupTo(AGSCharacterBase* Pawn)
{
	// 将角色的ASC组件赋值给当前组件的ASC。
	AbilitySystemComponent = Cast<UGSAbilitySystemComponent>(Pawn->GetAbilitySystemComponent());

	if (!AbilitySystemComponent)
	{
		UE_LOG(LogHints, Error, TEXT("%s Pawn's ASC is null."), *FString(__FUNCTION__));
	}

	// 比如说拾取到了之后有一些特殊效果之类的， 将这些属性添加到到其中。
	// for (TSubclassOf<UGSGameplayAbility> AbilityClass : AbilityClasses)
	// {
	// 	if (!AbilityClass)
	// 	{
	// 		continue;
	// 	}
	// 	
	// 	FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1, static_cast<int32>(AbilityClass.GetDefaultObject()->AbilityInputID), this);
	// 	AbilitySystemComponent->GiveAbilityAndActivateOnce(AbilitySpec);
	// }
	//
	// FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	// EffectContext.AddSourceObject(this);
	//
	// for (TSubclassOf<UGameplayEffect> EffectClass : EffectClasses)
	// {
	// 	if (!EffectClass)
	// 	{
	// 		continue;
	// 	}
	//
	// 	FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Pawn->GetCharacterLevel(), EffectContext);
	//
	// 	if (NewHandle.IsValid())
	// 	{
	// 		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*NewHandle.Data.Get());
	// 	}
	// }
}

UAnimMontage* AGSPickup::GetPickUpMontage(EALSStance Stance) const
{
	if (Stance == EALSStance::Standing) return PickUpMontage_Standing;
	return PickUpMontage_Crouch;
}

void AGSPickup::OnPickedUp()
{
	K2_OnPickedUp();

	if (PickedUpBy)
	{
		
		
		if (PickupSound)
		{
			UGameplayStatics::SpawnSoundAttached(PickupSound, PickedUpBy->GetRootComponent());
		}
	}
	
}

void AGSPickup::RespawnPickup()
{
	bIsActive = true;
	PickedUpBy = NULL;
	OnRespawned();

	TSet<AActor*> OverlappingPawns;
	GetOverlappingActors(OverlappingPawns, AGSCharacterBase::StaticClass());

	for (AActor* OverlappingPawn : OverlappingPawns)
	{
		PickupOnTouch(CastChecked<AGSCharacterBase>(OverlappingPawn));
	}
}

void AGSPickup::OnRespawned()
{
	K2_OnRespawned();
}

void AGSPickup::OnRep_IsActive()
{
	if (bIsActive)
	{
		OnRespawned();
	}
	else
	{
		OnPickedUp();
	}
}
