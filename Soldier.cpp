#include "FPSTemplate.h"
#include "Soldier.h"
#include "UnrealNetwork.h"
#include "Soldier/SoldierAnimInstance.h"
#include "Game/BaseGameMode.h"
#include "Soldier/SoldierMovementComponent.h"
#include "StaticFunctionLibrary.h"
#include "Player/BasePlayerController.h"
#include "Attachments/Sight.h"
#include "Damage/DamageType_Knife.h"
#include "Damage/DamageType_Environment.h"
#include "Damage/DamageType_Headshot.h"
#include "Items/Firearm.h"
#include "Items/ThrowableItem.h"
#include "Items/Knife.h"
#include "BaseGameInstance.h"

#define DefaultHealth 100.f
#define DefaultFOV 90.f
#define LeanTraceDist 100.f
#define LeanSideTraceDist 35.f
#define LeanTraceChannel ECC_Camera
#define MaxHitPenaltyRotation 10.f
#define MaxArmsLagRotation 20.f

#define MaxInteractionDistanceSquared 22500.f
#define MaxInteractionAngle 45.f

#define ItemAnimationSlot FName(TEXT("UpperBody"))

ASoldier::ASoldier(const FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USoldierMovementComponent>(ASoldier::CharacterMovementComponentName))
{
	Camera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("Camera"));
	Camera->AttachParent = GetMesh();
	Camera->AttachSocketName = FName(TEXT("camera_socket"));
	Camera->bUsePawnControlRotation = false;
	FPostProcessSettings & PPS = Camera->PostProcessSettings;
	PPS.bOverride_FilmSaturation = true;
	PPS.bOverride_SceneFringeIntensity = true;
	PPS.bOverride_VignetteIntensity = true;
	//PPS.bOverride_MotionBlurAmount = true;
	//PPS.MotionBlurAmount = 0.f;

	CameraBoom = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CameraBoom"));
	CameraBoom->AttachParent = GetCapsuleComponent();
	//CameraBoom->AttachSocketName = FName(TEXT("root"));
	//CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->TargetArmLength = 100.f;
	CameraBoom->SocketOffset = FVector(0.f, 30.f, 80.f);

	CameraTP = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("CameraTP"));
	CameraTP->AttachParent = CameraBoom;
	CameraTP->bUsePawnControlRotation = false;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> NewMesh(TEXT("SkeletalMesh'/Game/FPSTemplate/Soldier/Meshes/SK_Mannequin.SK_Mannequin'"));
	GetMesh()->SetSkeletalMesh(NewMesh.Object);
	static ConstructorHelpers::FObjectFinder<UAnimBlueprintGeneratedClass> AnimBlueprint(TEXT("AnimBlueprint'/Game/FPSTemplate/Soldier/ABP_Soldier.ABP_Soldier_C'"));
	GetMesh()->AnimBlueprintGeneratedClass = AnimBlueprint.Object;
	GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
	GetMesh()->RelativeRotation.Yaw = -90.f;
	GetMesh()->RelativeLocation.Z = -90.f;
	GetMesh()->SetCollisionProfileName(FName(TEXT("Ragdoll")));
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCapsuleHalfHeight(90.f);
	GetCapsuleComponent()->SetCapsuleRadius(30.f);

	SceneCapture = ObjectInitializer.CreateDefaultSubobject<USceneCaptureComponent2D>(this, TEXT("SceneCapture"));
	SceneCapture->AttachParent = Camera;
	SceneCapture->bCaptureEveryFrame = true;
	SceneCapture->bHiddenInGame = true;
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> NewRenderTarget(TEXT("TextureRenderTarget2D'/Game/FPSTemplate/Soldier/SightRenderTarget.SightRenderTarget'"));
	SceneCapture->TextureTarget = NewRenderTarget.Object;

	Health = DefaultHealth;
}

void ASoldier::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASoldier, Health);
	DOREPLIFETIME(ASoldier, Items);
	DOREPLIFETIME(ASoldier, CurrentItem);
	DOREPLIFETIME(ASoldier, InteractionActor);
	DOREPLIFETIME(ASoldier, SoldierTask);
	DOREPLIFETIME_CONDITION(ASoldier, Perk, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ASoldier, CharacterMesh, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ASoldier, bAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ASoldier, LeanRatio, COND_SkipOwner)
}

void ASoldier::SetupPlayerInputComponent(class UInputComponent * InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	// Bind input axes
	InputComponent->BindAxis("MoveForward", this, &ASoldier::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ASoldier::MoveRight);
	InputComponent->BindAxis("LookUp", this, &ASoldier::AddControllerPitchInput);
	InputComponent->BindAxis("LookRight", this, &ASoldier::AddControllerYawInput);

	//Bind input actions
	InputComponent->BindAction("Interact", IE_Pressed, this, &ASoldier::StartInteract);
	InputComponent->BindAction("Interact", IE_Released, this, &ASoldier::StopInteract);
	InputComponent->BindAction("Sprint", IE_Pressed, this, &ASoldier::StartSprinting);
	InputComponent->BindAction("Sprint", IE_Released, this, &ASoldier::StopSprinting);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &ASoldier::StartCrouching);
	InputComponent->BindAction("Crouch", IE_Released, this, &ASoldier::StopCrouching);
	InputComponent->BindAction("Jump", IE_Pressed, this, &ASoldier::Jump);

	InputComponent->BindAction("Fire", IE_Pressed, this, &ASoldier::StartFiring);
	InputComponent->BindAction("Fire", IE_Released, this, &ASoldier::StopFiring);
	InputComponent->BindAction("ToggleFireMode", IE_Pressed, this, &ASoldier::ToggleFireMode);
	InputComponent->BindAction("Aim", IE_Pressed, this, &ASoldier::StartAiming);
	InputComponent->BindAction("Aim", IE_Released, this, &ASoldier::StopAiming);
	InputComponent->BindAction("Reload", IE_Pressed, this, &ASoldier::Reload);
	InputComponent->BindAction("DropItem", IE_Pressed, this, &ASoldier::DropItem);
	InputComponent->BindAction("ToggleCamera", IE_Pressed, this, &ASoldier::ToggleCameraMode);

	InputComponent->BindAction("Slot0", IE_Pressed, this, &ASoldier::EquipSlot0);
	InputComponent->BindAction("Slot1", IE_Pressed, this, &ASoldier::EquipSlot1);
	InputComponent->BindAction("Slot2", IE_Pressed, this, &ASoldier::EquipSlot2);
	InputComponent->BindAction("Slot3", IE_Pressed, this, &ASoldier::EquipSlot3);
	InputComponent->BindAction("Slot4", IE_Pressed, this, &ASoldier::EquipSlot4);
	InputComponent->BindAction("Slot5", IE_Pressed, this, &ASoldier::EquipSlot5);
	InputComponent->BindAction("Slot6", IE_Pressed, this, &ASoldier::EquipSlot6);

	InputComponent->BindAction("NextItem", IE_Pressed, this, &ASoldier::EquipNextItem);
	InputComponent->BindAction("PreviousItem", IE_Pressed, this, &ASoldier::EquipPreviousItem);

	InputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ASoldier::ThrowGrenade);
	InputComponent->BindAction("Knife", IE_Pressed, this, &ASoldier::Knife);
	//InputComponent->BindAction("Knife", IE_Released, this, &ASoldier::StopFiring);
}

