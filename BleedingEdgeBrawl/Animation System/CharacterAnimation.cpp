// Copyright Epic Games, Inc. All Rights Reserved.


#include "Animation/CharacterAnimation.h"
#include "Actor/BasePlayer.h"
#include "Actor/BaseEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/DazedSystemComponent.h"
#include "../BleedingEdgeBrawl.h"

void UCharacterAnimation::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerRef = Cast<ABasePlayer>(TryGetPawnOwner());
	if (!PlayerRef)
	{
		EnemyRef = Cast<ABaseEnemy>(TryGetPawnOwner());
		if (!EnemyRef)
		{
			// Both Charcters are Null!
			UE_LOG(Game, Error, TEXT("PlayerRef & EnemyRef is NULL!!!"));
		}
	}
}

void UCharacterAnimation::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (TryGetPawnOwner())
	{
		Velocity = TryGetPawnOwner()->GetVelocity();
		Speed = Velocity.Length();
		EnemyTransform = TryGetPawnOwner()->GetActorTransform();
		if (PlayerRef)
		{
			isFalling = PlayerRef->GetCharacterMovement()->IsFalling();
			IsJumping = PlayerRef->GetIsJumping();
			IsSprinting = PlayerRef->GetIsSprinting();
			IsBlocking = PlayerRef->GetIsBlocking();
			IsKnockdown = PlayerRef->GetIsKnockdown();
			IsRecovering = PlayerRef->GetIsRecovering();
		}
	}
	else
		PreviewWindow();
}

void UCharacterAnimation::PreviewWindow_Implementation()
{
	if (DebugQuickAttacks)
	{
		QuickAttackCombo(0.f);
		DebugQuickAttacks = false;
	}
	else if (DebugHeavyAttacks)
	{
		HeavyAttackCombo(0.f);
		DebugHeavyAttacks = false;
	}
	else if (DebugSpecialAttacks)
	{
		SpecialAttack(0.f);
		DebugSpecialAttacks = false;
	}
	else if (DebugDash)
	{
		Dash();
		DebugDash = false;
	}
	else if (DebugBlockHitReact)
	{
		BlockHitReact(0.f);
		DebugBlockHitReact = false;
	}
	else if (DebugParryReact)
	{
		ParryHitReact(0.f, nullptr);
		DebugParryReact = false;
	}
	else if (DebugDeath)
	{
		PlayDeath(0.f);
		DebugDeath = false;
	}
	else if (DebugHurt)
	{
		PlayHurts(0.f);
		DebugHurt = false;
	}
	else if (DebugKnockdown)
	{
		Knockdown(0.f);
		DebugKnockdown = false;
	}
	else if (DebugRecovery)
	{
		Recover(0.f);
		DebugRecovery = false;
	}
}

void UCharacterAnimation::ResetQuickAttackCombo_Implementation()
{
	if (PlayerRef)
		PlayerRef->OnActionStopped.Broadcast(false);

	QuickAttackIndex = 0;
}

void UCharacterAnimation::IncrementQuickAttackCombo_Implementation()
{
	QuickAttackIndex++;
	QuickAttackIndex = FMath::Clamp(QuickAttackIndex, 0, QuickAttackAssets.Num());
}

void UCharacterAnimation::ResetHeavyAttackCombo_Implementation()
{
	if (PlayerRef)
		PlayerRef->OnActionStopped.Broadcast(false);
	HeavyAttackIndex = 0;
}

void UCharacterAnimation::IncrementHeavyAttackCombo_Implementation()
{
	HeavyAttackIndex++;
	HeavyAttackIndex = FMath::Clamp(HeavyAttackIndex, 0, HeavyAttackAssets.Num());
}

void UCharacterAnimation::IncrementACombo_Implementation(enum EAttackType AttackType)
{
	switch (AttackType)
	{
	case EAttackType::QuickAttack:
		IncrementQuickAttackCombo();
		break;
	case EAttackType::HeavyAttack:
		IncrementHeavyAttackCombo();
		break;
	}
}
