#include "FPSTemplate.h"
#include "BaseCheatManager.h"
#include "StaticFunctionLibrary.h"
#include "Soldier/Soldier.h"
#include "Items/Item.h"
#include "BaseGameInstance.h"

void UBaseCheatManager::GiveWeapon(const FName & ClassName)
{
	UBaseGameInstance * GameInstance = UStaticFunctionLibrary::GetBaseGameInstance(this);
	if (!GameInstance) return;

	ASoldier * Soldier = Cast<ASoldier>(GetOuterAPlayerController()->GetPawn());
	if (Soldier)
	{
		Soldier->SpawnItem(GameInstance->GetItem(ClassName));
	}
}

void UBaseCheatManager::GiveWeaponToAll(const FName & ClassName)
{
	UBaseGameInstance * GameInstance = UStaticFunctionLibrary::GetBaseGameInstance(this);
	if (!GameInstance) return;

	TSubclassOf<AItem> ItemClass = GameInstance->GetItem(ClassName);
	for (TActorIterator<ASoldier> Soldier(GetWorld()); Soldier; ++Soldier)
	{
		Soldier->SpawnItem(ItemClass);
	}
}

void UBaseCheatManager::DamageSoldier(float DamageAmount)
{
	ASoldier * Soldier = Cast<ASoldier>(GetOuterAPlayerController()->GetPawn());
	if (Soldier)
	{
		Soldier->TakeDamage(DamageAmount, FPointDamageEvent(), NULL, NULL);
	}
}