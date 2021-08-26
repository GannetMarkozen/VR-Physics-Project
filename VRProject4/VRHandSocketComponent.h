// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grippables/HandSocketComponent.h"
#include "VRHandSocketComponent.generated.h"

/**
 * 
 */
UCLASS()
class VRPROJECT4_API UVRHandSocketComponent : public UHandSocketComponent
{
	GENERATED_BODY()
public:
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override { return false; }
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override { return false; }
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override { return false; }
};
