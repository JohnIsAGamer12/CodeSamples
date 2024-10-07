// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Utilities/SideScrollingCameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Actor/BasePlayer.h"
#include "../BleedingEdgeBrawl.h"


USideScrollingCameraComponent::USideScrollingCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	GivenLocation = FVector(0.f, 0.f, 0.f);
	GivenRotation = FVector(0.f, 0.f, 0.f);
}

void USideScrollingCameraComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USideScrollingCameraComponent::SetCameraOwner(ACharacter* Player)
{
	CameraOwner = Player;
}

float USideScrollingCameraComponent::UpdateOrGetFloatParameter(ECameraTriggerUpdateFloatParameter Parameter, FCameraEventUpdateFloat* Update)
{
	float ret = 0.f;
	if (CameraOwner)
	{
		switch (Parameter)
		{
		case ECameraTriggerUpdateFloatParameter::ZoomDistance:
			if (Update)
				SetZoomDistance(Update->Value, Update->BlendTime);
			ret = CameraZoomDistance;
			break;
		case ECameraTriggerUpdateFloatParameter::TraverseThreshold:
			if (Update)
				SetTraverseThreshold(Update->Value);
			ret = TraverseThreshold;
			break;
		default:
			break;
		}
	}
	return ret;
}

bool USideScrollingCameraComponent::UpdateOrGetBoolParameter(ECameraTriggerUpdateBoolParameter Parameter, FCameraEventUpdateBool* Update)
{
	bool ret = false;
	switch (Parameter)
	{
	case ECameraTriggerUpdateBoolParameter::TraverseLeftOrRight:
		if (Update)
			SetTraverseRightOrLeft(Update->Value);
		ret = TravelUpDown;
		break;
	case ECameraTriggerUpdateBoolParameter::TraverseUpOrDown:
		if (Update)
			SetTraverseUpOrDown(Update->Value);
		ret = TravelForwardBack;
		break;
	default:
		break;
	}

	return ret;
}

FVector USideScrollingCameraComponent::UpdateOrGetVectorParameter(ECameraTriggerUpdateVectorParameter Parameter, FCameraEventUpdateVector* Update)
{
	FVector ret{};
	if (CameraOwner)
	{
		switch (Parameter)
		{
		case ECameraTriggerUpdateVectorParameter::GivenLocation:
			if (Update)
				SetNewLocation(Update->Value, Update->BlendTime);
			ret = GivenLocation;
		case ECameraTriggerUpdateVectorParameter::LocationOffset:
			if (Update)
				SetLocationOffset(Update->Value, Update->BlendTime);
			ret = CameraLocationOffset;
			break;
		case ECameraTriggerUpdateVectorParameter::GivenRotation:
			if (Update)
				SetNewRotation(Update->Value, Update->BlendTime);
			ret = GivenRotation;
			break;
		case ECameraTriggerUpdateVectorParameter::RotationOffset:
			if (Update)
				SetRotationOffset(Update->Value, Update->BlendTime);
			ret = CameraRotationOffset;
			break;
		default:
			break;
		}
	}

	return ret;
}

void USideScrollingCameraComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Old Implementation
	//if (!CameraLeadLock)
	//{
	//	float Target = FixLeadAtMax ? CameraOwner->GetActorForwardVector().X : CameraOwner->GetVelocity().X / CameraOwner->GetCharacterMovement()->GetMaxSpeed();
	//	Target *= CameraLeadMaxDistance;
	//	CurrentLeadOffsetX = FMath::FInterpTo(CurrentLeadOffsetX, Target, DeltaTime, CameraLeadSpeed);
	//}

	// Move Camera
	//FVector  TargetLocation = CameraOwner->GetActorLocation();
	//FVector CameraLocation = GetComponentLocation();
	//TargetLocation.X += CurrentLeadOffsetX + CurrentLocationOffset.X;
	//TargetLocation.Y += CurrentZoomDistance + CurrentLocationOffset.Y;
	//TargetLocation.Z = CameraZLock ? CurrentZLockHeight : (TargetLocation.Z + CurrentLocationOffset.Z);
	//SetWorldLocation(TargetLocation);
}

