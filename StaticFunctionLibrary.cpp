#include "FPSTemplate.h"
#include "StaticFunctionLibrary.h"
#include "Items/Firearm.h"

#define TeamColor FLinearColor(0.f, 1.f, 0.f, 1.f)
#define EnemyColor FLinearColor(1.f, 0.2f, 0.f, 1.f)

#define PlayerConfigSection TEXT("Player")

// Path of the item library blueprint
//#define ItemLibraryPath TEXT("Blueprint'/Game/FPSTemplate/Items/BP_ItemLibrary.BP_ItemLibrary_C'")

//UItemLibrary * UStaticFunctionLibrary::GetItemLibraryCDO()
//{
//	FStringAssetReference Reference(ItemLibraryPath);
//	FStreamableManager StreamableManager;
//	TSubclassOf<UItemLibrary> ItemLibraryClass = (UClass*)StreamableManager.SynchronousLoad(Reference);
//	return ItemLibraryClass.GetDefaultObject();
//}
//
//TArray<FItemProperties> UStaticFunctionLibrary::GetItemPropertiesArray()
//{
//	UItemLibrary * ItemLibrary = GetItemLibraryCDO();
//	if (ItemLibrary)
//	{
//		return ItemLibrary->Items;
//	}
//
//	// Return an empty array if ItemLibrary is not valid
//	return TArray<FItemProperties>();
//}
//
//FItemProperties UStaticFunctionLibrary::GetItemProperties(const FName & ItemName)
//{
//	for (const FItemProperties & ItemProperties : GetItemPropertiesArray())
//	{
//		if (ItemProperties.ItemClass && ItemProperties.ItemClass->GetFName() == ItemName)
//		{
//			return ItemProperties;
//		}
//	}
//
//	return FItemProperties();
//}

TSubclassOf<AItem> UStaticFunctionLibrary::GetItemClass(const UObject * WorldContextObject, const FName & ItemName)
{
	UBaseGameInstance * GameInstance = GetBaseGameInstance(WorldContextObject);
	if (GameInstance)
	{
		TArray<TSubclassOf<AItem>> Items;
		GameInstance->GetItems(Items);
		for (const TSubclassOf<AItem> & ItemClass : Items)
		{
			if (ItemClass->GetFName() == ItemName)
			{
				return ItemClass;
			}
		}
	}

	return NULL;
}

AItem * UStaticFunctionLibrary::GetItemCDO(TSubclassOf<AItem> ItemClass)
{
	return ItemClass.GetDefaultObject();
}

USight * UStaticFunctionLibrary::GetSightCDO(TSubclassOf<USight> SightClass)
{
	return SightClass.GetDefaultObject();
}

//AItem * UStaticFunctionLibrary::GetItemCDOFromName(const UObject * WorldContextObject, const FName & ItemName)
//{
//	return GetItemClass(WorldContextObject, ItemName).GetDefaultObject();
//}

//FName UStaticFunctionLibrary::GetItemClassName(TSubclassOf<AItem> ItemClass)
//{
//	if (ItemClass)
//	{
//		return ItemClass->GetFName();
//	}
//
//	return NAME_None;
//}

//TArray<FProjectileCollection> UStaticFunctionLibrary::GetProjectileCollectionsArray()
//{
//	UItemLibrary * ItemLibrary = GetItemLibraryCDO();
//	if (ItemLibrary)
//	{
//		return ItemLibrary->ProjectileCollections;
//	}
//
//	return TArray<FProjectileCollection>();
//}

