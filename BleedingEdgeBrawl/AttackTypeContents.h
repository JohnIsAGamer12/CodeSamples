// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *  An Enum Type where we check what kind of attack the Player has inputted
 */
UENUM(BlueprintType)
enum class EAttackType : uint8
{
	QuickAttack UMETA(DisplayName = "Quick Attack"),
	HeavyAttack UMETA(DisplayName = "Heavy Attack"),
	SpecialAttack UMETA(DisplayName = "Special Attack")
};