void ASoldier::Reset()
{
	if (!Controller)
	{
		// Destroy this and all items when not possessed by a controller

		for (AItem * Item : Items)
		{
			if (Item)
			{
				Item->Destroy();
			}
		}

		Destroy();
	}
	else
	{
		// Reset all important values
		Health = DefaultHealth;
		DamageInfoArray.Empty();
		InteractionActor = NULL;
		BulletCount = 0.f;
		TargetItemRecoil = FVector::ZeroVector;
		TargetPitchRecoil = 0.f;
		TargetYawRecoil = 0.f;

		if (CurrentItem)
		{
			CurrentItem->Draw();
		}
	}
}

void ASoldier::UnPossessed()
{
	Super::UnPossessed();

	// Stop firing and aiming when being unpossessed
	bFiring = false;
	bAiming = false;
}

void ASoldier::TurnOff()
{
	Super::TurnOff();

	SetActorTickEnabled(false);
}

void ASoldier::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetNetMode() != NM_Client)
	{
		// Check if soldier is interaction and still can interact
		if (InteractionActor)
		{
			if (ItemTask != EItemTaskEnum::None || !CanInteractWith(InteractionActor))
			{
				InteractionActor = NULL;
				StopSoldierTask();
			}
		}

	}

	if (Controller && Controller->IsLocalController())
	{
		// Check if player should lean
		LeanRatio = 0.f;

		FVector CameraLocation;
		FRotator CameraRotation;
		GetActorEyesViewPoint(CameraLocation, CameraRotation);
		CameraLocation.X = GetActorLocation().X;
		CameraLocation.Y = GetActorLocation().Y;
		const FVector TraceOffset = CameraRotation.Vector() * LeanTraceDist;

		FHitResult FrontHit;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);
		if (GetWorld()->LineTraceSingleByChannel(FrontHit, CameraLocation, CameraLocation + TraceOffset, LeanTraceChannel, CollisionParams))
		{
			//DrawDebugLine(GetWorld(), CameraLocation, Hit.Location, FColor::Red);

			const FVector RightOffset = GetActorRightVector() * LeanSideTraceDist;
			FHitResult SideHit;
			if (!GetWorld()->LineTraceSingleByChannel(SideHit, CameraLocation + RightOffset, CameraLocation + RightOffset + TraceOffset, LeanTraceChannel, CollisionParams))
			{
				LeanRatio = 1.f;
				//DrawDebugLine(GetWorld(), CameraLocation + RightOffset, CameraLocation + RightOffset + TraceOffset, FColor::Green);
			}
			else if (!GetWorld()->LineTraceSingleByChannel(SideHit, CameraLocation - RightOffset, CameraLocation - RightOffset + TraceOffset, LeanTraceChannel, CollisionParams))
			{
				//DrawDebugLine(GetWorld(), CameraLocation + RightOffset, Hit.Location, FColor::Red);
				LeanRatio = -1.f;
			}

			if (LeanRatio != 0.f)
			{
				LeanRatio *= -FMath::Pow(-(1.f - (FrontHit.Location - CameraLocation).Size() / LeanTraceDist) + 1.f, 2.f) + 1.f;
			}
		}
		/*else
		{
			DrawDebugLine(GetWorld(), CameraLocation, CameraLocation + TraceOffset, FColor::Green);
		}*/

		if (GetNetMode() == NM_Client)
		{
			// Send input to server
			ServerUpdateInput(FSoldierInputStruct(bFiring, bAiming, bSprinting, bInteracting, LeanRatio));
		}

		if (bInteracting && !InteractionActor)
		{
			StartInteract();
		}
	}

	// Cosmetics only from here on
	if (GetNetMode() == NM_DedicatedServer) return;

	float Weight = 0.f;
	float SourceWeight = 0.f;
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->GetSlotWeight(ItemAnimationSlot, Weight, SourceWeight);
	}

	// Soldier should only aim when aiming is pressed, item supports aiming and no montage is playing in item animation slot node
	const bool bShouldAim = bAiming && Weight == 0.f && CurrentItem ? CurrentItem->CanAim() : false;
	float TargetAimRation = bShouldAim ? 1.f : 0.f;
	if (TargetAimRation != AimRatio)
	{
		if (AimRatio < TargetAimRation)
		{
			AimRatio += DeltaTime * 5.f;
			TargetItemRecoil.X += DeltaTime * 10.f;

			if (AimRatio > 1.f)
			{
				AimRatio = 1.f;
				OnStartedAiming();
			}
		}
		else if (AimRatio > TargetAimRation)
		{
			if (AimRatio == 1.f)
			{
				OnStoppedAiming();
			}

			AimRatio -= DeltaTime * 5.f;

			if (AimRatio < 0.f)
			{
				AimRatio = 0.f;
			}
		}

		// Camera FOV and location
		const float ZoomedFOV = 60.f;

		Camera->FieldOfView = FMath::Lerp(DefaultFOV, ZoomedFOV, AimRatio);
	}

	// Calculate arms lag
	const float ViewPitch = GetViewPitch();
	const float ViewYaw = GetActorRotation().Yaw;
	const float NewPitchLag = FMath::ClampAngle((ViewPitch - OldViewPitchRotation) * 2.f, -MaxArmsLagRotation, MaxArmsLagRotation);
	const float NewYawLag = FMath::ClampAngle((OldViewYawRotation - ViewYaw) * 2.f, -MaxArmsLagRotation, MaxArmsLagRotation);
	ArmsLagRotation = FMath::RInterpTo(ArmsLagRotation, FRotator(NewPitchLag, NewYawLag, NewYawLag), DeltaTime, 8.f);

	OldViewPitchRotation = ViewPitch;
	OldViewYawRotation = ViewYaw;

	// Reduce bullet count
	if (ItemTask != EItemTaskEnum::Fire)
	{
		BulletCount = FMath::FInterpTo(BulletCount, 0.f, DeltaTime, 4.f);
	}

	// Interpolate target recoils to 0, only when not shooting
	TargetPitchRecoil = FMath::FInterpConstantTo(TargetPitchRecoil, 0.f, DeltaTime, 16.f);
	TargetYawRecoil = FMath::FInterpConstantTo(TargetYawRecoil, 0.f, DeltaTime, 16.f);

	// Interpolaterecoil to target recoil
	PitchRecoil = FMath::FInterpTo(PitchRecoil, TargetPitchRecoil * 0.5f, DeltaTime, 8.f);
	YawRecoil = FMath::FInterpTo(YawRecoil, TargetYawRecoil * 0.5f, DeltaTime, 8.f);

	// Apply recoil to view rotation
	AddControllerPitchInput(-TargetPitchRecoil * 0.5f * DeltaTime);
	AddControllerYawInput(TargetYawRecoil * 0.5f * DeltaTime);

	// Interpolate item recoil to target item recoil
	TargetItemRecoil = FMath::VInterpTo(TargetItemRecoil, FVector::ZeroVector, DeltaTime, 32.f);
	ItemRecoil = FMath::VInterpTo(ItemRecoil, TargetItemRecoil, DeltaTime, 16.f);
}

