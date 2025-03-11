// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Utilities/AttackComboComponent.h"
#include "Actor/BasePlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/CharacterAnimation.h"
#include "Actor/Utilities/AttackTypeContents.h"
#include "Component/DazedSystemComponent.h"
#include "Engine/DamageEvents.h"
#include "../BleedingEdgeBrawl.h"


// Sets default values for this component's properties
UAttackComboComponent::UAttackComboComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttackComboComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef = GetOwner<ABasePlayer>();
	if (!PlayerRef)
		UE_LOG(LogTemp, Error, TEXT("PlayerRef is Null!"));

	CurrentSpecialMeter = MaxSpecialMeter;
}

void UAttackComboComponent::HandleQuickAttackHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == PlayerRef)
		return;


	TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
	FDamageEvent DamageEvent(ValidDamageTypeClass);

	OtherActor->TakeDamage(BaseDamage, DamageEvent, PlayerRef->GetPlayerController(), PlayerRef);
	ContinueQuickAttack.Broadcast(EAttackType::QuickAttack);
	
	PlayerRef->OnComboHit.Broadcast(1.f);
	SaveComboScore += 1;

	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, PlayerRef->AnimationBP, &UCharacterAnimation::ResetQuickAttackCombo, ContinueComboThreshold);
}

void UAttackComboComponent::HandleHeavyAttackHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == PlayerRef)
		return;

	TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
	FDamageEvent DamageEvent(ValidDamageTypeClass);

	OtherActor->TakeDamage(BaseDamage + HeavyDamageAdditive, DamageEvent, PlayerRef->GetPlayerController(), PlayerRef);
	ContinueHeavyAttack.Broadcast(EAttackType::HeavyAttack);
	
	PlayerRef->OnComboHit.Broadcast(1.f);
	SaveComboScore += 1;

	FTimerHandle TimerContinueCombo;
	FTimerHandle TimerResetCombo;
	GetWorld()->GetTimerManager().SetTimer(TimerContinueCombo, PlayerRef->AnimationBP, &UCharacterAnimation::ResetHeavyAttackCombo, ContinueComboThreshold);
	GetWorld()->GetTimerManager().SetTimer(TimerResetCombo, this, &UAttackComboComponent::ResetCombo, ResetComboThreshold);
}

void UAttackComboComponent::HandleSpecialAttackHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == PlayerRef)
		return;

	TSubclassOf<UDamageType> const ValidDamageTypeClass = TSubclassOf<UDamageType>(UDamageType::StaticClass());
	FDamageEvent DamageEvent(ValidDamageTypeClass);

	PlayerRef->OnComboHit.Broadcast(1.f);
	SaveComboScore += 1;

	OtherActor->TakeDamage(BaseDamage + SpecialDamageAdditive, DamageEvent, PlayerRef->GetPlayerController(), PlayerRef);
}

void UAttackComboComponent::StartQuickAttack_Implementation()
{
	if (!ActionIsHappening)
	{
		SetAttackType(EAttackType::QuickAttack);
		QuickAttack.Broadcast(0.f);
		ActionIsHappening = true;
	}
}

void UAttackComboComponent::StartHeavyAttack_Implementation()
{
	if (!ActionIsHappening)
	{
		SetAttackType(EAttackType::HeavyAttack);
		HeavyAttack.Broadcast(0.f);
		ActionIsHappening = true;
	}
}

void UAttackComboComponent::StartSpecialAttack_Implementation()
{
	if (!ActionIsHappening)
	{
		SetAttackType(EAttackType::SpecialAttack);
		UpdateSpecialMeter(SpecialMeterDeduction);
		SpecialAttack.Broadcast(CurrentSpecialMeter / MaxSpecialMeter);
		ActionIsHappening = true;
	}
}

void UAttackComboComponent::IncreaseHitCounter()
{
	HitCounter++;
}

void UAttackComboComponent::ResetHitCounter_Implementation()
{
	HitCounter = 0;
}

void UAttackComboComponent::UpdateSpecialMeter_Implementation(float NewValue)
{
	CurrentSpecialMeter -= NewValue;
}

void UAttackComboComponent::ResetCombo_Implementation()
{
	PlayerRef->OnComboHit.Broadcast(0.f);
	PlayerRef->OnFinishCombo.Broadcast(SaveComboScore);
}

void UAttackComboComponent::BlockHitReact(float Knockback)
{
	PlayerRef->LaunchCharacter(FVector(-Knockback * 100.f, 0.f, 0.f), true, false);
}

// On a Successfully Parry Aganist a Foe, The Player wil Inflict a Huge Points into their Dazed "Meter" or "Status"
void UAttackComboComponent::SuccessfulParry(float Knockback, ACharacter* OtherActor)
{
	if (OtherActor)
	{
		// Find their Dazed Component
		// Inflict Daze on to them
		OtherActor->LaunchCharacter(FVector(-Knockback * 100.f, 0.f, 0.f), true, false);
	}
}

int UAttackComboComponent::GetHitCounter() const
{
	return HitCounter;
}

void UAttackComboComponent::ActionHasStopped(bool NotUsed)
{
	ActionIsHappening = false;
}

bool UAttackComboComponent::GetActionHappening()
{
	return ActionIsHappening;
}

void UAttackComboComponent::SetActionHappening(bool in)
{
	ActionIsHappening = in;
}

EAttackType UAttackComboComponent::GetAttackType()
{
	return CurrentAttackType;
}

void UAttackComboComponent::SetAttackType(EAttackType NewAttackType)
{
	CurrentAttackType = NewAttackType;
}


