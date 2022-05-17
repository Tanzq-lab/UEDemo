// Copyright ©2022 Tanzq. All rights reserved.


#include "Characters/GSCharacterBase.h"

#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/GSGameplayAbility.h"


AGSCharacterBase::AGSCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UAbilitySystemComponent* AGSCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool AGSCharacterBase::IsAlive() const
{
	return GetHealth() > 0.0f;
}

int32 AGSCharacterBase::GetAbilityLevel(EGSAbilityInputID AbilityID) const
{
	//TODO
	return 1;
}

float AGSCharacterBase::GetHealth() const
{
	/*if (IsValid(AttributeSetBase))
	{
		return AttributeSetBase->GetHealth();
	}

	return 0.0f;*/
	return 100.f;
}

// Called when the game starts or when spawned
void AGSCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void AGSCharacterBase::AddCharacterAbilities()
{
	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority || !IsValid(AbilitySystemComponent) || AbilitySystemComponent->bCharacterAbilitiesGiven)
	{
		return;
	}
	
	for (TSubclassOf<UGSGameplayAbility>& StartupAbility : CharacterAbilities)
	{
		AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(StartupAbility, GetAbilityLevel(StartupAbility.GetDefaultObject()->AbilityID), static_cast<int32>(StartupAbility.GetDefaultObject()->AbilityInputID), this));
	}

	AbilitySystemComponent->bCharacterAbilitiesGiven = true;
}

// Called every frame
void AGSCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
// void AGSCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
// {
// 	Super::SetupPlayerInputComponent(PlayerInputComponent);
// }

void AGSCharacterBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	GASAnimInstance = Cast<UGASCharacterAnimInstance>(MainAnimInstance);
}

int32 AGSCharacterBase::GetCharacterLevel() const
{
	//TODO
	return 1;
}