//TArray<TSubclassOf<ABaseProjectile>> UStaticFunctionLibrary::GetItemCompatibleProjectilesArray(const UObject * WorldContextObject, const FName & ItemName)
//{
//	UBaseGameInstance * GameInstance = GetBaseGameInstance(WorldContextObject);
//	if (GameInstance)
//	{
//		AFirearm * ItemCDO = Cast<AFirearm>(GameInstance->GetItem(ItemName).GetDefaultObject());
//		if (ItemCDO)
//		{
//			ABaseProjectile * DefaultProjectile = ItemCDO->ProjectileClass.GetDefaultObject();
//			if (DefaultProjectile)
//			{
//				TArray<TSubclassOf<ABaseProjectile>> OutProjectiles;
//
//				TArray<TSubclassOf<ABaseProjectile>> Projectiles;
//				GameInstance->GetProjectiles(Projectiles);
//				for (const TSubclassOf<ABaseProjectile> & ProjectileClass : Projectiles)
//				{
//					ABaseProjectile * ProjectileCDO = ProjectileClass.GetDefaultObject();
//					if (ProjectileCDO && ProjectileCDO->CompatibilityTag == DefaultProjectile->CompatibilityTag)
//					{
//						OutProjectiles.Add(ProjectileClass);
//					}
//				}
//
//				return OutProjectiles;
//			}
//		}
//	}
//
//	return TArray<TSubclassOf<ABaseProjectile>>();
//}

//FProjectileProperties UStaticFunctionLibrary::GetProjectileProperties(const FName & ProjectileName)
//{
//	for (const FProjectileCollection & Collection : GetProjectileCollectionsArray())
//	{
//		for (const FProjectileProperties & Props : Collection.Projectiles)
//		{
//			if (Props.ProjectileClass && Props.ProjectileClass->GetFName() == ProjectileName)
//			{
//				return Props;
//			}
//		}
//	}
//
//	return FProjectileProperties();
//}
//
//FName UStaticFunctionLibrary::GetProjectileCartridge(const FName & ProjectileName)
//{
//	for (const FProjectileCollection & Collection : GetProjectileCollectionsArray())
//	{
//		for (const FProjectileProperties & Props : Collection.Projectiles)
//		{
//			if (Props.ProjectileClass && Props.ProjectileClass->GetFName() == ProjectileName)
//			{
//				return Collection.CartridgeName;
//			}
//		}
//	}
//
//	return NAME_None;
//}
//
//TArray<FSightProperties> UStaticFunctionLibrary::GetSightPropertiesArray()
//{
//	UItemLibrary * ItemLibrary = GetItemLibraryCDO();
//	if (ItemLibrary)
//	{
//		return ItemLibrary->Sights;
//	}
//
//	return TArray<FSightProperties>();
//}
//
//FSightProperties UStaticFunctionLibrary::GetSightProperties(const FName & SightName)
//{
//	if (SightName != NAME_None)
//	{
//		for (const FSightProperties & Props : GetSightPropertiesArray())
//		{
//			if (*Props.SightClass && Props.SightClass->GetFName() == SightName)
//			{
//				return Props;
//			}
//		}
//	}
//
//	return FSightProperties();
//}
//
//TArray<FAttachmentProperties> UStaticFunctionLibrary::GetAttachmentPropertiesArray()
//{
//	UItemLibrary * ItemLibrary = GetItemLibraryCDO();
//	if (ItemLibrary)
//	{
//		return ItemLibrary->Attachments;
//	}
//
//	return TArray<FAttachmentProperties>();
//}

FText UStaticFunctionLibrary::ClassNameToDisplayName(FString ClassName)
{
	ClassName.RemoveFromStart(TEXT("BP_"));
	ClassName.RemoveFromEnd(TEXT("_C"));
	ClassName.Replace(TEXT("_"), TEXT(" "));

	return FText::FromString(ClassName);
}

FLinearColor UStaticFunctionLibrary::GetPlayerColor(ABasePlayerState * OwnerPS, ABasePlayerState * OtherPS)
{
	if (OwnerPS && OwnerPS != OtherPS)
	{
		return OwnerPS->IsEnemyFor(OtherPS) ? EnemyColor : TeamColor;
	}

	return FLinearColor::White;
}

ABaseGameMode * UStaticFunctionLibrary::GetGameModeCDO(UObject * WorldContextObject)
{
	AGameState * GameState = UGameplayStatics::GetGameState(WorldContextObject);
	if (GameState)
	{
		return Cast<ABaseGameMode>(GameState->GetDefaultGameMode());
	}

	return NULL;
}