void ASoldier::OnStartCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	Super::OnStartCrouch(HeightAdjust, ScaledHeightAdjust);

	//CameraBoom->RelativeLocation.Z = HeightAdjust;
}

void ASoldier::OnEndCrouch(float HeightAdjust, float ScaledHeightAdjust)
{
	Super::OnEndCrouch(HeightAdjust, ScaledHeightAdjust);

	//CameraBoom->RelativeLocation.Z = 0.f;
}

float ASoldier::TakeDamage(float DamageAmount, const FDamageEvent & DamageEvent, AController * EventInstigator, AActor * DamageCauser)
{
	if (GetNetMode() == NM_Client || Health <= 0.f || DamageAmount == 0.f) return 0.f;

	//if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	//{
	//	const FPointDamageEvent * PointDamageEvent = (FPointDamageEvent *) & DamageEvent;
	//	if (PointDamageEvent->HitInfo.BoneName == FName(TEXT("head")))
	//	{
	//		// Double damage if soldiers head was hit
	//		DamageAmount *= 2.f;
	//	}
	//}

	if (DamageEvent.DamageTypeClass == UDamageType_Headshot::StaticClass())
	{
		DamageAmount *= 2.f;
	}

	if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		const FRadialDamageEvent * RadialDamageEvent = (FRadialDamageEvent *)& DamageEvent;
		const float DistanceFactor = FMath::Pow((1.f - (RadialDamageEvent->Origin - GetActorLocation()).Size() / RadialDamageEvent->Params.OuterRadius), RadialDamageEvent->Params.DamageFalloff);
		DamageAmount *= FMath::Clamp(DistanceFactor, 0.f, 1.f);
	}

	ABaseGameMode * GameMode = Cast<ABaseGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode) return DamageAmount;

	ABasePlayerController * PC = Cast<ABasePlayerController>(Controller);
	if (!PC) return DamageAmount;

	ABasePlayerController * InstigatorPC = Cast<ABasePlayerController>(EventInstigator);

	// Let the game mode adjust the DamageAmount (e.g. disable friendly fire)
	GameMode->CalcDamage(DamageAmount, PC, InstigatorPC);

	// Notify instigator that he hit another soldier, draws hit marker if this is an enemy
	if (InstigatorPC)
	{
		InstigatorPC->ClientOnDamagedSoldier(this, DamageAmount);
	}

	// Notify all clients that this soldier was hit
	MulticastTookDamage(DamageAmount, DamageCauser);

	Health -= DamageAmount;
	if (Health <= 0.f)
	{
		if (CurrentItem)
		{
			CurrentItem->Drop();
		}
		
		for (uint8 i = 0; i < NumItemSlots; i++)
		{
			if (Items[i] == CurrentItem)
			{
				// Remove CurrentItem from Items
				Items[i] = NULL;
				break;
			}
		}

		CurrentItem = NULL;
		StopItemTask();
		StopSoldierTask();
		GameMode->OnSoldierDied(this, DamageEvent, InstigatorPC, DamageCauser);
	}
	else if (InstigatorPC)
	{
		// Check if instigator already has an entry in DamageInfoArray, else create one
		bool bFoundInfo = false;
		for (FDamageInfo & DamageInfo : DamageInfoArray)
		{
			if (DamageInfo.Instigator == InstigatorPC)
			{
				DamageInfo.DamageAmount += DamageAmount;
				bFoundInfo = true;
				break;
			}
		}

		if (!bFoundInfo)
		{
			DamageInfoArray.Add(FDamageInfo(InstigatorPC, DamageAmount));
		}
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		OnRep_Health();
	}

	return DamageAmount;
}

void ASoldier::MulticastTookDamage_Implementation(float DamageAmount, AActor * Causer)
{
	TargetPitchRecoil = FMath::Clamp(TargetPitchRecoil - DamageAmount * 0.1f, -MaxHitPenaltyRotation, MaxHitPenaltyRotation);
	TargetYawRecoil = FMath::Clamp(TargetYawRecoil - DamageAmount * 0.1f, -MaxHitPenaltyRotation, MaxHitPenaltyRotation);

	if (Controller && Controller->IsLocalController())
	{
		ABasePlayerController * PC = Cast<ABasePlayerController>(Controller);
		if (PC)
		{
			// Notify player controller that we got hit
			PC->OnTookDamage(DamageAmount, Causer);
		}
	}
}

void ASoldier::Heal(float Amount)
{
	Health += Amount;
	if (Health > DefaultHealth)
	{
		Amount -= (Health - DefaultHealth);
		Health = DefaultHealth;
	}

	if (DamageInfoArray.Num() > 0)
	{
		float Fraction = Amount / DamageInfoArray.Num();
		for (int i = 0; i < DamageInfoArray.Num(); ++i)
		{
			FDamageInfo & DamageInfo = DamageInfoArray[i];
			DamageInfo.DamageAmount -= DamageInfo.DamageAmount * Fraction;
			if (DamageInfo.DamageAmount <= 0.f)
			{
				DamageInfoArray.RemoveAt(i);
				--i;
			}
		}
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		OnRep_Health();
	}
}

