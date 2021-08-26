// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractInterface.h"
#include "Grippables/GrippableStaticMeshActor.h"

#include "GunBase.generated.h"

UCLASS()
class VRPROJECT4_API AGunBase : public AGrippableStaticMeshActor, public IInteractInterface
{
	GENERATED_BODY()
	
public:
	// Constructor
	AGunBase(const FObjectInitializer& ObjectInitializer);
	
	// Unused overrides
	bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override { return false; }
	bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override { return false; }
	bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override { return false; }

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(const float DeltaTime) override;

private:

};
