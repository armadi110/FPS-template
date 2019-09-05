#pragma once

#include "BaseGameInstance.h"
#include "BaseWorldSettings.h"
#include "Game/BaseGameMode.h"
#include "Player/BasePlayerState.h"
#include "Items/Item.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StaticFunctionLibrary.generated.h"

// Library for static functions, partially exposed to blueprints
UCLASS()
class FPSTEMPLATE_API UStaticFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Get the class default object of item library
	//static UItemLibrary * GetItemLibraryCDO();
	
	// Get variable Items from item library
	//UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
	//	static TArray<FItemProperties> GetItemPropertiesArray();

	//// Get item properties from item class name
	//UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
	//	static FItemProperties GetItemProperties(const FName & ItemName);

	// Get item class from item class name
	UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary, Meta = (WorldContext = WorldContextObject))
		static TSubclassOf<AItem> GetItemClass(const UObject * WorldContextObject, const FName & ItemName);

	// Get item class default object for item class name
	UFUNCTION(BlueprintPure, Category = StaticFunctionLibrary)
		static AItem * GetItemCDO(TSubclassOf<AItem> ItemClass);

	UFUNCTION(BlueprintPure, Category = StaticFunctionLibrary)
		static USight * GetSightCDO(TSubclassOf<USight> SightClass);

	// Get item class default object for item class name
	/*UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrar, Meta = (WorldContext = WorldContextObject))
		static AItem * GetItemCDOFromName(const UObject * WorldContextObject, const FName & ItemName);*/

	// Helper to return the class name from an item properties struct
	/*UFUNCTION(BlueprintPure, Category = StaticFunctionLibrary)
		static FName GetItemClassName(TSubclassOf<AItem> ItemClass);*/

	// Return all projectile types
	/*UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
		static TArray<TSubclassOf<ABaseProjectile>> GetProjectilesArray();*/

	/*UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary, Meta = (WorldContext = WorldContextObject))
		static TArray<TSubclassOf<ABaseProjectile>> GetItemCompatibleProjectilesArray(const UObject * WorldContextObject, const FName & ItemName);*/

	//// Return props of a specific projectile type
	//UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
	//	static FProjectileProperties GetProjectileProperties(const FName & ProjectileName);

	//// Return the cartridge of a specific projectile
	//UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
	//	static FName GetProjectileCartridge(const FName & ProjectileName);

	//// Get all sights from item library
	//UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
	//	static TArray<FSightProperties> GetSightPropertiesArray();

	//// Return properties of sight with given name
	//UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
	//	static FSightProperties GetSightProperties(const FName & SightName);

	//// Get all attachments from item library
	//UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
	//	static TArray<FAttachmentProperties> GetAttachmentPropertiesArray();

	// Convert the name of a class to a text without prefixes and underscores
	UFUNCTION(BlueprintPure, Category = StaticFunctionLibrary)
		static FText ClassNameToDisplayName(FString ClassName);

	// Return color to paint a player in
	UFUNCTION(BlueprintPure, Category = StaticFunctionLibrary)
		static FLinearColor GetPlayerColor(ABasePlayerState * OwnerPS, ABasePlayerState * OtherPS);

	// Return class default object of game mode class
	UFUNCTION(BlueprintPure, Category = StaticFunctionLibrary, Meta = (WorldContext = WorldContextObject))
		static ABaseGameMode * GetGameModeCDO(UObject * WorldContextObject);

	// Return a valid save game object, creates a new one if neccessary
	UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
		static USaveGame * LoadOrCreateSaveGame(const FString & SlotName, TSubclassOf<USaveGame> SaveGameClass, int32 UserIndex);

	UFUNCTION(BlueprintPure, Category = StaticFunctionLibrary, Meta = (WorldContext = WorldContextObject))
		static ABaseWorldSettings * GetWorldSettings(UObject * WorldContextObject);

	UFUNCTION(BlueprintPure, Category = StaticFunctionLibrary, Meta = (WorldContext = WorldContextObject))
		static UBaseGameInstance * GetBaseGameInstance(const UObject * WorldContextObject);

	// Return the class default object of any class
	UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
		static UObject * GetDefaultObject(TSubclassOf<UObject> InClass);

	// Console
	UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
		static float GetConsoleVariableFloat(const FString & VarName);

	UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
		static int32 GetConsoleVariableInt(const FString & VarName);

	UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
		static FString GetConsoleVariableString(const FString & VarName);

	// Game user settings
	UFUNCTION(BlueprintPure, Category = StaticFunctionLibrary)
		static void GetScreenResolution(int32 & OutX, int32 & OutY);

	UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
		static void SetScreenResolution(int32 X, int32 Y);

	UFUNCTION(BlueprintPure, Category = StaticFunctionLibrary)
		static uint8 GetFullscreenMode();

	UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
		static void SetFullscreenMode(uint8 Mode);

	// Config
	UFUNCTION(BlueprintPure, Category = StaticFunctionLibrary)
		static FString GetPlayerName();

	UFUNCTION(BlueprintCallable, Category = StaticFunctionLibrary)
		static void SetPlayerName(APlayerController * PC, const FString & NewName);
	
};
