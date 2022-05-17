// Copyright ©2022 Tanzq. All rights reserved.


#include "Characters/Heroes/GSHeroCharacter.h"

#include "MotionWarpingComponent.h"
#include "Characters/Abilities/GSAbilitySystemComponent.h"
#include "Characters/Abilities/AttributeSets/GSAmmoAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Weapons/GSWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/GASPlayerController.h"
#include "Player/GSPlayerState.h"
#include "Sound/SoundCue.h"


static const FName NAME__GASCharacter_root(TEXT("root"));
static const FName NAME__Rifle_Attachment(TEXT("Rifle_Attachment"));
static const FName NAME__Pistol_Attachment(TEXT("Pistol_Attachment"));
static const FName NAME__Bow_Attachment(TEXT("Rifle_Attachment"));
static const FName NAME__PickUp_Warping(TEXT("PickUp"));


// Sets default values
AGSHeroCharacter::AGSHeroCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RifleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("RifleMesh"));
	RifleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RifleMesh->SetupAttachment(GetMesh(), NAME__Rifle_Attachment);
	RifleMesh->CastShadow = false;
	RifleMesh->SetVisibility(false);

	PistolMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("PistolMesh"));
	PistolMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PistolMesh->SetupAttachment(GetMesh(), NAME__Pistol_Attachment);
	PistolMesh->CastShadow = false;
	PistolMesh->SetVisibility(false);

	BowMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("BowMesh"));
	BowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BowMesh->SetupAttachment(GetMesh(), NAME__Bow_Attachment);
	BowMesh->CastShadow = false;
	BowMesh->SetVisibility(false);

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(FName("MotionWarping"));

	NoWeaponTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.None"));
	WeaponChangingDelayReplicationTag = FGameplayTag::RequestGameplayTag(
		FName("Ability.Weapon.IsChangingDelayReplication"));
	WeaponAmmoTypeNoneTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.None"));
	WeaponAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon"));
	CurrentWeaponTag = NoWeaponTag;
	Inventory = FGSHeroInventory();

	KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
	InteractingTag = FGameplayTag::RequestGameplayTag("State.Interacting");
}

void AGSHeroCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGSHeroCharacter, Inventory);
	// Only replicate CurrentWeapon to simulated clients and manually sync CurrentWeeapon with Owner when we're ready.
	// This allows us to predict weapon changing.
	DOREPLIFETIME_CONDITION(AGSHeroCharacter, CurrentWeapon, COND_SimulatedOnly);
}


/**
 * @brief 返回true 代表增加了新的物体到背包中， 返回false, 代表替换了一个新的同类型枪支到背包中。
 */
bool AGSHeroCharacter::AddWeaponToInventory(AGSPickup* NewItem)
{
	LastTouchItem = NewItem;
	AGSWeapon* NewWeapon = Cast<AGSWeapon>(NewItem);

	// 判断是否存在同类型武器
	if (NewWeapon && DoesWeaponExistInInventory(NewWeapon))
	{
		USoundCue* PickupSound = NewWeapon->GetPickupSound();
	
		if (PickupSound && IsLocallyControlled())
		{
			UGameplayStatics::SpawnSoundAttached(PickupSound, GetRootComponent());
		}
	
		if (GetLocalRole() < ROLE_Authority)
		{
			return false;
		}
	
		
	
		return false;
	}

	// 如果不是服务器端，就返回false
	if (GetLocalRole() < ROLE_Authority)
	{
		return false;
	}
	// 设置为默认模式进行拾取操作
	SetOverlayState(EALSOverlayState::Default);

	// 没有这个武器，将武器添加到背包中
	if (NewWeapon)
	{
		Inventory.Weapons.Add(NewWeapon);
	}
	else
	{
		Inventory.PickUpItems.Add(NewItem);
	}

	UAnimMontage* PickUpAnimMontage = NewItem->GetPickUpMontage(GetStance());

	// 播放拾取动画蒙太奇
	if (IsValid(PickUpAnimMontage))
	{
		bCanInputMove = false;


		FVector TargetLocation = NewItem->GetActorLocation();
		TargetLocation.Z = GetActorLocation().Z;
		FTransform TouchTarget = FTransform::Identity;
		TouchTarget.SetLocation(TargetLocation);
		MotionWarpingComponent->AddOrUpdateWarpTarget(NAME__PickUp_Warping, TouchTarget);

		if (MainAnimInstance)
		{
			MainAnimInstance->Montage_Play(PickUpAnimMontage, 1.0f,
			                               EMontagePlayReturnType::MontageLength, 0.0f, true);
		}
	}
	else
	{
		// 如果没有配置蒙太奇动画，直接设置成动画结束，结束交互。
		NewItem->SetActorHiddenInGame(true);
		GetGASAnimInstance()->PickUpEnd();
	}


	// NewWeapon->SetOwningCharacter(this);
	// NewWeapon->AddAbilities();
	// switch (NewWeapon->EquipType)
	// {
	// case EGSEquipType::Rifle:
	// 	RifleMesh->SetVisibility(true);
	// 	break;
	// case EGSEquipType::Pistol:
	// 	PistolMesh->SetVisibility(true);
	// 	break;
	// default: ;
	// }


	//
	// if (AbilitySystemComponent)
	// {
	// 	// 添加能力
	// 	AbilitySystemComponent->AddLooseGameplayTag(InteractingTag);
	// }
	//
	// if (GASAnimInstance)
	// {
	// 	GASAnimInstance->bIsInteract = true;
	// }

	// 待会再移动这个逻辑对应的位置。
	// if (bEquipWeapon)
	// {
	// 	// 等拾取动画播放完之后再进行下面的逻辑
	// 	EquipWeapon(NewWeapon);
	// 	ClientSyncCurrentWeapon(CurrentWeapon);
	// }

	return true;
}

