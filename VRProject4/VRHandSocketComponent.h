// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grippables/HandSocketComponent.h"
#include "VRHandSocketComponent.generated.h"

UENUM(BlueprintType)
enum class EGripLaterality : uint8
{
	Ambidextrous, LeftOnly, RightOnly
};

/**
 * 
 */
UCLASS()
class VRPROJECT4_API UVRHandSocketComponent : public UHandSocketComponent
{
	GENERATED_BODY()
private:
	UPROPERTY(EditDefaultsOnly, Category="Configurations")
	EGripLaterality GripLaterality = EGripLaterality::Ambidextrous;

	UPROPERTY(EditDefaultsOnly, Category="Configurations")
	FTransform GripOffset;
	
	UPROPERTY(EditDefaultsOnly, Category="Configurations")
	bool bUseHandTargetTransform = false;
	
public:
	UFUNCTION(BlueprintCallable)
	EGripLaterality GetGripLaterality() const { return GripLaterality; }

	UFUNCTION(BlueprintCallable)
	FTransform GetGripOffset() const { return GripOffset; }
	
	UFUNCTION(BlueprintCallable)
	bool GetUseHandTargetTransform() const { return bUseHandTargetTransform; }
	
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override { return false; }
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override { return false; }
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override { return false; }
};
