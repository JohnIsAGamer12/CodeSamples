// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComboComponent.generated.h"

class UDazedSystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboAttack, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboContinue, EAttackType, type);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLEEDINGEDGEBRAWL_API UAttackComboComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttackComboComponent();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	int HitCounter;	

	// The Amount of Special Juice left
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	float CurrentSpecialMeter;	

	// Maximum Number for the Special Juice
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	float MaxSpecialMeter;	

	// Hpw much the Player loses in their Special Meter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	float SpecialMeterDeduction;

	// A Window for how long the player can continue the Combo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	float ContinueComboThreshold;

	// Reset Combo after this Certain amount of time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	float ResetComboThreshold;

	// An Action Is Happening
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	bool ActionIsHappening;

	// The Current type of Attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	EAttackType CurrentAttackType;

	// Reference to the Owner of this Component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	class ABasePlayer* PlayerRef;

	// TODO: Make Private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DamageValues)
	float BaseDamage;
	// TODO: Make Private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DamageValues)
	float HeavyDamageAdditive;
	// TODO: Make Private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DamageValues)
	float SpecialDamageAdditive;

public:	

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Event that Calls any Function bind to the Quick Attack State
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = Quick_Attacks)
	FOnComboAttack QuickAttack;
	// Event that Calls any Function bind to that continues the Quick Attack State
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = Quick_Attacks)
	FOnComboContinue ContinueQuickAttack;

	// Event that Calls any Function bind to the Heavy Attack State
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = Heavy_Attacks)
	FOnComboAttack HeavyAttack;
	// Event that Calls any Function bind to that continues the Heavy Attack State
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = Heavy_Attacks)
	FOnComboContinue ContinueHeavyAttack;

	// Event that Calls any Function bind to the Special Attack State
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = Special_Attacks)
	FOnComboAttack SpecialAttack;

	// handles quick attack functionality
	UFUNCTION(BlueprintCallable, Category = Collision)
	void HandleQuickAttackHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);	
	// handles heavy attack functionality
	UFUNCTION(BlueprintCallable, Category = Collision)
	void HandleHeavyAttackHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);	
	// handles special attack functionality
	UFUNCTION(BlueprintCallable,  Category = Collision)
	void HandleSpecialAttackHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Start the Quick Attack Function
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Attacks)
	void StartQuickAttack();
	void StartQuickAttack_Implementation();
	// Start the Heavy Attack Function
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Attacks)
	void StartHeavyAttack();
	void StartHeavyAttack_Implementation();	
	// Start the Special Attack Function
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Attacks)
	void StartSpecialAttack();
	void StartSpecialAttack_Implementation();

	// Increment the Hit Counter
	UFUNCTION(BlueprintCallable, Category = Attacks)
	void IncreaseHitCounter();
	// Reset the Hit Counter
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Attacks)
	void ResetHitCounter();
	void ResetHitCounter_Implementation();	
	// Update the Special Meter in the UI
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Attacks)
	void UpdateSpecialMeter(float NewValue);
	void UpdateSpecialMeter_Implementation(float NewValue);

	// Reset Combo Meter
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Combo)
	void ResetCombo();

	// Owner reacts to the Hit
	UFUNCTION(BlueprintCallable, Category = Defense)
	void BlockHitReact(float Knockback);	
	// Owner Successful Lands a Parry
	UFUNCTION(BlueprintCallable, Category = Defense)
	void SuccessfulParry(float Knockback, ACharacter* OtherActor);

	// Getter for the Hit Counter
	UFUNCTION(BlueprintCallable)
	int GetHitCounter() const;

	// reset the action has stopped
	UFUNCTION(BlueprintCallable)
	void ActionHasStopped(bool NotUsed);

	// Getter for Action Happening
	UFUNCTION(BlueprintCallable)
	bool GetActionHappening();

	// Setter for Action Happening
	UFUNCTION(BlueprintCallable)
	void SetActionHappening(bool in);

	// Getter for the Current Attack Type
	UFUNCTION(BlueprintCallable)
	EAttackType GetAttackType();

	// Set for the Current Attack Type
	UFUNCTION(BlueprintCallable)
	void SetAttackType(EAttackType NewAttackType);

private:
	// saves value for the Combo Score
	int SaveComboScore = 0;
};