// Called when the game starts or when spawned
void AGSHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
	PlayerController = GetController<AGASPlayerController>();
}

/**
 * @brief 如果背包中存在相同的武器类型，就说明有同类武器，返回 true.
 */
bool AGSHeroCharacter::DoesWeaponExistInInventory(const AGSWeapon* InWeapon)
{
	//UE_LOG(LogTemp, Log, TEXT("%s InWeapon class %s"), *FString(__FUNCTION__), *InWeapon->GetClass()->GetName());
	for (const AGSWeapon* Weapon : Inventory.Weapons)
	{
		if (Weapon && InWeapon && Weapon->WeaponTag == InWeapon->WeaponTag)
		{
			return true;
		}
	}

	return false;
}

void AGSHeroCharacter::OnRep_Inventory()
{
	if (GetLocalRole() == ROLE_AutonomousProxy && Inventory.Weapons.Num() > 0 && !CurrentWeapon)
	{
		// Since we don't replicate the CurrentWeapon to the owning client, this is a way to ask the Server to sync
		// the CurrentWeapon after it's been spawned via replication from the Server.
		// The weapon spawning is replicated but the variable CurrentWeapon is not on the owning client.
		ServerSyncCurrentWeapon();
	}
}

void AGSHeroCharacter::OnRep_CurrentWeapon(AGSWeapon* LastWeapon)
{
	bChangedWeaponLocally = false;
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}

void AGSHeroCharacter::ServerSyncCurrentWeapon_Implementation()
{
	ClientSyncCurrentWeapon(CurrentWeapon);
}

bool AGSHeroCharacter::ServerSyncCurrentWeapon_Validate()
{
	return true;
}

void AGSHeroCharacter::ClientSyncCurrentWeapon_Implementation(AGSWeapon* InWeapon)
{
	AGSWeapon* LastWeapon = CurrentWeapon;
	CurrentWeapon = InWeapon;
	OnRep_CurrentWeapon(LastWeapon);
}

bool AGSHeroCharacter::ClientSyncCurrentWeapon_Validate(AGSWeapon* InWeapon)
{
	return true;
}

