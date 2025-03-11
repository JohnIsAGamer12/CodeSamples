// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BasePlayer.h"
#include "JoeNandu.generated.h"


/**
 * 
 */
UCLASS()
class BLEEDINGEDGEBRAWL_API AJoeNandu : public ABasePlayer
{
	GENERATED_BODY()

	// Sets Default Properties to Joe Nandu
	AJoeNandu();

public:
	// Called to bind functionality to input to the Special Movement
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	// Special Movement (i.e. Dash & Sprint)
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Locomotion)
	void SpecialMovement();
};