bool ASoldier::IsDead() const
{
	return Health <= 0.f;
}

bool ASoldier::HasFullHealth() const
{
	return Health == DefaultHealth;
}

const TArray<FDamageInfo> & ASoldier::GetDamageInfoArray() const
{
	return DamageInfoArray;
}

bool ASoldier::AllItemsFullAmmo() const
{
	for (AItem * Item : Items)
	{
		if (Item && !Item->HasFullAmmo())
		{
			return false;
		}
	}

	return true;
}

void ASoldier::RefillAmmo(bool bAllItems)
{
	for (AItem * Item : Items)
	{
		if (Item && !Item->HasFullAmmo())
		{
			Item->RefillAmmo();

			if (!bAllItems) break;
		}
	}
}

void ASoldier::Landed(const FHitResult & Hit)
{
	if (GetNetMode() != NM_Client)
	{
		// Apply falling damage
		const float Speed = GetCharacterMovement()->Velocity.Z;
		if (Speed < -1000.f)
		{
			FDamageEvent DamageEvent;
			DamageEvent.DamageTypeClass = UDamageType_Environment::StaticClass();
			TakeDamage((-Speed - 1000.f) * 0.5f, DamageEvent, Controller, NULL);
		}
	}
}

void ASoldier::FaceRotation(FRotator NewControlRotation, float DeltaTime)
{
	SetActorRotation(FRotator(0.f, NewControlRotation.Yaw + YawRecoil, 0.f));
}

void ASoldier::PostRenderFor(APlayerController * PC, UCanvas * Canvas, FVector CameraPosition, FVector CameraDir)
{
	if (CurrentItem)//&& !bInThirdPersonMode)
	{
		CurrentItem->DrawItemHUD(PC, Canvas, CameraPosition, CameraDir, AimRatio, BulletCount, GetCharacterMovement()->Velocity);
	}
}

void ASoldier::BecomeViewTarget(APlayerController * PC)
{
	if (AimRatio == 1.f)
	{
		OnStartedAiming();
	}

	if (CurrentItem)
	{
		CurrentItem->BecomeViewTarget(PC);
	}
}

void ASoldier::EndViewTarget(APlayerController * PC)
{
	OnStoppedAiming();

	if (CurrentItem)
	{
		CurrentItem->EndViewTarget(PC);
	}
}

void ASoldier::CalcCamera(float DeltaTime, FMinimalViewInfo & OutResult)
{
	Camera->GetCameraView(DeltaTime, OutResult);

	if (bInThirdPersonMode)
	{
		CameraBoom->RelativeRotation.Pitch = GetViewPitch();
		CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, GetMesh()->GetSocketTransform(FName(TEXT("pelvis")), RTS_Actor).GetLocation().Z));
		float OffsetY = GetDefault<ASoldier>(GetClass())->CameraBoom->SocketOffset.Y;
		//OffsetY *= LeanRatio > 0.f ? FMath::GetMappedRangeValueUnclamped(FVector2D(-1.f, 0.f), FVector2D(-2.5f, 1.f), LeanRatio) : FMath::GetMappedRangeValueUnclamped(FVector2D(0.f, 1.f), FVector2D(1.f, 1.5f), LeanRatio);
		CameraBoom->SocketOffset.Y = FMath::FInterpTo(CameraBoom->SocketOffset.Y, LeanRatio < 0.f ? -OffsetY : OffsetY, DeltaTime, 8.f);

		FMinimalViewInfo TPView;
		CameraTP->GetCameraView(DeltaTime, TPView);
		OutResult.BlendViewInfo(TPView, 1.f - AimRatio);
	}
}

bool ASoldier::IsViewTarget() const
{
	// TODO
	return Controller && Controller->IsLocalPlayerController() && (bAiming || !bInThirdPersonMode);

	/*if (Controller && Controller->IsLocalPlayerController())
	{
		return true;
	}
	else
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController * PlayerController = *Iterator;
			if (PlayerController && PlayerController->PlayerCameraManager->GetViewTargetPawn() == this)
			{
				return true;
			}
		}
	}*/

	return false;
}

void ASoldier::OnStartedAiming()
{
	if (SceneCapture->FOVAngle > 0.f)
	{
		SceneCapture->bHiddenInGame = false;
	}
}

void ASoldier::OnStoppedAiming()
{
	SceneCapture->bHiddenInGame = true;
}

float ASoldier::GetViewPitch() const
{
	if (Controller)
	{
		return Controller->GetControlRotation().Pitch + PitchRecoil;
	}

	return RemoteViewPitch / 255.f * 360.f + PitchRecoil;
}

void ASoldier::GetActorEyesViewPoint(FVector & OutLocation, FRotator & OutRotation) const
{
	if (!bInThirdPersonMode || bAiming)
	{
		OutLocation = Camera->GetComponentLocation();
		OutRotation = Camera->GetComponentRotation();
	}
	else
	{
		OutRotation = CameraTP->GetComponentRotation();

		OutLocation = CameraTP->GetComponentLocation();
		OutLocation = FVector::PointPlaneProject(OutLocation, FPlane(Camera->GetComponentLocation(), OutRotation.Vector()));
	}
}

void ASoldier::GetArmsOffset(FVector & OutOffset, FRotator & OutRotation) const
{
	OutOffset = ItemRecoil;
	if (CurrentItem && IsViewTarget())
	{
		// Add items eye height to arm offset, so the sights can line up with the camera
		OutOffset.Z -= CurrentItem->GetEyeHeight() * AimRatio;
	}

	OutOffset = Camera->GetComponentRotation().RotateVector(OutOffset);

	OutRotation = ArmsLagRotation;
}

float ASoldier::GetInteractionPercentage() const
{
	IInteractionInterface * Interface = Cast<IInteractionInterface>(InteractionActor);
	if (Interface)
	{
		const float InteractionLength = Interface->GetInteractionLength();
		if (InteractionLength > 0.f)
		{
			return GetWorld()->GetTimerManager().GetTimerElapsed(TimerHandle_SoldierTask) / InteractionLength;
		}
	}

	return -1.f;
}

AItem * ASoldier::GetCurrentItem() const
{
	return CurrentItem;
}

bool ASoldier::HasPerk(EPerkEnum InPerk) const
{
	return InPerk == Perk;
}

