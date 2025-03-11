// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DazedSystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnApplyDaze, float, CurrentDazeRatio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStun, bool, In);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLEEDINGEDGEBRAWL_API UDazedSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDazedSystemComponent();

protected:
	// Maximum Daze Limit
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = Default)
	float DazeLimit;
	// Threshold before the Character loses balance
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = Default)
	float DazeThreshold;
	// a Delay before the owner starts to recover
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	float DelayBeforeRecovering;

	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Events to on Damaged 
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = Default)
	FOnApplyDaze OnDamaged;	
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = Default)
	// Events to on Stunned 
	FOnStun OnStunned;
	// Events to on Recovery 
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = Default)
	FOnApplyDaze OnRecovery;


	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Bind Daze Affect Application
	UFUNCTION(BlueprintCallable)
	void HandleDazeEffect(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	// Reset Recovery
	void CharacterRecovery();
private:

	// Set the Owner to the Component
	void SetOwnerComponent();

	// Reference to the Owner of this Component
	class ABaseEnemy* EnemyRef;

	// Private Variables
	float CurrentDaze;
	bool bIsRecovering;
};
