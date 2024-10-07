// Copyright Epic Games, Inc. All Rights Reserved.



#include "Actor/BasePlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/CharacterAnimation.h"
#include "Actor/Utilities/AttackTypeContents.h"
#include "Actor/Utilities/SideScrollingCameraComponent.h"
#include "Component/DazedSystemComponent.h"
#include "Actor/Utilities/AttackComboComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Widget/PauseMenuWidget.h"
#include "../BleedingEdgeBrawl.h"

ABasePlayer::ABasePlayer() : bCanWeJump(true)
{
	// Set Class Values
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// Create Components
	AttackCombo = CreateDefaultSubobject<UAttackComboComponent>("Attack Component");

	LeftArmHitBox = CreateDefaultSubobject<UBoxComponent>("LeftArmBox");
	RightArmHitBox = CreateDefaultSubobject<UBoxComponent>("RightArmBox");
	RightLegsHitBox = CreateDefaultSubobject<UBoxComponent>("RightLegBox");
	LeftLegsHitBox = CreateDefaultSubobject<UBoxComponent>("LeftLegBox");

	BlockWindow = CreateDefaultSubobject<USphereComponent>("Block Sphere");
	ParryWindow = CreateDefaultSubobject<USphereComponent>("Parry Sphere");

	// Setup Attachments for the Hitboxes
	LeftArmHitBox->SetupAttachment(GetMesh(), FName("LeftForeArm"));
	RightArmHitBox->SetupAttachment(GetMesh(), FName("RightForeArm"));
	RightLegsHitBox->SetupAttachment(GetMesh(), FName("RightLeg"));
	LeftLegsHitBox->SetupAttachment(GetMesh(), FName("LeftLeg"));

	//Setup Attachments for the Block & Parry Window
	BlockWindow->SetupAttachment(GetMesh());
	ParryWindow->SetupAttachment(GetMesh());
}

// Called when the game starts or when spawned
void ABasePlayer::BeginPlay()
{
	Super::BeginPlay();


	if (APlayerController* PC = GetController<APlayerController>())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
				PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(PlayerMappingContext, 0);
		}
	}

	AnimationBP = Cast<UCharacterAnimation>(GetMesh()->GetAnimInstance());
	if (AnimationBP)
	{
		OnDash.AddDynamic(AnimationBP, &UCharacterAnimation::Dash);
		AttackCombo->QuickAttack.AddDynamic(AnimationBP, &UCharacterAnimation::QuickAttackCombo);
		AttackCombo->HeavyAttack.AddDynamic(AnimationBP, &UCharacterAnimation::HeavyAttackCombo);
		AttackCombo->SpecialAttack.AddDynamic(AnimationBP, &UCharacterAnimation::SpecialAttack);

		AttackCombo->ContinueQuickAttack.AddDynamic(AnimationBP, &UCharacterAnimation::IncrementACombo);
		AttackCombo->ContinueHeavyAttack.AddDynamic(AnimationBP, &UCharacterAnimation::IncrementACombo);

		OnHit.AddDynamic(AnimationBP, &UCharacterAnimation::BlockHitReact);
	}


	OnLaunch.AddDynamic(this, &ABasePlayer::Jump);
	LandedDelegate.AddDynamic(this, &ABasePlayer::HandleOnLand);


	BlockWindow->OnComponentBeginOverlap.AddDynamic(this, &ABasePlayer::HandleBlock);
	ParryWindow->OnComponentBeginOverlap.AddDynamic(this, &ABasePlayer::HandleParry);
	OnParrySuccess.AddDynamic(AttackCombo, &UAttackComboComponent::SuccessfulParry);

	OnActionStopped.AddDynamic(AttackCombo, &UAttackComboComponent::ActionHasStopped);
	ResetActionHappening.AddDynamic(AttackCombo, &UAttackComboComponent::ActionHasStopped);

	OnHit.AddDynamic(AttackCombo, &UAttackComboComponent::BlockHitReact);

	BindHitBoxToggles();
	DisableAllHitboxes(ECollisionEnabled::NoCollision);
	DisableAllDefenseHitBoxes(ECollisionEnabled::NoCollision);
}



