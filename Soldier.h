#pragma once

#include "Projectiles/ThrowableProjectile.h"
#include "Soldier/InteractionInterface.h"
#include "Game/LoadoutBuilderComponent.h"
#include "Projectiles/BaseProjectile.h"
#include "Player/BasePlayerState.h"
#include "Items/Item.h"
#include "GameFramework/Character.h"
#include "Soldier.generated.h"

#define NumItemSlots 7
#define FastAnimationRate 1.5f

class ABasePlayerController;

UENUM()
enum class ESoldierTaskEnum : uint8
{
	None,
	FullBody,
	Interact,
};

USTRUCT()
struct FDamageInfo
{
	GENERATED_BODY()

	UPROPERTY()
		ABasePlayerController * Instigator = NULL;

	UPROPERTY()
		float DamageAmount;

	FDamageInfo()
	{

	}

	FDamageInfo(ABasePlayerController * InInstigator, float InDamageAmount)
	{
		Instigator = InInstigator;
		DamageAmount = InDamageAmount;
	}
};

// Struct used to send player input to the server
USTRUCT()
struct FSoldierInputStruct
{
	GENERATED_BODY()

	UPROPERTY()
		bool bPressedFire;

	UPROPERTY()
		bool bPressedAim;

	UPROPERTY()
		bool bPressedSprint;

	UPROPERTY()
		bool bPressedInteract;

	UPROPERTY()
		//TEnumAsByte<ELeanDirection> LeanDirection;
		float LeanRatio;

	FSoldierInputStruct()
	{
		
	}

	FSoldierInputStruct(bool bFiring, bool bAiming, bool bSprinting, bool bInteracting, float InLeanRatio)
	{
		bPressedFire = bFiring;
		bPressedAim = bAiming;
		bPressedSprint = bSprinting;
		bPressedInteract = bInteracting;
		LeanRatio = InLeanRatio;
	}
};

UCLASS()
class FPSTEMPLATE_API ASoldier : public ACharacter
{
	GENERATED_BODY()

public:
	// Properties
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animations)
		UAnimMontage * PickUpMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animations)
		UAnimMontage * PushMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Animations)
		UAnimMontage * SupplyMontage;

	//Runtime variables

	// Main first person camera
	UPROPERTY(EditDefaultsOnly, Category = Soldier)
		UCameraComponent * Camera;

	// Third person camera
	UPROPERTY(EditDefaultsOnly, Category = Soldier)
		UCameraComponent * CameraTP;

	UPROPERTY(EditDefaultsOnly, Category = Soldier)
		USpringArmComponent * CameraBoom;

private:
	UPROPERTY()
		USceneCaptureComponent2D * SceneCapture;

	// Health of a soldier (0-100), replicated
	UPROPERTY(ReplicatedUsing = OnRep_Health)
		float Health;

	UFUNCTION()
		void OnRep_Health();

	UPROPERTY(ReplicatedUsing = OnRep_CharacterMesh)
		USkeletalMesh * CharacterMesh;

		UFUNCTION()
			void OnRep_CharacterMesh();

	// Array of info structs about damage applied
	TArray<FDamageInfo> DamageInfoArray;

	// Static array containing all items, replicated
	UPROPERTY(Replicated)
		AItem * Items[NumItemSlots];

	// Stores the item which is currently drawn, replicated
	UPROPERTY(ReplicatedUsing = OnRep_CurrentItem)
		AItem * CurrentItem;

	UFUNCTION()
		void OnRep_CurrentItem(AItem * OldItem);

	// Stores the index of the next item that will be drawn
	UPROPERTY()
		uint8 NextSlot;

	// Stores the item task current performed, replicated
	UPROPERTY()
		TEnumAsByte<EItemTaskEnum::Type> ItemTask;

	UFUNCTION()
		void OnRep_ItemTask(EItemTaskEnum::Type OldTask);

	// Timer handle used for item task
	FTimerHandle TimerHandle_ItemTask;

	UPROPERTY(ReplicatedUsing = OnSoldierTaskChanged)
		ESoldierTaskEnum SoldierTask;

	UPROPERTY()
		UAnimMontage * SoldierTaskMontage;

	UFUNCTION()
		void OnSoldierTaskChanged();

	// Timer handle used for soldier task
	FTimerHandle TimerHandle_SoldierTask;

	// Whether item should instantly be fired after old one was undrawn
	bool bInstantFireItem;

	UPROPERTY(ReplicatedUsing = OnRep_InteractionActor)
		AActor * InteractionActor;

	UFUNCTION()
		void OnRep_InteractionActor();

	// Timer handle for interaction interface
	//FTimerHandle TimerHandle_Interact;

	// Perk this player select, for example faster reload
	UPROPERTY(Replicated)
		EPerkEnum Perk;

	// Counts, bullets, used for recoil and accuracy per shot
	float BulletCount;

	// Target vertical recoil
	float TargetPitchRecoil;

	// Current vertical recoil
	float PitchRecoil;

	// Target horizontal recoil
	float TargetYawRecoil;

	// Current horizontal recoil
	float YawRecoil;

	// Target item recoil offset
	FVector TargetItemRecoil;

	// Current recoil offset of item
	FVector ItemRecoil;

	// Cached value of view pitch, needed for arms lag
	float OldViewPitchRotation;

	// Cached value of view yaw, needed for arms lag
	float OldViewYawRotation;

	// Arms rotation offset for lag
	FRotator ArmsLagRotation;