void ASoldier::SetLoadout(const FLoadoutStruct & Loadout, USkeletalMesh * CustomMesh)
{
	UBaseGameInstance * GameInstance = UStaticFunctionLibrary::GetBaseGameInstance(this);
	if (!GameInstance) return;

	CharacterMesh = CustomMesh;
	OnRep_CharacterMesh();
	Perk = Loadout.Perk;

	for (const FLoadoutSlotStruct Slot : Loadout.ItemSlots)
	{
		AItem * Item = SpawnItem(GameInstance->GetItem(Slot.ItemClassName));
		if (Item)
		{
			// Apply item specific loadout
			Item->SetLoadout(Slot);
		}
	}
}

AItem * ASoldier::SpawnItem(TSubclassOf<AItem> ItemClass)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.bNoFail = true;
	// Spawn actual item
	AItem * NewItem = GetWorld()->SpawnActor<AItem>(ItemClass, SpawnParams);
	// Add new item to loadout
	AddItem(NewItem);

	return NewItem;
}

void ASoldier::AddItem(AItem * Item)
{
	if (Item)
	{
		// Get correct slot for the new item
		uint8 Slot = Item->ItemSlot - 1;
		if (Slot == EItemSlotEnum::Grenade - 1)
		{
			for (uint8 i = EItemSlotEnum::Grenade - 1; i < EItemSlotEnum::Grenade + 3; i++)
			{
				if (!Items[i])
				{
					Slot = i;
					break;
				}
			}
		}

		const bool bCurrentItem = Items[Slot] == CurrentItem;
		if (Items[Slot])
		{
			// Drop the old item
			Items[Slot]->Drop();
		}

		Items[Slot] = Item;
		Item->PickUp(this);

		if (bCurrentItem)
		{
			CurrentItem = Item;
			InitCurrentItem();
			CurrentItem->Draw();
		}
		else
		{
			StoreItem(Item);
		}
	}
}

void ASoldier::InitCurrentItem()
{
	if (CurrentItem)
	{
		CurrentItem->AttachRootComponentTo(GetMesh(), FName(TEXT("hand_r")), EAttachLocation::SnapToTarget);

		// From here on only on listen server and clients
		if (GetNetMode() == NM_DedicatedServer) return;

		CurrentItem->BecomeActiveItem();
		CurrentItem->SetActorHiddenInGame(false);

		if (IsViewTarget())
		{
			CurrentItem->BecomeViewTarget(NULL);
		}

		USoldierAnimInstance * AnimInstance = Cast<USoldierAnimInstance>(GetMesh()->GetAnimInstance());
		{
			if (AnimInstance)
			{
				// Update item specific properties on AnimInstance for CurrentItem
				AnimInstance->InitItem(CurrentItem);
			}
		}

		// Update scene capture FOV
		USight * Sight = CurrentItem->SightComponent ? CurrentItem->SightComponent->SightClass.GetDefaultObject() : NULL;
		if (Sight)
		{
			SceneCapture->FOVAngle = Sight->FieldOfView;
		}
		else
		{
			SceneCapture->FOVAngle = 0.f;
		}
	}
}

void ASoldier::StoreItem(AItem * Item)
{
	if (Item)
	{
		Item->EndActiveItem();

		/*uint8 Slot = 0;
		for (uint8 i = 0; i < 7; i++)
		{
			if (Items[i] == Item)
			{
				Slot = i;
				break;
			}
		}

		FName SocketName = NAME_None;
		switch (Slot)
		{
		case (0) :
			SocketName = FName(TEXT("primary_socket"));
			break;
		case (1) :
			SocketName = FName(TEXT("secondary_socket"));
			break;
		case (2) :
			SocketName = FName(TEXT("throwable1_socket"));
			break;
		case (3) :
			SocketName = FName(TEXT("throwable2_socket"));
			break;
		case (4) :
			SocketName = FName(TEXT("throwable3_socket"));
			break;
		case (5) :
			SocketName = FName(TEXT("throwable4_socket"));
			break;
		case (6) :
			SocketName = FName(TEXT("knife_socket"));
			break;
		}*/

		if (Item == Items[0])
		{
			Item->AttachRootComponentTo(GetMesh(), FName(TEXT("primary_socket")), EAttachLocation::SnapToTarget);
		}
		else
		{
			Item->SetActorHiddenInGame(true);
		}
	}
}

bool ASoldier::PerformItemTask(EItemTaskEnum::Type Task, float Length)
{
	if (SoldierTask < ESoldierTaskEnum::FullBody && ItemTask == EItemTaskEnum::None || (Task <= EItemTaskEnum::Undraw) && ItemTask != Task)
	{
		float Rate = 1.f;
		if ((Task >= EItemTaskEnum::Reload && Task <= EItemTaskEnum::ReloadChamber && HasPerk(EPerkEnum::FastReload)) || (Task <= EItemTaskEnum::Undraw && HasPerk(EPerkEnum::FastDraw)))
		{
			Rate = FastAnimationRate;
		}

		const EItemTaskEnum::Type OldTask = ItemTask;
		ItemTask = Task;

		// If length of task is <= 0 we want a really small number so that event gets executed on next tick
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ItemTask, this, &ASoldier::OnPerformedItemTask, Length > 0.f ? Length / Rate : 0.001f);

		// Multicast new task to all clients
		MulticastSetItemTask(ItemTask);

		return true;
	}

	return false;
}

void ASoldier::StopItemTask()
{
	ItemTask = EItemTaskEnum::None;
	MulticastSetItemTask(ItemTask = EItemTaskEnum::None);
	GetWorldTimerManager().ClearTimer(TimerHandle_ItemTask);

	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->StopSlotAnimation(0.25f, ItemAnimationSlot);
	}
}

void ASoldier::MulticastSetItemTask_Implementation(EItemTaskEnum::Type NewTask)
{
	if (GetNetMode() == NM_Client)
	{
		EItemTaskEnum::Type OldTask = ItemTask;
		ItemTask = NewTask;
		OnRep_ItemTask(OldTask);
	}
}

void ASoldier::OnPerformedItemTask()
{
	EItemTaskEnum::Type OldTask = ItemTask;
	ItemTask = EItemTaskEnum::None;

	if (OldTask != EItemTaskEnum::Undraw)
	{
		MulticastSetItemTask(ItemTask);
	}

	if (CurrentItem)
	{
		CurrentItem->OnPerformedTask(OldTask);
	}
}

