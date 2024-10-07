// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/CameraTriggerBox.h"
#include "Actor/BasePlayer.h"
#include "Components/BoxComponent.h"
#include "Actor/Utilities/SideScrollingCameraComponent.h"
#include "Actor/MainCamera.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ACameraTriggerBox::ACameraTriggerBox() : TriggerBox(CreateDefaultSubobject<UBoxComponent>(TEXT("Box")))
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ACameraTriggerBox::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACameraTriggerBox::HandleBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ACameraTriggerBox::HandleEndOverlap);
}

void ACameraTriggerBox::InitiateCameraUpdate_Implementation(AActor* Actor, bool Undo)
{
	class ABasePlayer* CameraOwner = Cast<ABasePlayer>(Actor);
	if (!CameraOwner)
		return;

	class AMainCamera* CameraPawn = CameraOwner->GetCameraRef();
	if (!CameraPawn)
		return;

	USideScrollingCameraComponent* SideScrollerComponent = CameraPawn->FindComponentByClass<USideScrollingCameraComponent>();
	if (!SideScrollerComponent)
		return;
	else if (!Cast<ABasePlayer>(Actor)->IsPlayerControlled())
		return;
	APlayerController* PC = Cast<ABasePlayer>(Actor)->GetController<APlayerController>();
	if (!PC)
		return;


	if (!Undo)
	{
		PreviousCamera = PC->GetViewTarget();
		for (const FCameraEventUpdateFloat Params : FloatParameters)
		{
			FCameraEventUpdateFloat Update = {
				Params.Parameter,
				SideScrollerComponent->UpdateOrGetFloatParameter(Params.Parameter),
				Params.BlendTime
			};
			PreviousFloatParameters.Push(Update);
		}
		for (const FCameraEventUpdateVector Params : VectorParameters)
		{
			FCameraEventUpdateVector Update =
			{
				Params.Parameter,
				SideScrollerComponent->UpdateOrGetVectorParameter(Params.Parameter),
				Params.BlendTime
			};
			PreviousVectorParameters.Push(Update);
		}
		for (const FCameraEventUpdateBool Params : BoolParameters)
		{
			FCameraEventUpdateBool Update =
			{
				Params.Parameter,
				SideScrollerComponent->UpdateOrGetBoolParameter(Params.Parameter),
				0.f
			};
		}
	}


	AActor* ViewTarget = Undo ? PreviousCamera : TargetCamera;
	if (ViewTarget)
	{
		CameraOwner->SetCameraRef(Cast<AMainCamera>(ViewTarget));
		PC->SetViewTargetWithBlend(
			ViewTarget,
			TargetCameraBlendTime,
			VTBlend_EaseInOut,
			1,
			true
		);

	}
	else
	{
		PC->SetViewTargetWithBlend(
			CameraPawn,
			SideScrollerComponent->GetDefaultCameraBlendSpeed(),
			VTBlend_EaseInOut,
			1,
			true
		);
	}


	for (int i = 0; i < FloatParameters.Num(); i++)
	{
		FCameraEventUpdateFloat Update =
		{
			FloatParameters[i].Parameter,
			Undo ? PreviousFloatParameters[i].Value : FloatParameters[i].Value,
			FloatParameters[i].BlendTime
		};
		SideScrollerComponent->UpdateOrGetFloatParameter(FloatParameters[i].Parameter, &Update);
	}
	for (int i = 0; i < VectorParameters.Num(); i++)
	{
		FCameraEventUpdateVector Update =
		{
			VectorParameters[i].Parameter,
			Undo ? PreviousVectorParameters[i].Value : VectorParameters[i].Value,
			VectorParameters[i].BlendTime
		};
		SideScrollerComponent->UpdateOrGetVectorParameter(VectorParameters[i].Parameter, &Update);
	}
	for (size_t i = 0; i < BoolParameters.Num(); i++)
	{
		FCameraEventUpdateBool Update =
		{
			BoolParameters[i].Parameter,
			Undo ? PreviousBoolParameters[i].Value : BoolParameters[i].Value,
			0.f
		};
		SideScrollerComponent->UpdateOrGetBoolParameter(BoolParameters[i].Parameter, &Update);
	}
}

void ACameraTriggerBox::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	InitiateCameraUpdate(OtherActor, false);
}

void ACameraTriggerBox::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (UndoAfterEndOverlap)
		InitiateCameraUpdate(OtherActor, true);
}

