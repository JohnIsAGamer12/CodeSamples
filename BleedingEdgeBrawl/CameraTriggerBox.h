// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actor/Utilities/CameraEventContents.h"

#include "CameraTriggerBox.generated.h"

struct FCameraEventUpdateFloat;
struct FCameraEventUpdateBool;
struct FCameraEventUpdateVector;


UCLASS()
class BLEEDINGEDGEBRAWL_API ACameraTriggerBox : public AActor
{
	GENERATED_BODY()

public:	
	// Sets Default Values for Camera Trigger Box
	ACameraTriggerBox();

protected:

	// Trigger Box for the Player to Overlap and change the Camera Setting Values
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBoxComponent* TriggerBox;

	// Undo Camera Settings from it's original State
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	bool UndoAfterEndOverlap;

	// Set A Target Camera Actor in the Scene
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TObjectPtr<class APawn> TargetCamera;

	// A Blend Time for Camera to Interpolate between different points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float TargetCameraBlendTime;

	// Array of Float paramters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TArray<FCameraEventUpdateFloat> FloatParameters;
	// Array of Vector paramters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TArray<FCameraEventUpdateVector> VectorParameters;
	// Array of Bool paramters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TArray<FCameraEventUpdateBool> BoolParameters;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Update the Cameras Position/Settings
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Camera)
	void InitiateCameraUpdate(AActor* Actor, bool Undo);
	void InitiateCameraUpdate_Implementation(AActor* Actor, bool Undo);

private:
	// saves the Previous Camera we were using
	class AActor* PreviousCamera;
	// Saves Camera's Previous Settings
	TArray<FCameraEventUpdateFloat> PreviousFloatParameters;
	TArray<FCameraEventUpdateVector> PreviousVectorParameters;
	TArray<FCameraEventUpdateBool> PreviousBoolParameters;
	
	// Handles the Box Begin Overlap Event
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// Handles the Box End Overlap Event
	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
