// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/JoeNandu.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Actor/Utilities/AttackComboComponent.h"


AJoeNandu::AJoeNandu()
{
	// Setup Attachments to The Correct Skeletons
	LeftArmHitBox->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	RightArmHitBox->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	RightLegsHitBox->SetupAttachment(GetMesh(), FName("calf_r"));
	LeftLegsHitBox->SetupAttachment(GetMesh(), FName("calf_l"));

	BlockWindow->SetupAttachment(GetMesh());
	ParryWindow->SetupAttachment(GetMesh());
}

void AJoeNandu::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(DashSprintAction, ETriggerEvent::Started, this, &AJoeNandu::SpecialMovement);
	}
}

void AJoeNandu::SpecialMovement_Implementation()
{
	if (!IsBlocking && !AttackCombo->GetActionHappening())
	{
		OnDash.Broadcast();
	}
}