bool ASoldier::PerformSoldierTask(ESoldierTaskEnum Task, UAnimMontage * Montage, float Length)
{
	if (SoldierTask == ESoldierTaskEnum::None)
	{
		/*if (Task > ESoldierTaskEnum::FullBody && ItemTask != EItemTaskEnum::None)
		{
			StopItemTask();
		}*/
		SoldierTask = Task;

		const float MontageLength = PlayAnimMontage(Montage);
		GetWorldTimerManager().SetTimer(TimerHandle_SoldierTask, this, &ASoldier::OnPerformedSoldierTask, Length != -1.f ? (Length > 0.f ? Length : 0.001f) : MontageLength);

		OnSoldierTaskChanged();

		return true;
	}

	return false;
}

void ASoldier::StopSoldierTask()
{
	SoldierTask = ESoldierTaskEnum::None;
	GetWorldTimerManager().ClearTimer(TimerHandle_SoldierTask);
	OnSoldierTaskChanged();
}

void ASoldier::OnPerformedSoldierTask()
{
	ESoldierTaskEnum OldTask = SoldierTask;
	SoldierTask = ESoldierTaskEnum::None;
	OnSoldierTaskChanged();

	switch (OldTask)
	{
	case (ESoldierTaskEnum::None) :
		break;
	case (ESoldierTaskEnum::Interact) :
		OnFinishedInteraction();
		break;
	}
}

void ASoldier::OnUndrewItem()
{
	// CurrentItem was undrawn, setup new item
	if (CurrentItem)
	{
		StoreItem(CurrentItem);

		CurrentItem = Items[NextSlot];
		if (CurrentItem)
		{
			InitCurrentItem();

			if (bInstantFireItem && !CurrentItem->IsA<AFirearm>())
			{
				CurrentItem->StartFiring();
			}
			else
			{
				CurrentItem->Draw();
			}
		}
	}
}

void ASoldier::PlayItemAnimation(UAnimSequenceBase * Sequence, float BlendInTime, float BlendOutTime, float PlaybackSpeed)
{
	if (!Sequence) return;

	if (Sequence->IsA<UAnimMontage>())
	{
		// TODO
		UAnimMontage * Montage = Cast<UAnimMontage>(Sequence);
		//Montage->BlendInTime = BlendInTime;
		//Montage->BlendOutTime = BlendOutTime;
		PlayAnimMontage(Montage, PlaybackSpeed);
	}
	else
	{
		if (GetMesh()->GetAnimInstance())
		{
			GetMesh()->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(Sequence, ItemAnimationSlot, BlendInTime, BlendOutTime, PlaybackSpeed);
		}
	}
}

void ASoldier::FireProjectile(TSubclassOf<ABaseProjectile> ProjectileClass, UCurveVector * RecoilCurve, const FVector & VisualRecoil)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = this;
	SpawnParams.Owner = CurrentItem;

	FVector Location;
	FRotator Rotation;
	GetActorEyesViewPoint(Location, Rotation);

	if (RecoilCurve)
	{
		// Evalute curve and update recoil
		const FVector Recoil = RecoilCurve->GetVectorValue(BulletCount);
		const float Imprecision = CurrentItem->GetImprecision(AimRatio, BulletCount, GetCharacterMovement()->Velocity);
		Rotation += FRotator(FMath::FRandRange(-Imprecision, Imprecision), FMath::FRandRange(-Imprecision, Imprecision), FMath::FRandRange(-Imprecision, Imprecision));
		TargetPitchRecoil += Recoil.X;
		TargetYawRecoil += Recoil.Y;
	}

	// Multiplyer for item recoil
	const float AimMultiplyer = FMath::Lerp(1.f, 0.8f, AimRatio);
	// Apply item recoil to target item recoil
	TargetItemRecoil += FVector(-VisualRecoil.X, FMath::FRandRange(-VisualRecoil.Y, VisualRecoil.Y), FMath::FRandRange(-VisualRecoil.Z, VisualRecoil.Z)) * AimMultiplyer;

	// Raise bullet count
	BulletCount += 1.f;

	// Spawn actual projectile
	GetWorld()->SpawnActor<AActor>(ProjectileClass, Location, Rotation, SpawnParams);
}

void ASoldier::ThrowProjectile(TSubclassOf<AThrowableProjectile> ProjectileClass)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = this;
	SpawnParams.Owner = CurrentItem;

	FVector Location;
	FRotator Rotation;
	GetActorEyesViewPoint(Location, Rotation);

	// Spawn projectile
	GetWorld()->SpawnActor<AThrowableProjectile>(ProjectileClass, Location, Rotation, SpawnParams);
}

void ASoldier::OnClientStartAutoFire(float FireLength)
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_ItemTask, this, &ASoldier::OnClientAutoFire, FireLength);
}

void ASoldier::OnClientAutoFire()
{
	// Only fire next round if server is still firing
	if (CurrentItem && ItemTask == EItemTaskEnum::Fire)
	{
		CurrentItem->OnFire();
	}
}

bool ASoldier::Stab(FHitResult & OutHit, float Damage, float MaxDistance)
{
	FVector Location;
	FRotator Rotation;
	GetActorEyesViewPoint(Location, Rotation);

	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	CollisionParams.bReturnPhysicalMaterial = true;

	// Trace prjectile channel
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Location, Location + Rotation.Vector() * MaxDistance, ECC_GameTraceChannel1, CollisionParams))
	{
		if (OutHit.GetActor())
		{
			FPointDamageEvent DamageEvent;
			DamageEvent.DamageTypeClass = UDamageType_Knife::StaticClass();
			DamageEvent.Damage = Damage;
			DamageEvent.HitInfo = OutHit;
			DamageEvent.ShotDirection = Rotation.Vector();
			OutHit.Actor->TakeDamage(Damage, DamageEvent, Controller, CurrentItem);
		}

		return true;
	}

	return false;
}

ETeamEnum ASoldier::GetTeam() const
{
	ABasePlayerState * PS = Cast<ABasePlayerState>(PlayerState);
	if (PS)
	{
		return PS->GetTeam();
	}

	return ETeamEnum::None;
}

bool ASoldier::IsEnemyFor(ASoldier * OtherSoldier) const
{
	if (OtherSoldier)
	{
		return IsEnemyFor(Cast<ABasePlayerState>(OtherSoldier->PlayerState));
	}

	return false;
}

bool ASoldier::IsEnemyFor(ABasePlayerState * OtherPlayerState) const
{
	ABasePlayerState * BasePlayerState = Cast<ABasePlayerState>(PlayerState);
	if (BasePlayerState)
	{
		return BasePlayerState->IsEnemyFor(OtherPlayerState);
	}

	return false;
}

bool ASoldier::IsEnemyFor(ABasePlayerController * OtherPlayerController) const
{
	if (OtherPlayerController)
	{
		return IsEnemyFor(Cast<ABasePlayerState>(OtherPlayerController->PlayerState));
	}

	return false;
}