public:
	// Is player currently in third person camera mode
	bool bInThirdPersonMode;

	// True if player wants to sprint
	bool bSprinting;

	// True if player is pressing fire button, else false
	bool bFiring;

	// True if player is aiming, else false
	UPROPERTY(Replicated)
		bool bAiming;

	// Ration at which the the soldier is zoomed in, 0.0 - 1.0
	float AimRatio;

	bool bInteracting;

	UPROPERTY(Replicated)
		float LeanRatio;

	// Initialize variables and components
	ASoldier(const FObjectInitializer & ObjectInitializer);

	void SetupPlayerInputComponent(class UInputComponent * InputComponent) override;

	void Reset() override;

	void UnPossessed() override;

	void TurnOff() override;

	void Tick(float DeltaSeconds) override;

	// Override these functions to adjust capsule component children when crouching
	void OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust) override;

	void OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust) override;

	// Called when any kind of damage is applied, subtracts health and kills soldier
	float TakeDamage(float DamageAmount, const FDamageEvent & DamageEvent, AController * EventInstigator, AActor * DamageCauser) override;

	// Called on server and clients
	UFUNCTION(NetMulticast, Reliable)
		void MulticastTookDamage(float DamageAmount, AActor * Causer);

	// Heal this soldier, e.g. called from Medic Kits
	void Heal(float Amount);

	// Returns whether health <= 0.0
	UFUNCTION(BlueprintPure, Category = Soldier)
		bool IsDead() const;

	bool HasFullHealth() const;

	UFUNCTION(BlueprintPure, Category = Soldier)
		float GetHealth() const { return Health; };

	const TArray<FDamageInfo> & GetDamageInfoArray() const;

	// Ammo
	bool AllItemsFullAmmo() const;

	void RefillAmmo(bool bAllItems);

	// Overriden to apply fall damage
	void Landed(const FHitResult & Hit) override;

	// Set soldier yaw rotation based on new control rotation
	void FaceRotation(FRotator NewControlRotation, float DeltaTime) override;

	// Draw HUD for soldier (e.g. crosshair)
	void PostRenderFor(APlayerController * PC, UCanvas * Canvas, FVector CameraPosition, FVector CameraDir) override;

	void BecomeViewTarget(APlayerController * PC) override;

	void EndViewTarget(APlayerController * PC) override;

	// Select camera for view
	void CalcCamera(float DeltaTime, FMinimalViewInfo & OutResult) override;

	// Return true if owned by local player or being spectated
	bool IsViewTarget() const;

	// Called when soldier is aimed in
	void OnStartedAiming();

	// Called when soldier was aimed in and stopped aiming
	void OnStoppedAiming();

	// Returns the view pitch rotation
	UFUNCTION(BlueprintPure, Category = Soldier)
		float GetViewPitch() const;

	// Returns camera location and rotation
	void GetActorEyesViewPoint(FVector & OutLocation, FRotator & OutRotation) const override;

	// Returns location and rotation offset of arms, in world space
	void GetArmsOffset(FVector & OutOffset, FRotator & OutRotation) const;

	UFUNCTION(BlueprintPure, Category = Interaction)
		float GetInteractionPercentage() const;

	// Return variable CurrentItem
	UFUNCTION(BlueprintPure, Category = Soldier)
		AItem * GetCurrentItem() const;

	bool HasPerk(EPerkEnum InPerk) const;

	// Set loadout, i.e. spawn all items from it
	void SetLoadout(const FLoadoutStruct & Loadout, USkeletalMesh * CustomMesh = NULL);

	// Spawn an item and add it to inventory
	AItem * SpawnItem(TSubclassOf<AItem> ItemClass);

	// Add an existing item to inventory
	void AddItem(AItem * Item);

	// Initialize CurrentItem, amongst other things attach it to hand bone
	void InitCurrentItem();

	// Store non active item
	void StoreItem(AItem * Item);

	// Perform an item task, e.g. draw, fire or reload
	bool PerformItemTask(EItemTaskEnum::Type Task, float Length);

	// Stop current item task and animation
	void StopItemTask();

private:
	UFUNCTION(NetMulticast, Reliable)
		void MulticastSetItemTask(EItemTaskEnum::Type NewTask);

	// Called when item task was finished
	void OnPerformedItemTask();

