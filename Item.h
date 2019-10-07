#pragma once

#include "Soldier/InteractionInterface.h"
#include "Attachments/SightComponent.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

// Enum for the item task
UENUM()
namespace EItemTaskEnum
{
	enum Type
	{
		None,
		Draw,
		Undraw,
		Fire,
		Reload,
		ReloadLong,
		ReloadChamber,
		Max,
	};
}

// Enum for the different item slots
UENUM(BlueprintType)
namespace EItemSlotEnum
{
	enum Type
	{
		None,
		Primary,
		Secondary,
		Knife,
		Grenade,
		Max,
	};
}

// Struct to store information about an item animation, e.g. draw, fire or reload animation
USTRUCT()
struct FItemAnimation
{
	GENERATED_BODY()

	// Sequence which is played on item mesh
	UPROPERTY(EditDefaultsOnly, Category = Defaults)
		UAnimSequenceBase * Sequence;

	// Sequence played on soldier mesh
	UPROPERTY(EditDefaultsOnly, Category = Defaults)
		UAnimSequenceBase * CharacterSequence;

	// Length of actual task, i.e. when a new task can be started
	UPROPERTY(EditDefaultsOnly, Category = Defaults)
		float Length;
};

// Base class for all usable items. For specific functionality like firing look at child classes (e.g. Items/Firearm.h, Items/Grenade.h)
UCLASS()
class FPSTEMPLATE_API AItem : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	// Components
	UPROPERTY(EditAnywhere, Category = Components)
		USkeletalMeshComponent * Mesh;

	// Sight component has to be set to visible for items which may have a sight
	UPROPERTY(EditAnywhere, Category = Components)
		USightComponent * SightComponent;

	// Thumbnail for this item used in customization
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Item)
		UTexture2D * ItemThumbnail;

	// Slot of the item, can be in primary, secondary, knife etc. slot
	UPROPERTY(EditDefaultsOnly, Category = Item)
		TEnumAsByte<EItemSlotEnum::Type> ItemSlot;

	// Index of material which should be replaced with pattern materials, 0 for most cases
	UPROPERTY(EditDefaultsOnly, Category = Item)
		uint8 PatternMaterialSlot;

	// Array of pattern materials which can be assigned in customization menu
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Item)
		TArray<UMaterialInstance *> PatternMaterials;*/

	// Index of pattern material to use for item skin, replicated initially (see OnRep_PatternMaterialIndex for actual assignment of material)
	/*UPROPERTY(ReplicatedUsing = OnRep_PatternMaterialIndex)
		int16 PatternMaterialIndex;*/

	UPROPERTY(ReplicatedUsing = OnRep_PatternMaterialName)
		FName PatternMaterialName;

private:
	UFUNCTION()
		void OnRep_PatternMaterialName();

public:
	// Texture used for crosshair
	UPROPERTY(EditDefaultsOnly, Category = Item)
		UTexture2D * CrosshairTexture;

	// Anim blueprint class used when this item is currently equipped
	UPROPERTY(EditDefaultsOnly, Category = Item)
		TSubclassOf<UAnimInstance> ActiveAnimBlueprint;

	// Character animations
	UPROPERTY(EditDefaultsOnly, Category = Animations)
		UAnimSequenceBase * CharacterIdleSequence;

	UPROPERTY(EditDefaultsOnly, Category = Animations)
		UAnimSequenceBase * CharacterAimSequence;

	UPROPERTY(EditDefaultsOnly, Category = Animations)
		UAnimSequenceBase * CharacterSprintSequence;

private:
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedLocation)
		FVector ReplicatedLocation;

	UFUNCTION()
		void OnRep_ReplicatedLocation();

