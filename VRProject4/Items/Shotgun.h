// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunBase.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class VRPROJECT4_API AShotgun : public AGunBase
{
	GENERATED_BODY()
	
public:
	AShotgun(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;

private:
	// Components
	UPROPERTY(VisibleAnywhere)
	class UVRSliderComponent* Slider;

	UPROPERTY(VisibleAnywhere)
	class UArrowComponent* EjectionArrow;

	// Defaults
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AActor> AmmoClass;

	// Gun Tools grip script reference
	UPROPERTY()
	class UGS_GunTools* GunTools;
};
