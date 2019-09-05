#pragma once

#include "Soldier/Soldier.h"
#include "Items/Item.h"
#include "Animation/AnimInstance.h"
#include "SoldierAnimInstance.generated.h"

UCLASS()
class FPSTEMPLATE_API USoldierAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, Category = Defaults)
		ASoldier * Soldier;

	// Whether the character is seen from first person view
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Defaults)
		bool bFirstPersonMode;

	// View pitch rotation of soldier
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Defaults)
		float ViewPitch;

	// Amount the player is leaning to the side
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Defaults)
		float LeanRatio;

	// Ration at which soldier is zoomed in, 0.0-1.0
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Defaults)
		float AimRatio;

	// Offset of arms
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Defaults)
		FVector ArmsOffset;

	// Rotation offset of arms
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Defaults)
		FRotator ArmsRotation;

	// Movement speed of the soldier in 0-1
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Defaults)
		float SpeedRatio;

	// Delayed movement speed of upper body for smooth transitions
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Defaults)
		float UpperBodySpeedRatio;

	// Movement direction in -180 to 180 degrees
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Defaults)
		float Direction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Defaults)
		TEnumAsByte<EMovementMode> MovementMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Defaults)
		bool bCrouching;

	// Item specific sequences
	UPROPERTY(BlueprintReadOnly, Category = ItemSequences)
		UAnimSequenceBase * ItemIdleSequence;

	UPROPERTY(BlueprintReadOnly, Category = ItemSequences)
		UAnimSequenceBase * ItemAimSequence;

	UPROPERTY(BlueprintReadOnly, Category = ItemSequences)
		UAnimSequenceBase * ItemSprintSequence;

	// Initialize variables
	void NativeInitializeAnimation() override;

	// Called from soldier when a new item is equipped
	void InitItem(AItem * Item);

	// Update variables
	void NativeUpdateAnimation(float DeltaTime) override;
	
	
};
