// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VRCharacter.h"

#include "VRCharacterBase.generated.h"

UENUM(BlueprintType)
enum class ETurnMode : uint8
{
	Constant, Snap, Mouse
};

UCLASS()
class VRPROJECT4_API AVRCharacterBase : public AVRCharacter
{
	GENERATED_BODY()

public:
	AVRCharacterBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Public Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCapsuleComponent* RGrabComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCapsuleComponent* LGrabComponent;

	// Hand references from child actor components, setup in BeginPlay
	UPROPERTY()
	class AGraspingHand* RHand;

	UPROPERTY()
	class AGraspingHand* LHand;

	UPROPERTY()
	class AGraspingHand* ClimbingHand = nullptr;

private:
	// Bindings
	void MoveForward(const float Value);
	void MoveRight(const float Value);
	void LookUp(const float Value);
	void LookRight(const float Value);

	void RightTriggerPressed();
	void RightTriggerReleased();
	void LeftTriggerPressed();
	void LeftTriggerReleased();

	void RightGripPressed();
	void LeftGripPressed();
	void RightGripReleased();
	void LeftGripReleased();
	
	UPROPERTY(VisibleAnywhere)
	class UChildActorComponent* RightHandComponent;

	UPROPERTY(VisibleAnywhere)
	class UChildActorComponent* LeftHandComponent;

	// Configurations
	UPROPERTY(EditAnywhere, Category="Configurations")
	float TurnRate = 10.f;

	UPROPERTY(EditAnywhere, Category="Configurations")
	float MouseSensitivity = 1.f;

	UPROPERTY(EditAnywhere, Category="Configurations")
	ETurnMode TurnMode = ETurnMode::Constant;

	bool bCanSnap = true;
};