public:
	// Task animations
	UPROPERTY(EditDefaultsOnly, Category = Animations)
		FItemAnimation DrawAnimation;

	UPROPERTY(EditDefaultsOnly, Category = Animations)
		FItemAnimation UndrawAnimation;

	// Create components and set default values
	AItem(const FObjectInitializer & ObjectInitializer);

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	virtual void Reset() override;

	virtual void Tick(float DeltaTime) override;

	// Returns the owning soldier (casted from variable Instigator), class ASoldier is a forward declaration
	class ASoldier * GetOwningSoldier() const;

	// Return whether soldier wants to fire
	bool IsSoldierFiring() const;

	// Apply item loadout, i.e. camouflage, sight, attachments
	virtual void SetLoadout(const struct FLoadoutSlotStruct & LoadoutSlot);

	// Called when started to be the active item
	void BecomeActiveItem();

	// Called when no longer active item
	void EndActiveItem();

	bool IsActiveItem() const;

	void BecomeViewTarget(APlayerController * PC) override;

	void EndViewTarget(APlayerController * PC) override;

	// Return whether this item can be dropped
	virtual bool CanBeDropped() const;

	// Called from soldier to drop this item
	void Drop();

	// Return whether this item can be picked up
	virtual bool CanBePickedUp() const;

	// Called from soldier when this item is being picked up
	void PickUp(ASoldier * Soldier);

	// Calls PerformItemTask on owning soldier
	bool PerformTask(EItemTaskEnum::Type Task, float Length);

	// Called from soldier when this == CurrentItem and ItemTask was replicated
	void OnRep_Task(EItemTaskEnum::Type Task);

	// Called from soldier when task finished
	void OnPerformedTask(EItemTaskEnum::Type Task);

	// Play an item animation on own mesh and on soldier
	void PlayAnimation(const FItemAnimation & Animation, float BlendInTime = 0.25f, float BlendOutTime = 0.25f, float PlaybackSpeed = 1.f);

	// Return precision of item, used for firearms
	virtual float GetImprecision(float AimRatio, float BulletCount, const FVector & SoldierVelocity) const;

	// Return whether this item can aim with current task
	virtual bool CanAim() const;

	// Returns the Z offset of item when aiming, 0 by default
	float GetEyeHeight() const;

	// Set aim sequence and update soldier anim instance if neccessary
	void SetCharacterAimSequence(UAnimSequenceBase * NewSequence);

	// Get magazine for HUD
	UFUNCTION(BlueprintPure, Category = Item)
		virtual int32 GetMagazine() const;

	// Get ammunition for HUD
	UFUNCTION(BlueprintPure, Category = Item)
		virtual int32 GetAmmunition() const;

	// Return if this item is in auto mode for HUD
	UFUNCTION(BlueprintPure, Category = Item)
		virtual bool GetIsAuto() const;

	virtual bool HasFullAmmo() const { return true; };

	virtual void RefillAmmo() {};

	// Draw item HUD (crosshair)
	virtual void DrawItemHUD(APlayerController * PC, UCanvas * Canvas, FVector CameraPosition, FVector CameraDir, float AimRatio, float BulletCount, const FVector & SoldierVelocity);

	// Handle events for player starting and stopping to fire
	virtual void StartFiring();

	virtual void StopFiring();

	virtual void ToggleFireMode();

	// Tasks
	void Draw();

	void Undraw();

	virtual void Fire();

	// Reload a new magazine
	virtual void Reload();

protected:
	// Reload with empty magazine and no bullet in chamber
	virtual void ReloadLong();

	// Reload a new bullet into chamber, usually after reload
	virtual void ReloadChamber();
	// End of tasks

	// Task notifies
	virtual void OnDrew();

	void OnUndrew();

	virtual void OnFired();

	virtual void OnReloaded();

	virtual void OnReloadedLong();

	virtual void OnReloadedChamber();
	// End of task notifes

public:
	// Task events
	void OnDraw();

	void OnUndraw();

	virtual void OnFire();

protected:
	virtual void OnReload();

	virtual void OnReloadLong();

	virtual void OnReloadChamber();
	// End of task events

public:
	void OnRep_Instigator() override;

	// Interaction interface
	virtual bool CanSoldierInteract(ASoldier * Soldier) const override;

	virtual float GetInteractionLength() const override;

	virtual FVector GetInteractionLocation() const override;

	virtual UAnimMontage * GetInteractionMontage(ASoldier * Soldier) const override;

	virtual void OnInteract(ASoldier * Soldier) override;
	
};
