#pragma once

#include "Projectiles/BaseProjectile.h"
#include "Items/Item.h"
#include "Engine/GameInstance.h"
#include "BaseGameInstance.generated.h"

UCLASS()
class FPSTEMPLATE_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
	FStreamableManager StreamableManager;

	// Asset library for all levels
	UPROPERTY()
		UObjectLibrary * LevelLibrary;

	// Asset library for item classes
	UPROPERTY()
		UObjectLibrary * ItemLibrary;

	// Asset library for attachment classes
	UPROPERTY()
		UObjectLibrary * SightLibrary;

	UPROPERTY()
		UObjectLibrary * ProjectileLibrary;

public:
	UBaseGameInstance();

	UFUNCTION(BlueprintCallable, Category = Library)
		void GetLevels(TArray<FName> & OutLevels);

	UFUNCTION(BlueprintCallable, Category = Library)
		void GetItems(TArray<TSubclassOf<AItem>> & OutItems);

	UFUNCTION(BlueprintCallable, Category = Library)
		TSubclassOf<AItem> GetItem(const FName & ItemName);

	UFUNCTION(BlueprintCallable, Category = Library)
		void GetSights(TArray<TSubclassOf<USight>> & OutSights);

	UFUNCTION(BlueprintCallable, Category = Library)
		TSubclassOf<USight> GetSight(const FName & SightName);

	UFUNCTION(BlueprintCallable, Category = Library)
		void GetProjectiles(TArray<TSubclassOf<ABaseProjectile>> & OutProjectiles);

	UFUNCTION(BlueprintCallable, Category = Library)
		TSubclassOf<ABaseProjectile> GetProjectile(const FName & ProjectileName);

	UFUNCTION(BlueprintCallable, Category = Library)
		void GetItemPatternMaterials(TSubclassOf<AItem> ItemClass, TArray<FName> & OutMaterials);

	void GetItemPatternMaterialDatas(TSubclassOf<AItem> ItemClass, TArray<FAssetData> & OutDatas);

	UFUNCTION(BlueprintCallable, Category = Library)
		UMaterialInterface * GetItemPatternMaterial(TSubclassOf<AItem> ItemClass, const FName & MaterialName);

private:
	template<typename T>
	void GetAssetsFromLibrary(UObjectLibrary * Library, TArray<T *> & OutAssets);

	template<typename T>
	TSubclassOf<T> GetBlueprintClassFromLibrary(UObjectLibrary * Library, const FName & ClassName);

	template<typename T>
	void GetBlueprintClassesFromLibrary(UObjectLibrary * Library, TArray<TSubclassOf<T>> & OutClasses);

};

template<typename T>
inline void UBaseGameInstance::GetAssetsFromLibrary(UObjectLibrary * Library, TArray<T *> & OutAssets)
{
	OutAssets.Empty();

	TArray<FAssetData> AssetDataArray;
	Library->GetAssetDataList(AssetDataArray);

	for (int32 i = 0; i < AssetDataArray.Num(); ++i)
	{
		FAssetData & AssetData = AssetDataArray[i];

		T * Asset = Cast<T>(StreamableManager.SynchronousLoad(AssetData.ToStringReference()));
		if (Asset)
		{
			OutAssets.Add(Asset);
		}
	}
}

template<typename T>
inline TSubclassOf<T> UBaseGameInstance::GetBlueprintClassFromLibrary(UObjectLibrary * Library, const FName & ClassName)
{
	TArray<UBlueprintGeneratedClass *> ClassesArray;
	Library->GetObjects<UBlueprintGeneratedClass>(ClassesArray);

	for (int32 i = 0; i < ClassesArray.Num(); ++i)
	{
		UClass * BlueprintClass = ClassesArray[i];
		if (BlueprintClass->GetFName() == ClassName)
		{
			return BlueprintClass;
		}
	}

	return NULL;
}

template<typename T>
inline void UBaseGameInstance::GetBlueprintClassesFromLibrary(UObjectLibrary * Library, TArray<TSubclassOf<T>> & OutClasses)
{
	OutClasses.Empty();

	TArray<UBlueprintGeneratedClass *> ClassesArray;
	Library->GetObjects<UBlueprintGeneratedClass>(ClassesArray);

	for (int32 i = 0; i < ClassesArray.Num(); ++i)
	{
		UBlueprintGeneratedClass * BlueprintClass = ClassesArray[i];
		if (!BlueprintClass->GetName().StartsWith("SKEL_"))
		{
			OutClasses.Add(BlueprintClass);
		}
	}
}