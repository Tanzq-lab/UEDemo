// Copyright ©2022 Tanzq. All rights reserved.

#include "Items/GSASCActorBase.h"

#include "Game/AnimationSystem.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Heroes/GSHeroCharacter.h"
#include "Components/CapsuleComponent.h"


// Sets default values
AGSASCActorBase::AGSASCActorBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(FName("CollisionComp"));
	CollisionComp->InitCapsuleSize(30.0f, 50.0f);
	CollisionComp->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	CollisionComp->SetCollisionObjectType(COLLISION_PICKUP);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionComp;
	
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UGSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Minimal mode means GameplayEffects are not replicated to anyone. Only GameplayTags and Attributes are replicated to clients.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
}

// void AGSASCActorBase::NotifyActorBeginOverlap(AActor* Other)
// {
// 	Super::NotifyActorBeginOverlap(Other);
// 	bNearlyPawn = true;
// }
//
// void AGSASCActorBase::NotifyActorEndOverlap(AActor* OtherActor)
// {
// 	Super::NotifyActorEndOverlap(OtherActor);
// 	bNearlyPawn = false;
// }

UAbilitySystemComponent* AGSASCActorBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGSASCActorBase::OnTouch(AGSHeroCharacter* Pawn)
{
	K2_OnTouch(Pawn);
}

void AGSASCActorBase::TouchEnd(AGSHeroCharacter* Pawn)
{
	K2_TouchEnd(Pawn);
}

/*void AGSASCActorBase::OnRep_DesiredInteract(bool PreDesiredInteract)
{
	
}*/

// void AGSASCActorBase::OnRep_DesiredInteract(int32 PreDesiredInteract)
// {
// 	if (PreDesiredInteract != bDesiredInteract)
// 	{
// 		if (!bDesiredInteract)
// 		{
// 			TouchEnd()
// 		}
// 	}
// }

// Called when the game starts or when spawned
void AGSASCActorBase::BeginPlay()
{
	Super::BeginPlay();
	
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	// CollisionComp->OnComponentBeginOverlap.Add(this, &AGSASCActorBase::NotifyActorBeginOverlap)
}

