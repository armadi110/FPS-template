#pragma once

#include "World/CustomizationScreen.h"
#include "GameFramework/WorldSettings.h"
#include "BaseWorldSettings.generated.h"

USTRUCT()
struct FMiniMapFloorStruct
{
	GENERATED_BODY()

	// Height of the floor for this mini map
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = MiniMap)
		float FloorHeight;

	// Texture used in the mini map
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = MiniMap)
		UTexture2D * MiniMapTexture;
};

UCLASS()
class FPSTEMPLATE_API ABaseWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
	
public:
	// Camera being used as a view target in spawn screen etc.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WorldSettings)
		TArray<ACameraActor *> MenuCameras;

	// Instance of ACustomizationScreen in the level
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WorldSettings)
		ACustomizationScreen * CustomizationScreen;

	// Mesh to use for players in team A or all players in FFA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WorldSettings)
		TArray<USkeletalMesh *> TeamASoldierMeshes;
	
	// Mesh for team B players
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = WorldSettings)
		TArray<USkeletalMesh *> TeamBSoldierMeshes;

	// Foors for all mini maps
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = MiniMap)
		TArray<FMiniMapFloorStruct> MiniMapFloors;

	// Center of the minimap in X and Y world coordinates
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = MiniMap)
		FVector2D MiniMapOrigin;

	// World size representation of the mini map texture in X and Y direction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = MiniMap)
		FVector2D MiniMapSize;
	
};
