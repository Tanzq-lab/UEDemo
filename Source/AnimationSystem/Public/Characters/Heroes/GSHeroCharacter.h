// Copyright ©2022 Tanzq. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Characters/GSCharacterBase.h"
#include "Characters/Abilities/GSInteractable.h"
#include "GameFramework/Character.h"
#include "Items/Pickups/GSPickup.h"
#include "Library/GSCharacterEnumLibrary.h"
#include "GSHeroCharacter.generated.h"

class AGASPlayerController;
class UMotionWarpingComponent;
USTRUCT()
struct ANIMATIONSYSTEM_API FGSHeroInventory
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<AGSWeapon*> Weapons;

	UPROPERTY()
	TArray<AGSPickup*> PickUpItems;

	// Consumable items

	// Passive items like armor

	// Door keys

	// Etc
};

UCLASS()
class ANIMATIONSYSTEM_API AGSHeroCharacter : public AGSCharacterBase, public IGSInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGSHeroCharacter(const FObjectInitializer& ObjectInitializer);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Only called on the Server. Calls before Server's AcknowledgePossession.
	virtual void PossessedBy(AController* NewController) override;


	/*
	 *  input
	 */

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called from both SetupPlayerInputComponent and OnRep_PlayerState because of a potential race condition where the PlayerController might
	// call ClientRestart which calls SetupPlayerInputComponent before the PlayerState is repped to the client so the PlayerState would be null in SetupPlayerInputComponent.
	// Conversely, the PlayerState might be repped before the PlayerController calls ClientRestart so the Actor's InputComponent would be null in OnRep_PlayerState.
	void BindASCInput();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnOverlayStateChanged(EALSOverlayState PreviousState) override;

	/*
	 * Inventory
	 */

	// 增加一个新武器到背包中。
	// 如果这种武器在背包中已经有一个了，就返回true，否则返回false
	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	bool AddWeaponToInventory(AGSPickup* NewItem);

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	AGSWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	void EquipWeapon(AGSWeapon* NewWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(AGSWeapon* NewWeapon);
	void ServerEquipWeapon_Implementation(AGSWeapon* NewWeapon);
	static bool ServerEquipWeapon_Validate(AGSWeapon* NewWeapon);

	/**
	 * @brief 返回与装配类型相匹配的插槽，如果没有找到就返回根节点。
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	FName GetEquipSocket(EGSEquipType EquipType) const;

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	void SetRifleSkeletalMesh(USkeletalMesh* m_Mesh) const { RifleMesh->SetSkeletalMesh(m_Mesh); }

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	void SetPistolSkeletalMesh(USkeletalMesh* m_Mesh) const { PistolMesh->SetSkeletalMesh(m_Mesh); }

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	void SetBowSkeletalMesh(USkeletalMesh* m_Mesh) const { BowMesh->SetSkeletalMesh(m_Mesh); }

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	void ShowPistolMesh(bool State) const { PistolMesh->SetVisibility(State); }

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	void ShowRifleMesh(bool State) const { RifleMesh->SetVisibility(State); }

	UFUNCTION(BlueprintCallable, Category = "GAS|Inventory")
	void ShowBowMesh(bool State) const { BowMesh->SetVisibility(State); }


	/**
	* Interactable interface
	*/

	/**
	* 可以在以下情况下与其他英雄进行互动:  
	* 被击倒了 - 可以去恢复他们
	*/
	virtual bool IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent) const override;

	/**
	* 与玩家互动时间
	* Knocked Down - to revive them
	*/
	virtual float GetInteractionDuration_Implementation(UPrimitiveComponent* InteractionComponent) const override;

	/**
	* Interaction:
	* Knocked Down - activate revive GA (plays animation)
	*/
	virtual void
	PreInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent) override;

	/**
	* Interaction:
	* Knocked Down - apply revive GE
	*/
	virtual void
	PostInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent) override;

	/**
	* Should we wait and who should wait to sync before calling PreInteract():
	* Knocked Down - Yes, client. This will sync the local player's Interact Duration Timer with the knocked down player's
	* revive animation. If we had a picking a player up animation, we could play it on the local player in PreInteract().
	*/
	virtual void GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type,
	                                                   UPrimitiveComponent* InteractionComponent) const override;

	/**
	* Cancel interaction:
	* Knocked Down - cancel revive ability
	*/
	virtual void CancelInteraction_Implementation(UPrimitiveComponent* InteractionComponent) override;

	/**
	* Get the delegate for this Actor canceling interaction:
	* Knocked Down - cancel being revived if killed
	*/
	virtual FSimpleMulticastDelegate*
	GetTargetCancelInteractionDelegate(UPrimitiveComponent* InteractionComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * @brief 判断该武器是否已经存在。
	 */
	bool DoesWeaponExistInInventory(const AGSWeapon* InWeapon);

	UFUNCTION()
	void OnRep_Inventory();

	UFUNCTION()
	void OnRep_CurrentWeapon(AGSWeapon* LastWeapon);


	// The CurrentWeapon is only automatically replicated to simulated clients.
	// The autonomous client can use this to request the proper CurrentWeapon from the server when it knows it may be
	// out of sync with it from predictive client-side changes.
	UFUNCTION(Server, Reliable)
	void ServerSyncCurrentWeapon();
	void ServerSyncCurrentWeapon_Implementation();
	static bool ServerSyncCurrentWeapon_Validate();

	// The CurrentWeapon is only automatically replicated to simulated clients.
	// Use this function to manually sync the autonomous client's CurrentWeapon when we're ready to.
	// This allows us to predict weapon changes (changing weapons fast multiple times in a row so that the server doesn't
	// replicate and clobber our CurrentWeapon).
	UFUNCTION(Client, Reliable)
	void ClientSyncCurrentWeapon(AGSWeapon* InWeapon);
	void ClientSyncCurrentWeapon_Implementation(AGSWeapon* InWeapon);
	static bool ClientSyncCurrentWeapon_Validate(AGSWeapon* InWeapon);

	/**
	 * @brief 设置当前武器
	 */
	void SetCurrentWeapon(AGSWeapon* NewWeapon, const AGSWeapon* LastWeapon);

public:
	FGameplayTag CurrentWeaponTag;

	UPROPERTY(BlueprintReadOnly, Category = "GAS|PickUp")
	AGSPickup* LastTouchItem;

	// 测试使用的点
	UPROPERTY(BlueprintReadOnly, Category = "GAS|Debug")
	FVector TestPoint;

	

protected:
	bool bASCInputBound = false;

	// 在蓝图实例中进行初始化，如果蓝图中没有默认初始化，对应的参数，就返回根插槽。
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS|Config")
	TMap<EGSEquipType, FName> EquipSockets;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS|Abilities")
	float ReviveDuration = 4.f;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS|Character")
	TSubclassOf<UGameplayEffect> ReviveEffect;

	FSimpleMulticastDelegate InteractionCanceledDelegate;

	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	FGSHeroInventory Inventory;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	AGSWeapon* CurrentWeapon;

	// 当我们预先更改武器时设置为true，当服务器进行复制时设置为false。
	// 如果服务器拒绝激活一个武器改变能力，要求服务器将客户端同步到正确的CurrentWeapon，我们就使用这个。
	bool bChangedWeaponLocally = false;


	// tag 缓存
	FGameplayTag NoWeaponTag;
	FGameplayTag WeaponChangingDelayReplicationTag;
	FGameplayTag WeaponAmmoTypeNoneTag;
	FGameplayTag WeaponAbilityTag;
	FGameplayTag KnockedDownTag;
	FGameplayTag InteractingTag;

	UPROPERTY(BlueprintReadOnly, Category = "GAS|Utility")
	AGASPlayerController* PlayerController = nullptr;

private:
	// 手枪 Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "GAS|Weapon")
	USkeletalMeshComponent* PistolMesh;

	// 步枪Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "GAS|Weapon")
	USkeletalMeshComponent* RifleMesh;

	// 弓箭 Mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "GAS|Weapon")
	USkeletalMeshComponent* BowMesh;

	// 动画扭曲组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category = "GAS|Animation")
	UMotionWarpingComponent* MotionWarpingComponent;
};