void ASoldier::ServerUpdateInput_Implementation(const FSoldierInputStruct & PlayerInput)
{
	bAiming = PlayerInput.bPressedAim;

	// Fire
	if (PlayerInput.bPressedFire)
	{
		if (!bFiring)
		{
			StartFiring();
		}
	}
	else
	{
		if (bFiring)
		{
			StopFiring();
		}
	}

	// Sprint
	if (PlayerInput.bPressedSprint)
	{
		if (!bSprinting)
		{
			StartSprinting();
		}
	}
	else
	{
		if (bSprinting)
		{
			StopSprinting();
		}
	}

	if (InteractionActor && !PlayerInput.bPressedInteract)
	{
		StopInteract();
	}

	LeanRatio = PlayerInput.LeanRatio;
}

bool ASoldier::ServerUpdateInput_Validate(const FSoldierInputStruct & PlayerInput)
{
	return true;
}

void ASoldier::StartInteract()
{
	bInteracting = true;

	// Iterate all actors and try to interact with them
	InteractWith(GetBestInteractionTarget());
}

bool ASoldier::InteractWith(AActor * TargetActor)
{
	if (CanInteractWith(TargetActor))
	{
		if (GetNetMode() != NM_Client)
		{
			IInteractionInterface * InterfaceActor = Cast<IInteractionInterface>(TargetActor);
			if (InterfaceActor)
			{
				if (PerformSoldierTask(ESoldierTaskEnum::Interact, InterfaceActor->GetInteractionMontage(this), InterfaceActor->GetInteractionLength()))
				{
					InteractionActor = TargetActor;
					return true;
				}
			}
		}
		else
		{
			ServerInteractWith(TargetActor);
		}
	}

	return false;
}

void ASoldier::ServerInteractWith_Implementation(AActor * TargetActor)
{
	InteractWith(TargetActor);
}

bool ASoldier::ServerInteractWith_Validate(AActor * TargetActor)
{
	return true;
}

bool ASoldier::CanInteractWith(AActor * InterfaceActor)
{
	IInteractionInterface * Interface = Cast<IInteractionInterface>(InterfaceActor);
	if (Interface)
	{
		FVector CameraLocation = Camera->GetComponentLocation();
		const FVector InteractionLocation = Interface->GetInteractionLocation();
		FVector ToVector = InteractionLocation - CameraLocation;
		const float DistanceSquared = ToVector.SizeSquared();
		ToVector.Normalize();
		const float Angle = FMath::Acos(ToVector | GetControlRotation().Vector());
		return DistanceSquared <= MaxInteractionDistanceSquared && Angle < MaxInteractionAngle &&  Interface->CanSoldierInteract(this);
	}

	return false;
}

AActor * ASoldier::GetBestInteractionTarget()
{
	if (InteractionActor || SoldierTask == ESoldierTaskEnum::Interact) return InteractionActor;

	// Iterate all actors and try to interact with them
	for (FActorIterator ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (CanInteractWith(*ActorItr))
		{
			return *ActorItr;
		}
	}

	return NULL;
}

FVector ASoldier::GetInteractionLocation(AActor * Actor)
{
	IInteractionInterface * Interface = Cast<IInteractionInterface>(Actor);
	if (Interface)
	{
		return Interface->GetInteractionLocation();
	}

	return FVector::ZeroVector;
}

void ASoldier::StopInteract()
{
	bInteracting = false;

	if (GetNetMode() == NM_Client) return;

	if (InteractionActor)
	{
		InteractionActor = NULL;
		StopSoldierTask();
	}
}

void ASoldier::OnFinishedInteraction()
{
	IInteractionInterface * Interface = Cast<IInteractionInterface>(InteractionActor);
	if (Interface && Interface->CanSoldierInteract(this))
	{
		// Execute actual interaction logic
		Interface->OnInteract(this);
		InteractionActor = NULL;
		StopInteract();
	}
}

void ASoldier::PickUpItem(AItem * Item)
{
	if (!Item || !Item->CanBePickedUp()) return;

	AddItem(Item);

	if (Item != CurrentItem)
	{
		StoreItem(CurrentItem);
		CurrentItem = Item;
		InitCurrentItem();

		CurrentItem->Draw();
	}

	// Set slot to the picked up item
	/*for (uint8 i = 0; i < NumItemSlots; i++)
	{
		if (Items[i] == Item)
		{
			SetSlot(i);
			break;
		}
	}*/
}

void ASoldier::MoveForward(float Val)
{
	AddMovementInput(GetActorForwardVector(), Val);
}

void ASoldier::MoveRight(float Val)
{
	AddMovementInput(GetActorRightVector(), Val);
}

void ASoldier::AddControllerPitchInput(float Val)
{
	Super::AddControllerPitchInput(Val * Camera->FieldOfView / DefaultFOV);
}

void ASoldier::AddControllerYawInput(float Val)
{
	Super::AddControllerYawInput(Val * Camera->FieldOfView / DefaultFOV);
}

void ASoldier::StartSprinting()
{
	bSprinting = true;
}

void ASoldier::StopSprinting()
{
	bSprinting = false;
}

void ASoldier::StartCrouching()
{
	Crouch();
}

void ASoldier::StopCrouching()
{
	UnCrouch();
}

void ASoldier::StartFiring()
{
	bFiring = true;

	if (GetNetMode() == NM_Client) return;

	if (CurrentItem)
	{
		CurrentItem->StartFiring();
	}
}

void ASoldier::StopFiring()
{
	bFiring = false;

	if (GetNetMode() == NM_Client) return;

	if (CurrentItem)
	{
		CurrentItem->StopFiring();
	}
}

void ASoldier::StartAiming()
{
	bAiming = true;
}

void ASoldier::StopAiming()
{
	bAiming = false;
}

void ASoldier::ToggleFireMode()
{
	if (GetNetMode() == NM_Client)
	{
		ServerToggleFireMode();
		return;
	}

	if (CurrentItem)
	{
		CurrentItem->ToggleFireMode();
	}
}

void ASoldier::ToggleCameraMode()
{
	bInThirdPersonMode = !bInThirdPersonMode;
}

void ASoldier::ServerToggleFireMode_Implementation()
{
	ToggleFireMode();
}

bool ASoldier::ServerToggleFireMode_Validate()
{
	return true;
}

void ASoldier::Reload()
{
	if (GetNetMode() == NM_Client)
	{
		ServerReload();
		return;
	}

	if (CurrentItem)
	{
		CurrentItem->Reload();
	}
}