void AGSHeroCharacter::SetCurrentWeapon(AGSWeapon* NewWeapon, const AGSWeapon* LastWeapon)
{
	if (NewWeapon == LastWeapon)
	{
		return;
	}

	ClearHeldObject();

	// 取消将要装配的武器之前拥有的能力
	if (AbilitySystemComponent)
	{
		const FGameplayTagContainer AbilityTagsToCancel = FGameplayTagContainer(WeaponAbilityTag);
		AbilitySystemComponent->CancelAbilities(&AbilityTagsToCancel);
	}

	if (NewWeapon)
	{
		if (AbilitySystemComponent)
		{
			// 清除潜在的NoWeaponTag
			AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		}

		// 从 OnRep_CurrentWeapon 调用过来的武器不会有所有者设置  
		CurrentWeapon = NewWeapon;
		CurrentWeaponTag = CurrentWeapon->WeaponTag;

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
		}

		// 暂时取消
		AGASPlayerController* PC = GetController<AGASPlayerController>();
		if (PC && PC->IsLocalController())
		{
			// 更新武器相关信息
			// PC->SetEquippedWeaponPrimaryIconFromSprite(CurrentWeapon->PrimaryIcon);
			// PC->SetEquippedWeaponStatusText(CurrentWeapon->StatusText);
			// PC->SetPrimaryClipAmmo(CurrentWeapon->GetPrimaryClipAmmo());
			// PC->SetPrimaryReserveAmmo(GetPrimaryReserveAmmo());
			// PC->SetHUDReticle(CurrentWeapon->GetPrimaryHUDReticleClass());
		}

		// NewWeapon->OnPrimaryClipAmmoChanged.AddDynamic(this, &AGSHeroCharacter::CurrentWeaponPrimaryClipAmmoChanged);
		// NewWeapon->OnSecondaryClipAmmoChanged.AddDynamic(this, &AGSHeroCharacter::CurrentWeaponSecondaryClipAmmoChanged);
		//
		// if (AbilitySystemComponent)
		// {
		// 	PrimaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(CurrentWeapon->PrimaryAmmoType)).AddUObject(this, &AGSHeroCharacter::CurrentWeaponPrimaryReserveAmmoChanged);
		// 	SecondaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(CurrentWeapon->SecondaryAmmoType)).AddUObject(this, &AGSHeroCharacter::CurrentWeaponSecondaryReserveAmmoChanged);
		// }

		if (CurrentWeapon->EquipSound)
		{
			UGameplayStatics::SpawnSoundAttached(CurrentWeapon->EquipSound, GetRootComponent());
		}
	}
	else
	{
		// This will clear HUD, tags etc
		// UnEquipCurrentWeapon();
	}
}


// Called every frame
void AGSHeroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGSHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);


	AGSPlayerState* PS = GetPlayerState<AGSPlayerState>();
	if (PS)
	{
		// Set the ASC on the Server. Clients do this in OnRep_PlayerState()
		AbilitySystemComponent = Cast<UGSAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		// AI won't have PlayerControllers so we can init again here just to be sure. No harm in initing twice for heroes that have PlayerControllers.
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

		AddCharacterAbilities();
	}
}

// Called to bind functionality to input
void AGSHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	BindASCInput();
}

void AGSHeroCharacter::BindASCInput()
{
	if (!bASCInputBound && IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, FGameplayAbilityInputBinds(
			                                                              FString(),
			                                                              FString(),
			                                                              FString("EGSAbilityInputID"),
			                                                              static_cast<int32>(
				                                                              EGSAbilityInputID::None),
			                                                              static_cast<int32>(
				                                                              EGSAbilityInputID::None)));

		bASCInputBound = true;
	}
}

void AGSHeroCharacter::EquipWeapon(AGSWeapon* NewWeapon)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerEquipWeapon(NewWeapon);
		SetCurrentWeapon(NewWeapon, CurrentWeapon);
		bChangedWeaponLocally = true;
	}
	else
	{
		SetCurrentWeapon(NewWeapon, CurrentWeapon);
	}
}

void AGSHeroCharacter::ServerEquipWeapon_Implementation(AGSWeapon* NewWeapon)
{
	EquipWeapon(NewWeapon);
}

bool AGSHeroCharacter::ServerEquipWeapon_Validate(AGSWeapon* NewWeapon)
{
	return true;
}

FName AGSHeroCharacter::GetEquipSocket(EGSEquipType EquipType) const
{
	const FName* SocketName = EquipSockets.Find(EquipType);
	if (!SocketName)
	{
		UE_LOG(LogHints, Log, TEXT("%s ： %s OwningCharacter can't find EquipSocketName!(use root)"),
		       *FString(__FUNCTION__), *GetName());
		return NAME__GASCharacter_root;
	}
	return *SocketName;
}