// Called to bind functionality to input
void ABasePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Pause Inputs
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ABasePlayer::PauseGame);

		// Locomotion Inputs
		EnhancedInputComponent->BindAction(ForwardMovementAction, ETriggerEvent::Triggered, this, &ABasePlayer::MoveForward);
		EnhancedInputComponent->BindAction(RightMovementAction, ETriggerEvent::Triggered, this, &ABasePlayer::MoveRight);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABasePlayer::StartJump);

		// Combat Inputs
		EnhancedInputComponent->BindAction(QuickAttackAction, ETriggerEvent::Started, AttackCombo, &UAttackComboComponent::StartQuickAttack);
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, AttackCombo, &UAttackComboComponent::StartHeavyAttack);
		EnhancedInputComponent->BindAction(SpecialAttackAction, ETriggerEvent::Started, AttackCombo, &UAttackComboComponent::StartSpecialAttack);

		// Defense Inputs
		EnhancedInputComponent->BindAction(BlockParryAction, ETriggerEvent::Started, this, &ABasePlayer::StartBlock);
		EnhancedInputComponent->BindAction(BlockParryAction, ETriggerEvent::Completed, this, &ABasePlayer::StopBlock);
	}
}

void ABasePlayer::StopJump_Implementation()
{
	bCanWeJump = true;
}

void ABasePlayer::MoveForward(const FInputActionValue& Value)
{
	if (!IsBlocking && !AttackCombo->GetActionHappening())
	{
		if (EInputActionValueType::Axis1D == Value.GetValueType())
		{
			float ScaleSpeed = Value.Get<float>();

			AddMovementInput(FVector(GetControlRotation().Vector().X,
				GetControlRotation().Vector().Y,
				0.f), ScaleSpeed);
		}
	}
}

void ABasePlayer::MoveRight(const FInputActionValue& Value)
{
	if (!IsBlocking && !AttackCombo->GetActionHappening())
	{
		if (EInputActionValueType::Axis1D == Value.GetValueType())
		{
			float ScaleSpeed = Value.Get<float>();

			AddMovementInput(FVector(FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::Y).X * 100.f,
				FRotationMatrix(GetControlRotation()).GetScaledAxis(EAxis::Y).Y,
				0.f), ScaleSpeed);
		}
	}
}

void ABasePlayer::StartJump()
{
	if ((bCanWeJump && !IsBlocking) && !AttackCombo->GetActionHappening())
	{
		bCanWeJump = false;
		bIsJumping = true;
	}
}

void ABasePlayer::StartBlock()
{
	if (!IsBlocking && !AttackCombo->GetActionHappening())
	{
		IsBlocking = true;
		AttackCombo->SetActionHappening(true);
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	}
}