public:
	bool PerformSoldierTask(ESoldierTaskEnum Task, UAnimMontage * Montage, float Length = -1.f);

	void StopSoldierTask();

private:
	void OnPerformedSoldierTask();

public:
	// Called from old item when it is undrawn
	void OnUndrewItem();

	// Called from item to play animation
	void PlayItemAnimation(UAnimSequenceBase * Sequence, float BlendInTime = 0.25f, float BlendOutTime = 0.25f, float PlaybackSpeed = 1.f);

	// Spawn projectile and add recoil, called from firearm
	void FireProjectile(TSubclassOf<ABaseProjectile> ProjectileClass, UCurveVector * RecoilCurve, const FVector & VisualRecoil);

	void ThrowProjectile(TSubclassOf<AThrowableProjectile> ProjectileClass);

	// Called when a firearm fired on client and is in auto firemode
	void OnClientStartAutoFire(float FireLength);

	// Called fire length seconds after OnClientStartAutoFire
	void OnClientAutoFire();

	// Called from a knife item, apply damage to actors hit by the knife and return whether something was hit
	bool Stab(FHitResult & OutHit, float Damage, float MaxDistance);

	// Returns the team from PlayerState
	ETeamEnum GetTeam() const;

	// Return whether this soldier is an enemy for OtherSoldier
	bool IsEnemyFor(ASoldier * OtherSoldier) const;

	// Return whether this soldier is an enemy for OtherPlayerState
	bool IsEnemyFor(ABasePlayerState * OtherPlayerState) const;

	// Return whether this soldier is an enemy for OtherPlayerController
	UFUNCTION(BlueprintPure, Category = Soldier)
		bool IsEnemyFor(ABasePlayerController * OtherPlayerController) const;

private:
	// Send client input to server
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerUpdateInput(const FSoldierInputStruct & PlayerInput);

private:
	// Misc input
	// Interact, i.e. pick up items, enter vehicles etc.
	void StartInteract();

private:
	bool InteractWith(AActor * TargetActor);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerInteractWith(AActor * TargetActor);

	bool CanInteractWith(AActor * InterfaceActor);

public:
	UFUNCTION(BlueprintCallable, Category = Soldier)
		AActor * GetBestInteractionTarget();

	UFUNCTION(BlueprintCallable, Category = Soldier)
		FVector GetInteractionLocation(AActor * Actor);

	void StopInteract();

private:
	void OnFinishedInteraction();

public:
	// Pick up an item within range
	/*void PickUpItem();

private:*/
	void PickUpItem(AItem * Item);

//	UFUNCTION(Server, Reliable, WithValidation)
//		void ServerPickUpItem(AItem * Item);
//	// End of misc input
//
//public:
	// Movement input
	void MoveForward(float Val);

	void MoveRight(float Val);

	// Look input
	void AddControllerPitchInput(float Val);

	void AddControllerYawInput(float Val);

	// Start and stop sprinting
	void StartSprinting();

	void StopSprinting();

	// Start and stop crouching, replicated through movement component
	void StartCrouching();

	void StopCrouching();
	// End of movement input

	// Item input
	// Called when fire button was pressed, server
	void StartFiring();

	// Called when fire button was released, server
	void StopFiring();

	// Events for start and stop aiming, sets bAiming
	void StartAiming();

	void StopAiming();

	// Toggle the fire mode of CurrentItem, server
	void ToggleFireMode();

	void ToggleCameraMode();

private:
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerToggleFireMode();

public:
	// Called to reload
	void Reload();

private:
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerReload();

public:
	// Drop CurrentItem
	void DropItem();

private:
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerDropItem();

public:
	// Switch to specific slot
	void EquipSlot0();

	void EquipSlot1();

	void EquipSlot2();

	void EquipSlot3();

	void EquipSlot4();

	void EquipSlot5();

	void EquipSlot6();

	// Equip next item in inventory
	void EquipNextItem();

	// Equip previous item in inventory
	void EquipPreviousItem();
	// End of item input

	// Draw item from items at specified index, server
	void SetSlot(uint8 NewSlot, bool bInstantFire = false);

private:
	UFUNCTION(Server, Reliable, WithValidation)
		void ServerSetSlot(uint8 NewSlot, bool bInstantFire);

public:
	// Equip grenade and instantly throw
	void ThrowGrenade();

	// Take out knife and attack
	void Knife();

	template<class ItemType>
	void EquipFirstItemOfClass(bool bInstantFire);

};

template<class ItemType>
inline void ASoldier::EquipFirstItemOfClass(bool bInstantFire)
{
	if (CurrentItem && CurrentItem->IsA<ItemType>())
	{
		if (bInstantFire)
		{
			CurrentItem->StartFiring();
		}

		return;
	}

	uint8 Slot = 0;
	for (AItem * Item : Items)
	{
		if (Item && Item->IsA<ItemType>())
		{
			SetSlot(Slot, bInstantFire);
			break;
		}
		Slot++;
	}
}