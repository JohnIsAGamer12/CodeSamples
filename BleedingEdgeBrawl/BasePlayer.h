// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Actor/BaseCharacter.h"
#include "InputActionValue.h"
#include "BasePlayer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJump);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDash);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScoreboardUpdate, int, Score);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttack, bool, In);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackReset, bool, In);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHit, float, Knockback);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitParry, float, Knockback, ACharacter*, OtherActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DeadActor);

UCLASS(config=Game)
class BLEEDINGEDGEBRAWL_API ABasePlayer : public ABaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABasePlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Attack Hitbox Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class UBoxComponent* LeftArmHitBox;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class UBoxComponent* RightArmHitBox;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class UBoxComponent* RightLegsHitBox;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class UBoxComponent* LeftLegsHitBox;

	// Defense Hitbox Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* BlockWindow;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* ParryWindow;

	// Player Attack Combo Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class UAttackComboComponent* AttackCombo;

	// Reference to the Camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	class AMainCamera* CameraRef;


	// Player State Bools
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bCanWeJump;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool bIsJumping;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	float SprintMultipler;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	bool IsSprinting;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool IsBlocking;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool IsKnockdown;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool IsRecovering;

	// Player Standard Attributes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float StandardDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	float StandardHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
	int KnockbackThreshold;

	// Pause Widget Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	TSubclassOf<class UPauseMenuWidget> PauseWidgetClass; // NEEDS TO BE SET IN BLUEPRINT
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	class UPauseMenuWidget* PauseRef; // DO NOT SET IN BLUEPRINT

	// Reference to the Player Controller
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Default)
	APlayerController* OwnerController;

	// Input Mappings
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputMappingContext* PlayerMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* ForwardMovementAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* RightMovementAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* JumpAction;	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* DashSprintAction;	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* QuickAttackAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* HeavyAttackAction;	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* SpecialAttackAction;		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* BlockParryAction;	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	class UInputAction* PauseAction;

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	class UCharacterAnimation* AnimationBP;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Getters for the Player State Bools
	UFUNCTION(BlueprintCallable)
	bool GetIsJumping() const;
	UFUNCTION(BlueprintCallable)
	bool GetIsSprinting() const;
	UFUNCTION(BlueprintCallable)
	bool GetIsBlocking() const;
	UFUNCTION(BlueprintCallable)
	bool GetIsKnockdown() const;	
	UFUNCTION(BlueprintCallable)
	bool GetIsRecovering() const;

	// Setters for Player State Bools
	UFUNCTION(BlueprintCallable)
	void SetIsKnockdown(bool In);

	// Setters for the Camera References
	UFUNCTION(BlueprintCallable)
	void SetCameraRef(class AMainCamera* Camera = nullptr);

	// Set the Camera reference
	UFUNCTION(BlueprintCallable)
	class AMainCamera* GetCameraRef() const;

	// Getters for The Owner Controller
	UFUNCTION(BlueprintCallable)
	APlayerController* GetPlayerController() const;

	// Event Dispatcher to call the jump function based which frame the jump animation is on
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Locomotion")
	FOnJump OnLaunch;

	// DEPRECATED
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Locomotion")
	FOnDash OnDash;

	// Events for which Hit Box To Activate
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Combat")
	FOnAttack OnPunchRight;	
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Combat")
	FOnAttack OnPunchLeft;
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Combat")
	FOnAttack OnKickRight;	
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Combat")
	FOnAttack OnKickLeft;	

	// Events for which Defense Hit Box To Activate
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Combat")
	FOnAttack OnBlock;		
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Combat")
	FOnAttack OnParry;	

	// Stopping Any Action Happening
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Combat")
	FOnAttack OnActionStopped;

	// A Success for when the Player has made a successful parry
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Combat")
	FOnHitParry OnParrySuccess;

	// Sends an event for when the player gets hit while blocking
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Combat")
	FOnHit OnHit;

	// Might be Deprecated?
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Combat")
	FOnAttackReset ResetActionHappening;

	// An Event for when it sends a message to add to the Scoreboard
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Score")
	FScoreboardUpdate OnComboHit;	
	UPROPERTY(EditAnywhere, BlueprintCallable, BlueprintAssignable, Category = "Score")
	FScoreboardUpdate OnFinishCombo;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Locomotion")
	void StopJump();
	void StopJump_Implementation();


	// Binding Event Functions
	UFUNCTION(BlueprintCallable)
	void HandleOnLand(const FHitResult& Hit);

	// Binds the Handle Block to the Block Window Box Component
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Collision")
	void HandleBlock(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void HandleBlock_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);	
	
	// Binds the Handle Parry to the Parry Window Box Component
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Collision")
	void HandleParry(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void HandleParry_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Toggle Colliders
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void ToggleRightArm(bool in);
	// Toggle Colliders
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void ToggleRightLeg(bool in);
	// Toggle Colliders
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void ToggleLeftArm(bool in);
	// Toggle Colliders
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void ToggleLeftLeg(bool in);
	// Toggle Colliders
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void ToggleBlockCollider(bool in);
	// Toggle Colliders
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void ToggleParryCollider(bool in);

	// Disables all Hotboxes
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void DisableAllHitboxes(ECollisionEnabled::Type In);

	// Binding and UnBinds the Quick Attack Handle Boxes to a Specific Box Component
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void BindQuickAttacksToAHitBox(class UBoxComponent* HitBox);	
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void UnBindQuickAttacksToAHitBox(class UBoxComponent* HitBox);	

	// Binding and UnBinds the Heavy Attacks Handles to a Specific Box Component
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void BindHeavyAttacksToAHitBox(class UBoxComponent* HitBox);	
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void UnBindHeavyAttacksToAHitBox(class UBoxComponent* HitBox);	

	// Binding and UnBinds the Special Attacks Handles to a Specific Box Component
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void BindSpecialAttacksToAHitBox(class UBoxComponent* HitBox);	
	UFUNCTION(BlueprintCallable, Category = "Collision")
	void UnBindSpecialAttacksToAHitBox(class UBoxComponent* HitBox);

	// Knockback Event Bindings
	UFUNCTION(BlueprintCallable)
	void Knockback();
	UFUNCTION(BlueprintCallable)
	void Recover();

	//Health Component Information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintCallable, BlueprintAssignable, Category = "Health")
	FOnDeath PlayerIsDead;

	// Function that sends an event to the 
	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void PlayerHadDied(AActor* DeadActor);
	// Native Event for the Player to send an Event to display the Win Game Widget
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "GameMode")
	void WinGame();
	void WinGame_Implementation();

	// State Check for when the Pause Menu is Open
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget")
	bool IsPauseOpen;
	
private:

	// Moves Player Forward 
	void MoveForward(const FInputActionValue& Value); 
	// Move Player Right Or Left
	void MoveRight(const FInputActionValue& Value);
	// Starts the Jump Animation
	void StartJump();
	// Engages in Blocking
	void StartBlock(); 
	// Disengage in Blocking
	void StopBlock();
	// Pauses the Game
	void PauseGame();

	// Disabling all Defense Hitboxes
	void DisableAllDefenseHitBoxes(ECollisionEnabled::Type In);
	// Binds all Hitbox Toggles
	void BindHitBoxToggles();

};
