// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/MainCamera.h"
#include "Actor/Utilities/SideScrollingCameraComponent.h"
#include "Components/SplineComponent.h"
#include "Actor/BasePlayer.h"
#include "EngineUtils.h"


// Sets default values
AMainCamera::AMainCamera() : Camera(CreateDefaultSubobject<USideScrollingCameraComponent>("Side Scroller Camera")),
							 PriorityCamera(false)

{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(Camera);

	TStartingPositionOffset.Parameter = ECameraTriggerUpdateVectorParameter::LocationOffset;
	TStartingRotationOffset.Parameter = ECameraTriggerUpdateVectorParameter::RotationOffset;
}

// Called when the game starts or when spawned
void AMainCamera::BeginPlay()
{
	Super::BeginPlay();

	CachePlayer();

	if (PlayerTarget)
	{
		Camera->SetCameraOwner(PlayerTarget);

		if (PriorityCamera)
		{
			PlayerTarget->SetCameraRef(this);
			if (APlayerController* PC = PlayerTarget->GetController<APlayerController>())
			{
				FViewTargetTransitionParams Params;
				PC->SetViewTarget(this, Params);
			}
		}
		TStartingPosition.Parameter = ECameraTriggerUpdateVectorParameter::GivenLocation;
		TStartingPosition.Value = GetActorLocation();
		TStartingPosition.BlendTime = 0.f;
		Camera->UpdateOrGetVectorParameter(TStartingPosition.Parameter, &TStartingPosition);
		
		Camera->UpdateOrGetVectorParameter(TStartingPositionOffset.Parameter, &TStartingPositionOffset);
		Camera->UpdateOrGetVectorParameter(TStartingRotationOffset.Parameter, &TStartingRotationOffset);
	}
}

// Called every frame
void AMainCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMainCamera::CachePlayer()
{
	if (!PlayerClass)
		return;

	for (TActorIterator<AActor> It(GetWorld(), PlayerClass); It; ++It)
	{
		if (ABasePlayer* IsPlayer = Cast<ABasePlayer>(*It))
		{
			PlayerTarget = IsPlayer;
			break;
		}
	}
	check(PlayerTarget != nullptr);
}

