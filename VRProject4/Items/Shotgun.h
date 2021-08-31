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

	virtual void OnTriggerPressed() override;
	virtual void OnTriggerReleased() override;

	UFUNCTION(BlueprintCallable, Category="Shotgun")
	bool TryFire();

	UFUNCTION(BlueprintCallable, Category="Shotgun")
	void EjectShell();

	UPROPERTY(EditAnywhere, Category="Shotgun")
	int32 Ammo = 6;
	
private:
	// Components
	UPROPERTY(VisibleAnywhere)
	class UVRSliderComponent* Slider;

	UPROPERTY(VisibleAnywhere)
	class UArrowComponent* FiringArrow;

	UPROPERTY(VisibleAnywhere)
	class UArrowComponent* EjectionArrow;

	// Defaults
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AActor> AmmoClass;

	// Gun Tools grip script reference
	UPROPERTY()
	class UGS_GunTools* GunTools;

	UPROPERTY(EditDefaultsOnly, Category="Configurations")
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, Category="Configurations")
	class UParticleSystem* HitParticle;

	UPROPERTY(EditDefaultsOnly, Category="Configurations")
	class USoundBase* HitSound;

	UPROPERTY(EditDefaultsOnly, Category="Configurations")
	class USoundBase* FiringSound;

	UPROPERTY(EditDefaultsOnly, Category="Configurations")
	class USoundBase* PrimingSound;
	
	UPROPERTY(EditAnywhere, Category="Configurations")
	float EjectionSpeed = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Configurations")
	float Range = 3000.f;

	UPROPERTY(EditDefaultsOnly, Category="Configurations")
	float Spread = 15.f;

	UFUNCTION()
	void OnActorGripped(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation);
	
	UFUNCTION()
	void OnSliderHitPoint(const float SliderProgressPoint);

	// State
	bool bPrimed = true;

	UPROPERTY()
	class UVRHandSocketComponent* SecondaryHandSocket;
};
