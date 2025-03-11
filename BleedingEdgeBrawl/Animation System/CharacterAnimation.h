// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Actor/Utilities/AttackTypeContents.h"

#include "CharacterAnimation.generated.h"

/**
 *
 */
UCLASS()
class BLEEDINGEDGEBRAWL_API UCharacterAnimation : public UAnimInstance
{
	GENERATED_BODY()

protected:

	// Characters Refs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CharactersRef)
	class ABasePlayer* PlayerRef;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CharactersRef)
	class ABaseEnemy* EnemyRef;

	/* TRANSFORM FOR HURTBOX/HITBOX*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform EnemyTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	uint8 isFalling : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	FVector Velocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	uint8 IsJumping : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	uint8 IsSprinting : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	uint8 IsBlocking : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	uint8 IsKnockdown : 1;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	uint8 IsRecovering : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	float Speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	FName SlotNodeName;

	/* QUICK ATTACKS */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuickAttacks)
	UAnimSequence* CurrentQuickAttackAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuickAttacks)
	int QuickAttackIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuickAttacks)
	TArray<UAnimSequence*> QuickAttackAssets;

	/* HEAVY ATTACKS */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HeavyAttacks)
	UAnimSequence* CurrentHeavyAttackAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HeavyAttacks)
	int HeavyAttackIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HeavyAttacks)
	TArray<UAnimSequence*> HeavyAttackAssets;

	/* SPECIAL ATTACK */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpecialAttacks)
	UAnimSequence* SpecialAttackAsset;

	/* REACTIONS */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Reactions)
	TArray<UAnimSequence*> BlockReactAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Reactions)
	UAnimSequence* ParryReactAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Reactions)
	TArray<UAnimSequence*> HurtAssets;

	/* LOCOMOTION */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Locomotion)
	UAnimSequence* DashAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Locomotion)
	TArray<UAnimSequence*> KnockdownAssets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Locomotion)
	UAnimSequence* KnockdownAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Locomotion)
	TArray<UAnimSequence*> RecoveryAssets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Locomotion)
	UAnimSequence* RecoveryAsset;

	/* DEATH */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Deaths)
	TArray<UAnimSequence*> DeathAssets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Deaths)
	UAnimSequence* CurrentDeathAssets;

	/* TAKEDOWNS */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Takedowns)
	UAnimSequence* TakedownAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Takedowns)
	UAnimSequence* ExecutedAction;

	/* DEBUG ATTACKS */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = QuickAttacks)
	uint8 DebugQuickAttacks : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HeavyAttacks)
	uint8 DebugHeavyAttacks : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpecialAttacks)
	uint8 DebugSpecialAttacks : 1;

	/* DEBUG REACTIONS */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Reactions)
	uint8 DebugBlockHitReact : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Reactions)
	uint8 DebugParryReact : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Reactions)
	uint8 DebugHurt : 1;


	/* DEBUG LOCOMOTIONS */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Locomotion)
	uint8 DebugDash : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Locomotion)
	uint8 DebugKnockdown : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Locomotion)
	uint8 DebugRecovery : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Locomotion)
	uint8 Recovery : 1;

	/* DEBUG DEATHS */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Deaths)
	uint8 DebugDeath : 1;

public:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/* ATTACKS */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = QuickAttacks)
	void QuickAttackCombo(float NotUsed);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = QuickAttacks)
	void ResetQuickAttackCombo();
	void ResetQuickAttackCombo_Implementation();	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = QuickAttacks)
	void IncrementQuickAttackCombo();
	void IncrementQuickAttackCombo_Implementation();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = HeavyAttacks)
	void HeavyAttackCombo(float NotUsed);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = HeavyAttacks)
	void ResetHeavyAttackCombo();
	void ResetHeavyAttackCombo_Implementation();	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = HeavyAttacks)
	void IncrementHeavyAttackCombo();
	void IncrementHeavyAttackCombo_Implementation();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = HeavyAttacks)
	void IncrementACombo(enum EAttackType AttackType);
	void IncrementACombo_Implementation(enum EAttackType AttackType);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = SpecialAttacks)
	void SpecialAttack(float NotUsed);


	/* LOCOMOTIONS */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Locomotion)
	void Dash();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Locomotion)
	void Knockdown(float NotUsed);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Locomotion)
	void Recover(float Notused);
	/* DEATHS */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Deaths)
	void PlayDeath(float NotUsed);
	/* REACTIONS */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Reactions)
	void BlockHitReact(float NotUsed);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Reactions)
	void ParryHitReact(float Knockback, ACharacter* OtherActor);
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = Reactions)
	void PlayHurts(float NotUsed);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Debug)
	void PreviewWindow();
	void PreviewWindow_Implementation();
};
