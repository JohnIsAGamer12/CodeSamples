// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/DazedSystemComponent.h"
#include "Actor/BaseEnemy.h"

// Sets default values for this component's properties
UDazedSystemComponent::UDazedSystemComponent() : bIsRecovering(true),
DazeLimit(100.f)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UDazedSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	SetOwnerComponent();

	if (EnemyRef)
		EnemyRef->OnTakeAnyDamage.AddDynamic(this, &UDazedSystemComponent::HandleDazeEffect);
}


// Called every frame
void UDazedSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (bIsRecovering)
	{
		// Overtime Restores The Daze back up to limit unless bIsRecovering is turned back off
		CurrentDaze = FMath::Clamp(CurrentDaze + DeltaTime * 10.f, 0.f, DazeLimit);
		OnRecovery.Broadcast(CurrentDaze / DazeLimit);
	}
}

void UDazedSystemComponent::HandleDazeEffect(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	CurrentDaze = FMath::Clamp(CurrentDaze + Damage, 0.f, DazeLimit);

	if (CurrentDaze < DazeLimit)
	{
		if (bIsRecovering) // When character Gets hit it stops their recovery to increase their Daze Meter
			bIsRecovering = false;


		if ((CurrentDaze / DazeLimit) > DazeThreshold)
		{
			OnDamaged.Broadcast(CurrentDaze / DazeLimit); // Plays Stagger
			if (DamagedActor)
				Cast<ACharacter>(DamagedActor)->LaunchCharacter(FVector(-100, 0.f, 0.f), true, false);
		}

		// SetTimer to Recover certain number of Daze
		FTimerHandle timer;
		GetWorld()->GetTimerManager().SetTimer(timer, this, &UDazedSystemComponent::CharacterRecovery, DelayBeforeRecovering);
	}
	else
	{
		// Stagger the Character if Daze meter is full
		OnStunned.Broadcast(true);
	}
}

void UDazedSystemComponent::CharacterRecovery()
{
	bIsRecovering = true;
	OnRecovery.Broadcast(0.f);
}

void UDazedSystemComponent::SetOwnerComponent()
{
	EnemyRef = Cast<ABaseEnemy>(GetOwner());
	check(EnemyRef != nullptr);
}

