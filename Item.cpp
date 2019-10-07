#include "FPSTemplate.h"
#include "Item.h"
#include "Soldier/Soldier.h"
#include "UnrealNetwork.h"
#include "StaticFunctionLibrary.h"
#include "Game/RoundGameMode.h"

AItem::AItem(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
	Mesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionProfileName(FName(TEXT("PhysicsActor")));
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	Mesh->bEnablePhysicsOnDedicatedServer = true;
	Mesh->SetSimulatePhysics(true);
	Mesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	SightComponent = ObjectInitializer.CreateOptionalDefaultSubobject<USightComponent>(this, TEXT("SightComponent"));
	SightComponent->AttachParent = Mesh;
	SightComponent->AttachSocketName = FName(TEXT("b_gun_Root"));

	bReplicates = true;
	bAlwaysRelevant = true;
	PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.TickInterval = 0.5f;

	ItemSlot = EItemSlotEnum::Primary;

	static ConstructorHelpers::FObjectFinder<UTexture2D> NewCrosshairTexture(TEXT("Texture2D'/Game/FPSTemplate/HUD/Textures/T_SimpleCrosshair.T_SimpleCrosshair'"));
	CrosshairTexture = NewCrosshairTexture.Object;
}

void AItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AItem, Instigator);
	DOREPLIFETIME_CONDITION(AItem, PatternMaterialName, COND_InitialOnly);
	DOREPLIFETIME(AItem, ReplicatedLocation);
}

void AItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	Mesh->SetAnimInstanceClass(NULL);
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	// Destroy SighComponent if it is not needed
	if (SightComponent && !SightComponent->bVisible)
	{
		SightComponent->DestroyComponent();
	}
}

void AItem::Reset()
{
	Super::Reset();

	if (!Instigator)
	{
		// Destroy this item if the level is reset and it doesn't belong to a soldier
		Destroy();
	}
}

void AItem::Tick(float DeltaTime)
{
	// Tick will only execute when dropped
	if (Instigator)
	{
		SetActorTickEnabled(false);
		return;
	}

	if (GetNetMode() == NM_Client)
	{
		// Update location on clients
		if ((Mesh->GetComponentLocation() - ReplicatedLocation).SizeSquared() > 400.f)
		{
			Mesh->SetAllPhysicsPosition(ReplicatedLocation);//(Mesh->GetComponentLocation() + ReplicatedLocation) * 0.5f);
		}
	}
	else
	{
		// Set replicated location so clients will receive it
		ReplicatedLocation = Mesh->GetComponentLocation();
	}
}

ASoldier * AItem::GetOwningSoldier() const
{
	return Cast<ASoldier>(Instigator);
}

bool AItem::IsSoldierFiring() const
{
	ASoldier * Soldier = GetOwningSoldier();
	return Soldier != NULL ? Soldier->bFiring : false;
}

void AItem::SetLoadout(const FLoadoutSlotStruct & LoadoutSlot)
{
	UBaseGameInstance * GameInstance = UStaticFunctionLibrary::GetBaseGameInstance(this);
	if (!GameInstance) return;

	PatternMaterialName = LoadoutSlot.PatternMaterialName;
	//if (LoadoutSlot.PatternMaterialName != NAME_None)
	//{
		// Search pattern material name in PatternMaterials and assign index to PatternMaterialIndex
		//TArray<UMaterialInterface *> PatternMaterials;
		//GameInstance->GetItemPatternMaterials(GetClass(), PatternMaterials);
		//for (uint8 i = 0; i < PatternMaterials.Num(); i++)
		//{
		//	if (PatternMaterials[i] && PatternMaterials[i]->GetFName() == LoadoutSlot.PatternMaterialName)
		//	{
		//		// For actual update of material see OnRep_PatternMaterialIndex
		//		PatternMaterialIndex = i;
		//		break;
		//	}
		//}
	//}

	if (LoadoutSlot.SightClassName != NAME_None && SightComponent)
	{
		SightComponent->SetSightClass(GameInstance->GetSight(LoadoutSlot.SightClassName));
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		if (PatternMaterialName != NAME_None)
		{
			OnRep_PatternMaterialName();
		}
	}
}

void AItem::BecomeActiveItem()
{
	Mesh->SetAnimInstanceClass(ActiveAnimBlueprint);
}

void AItem::EndActiveItem()
{
	Mesh->SetAnimInstanceClass(GetDefault<AItem>(GetClass())->Mesh->AnimBlueprintGeneratedClass);
	EndViewTarget(NULL);
}

bool AItem::IsActiveItem() const
{
	ASoldier * Soldier = GetOwningSoldier();
	if (Soldier)
	{
		return Soldier->GetCurrentItem() == this;
	}

	return false;
}

void AItem::BecomeViewTarget(APlayerController * PC)
{
	if (SightComponent)
	{
		SightComponent->SetOnlyOwnerSee(true);
	}
}

void AItem::EndViewTarget(APlayerController * PC)
{
	if (SightComponent)
	{
		SightComponent->SetOnlyOwnerSee(false);
	}
}