void AGSHeroCharacter::OnOverlayStateChanged(EALSOverlayState PreviousState)
{
	
	// 如果之前是持枪状态，然后现在状态改变了，那么就要将枪设置成可见。
	if (CurrentWeapon)
	{
		if (PreviousState == EALSOverlayState::Rifle || PreviousState == EALSOverlayState::PistolOneHanded ||
			PreviousState == EALSOverlayState::PistolTwoHanded || PreviousState == EALSOverlayState::Bow)
		{
			if (CurrentWeapon->WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Pistol")))
			{
				if (!(OverlayState == EALSOverlayState::PistolOneHanded && PreviousState ==
					EALSOverlayState::PistolTwoHanded
					|| PreviousState == EALSOverlayState::PistolOneHanded && OverlayState ==
					EALSOverlayState::PistolTwoHanded))
				{
					PistolMesh->SetVisibility(true);
				}
			}
			else if (CurrentWeapon->WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Rifle")))
			{
				RifleMesh->SetVisibility(true);
			}
			else if (CurrentWeapon->WeaponTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.Bow")))
			{
				BowMesh->SetVisibility(true);
			}
		}
	}

	// 在背包中找到对应的枪，设置成当前枪支。
	if (OverlayState == EALSOverlayState::Rifle || OverlayState == EALSOverlayState::PistolOneHanded || OverlayState ==
		EALSOverlayState::PistolTwoHanded || OverlayState == EALSOverlayState::Bow)
	{
		for (const auto Weapon : Inventory.Weapons)
		{
			if (OverlayState == EALSOverlayState::Rifle && Weapon->WeaponTag == FGameplayTag::RequestGameplayTag(
				FName("Weapon.Equipped.Rifle")))
			{
				// RifleMesh->SetVisibility(false);
				SetCurrentWeapon(Weapon, CurrentWeapon);
				break;
			}
			if ((OverlayState == EALSOverlayState::PistolTwoHanded || OverlayState ==
				EALSOverlayState::PistolOneHanded) && Weapon->WeaponTag == FGameplayTag::RequestGameplayTag(
				FName("Weapon.Equipped.Pistol")))
			{
				// PistolMesh->SetVisibility(false);
				SetCurrentWeapon(Weapon, CurrentWeapon);
				break;
			}
			if (OverlayState == EALSOverlayState::Bow && Weapon->WeaponTag == FGameplayTag::RequestGameplayTag(
				FName("Weapon.Equipped.Bow")))
			{
				// PistolMesh->SetVisibility(false);
				SetCurrentWeapon(Weapon, CurrentWeapon);
				break;
			}
		}
	}
	else
	{
		// 在其他状态下是没有武器的。
		SetCurrentWeapon(nullptr, CurrentWeapon);
	}

	AALSBaseCharacter::OnOverlayStateChanged(PreviousState);

	if (PlayerController && PlayerController->IsLocalController())
	{
		PlayerController->OnOverlayStateChanged(PreviousState);
	}

}

bool AGSHeroCharacter::IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent) const
{
	// 英雄被击倒后可以被复活，但是还没有复活。  
	// 如果你想让一个人复活多个英雄来加速，你需要改变GA_Interact(不在本示例的范围内)。  
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(InteractingTag))
	{
		return true;
	}

	return IGSInteractable::IsAvailableForInteraction_Implementation(InteractionComponent);
}

float AGSHeroCharacter::GetInteractionDuration_Implementation(UPrimitiveComponent* InteractionComponent) const
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag))
	{
		return ReviveDuration;
	}

	return IGSInteractable::GetInteractionDuration_Implementation(InteractionComponent);
}

void AGSHeroCharacter::PreInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent)
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag) &&
		HasAuthority())
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(
			FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Revive")));
	}
}

void AGSHeroCharacter::PostInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent)
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag) &&
		HasAuthority())
	{
		AbilitySystemComponent->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(ReviveEffect->GetDefaultObject()), 1.0f,
		                                                  AbilitySystemComponent->MakeEffectContext());
	}
}

void AGSHeroCharacter::GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type,
                                                             UPrimitiveComponent* InteractionComponent) const
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag))
	{
		bShouldSync = true;
		Type = EAbilityTaskNetSyncType::OnlyClientWait;
		return;
	}

	IGSInteractable::GetPreInteractSyncType_Implementation(bShouldSync, Type, InteractionComponent);
}

void AGSHeroCharacter::CancelInteraction_Implementation(UPrimitiveComponent* InteractionComponent)
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag) &&
		HasAuthority())
	{
		FGameplayTagContainer CancelTags(FGameplayTag::RequestGameplayTag("Ability.Revive"));
		AbilitySystemComponent->CancelAbilities(&CancelTags);
	}
}

FSimpleMulticastDelegate* AGSHeroCharacter::GetTargetCancelInteractionDelegate(
	UPrimitiveComponent* InteractionComponent)
{
	return &InteractionCanceledDelegate;
}
