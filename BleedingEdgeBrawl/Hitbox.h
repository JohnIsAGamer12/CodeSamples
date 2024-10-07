// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hitbox.generated.h"

UENUM(BlueprintType)
enum class EHitboxEnum : uint8
{
	HB_PROXIMITY	UMETA(DisplayName = "Proximity"),
	HB_HITBOX		UMETA(DisplayName = "Hitbox"),
	HB_HURTBOX		UMETA(DisplayName = "Hurtbox"),
};

UCLASS()
class BLEEDINGEDGEBRAWL_API AHitbox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHitbox();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Tye of Hitbox
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EHitboxEnum HitboxType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector HitboxLocation;
	

};
