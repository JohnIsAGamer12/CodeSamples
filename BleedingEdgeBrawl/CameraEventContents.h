// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CameraEventContents.generated.h"

/**
 * An Enum for Which Float Variable we want to Change in the Camera
 */
UENUM(BlueprintType)
enum class ECameraTriggerUpdateFloatParameter : uint8
{
	ZoomDistance UMETA(DisplayName = "ZoomDistance "),
	TraverseThreshold UMETA(DisplayName = "TraverseThreshold") // DEPRECATED
};

/**
* A Struct that in an Event will update a Float Value in the Camera Component
*/
USTRUCT(BlueprintType)
struct BLEEDINGEDGEBRAWL_API FCameraEventUpdateFloat
{
	GENERATED_BODY()

	FCameraEventUpdateFloat()
	{
		Parameter = ECameraTriggerUpdateFloatParameter::ZoomDistance;
		Value = 0.f;
		BlendTime = 0.f;
	};
	const FCameraEventUpdateFloat(const ECameraTriggerUpdateFloatParameter _Param, const float _Value, const float _BlendTime)
	{
		Parameter = _Param;
		Value = _Value;
		BlendTime = _BlendTime;
	};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	ECameraTriggerUpdateFloatParameter Parameter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float Value;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BlendTime;
};

/**
 * An Enum for Which Bool Variable we want to Change in the Camera
 */
UENUM(BlueprintType)
enum class ECameraTriggerUpdateBoolParameter : uint8
{
	TraverseUpOrDown UMETA(DisplayName = "TraverseUpOrDown"),
	TraverseLeftOrRight UMETA(DisplayName = "TraverseLeftOrRight")
};

/**
* A Struct that in an Event will update a Bool Value in the Camera Component
*/
USTRUCT(BlueprintType)
struct BLEEDINGEDGEBRAWL_API FCameraEventUpdateBool
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	ECameraTriggerUpdateBoolParameter Parameter = ECameraTriggerUpdateBoolParameter::TraverseUpOrDown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	uint8 Value : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BlendTime = 0.f;
};

/**
 * An Enum for Which Vector Variable we want to Change in the Camera
 */
UENUM(BlueprintType)
enum class ECameraTriggerUpdateVectorParameter : uint8
{
	GivenLocation UMETA(DisplayName = "GivenLocation"),
	LocationOffset UMETA(DisplayName = "LocationOffset"),
	GivenRotation UMETA(DisplayName = "GivenRotation"),
	RotationOffset UMETA(DisplayName = "RotationOffset")
};


/**
* A Struct that in an Event will update a Vector Value in the Camera Component
*/
USTRUCT(BlueprintType)
struct BLEEDINGEDGEBRAWL_API FCameraEventUpdateVector
{
	GENERATED_BODY()

	FCameraEventUpdateVector() 
	{
		Parameter = ECameraTriggerUpdateVectorParameter::GivenLocation;
		Value = FVector(0.f, 0.f, 0.f);
		BlendTime = 0.f;
	};
	FCameraEventUpdateVector(const ECameraTriggerUpdateVectorParameter _Param, const FVector _Value, const float _BlendTime)
	{
		Parameter = _Param;
		Value = _Value;
		BlendTime = _BlendTime;
	};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	ECameraTriggerUpdateVectorParameter Parameter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	FVector Value;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BlendTime;
};