bool AItem::CanBeDropped() const
{
	return true;
}

void AItem::Drop()
{
	Instigator = NULL;
	SetOwner(NULL);

	// Do not destroy a dropped item when game mode is a ARoundGameMode, will be destroyed at the end of a round instead
	if (GetWorld()->GetAuthGameMode() && !GetWorld()->GetAuthGameMode()->IsA<ARoundGameMode>())
	{
		// Destroy item 20 seconds after it was dropped
		SetLifeSpan(20.f);
	}

	OnRep_Instigator();
}

bool AItem::CanBePickedUp() const
{
	return !Instigator;
}

void AItem::PickUp(ASoldier * Soldier)
{
	Instigator = Soldier;
	SetOwner(Soldier);
	SetLifeSpan(0.f);

	OnRep_Instigator();
}

bool AItem::PerformTask(EItemTaskEnum::Type Task, float Length)
{
	ASoldier * Soldier = GetOwningSoldier();
	if (Soldier)
	{
		// Try to initialize the task on soldier
		return Soldier->PerformItemTask(Task, Length);
	}

	return false;
}

void AItem::OnRep_Task(EItemTaskEnum::Type Task)
{
	switch (Task)
	{
	case (EItemTaskEnum::None) :
		break;
	case (EItemTaskEnum::Draw) :
		OnDraw();
		break;
	case (EItemTaskEnum::Undraw) :
		OnUndraw();
		break;
	case (EItemTaskEnum::Fire) :
		OnFire();
		break;
	case (EItemTaskEnum::Reload) :
		OnReload();
		break;
	case (EItemTaskEnum::ReloadLong) :
		OnReloadLong();
		break;
	case (EItemTaskEnum::ReloadChamber) :
		OnReloadChamber();
		break;
	}
}

void AItem::OnPerformedTask(EItemTaskEnum::Type Task)
{
	switch (Task)
	{
	case (EItemTaskEnum::None) :
		break;
	case (EItemTaskEnum::Draw) :
		OnDrew();
		break;
	case (EItemTaskEnum::Undraw) :
		OnUndrew();
		break;
	case (EItemTaskEnum::Fire) :
		OnFired();
		break;
	case (EItemTaskEnum::Reload) :
		OnReloaded();
		break;
	case (EItemTaskEnum::ReloadLong) :
		OnReloadedLong();
		break;
	case (EItemTaskEnum::ReloadChamber) :
		OnReloadedChamber();
		break;
	}
}

void AItem::PlayAnimation(const FItemAnimation & Animation, float BlendInTime, float BlendOutTime, float PlaybackSpeed)
{
	if (GetNetMode() == NM_DedicatedServer) return;

	// Play animation on this items' mesh
	if (Mesh->GetAnimInstance())
	{
		Mesh->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(Animation.Sequence, FName(TEXT("DefaultSlot")), BlendInTime, BlendOutTime, PlaybackSpeed);
	}

	// Play animation on soldier mesh
	ASoldier * Soldier = GetOwningSoldier();
	if (Soldier)
	{
		Soldier->PlayItemAnimation(Animation.CharacterSequence, BlendInTime, BlendOutTime, PlaybackSpeed);
	}
}

float AItem::GetImprecision(float AimRatio, float BulletCount, const FVector & SoldierVelocity) const
{
	return 0.f;
}

bool AItem::CanAim() const
{
	return CharacterAimSequence != NULL;
}

float AItem::GetEyeHeight() const
{
	if (SightComponent)
	{
		USight * Sight = SightComponent->SightClass.GetDefaultObject();
		if (Sight)
		{
			return Sight->EyeHeight;
		}
	}

	return 0.f;
}

void AItem::SetCharacterAimSequence(UAnimSequenceBase * NewSequence)
{
	CharacterAimSequence = NewSequence;
	ASoldier * Soldier = GetOwningSoldier();
	if (Soldier && Soldier->GetCurrentItem() == this)
	{
		// If this item is the current item the owning soldier has to recall ASoldier::InitCurrentItem
		Soldier->InitCurrentItem();
	}
}

int32 AItem::GetMagazine() const
{
	// Return -1 to indicate that this item does not have a magazine
	return -1;
}

int32 AItem::GetAmmunition() const
{
	// Return -1 to indicate that this item does not have ammunition
	return -1;
}

bool AItem::GetIsAuto() const
{
	return false;
}

void AItem::DrawItemHUD(APlayerController * PC, UCanvas * Canvas, FVector CameraPosition, FVector CameraDir, float AimRatio, float BulletCount, const FVector & SoldierVelocity)
{
	// Draw a simple texture in the middle of the screen
	if (CrosshairTexture)
	{
		const FVector2D TextureSize = FVector2D(CrosshairTexture->GetSizeX() * 0.5f, CrosshairTexture->GetSizeY() * 0.5f);
		const FVector2D ScreenPosition = FVector2D((Canvas->SizeX - TextureSize.X) * 0.5f, (Canvas->SizeY - TextureSize.Y) * 0.5f);
		Canvas->K2_DrawTexture(CrosshairTexture, ScreenPosition, TextureSize, FVector2D::ZeroVector);
	}
}