USaveGame * UStaticFunctionLibrary::LoadOrCreateSaveGame(const FString & SlotName, TSubclassOf<USaveGame> SaveGameClass, int32 UserIndex)
{
	USaveGame * SaveGame = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
	if (SaveGame)
	{
		return SaveGame;
	}

	return UGameplayStatics::CreateSaveGameObject(SaveGameClass);
}

ABaseWorldSettings * UStaticFunctionLibrary::GetWorldSettings(UObject * WorldContextObject)
{
	UWorld * World = GEngine->GetWorldFromContextObject(WorldContextObject);
	if (World)
	{
		return Cast<ABaseWorldSettings>(World->GetWorldSettings());
	}

	return NULL;
}

UBaseGameInstance * UStaticFunctionLibrary::GetBaseGameInstance(const UObject * WorldContextObject)
{
	UWorld * World = GEngine->GetWorldFromContextObject(WorldContextObject);
	if (World)
	{
		return Cast<UBaseGameInstance>(World->GetGameInstance());
	}

	return NULL;
}

UObject * UStaticFunctionLibrary::GetDefaultObject(TSubclassOf<UObject> InClass)
{
	return InClass.GetDefaultObject();
}

float UStaticFunctionLibrary::GetConsoleVariableFloat(const FString & VarName)
{
	const auto CVar = IConsoleManager::Get().FindConsoleVariable(*VarName);
	if (CVar)
	{
		return CVar->GetFloat();
	}

	return 0.f;
}

int32 UStaticFunctionLibrary::GetConsoleVariableInt(const FString & VarName)
{
	const auto CVar = IConsoleManager::Get().FindConsoleVariable(*VarName);
	if (CVar)
	{
		return CVar->GetInt();
	}

	return 0;
}

FString UStaticFunctionLibrary::GetConsoleVariableString(const FString & VarName)
{
	const auto CVar = IConsoleManager::Get().FindConsoleVariable(*VarName);
	if (CVar)
	{
		return CVar->GetString();
	}

	return FString();
}

void UStaticFunctionLibrary::GetScreenResolution(int32 & OutX, int32 & OutY)
{
	UGameUserSettings * UserSettings = GEngine->GetGameUserSettings();
	if (UserSettings)
	{
		const FIntPoint Point = UserSettings->GetScreenResolution();
		OutX = Point.X;
		OutY = Point.Y;
	}
}

void UStaticFunctionLibrary::SetScreenResolution(int32 X, int32 Y)
{
	UGameUserSettings * UserSettings = GEngine->GetGameUserSettings();
	if (UserSettings)
	{
		UserSettings->SetScreenResolution(FIntPoint(X, Y));
		UserSettings->RequestResolutionChange(X, Y, UserSettings->GetFullscreenMode());
		UserSettings->SaveConfig();
	}
}

uint8 UStaticFunctionLibrary::GetFullscreenMode()
{
	UGameUserSettings * UserSettings = GEngine->GetGameUserSettings();
	if (UserSettings)
	{
		return (uint8)UserSettings->GetFullscreenMode();
	}

	return EWindowMode::Fullscreen;
}

void UStaticFunctionLibrary::SetFullscreenMode(uint8 Mode)
{
	UGameUserSettings * UserSettings = GEngine->GetGameUserSettings();
	if (UserSettings)
	{
		const EWindowMode::Type WindowMode = (EWindowMode::Type)Mode;
		const FIntPoint Point = UserSettings->GetScreenResolution();
		UserSettings->SetFullscreenMode(WindowMode);
		UserSettings->RequestResolutionChange(Point.X, Point.Y, WindowMode);
		UserSettings->SaveConfig();
	}
}

FString UStaticFunctionLibrary::GetPlayerName()
{
	if (GConfig)
	{
		FString PlayerName;
		GConfig->GetString(PlayerConfigSection, TEXT("PlayerName"), PlayerName, GGameIni);
		return PlayerName;
	}

	return "NoName";
}

void UStaticFunctionLibrary::SetPlayerName(APlayerController * PC, const FString & NewName)
{
	if (!NewName.IsEmpty() && GConfig)
	{
		GConfig->SetString(PlayerConfigSection, TEXT("PlayerName"), *NewName, GGameIni);
		GConfig->Flush(false, GGameIni);
		PC->SetName(NewName);
	}
}