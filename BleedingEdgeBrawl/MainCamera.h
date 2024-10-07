// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Actor/Utilities/CameraEventContents.h"

#include "MainCamera.generated.h"

struct FCameraEventUpdateFloat;
struct FCameraEventUpdateBool;
struct FCameraEventUpdateVector;

UCLASS()
class BLEEDINGEDGEBRAWL_API AMainCamera : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMainCamera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Attack the Side Scroller Camera Component
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess), Category = Camera)
	class USideScrollingCameraComponent* Camera;

	// Set the Player Target // TODO: Make this Private
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	class ABasePlayer* PlayerTarget;

	// Sets what the First Camera the Player should see on Start
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	bool PriorityCamera;

	// Reference to the BasePlayer Class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	TSubclassOf<ABasePlayer> PlayerClass;

	// A Starting rotation Offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	struct FCameraEventUpdateVector TStartingRotationOffset;	
	// A Starting location Offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	struct FCameraEventUpdateVector TStartingPositionOffset;

private:
	// keep this Private: Sets the Starting Position on Begin Player
	struct FCameraEventUpdateVector TStartingPosition;	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Sets the Player Target and set the View Point to the First/Priority Camera
	UFUNCTION()
	void CachePlayer();
};