void ABasePlayer::StopBlock()
{
	IsBlocking = false;
	OnActionStopped.Broadcast(false);
	ToggleBlockCollider(false);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void ABasePlayer::PauseGame()
{
	if (!IsPauseOpen)
	{
		IsPauseOpen = true;
		if (!PauseRef)
			PauseRef = CreateWidget<UPauseMenuWidget>(OwnerController, PauseWidgetClass);

		PauseRef->AddToViewport();
		FInputModeUIOnly IPM;
		IPM.SetWidgetToFocus(PauseRef->TakeWidget());
		IPM.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		OwnerController->SetInputMode(IPM);
		OwnerController->bShowMouseCursor = true;
		OwnerController->SetPause(IsPauseOpen);
	}
	else if (IsPauseOpen)
	{
		IsPauseOpen = false;
		if (PauseRef)
		{
			PauseRef->RemoveFromParent();
			OwnerController->SetPause(IsPauseOpen);
			FInputModeGameOnly IMG;
			OwnerController->SetInputMode(IMG);
			OwnerController->bShowMouseCursor = false;
		}
	}
}

void ABasePlayer::DisableAllDefenseHitBoxes(ECollisionEnabled::Type In)
{
	BlockWindow->SetCollisionEnabled(In);
	ParryWindow->SetCollisionEnabled(In);
}

void ABasePlayer::BindHitBoxToggles()
{
	OnPunchRight.AddDynamic(this, &ABasePlayer::ToggleRightArm);
	OnPunchLeft.AddDynamic(this, &ABasePlayer::ToggleLeftArm);
	OnKickRight.AddDynamic(this, &ABasePlayer::ToggleRightLeg);
	OnKickLeft.AddDynamic(this, &ABasePlayer::ToggleLeftLeg);

	OnBlock.AddDynamic(this, &ABasePlayer::ToggleBlockCollider);
	OnParry.AddDynamic(this, &ABasePlayer::ToggleParryCollider);
}

void ABasePlayer::ToggleRightArm(bool in)
{
	RightArmHitBox->SetVisibility(in);
	if (in)
		RightArmHitBox->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	else
		RightArmHitBox->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void ABasePlayer::ToggleRightLeg(bool in)
{
	RightLegsHitBox->SetVisibility(in);
	if (in)
		RightLegsHitBox->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	else
		RightLegsHitBox->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void ABasePlayer::ToggleLeftArm(bool in)
{
	LeftArmHitBox->SetVisibility(in);
	if (in)
		LeftArmHitBox->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	else
		LeftArmHitBox->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void ABasePlayer::ToggleLeftLeg(bool in)
{
	LeftLegsHitBox->SetVisibility(in);
	if (in)
		LeftLegsHitBox->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	else
		LeftLegsHitBox->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void ABasePlayer::ToggleBlockCollider(bool in)
{
	BlockWindow->SetVisibility(in);
	if (in)
		BlockWindow->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	else
		BlockWindow->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

}

void ABasePlayer::ToggleParryCollider(bool in)
{
	ParryWindow->SetVisibility(in);
	if (in)
		ParryWindow->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	else
		ParryWindow->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void ABasePlayer::DisableAllHitboxes(ECollisionEnabled::Type In)
{
	LeftArmHitBox->SetCollisionEnabled(In);
	LeftLegsHitBox->SetCollisionEnabled(In);
	RightArmHitBox->SetCollisionEnabled(In);
	RightLegsHitBox->SetCollisionEnabled(In);
}

void ABasePlayer::BindQuickAttacksToAHitBox(UBoxComponent* HitBox)
{
	if (!HitBox->OnComponentBeginOverlap.IsBound())
		HitBox->OnComponentBeginOverlap.AddDynamic(AttackCombo, &UAttackComboComponent::HandleQuickAttackHit);
}

void ABasePlayer::UnBindQuickAttacksToAHitBox(UBoxComponent* HitBox)
{
	if (HitBox->OnComponentBeginOverlap.IsBound())
		HitBox->OnComponentBeginOverlap.RemoveDynamic(AttackCombo, &UAttackComboComponent::HandleQuickAttackHit);
}

void ABasePlayer::BindHeavyAttacksToAHitBox(UBoxComponent* HitBox)
{
	if (!HitBox->OnComponentBeginOverlap.IsBound())
		HitBox->OnComponentBeginOverlap.AddDynamic(AttackCombo, &UAttackComboComponent::HandleHeavyAttackHit);
}

void ABasePlayer::UnBindHeavyAttacksToAHitBox(UBoxComponent* HitBox)
{
	if (HitBox->OnComponentBeginOverlap.IsBound())
		HitBox->OnComponentBeginOverlap.RemoveDynamic(AttackCombo, &UAttackComboComponent::HandleHeavyAttackHit);
}

void ABasePlayer::BindSpecialAttacksToAHitBox(UBoxComponent* HitBox)
{
	if (!HitBox->OnComponentBeginOverlap.IsBound())
		HitBox->OnComponentBeginOverlap.AddDynamic(AttackCombo, &UAttackComboComponent::HandleSpecialAttackHit);
}

void ABasePlayer::UnBindSpecialAttacksToAHitBox(UBoxComponent* HitBox)
{
	if (HitBox->OnComponentBeginOverlap.IsBound())
		HitBox->OnComponentBeginOverlap.RemoveDynamic(AttackCombo, &UAttackComboComponent::HandleSpecialAttackHit);
}

void ABasePlayer::Knockback()
{
	IsRecovering = false;
	IsKnockdown = true;
	// Set Timer to Call Recover()
	FTimerHandle timer;
	GetWorld()->GetTimerManager().SetTimer(timer, this, &ABasePlayer::Recover, 0.8f);
}

void ABasePlayer::Recover()
{
	IsRecovering = IsKnockdown;
	AttackCombo->ResetHitCounter();
	IsKnockdown = false;
}

void ABasePlayer::PlayerHadDied(AActor* DeadActor)
{
	PlayerIsDead.Broadcast(DeadActor);
}

void ABasePlayer::WinGame_Implementation()
{
	DisableInput(OwnerController);
}

void ABasePlayer::HandleOnLand(const FHitResult& Hit)
{
	bIsJumping = false;
}

void ABasePlayer::HandleBlock_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this)
		return;

	OnHit.Broadcast(StandardDamage);
}

void ABasePlayer::HandleParry_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this)
		return;

	OnParrySuccess.Broadcast(StandardDamage, Cast<ACharacter>(OtherActor));
}

// Getters & Setters
bool ABasePlayer::GetIsJumping() const
{
	return bIsJumping;
}

bool ABasePlayer::GetIsSprinting() const
{
	return IsSprinting;
}

bool ABasePlayer::GetIsBlocking() const
{
	return IsBlocking;
}

void ABasePlayer::SetCameraRef(AMainCamera* Camera)
{
	CameraRef = Camera;
}

AMainCamera* ABasePlayer::GetCameraRef() const
{
	return CameraRef;
}

APlayerController* ABasePlayer::GetPlayerController() const
{
	return OwnerController;
}

bool ABasePlayer::GetIsKnockdown() const
{
	return IsKnockdown;
}

bool ABasePlayer::GetIsRecovering() const
{
	return IsRecovering;
}

void ABasePlayer::SetIsKnockdown(bool In)
{
	IsKnockdown = In;
}