void ASoldier::ServerReload_Implementation()
{
	Reload();
}

void ASoldier::DropItem()
{
	if (!CurrentItem || !CurrentItem->CanBeDropped()) return;

	if (GetNetMode() == NM_Client)
	{
		ServerDropItem();
		return;
	}

	CurrentItem->Drop();

	for (uint8 i = 0; i < NumItemSlots; i++)
	{
		if (Items[i] == CurrentItem)
		{
			// Remove CurrentItem from Items
			Items[i] = NULL;
			break;
		}
	}

	ItemTask = EItemTaskEnum::None;

	// Switch to first valid item
	for (AItem * Item : Items)
	{
		if (Item)
		{
			CurrentItem = Item;
			InitCurrentItem();
			CurrentItem->Draw();
			break;
		}
	}
}

void ASoldier::ServerDropItem_Implementation()
{
	DropItem();
}

bool ASoldier::ServerDropItem_Validate()
{
	return true;
}

bool ASoldier::ServerReload_Validate()
{
	return true;
}

void ASoldier::EquipSlot0()
{
	SetSlot(0);
}

void ASoldier::EquipSlot1()
{
	SetSlot(1);
}

void ASoldier::EquipSlot2()
{
	SetSlot(2);
}

void ASoldier::EquipSlot3()
{
	SetSlot(3);
}

void ASoldier::EquipSlot4()
{
	SetSlot(4);
}

void ASoldier::EquipSlot5()
{
	SetSlot(5);
}

void ASoldier::EquipSlot6()
{
	SetSlot(6);
}

void ASoldier::EquipNextItem()
{
	// Search for first item after CurrentItem
	bool bFoundCurrentItem = false;
	for (uint8 i = 0; i < NumItemSlots; i++)
	{
		if (bFoundCurrentItem)
		{
			if (Items[i])
			{
				SetSlot(i);
				break;
			}
		}
		else
		{
			bFoundCurrentItem = Items[i] == CurrentItem;
		}
	}
}

void ASoldier::EquipPreviousItem()
{
	// Search for first item before CurrentItem
	bool bFoundCurrentItem = false;
	for (uint8 i = NumItemSlots - 1; i >= 0; i--)
	{
		if (bFoundCurrentItem)
		{
			if (Items[i])
			{
				SetSlot(i);
				break;
			}
		}
		else
		{
			bFoundCurrentItem = Items[i] == CurrentItem;
		}
	}
}

void ASoldier::SetSlot(uint8 NewSlot, bool bInstantFire)
{
	if (GetNetMode() == NM_Client)
	{
		ServerSetSlot(NewSlot, bInstantFire);
		return;
	}

	if (NewSlot >= 0 && NewSlot < NumItemSlots && Items[NewSlot] && Items[NewSlot] != CurrentItem)
	{
		NextSlot = NewSlot;
		bInstantFireItem = bInstantFire;

		if (CurrentItem)
		{
			// Switch to new item after CurrentItem undrew
			CurrentItem->Undraw();
		}
		else
		{
			// CurrentItem does not exist, we can switch to new item right away
			CurrentItem = Items[NewSlot];

			InitCurrentItem();
			CurrentItem->Draw();
		}
	}
}

void ASoldier::ServerSetSlot_Implementation(uint8 NewSlot, bool bInstantFire)
{
	SetSlot(NewSlot, bInstantFire);
}

bool ASoldier::ServerSetSlot_Validate(uint8 NewSlot, bool bInstantFire)
{
	return true;
}

void ASoldier::ThrowGrenade()
{
	EquipFirstItemOfClass<AThrowableItem>(true);
}

void ASoldier::Knife()
{
	EquipFirstItemOfClass<AKnife>(true);
}

void ASoldier::OnRep_Health()
{
	if (Health <= 0.f)
	{
		// Activate ragdoll if health <= 0
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		bInThirdPersonMode = true;
	}

	// Set saturation
	FPostProcessSettings & PPS = Camera->PostProcessSettings;
	PPS.FilmSaturation = Health / DefaultHealth;
	PPS.SceneFringeIntensity = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 20.f), FVector2D(5.f, 0.f), Health);
	PPS.VignetteIntensity = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 50.f), FVector2D(1.f, 0.f), Health);
}

void ASoldier::OnRep_CharacterMesh()
{
	if (CharacterMesh)
	{
		GetMesh()->SetSkeletalMesh(CharacterMesh);
	}
}

void ASoldier::OnRep_CurrentItem(AItem * OldItem)
{
	if (OldItem)
	{
		for (AItem * Item : Items)
		{
			if (Item == OldItem)
			{
				// Store previous CurrentItem
				StoreItem(Item);
				break;
			}
		}
	}

	if (CurrentItem)
	{
		if (CurrentItem->Instigator != this)
		{
			// Update Instigator on item if it is not up to date
			CurrentItem->Instigator = this;
			CurrentItem->OnRep_Instigator();
		}

		InitCurrentItem();
		
		if (ItemTask == EItemTaskEnum::Undraw)
		{
			ItemTask = EItemTaskEnum::Draw;
		}
		else
		{
			OnRep_ItemTask(EItemTaskEnum::None);
		}
	}
}

void ASoldier::OnRep_ItemTask(EItemTaskEnum::Type OldTask)
{
	if (CurrentItem && OldTask != EItemTaskEnum::Undraw)
	{
		CurrentItem->OnRep_Task(ItemTask);
	}
}

void ASoldier::OnSoldierTaskChanged()
{
	if (ItemTask != EItemTaskEnum::None &&  SoldierTask > ESoldierTaskEnum::FullBody)
	{
		StopItemTask();
	}

	switch (SoldierTask)
	{
	case (ESoldierTaskEnum::None) :
		StopAnimMontage(SoldierTaskMontage);
		break;
	case (ESoldierTaskEnum::Interact) :
		IInteractionInterface * Interface = Cast<IInteractionInterface>(InteractionActor);
		if (Interface)
		{
			// TODO
			SoldierTaskMontage = Interface->GetInteractionMontage(this);
			PlayAnimMontage(SoldierTaskMontage);
		}
		break;
	}
}

void ASoldier::OnRep_InteractionActor()
{
	// Setup empty timer on clients so they also know the interaction time remaining
	IInteractionInterface * Interface = Cast<IInteractionInterface>(InteractionActor);
	if (Interface)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_SoldierTask, Interface->GetInteractionLength(), false);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_SoldierTask);
		StopInteract();
	}
}