void USideScrollingCameraComponent::UpdateCamera_Implementation(FVector StartPoint, FVector EndPoint, float DeltaTime)
{
	if (CameraOwner)
	{
		CurrentZoomDistance = FMath::FInterpTo(CurrentZoomDistance, CameraZoomDistance, DeltaTime, ZoomDistanceBlendSpeed);
		CurrentLocation = FMath::VInterpTo(CurrentLocation, GivenLocation, DeltaTime, LocationBlendSpeed);
		CurrentRotation = FMath::VInterpTo(CurrentRotation, GivenRotation, DeltaTime, RotationBlendSpeed);

		CurrentLocationOffset = FMath::VInterpTo(CurrentLocationOffset, CameraLocationOffset, DeltaTime, LocationOffsetBlendSpeed);
		CurrentRotationOffset = FMath::VInterpTo(CurrentRotationOffset, CameraRotationOffset, DeltaTime, LocationOffsetBlendSpeed);


		// Handle Zooming in on the Camera
		FVector TargetLocation(CameraOwner->GetActorLocation());
		TargetLocation.X = CurrentLocation.X;
		TargetLocation.Y = CurrentZoomDistance + CurrentLocation.Y;
		TargetLocation.Z = CurrentLocation.Z + CurrentLocationOffset.Z;

		// Move Along the X-Axis
		if (TravelForwardBack)
		{
			double r = CameraOwner->GetActorLocation().X - GetComponentLocation().X;
			if (r >= -TraverseThreshold && r <= TraverseThreshold)
			{
				if (CameraOwner->GetActorLocation().X >= StartPoint.X && CameraOwner->GetActorLocation().X <= EndPoint.X)
				{
					// Move Along the X-Axis
					CurrentLocation.X = CameraOwner->GetActorLocation().X;
					GivenLocation.X = CurrentLocation.X;
					TargetLocation.X = CurrentLocation.X;
				}
			}
		}

		// Move Along the Z-Axis
		if (TravelUpDown)
		{

			double distToDescend = -EndPoint.Z - GetRelativeLocation().Z,
				distToAscend = EndPoint.Z - GetRelativeLocation().Z;
			if (distToDescend > 0 || distToDescend < 0)
			{
				if (ZOffSet == 0)
				{
					if (CurrentLocation.Z > CameraOwner->GetActorLocation().Z)
						ZOffSet = CurrentLocation.Z - CameraOwner->GetActorLocation().Z;
					else if (CurrentLocation.Z < CameraOwner->GetActorLocation().Z)
						ZOffSet = ZOffSet - CameraOwner->GetActorLocation().Z;
				}


				TargetLocation.Z = CameraOwner->GetActorLocation().Z + ZOffSet + CurrentLocationOffset.Z;
			}
		}
		// Set Final Destination
		SetWorldLocation(TargetLocation);


		// Look at the Player and rotate only the pitch
		FRotator LookAtPlayer = FRotationMatrix::MakeFromX(CameraOwner->GetActorLocation() - GetRelativeLocation()).Rotator();
		CurrentRotation.X = FMath::Clamp(LookAtPlayer.Pitch, -MaxRotateUpDownThreshold, MaxRotateUpDownThreshold);

		FVector AdjustedRotation = FVector(CurrentRotation.Y + CurrentRotationOffset.Y, CurrentRotation.X, CurrentRotation.Z + CurrentRotationOffset.Z);
		AdjustedRotation.Z -= 90.f + CurrentRotationOffset.Z;
		SetWorldRotation(FQuat::MakeFromEuler(AdjustedRotation));
	}
}

void USideScrollingCameraComponent::SetZoomDistance_Implementation(float Value, float BlendTime)
{
	CameraZoomDistance = Value;
	ZoomDistanceBlendSpeed = 1 / BlendTime;
}

void USideScrollingCameraComponent::SetTraverseUpOrDown_Implementation(bool Value)
{
	TravelUpDown = Value;
}

void USideScrollingCameraComponent::SetTraverseRightOrLeft_Implementation(bool Value)
{
	TravelForwardBack = Value;
}

void USideScrollingCameraComponent::SetTraverseThreshold_Implementation(float Value)
{
	TraverseThreshold = Value;
}

void USideScrollingCameraComponent::SetLocationOffset_Implementation(FVector Value, float BlendTime)
{
	CameraLocationOffset = Value;
	LocationOffsetBlendSpeed = 1 / BlendTime;
}

void USideScrollingCameraComponent::SetRotationOffset_Implementation(FVector Value, float BlendTime)
{
	CameraRotationOffset = Value;
	RotationOffsetBlendSpeed = 1 / BlendTime;
}

void USideScrollingCameraComponent::SetNewLocation_Implementation(FVector Value, float BlendTime)
{
	GivenLocation = Value;
	LocationBlendSpeed = 1 / BlendTime;
}

void USideScrollingCameraComponent::SetNewRotation_Implementation(FVector Value, float BlendTime)
{
	GivenRotation = Value;
	RotationBlendSpeed = 1 / BlendTime;
}
