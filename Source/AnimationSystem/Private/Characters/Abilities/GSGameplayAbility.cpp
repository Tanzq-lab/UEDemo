// Copyright ©2022 Tanzq. All rights reserved.


#include "Characters/Abilities/GSGameplayAbility.h"

#include "AbilitySystemComponent.h"

UGSGameplayAbility::UGSGameplayAbility()
{
	// Default to Instance Per Actor
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// bSourceObjectMustEqualCurrentWeaponToActivate = false;
	// bCannotActivateWhileInteracting = true;

	// UGSAbilitySystemGlobals hasn't initialized tags yet to set ActivationBlockedTags
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.KnockedDown"));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.BlocksInteraction"));

	// InteractingTag = FGameplayTag::RequestGameplayTag("State.Interacting");
	// InteractingRemovalTag = FGameplayTag::RequestGameplayTag("State.InteractingRemoval");
}

void UGSGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	if (bActivateAbilityOnGranted)
	{
		bool ActivatedAbility = ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
	}
}