void AItem::StartFiring()
{
	
}

void AItem::StopFiring()
{

}

void AItem::ToggleFireMode()
{

}

void AItem::Draw()
{
	if (PerformTask(EItemTaskEnum::Draw, DrawAnimation.Length))
	{
		OnDraw();
	}
}

void AItem::Undraw()
{
	if (PerformTask(EItemTaskEnum::Undraw, UndrawAnimation.Length - 0.05f))
	{
		OnUndraw();
	}
}

void AItem::Fire()
{

}

void AItem::Reload()
{

}

void AItem::ReloadLong()
{

}

void AItem::ReloadChamber()
{

}

void AItem::OnDrew()
{

}

void AItem::OnUndrew()
{
	ASoldier * Soldier = GetOwningSoldier();
	if (Soldier)
	{
		Soldier->OnUndrewItem();
	}
}

void AItem::OnFired()
{

}

void AItem::OnReloaded()
{

}

void AItem::OnReloadedLong()
{

}

void AItem::OnReloadedChamber()
{

}

void AItem::OnDraw()
{
	ASoldier * Soldier = GetOwningSoldier();
	if (Soldier)
	{
		PlayAnimation(DrawAnimation, 0.f, 0.25f, Soldier->HasPerk(EPerkEnum::FastDraw) ? FastAnimationRate : 1.f);
	}
}

void AItem::OnUndraw()
{
	ASoldier * Soldier = GetOwningSoldier();
	if (Soldier)
	{
		PlayAnimation(UndrawAnimation, 0.25f, 0.f, Soldier->HasPerk(EPerkEnum::FastDraw) ? FastAnimationRate : 1.f);
	}
}

void AItem::OnFire()
{

}

void AItem::OnReload()
{

}

void AItem::OnReloadLong()
{

}

void AItem::OnReloadChamber()
{

}

void AItem::OnRep_Instigator()
{
	Super::OnRep_Instigator();

	if (!Instigator)
	{
		EndActiveItem();
		SetActorTickEnabled(true);

		// Velocity that will be applied
		FVector ThrowVelocity;

		ASoldier * OldSoldier = Cast<ASoldier>(GetAttachParentActor());
		if (OldSoldier)
		{
			// Adjust location and rotation to be in front of camera
			FVector ViewLocation;
			FRotator ViewRotation;
			OldSoldier->GetActorEyesViewPoint(ViewLocation, ViewRotation);

			// Move this item in front of the camera
			SetActorLocation(ViewLocation + ViewRotation.RotateVector(FVector::ForwardVector) * 100.f + ViewRotation.RotateVector(FVector::RightVector) * 20.f - ViewRotation.RotateVector(FVector::UpVector) * 20.f);
			SetActorRotation(ViewRotation + FRotator(0.f, 180.f, 0.f));

			// ThrowVelocity will be the current movement velocity of the soldier with an additional velocity in front direction
			ThrowVelocity = OldSoldier->GetVelocity() + ViewRotation.Vector() * 150.f;
		}
		else
		{
			ThrowVelocity = FVector::ZeroVector;
		}

		DetachRootComponentFromParent(true);
		Mesh->SetHiddenInGame(false);
		Mesh->SetSimulatePhysics(true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetAllPhysicsLinearVelocity(ThrowVelocity);
	}
	else
	{
		SetActorTickEnabled(false);
		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (GetNetMode() == NM_Client)
		{
			ASoldier * Soldier = GetOwningSoldier();
			if (Soldier && Soldier->GetCurrentItem() != this)
			{
				// Store item on soldier
				Soldier->StoreItem(this);
			}
		}
	}
}

void AItem::OnRep_PatternMaterialName()
{
	UBaseGameInstance * GameInstance = UStaticFunctionLibrary::GetBaseGameInstance(this);
	if (!GameInstance) return;

	UMaterialInterface * PatternMaterial = GameInstance->GetItemPatternMaterial(GetClass(), PatternMaterialName);

	if (PatternMaterial)
	{
		Mesh->SetMaterial(PatternMaterialSlot, PatternMaterial);
	}
}

void AItem::OnRep_ReplicatedLocation()
{
	if (!Instigator)
	{
		// Move mesh to replicated ragdoll location
		Mesh->SetAllPhysicsPosition(ReplicatedLocation);
	}
}

bool AItem::CanSoldierInteract(ASoldier * Soldier) const
{
	return CanBePickedUp();
}

float AItem::GetInteractionLength() const
{
	return 0.28f;
}

FVector AItem::GetInteractionLocation() const
{
	return Mesh->Bounds.Origin;
}

UAnimMontage * AItem::GetInteractionMontage(ASoldier * Soldier) const
{
	return Soldier->PickUpMontage;
}

void AItem::OnInteract(ASoldier * Soldier)
{
	Soldier->PickUpItem(this);
}