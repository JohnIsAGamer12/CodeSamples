// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Actor/Utilities/CameraEventContents.h"

#include "SideScrollingCameraComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class BLEEDINGEDGEBRAWL_API USideScrollingCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
protected:

	// Sets How close the Camera is the Player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float CameraZoomDistance;

	// Sets the speed of how fast the Blend interpolation is between two settings or cameras
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float CameraBlendSpeed;

	// A Location offset from where the camera original was 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	FVector CameraLocationOffset;		
	// A Rotation offset from where the camera original was 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	FVector CameraRotationOffset;	

	// A Traverse threshold for when we want the Player to start moving
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraTraversal, meta = (AllowPrivateAccess = "true"))
	double TraverseThreshold;	
	// For how much the Player rotate up or down
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraTraversal, meta = (AllowPrivateAccess = "true"))
	double MaxRotateUpDownThreshold;
	// If The Camera can travel forward and back
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraTraversal, meta = (AllowPrivateAccess = "true"))
	uint8 TravelForwardBack : 1;	
	// If The Camera can travel up and down
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CameraTraversal, meta = (AllowPrivateAccess = "true"))
	uint8 TravelUpDown : 1;	

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this Camera's properties
	USideScrollingCameraComponent();

	// DEPRECATED
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Update All of the Camera's Properties
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Camera)
	void UpdateCamera(FVector StartPoint, FVector EndPoint, float DeltaTime);
	void UpdateCamera_Implementation(FVector StartPoint, FVector EndPoint, float DeltaTime);
private:
	class ACharacter* CameraOwner;

	float CurrentZoomDistance;
	float ZoomDistanceBlendSpeed;
	float LocationOffsetBlendSpeed;
	float LocationBlendSpeed;
	float RotationOffsetBlendSpeed;
	float RotationBlendSpeed;

	FVector CurrentLocation;
	FVector CurrentLocationOffset;
	FVector GivenLocation;
	FVector CurrentRotation;
	FVector CurrentRotationOffset;
	FVector GivenRotation;

	// Keep These Private
	FVector CameraLocation;		
	FVector CameraRotation;

	float ZOffSet;

public:

	// Set The Owner of the Camera
	UFUNCTION(BlueprintCallable)
	void SetCameraOwner(ACharacter* Player);

	// Get the Current Zoom Distance
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FORCEINLINE float GetZoomDistance() const { return CameraZoomDistance; }

	// Get the Current Given Location
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FORCEINLINE FVector GetGivenLocation() const { return GivenLocation; }	
	// Get the Current Given Rotation
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FORCEINLINE FVector GetGivenRotation() const { return GivenRotation; }

	// Set Zoom
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera")
	void SetZoomDistance(float Value, float BlendTime);
	void SetZoomDistance_Implementation(float Value, float BlendTime);		
	// Set if Traverse Up Or Down
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera")
	void SetTraverseUpOrDown(bool Value);
	void SetTraverseUpOrDown_Implementation(bool Value);		
	// Set if Traverse Right Or Left
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera")
	void SetTraverseRightOrLeft(bool Value);
	void SetTraverseRightOrLeft_Implementation(bool Value);	
	
	// Set Traverse Threshold
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera")
	void SetTraverseThreshold(float Value);
	void SetTraverseThreshold_Implementation(float Value);

	// Set Location Offset
	// Set Location Offset
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera")
	void SetLocationOffset(FVector Value, float BlendTime);
	void SetLocationOffset_Implementation(FVector Value, float BlendTime);	
	// Set Rotation Offset
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera")
	void SetRotationOffset(FVector Value, float BlendTime);
	void SetRotationOffset_Implementation(FVector Value, float BlendTime);	
	
	// Set Given Location
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera")
	void SetNewLocation(FVector Value, float BlendTime);
	void SetNewLocation_Implementation(FVector Value, float BlendTime);	
	// Set Given rotation
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Camera")
	void SetNewRotation(FVector Value, float BlendTime);
	void SetNewRotation_Implementation(FVector Value, float BlendTime);

	// Get the Current Default BlendSpeed
	FORCEINLINE float GetDefaultCameraBlendSpeed() const { return CameraBlendSpeed; }
	
	// Get the Update Float Parameters
	float UpdateOrGetFloatParameter(ECameraTriggerUpdateFloatParameter Parameter, FCameraEventUpdateFloat* Update = nullptr);

	// Get the Update Bool Parameters
	bool UpdateOrGetBoolParameter(ECameraTriggerUpdateBoolParameter Parameter, FCameraEventUpdateBool* Update = nullptr);

	// Get the Update Vector Parameters
	FVector UpdateOrGetVectorParameter(ECameraTriggerUpdateVectorParameter Parameter, FCameraEventUpdateVector* Update = nullptr);
};
