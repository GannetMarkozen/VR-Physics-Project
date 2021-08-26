// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GraspingHand.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"

#include "FingerCurlComponent.generated.h"

UENUM(BlueprintType)
enum class EFinger : uint8
{
	Index_02, Index_03, Middle_02, Middle_03, Ring_02, Ring_03, Pinky_02, Pinky_03, Thumb_02, Thumb_03
};

USTRUCT(BlueprintType)
struct FFingerInfo
{
	GENERATED_BODY()
	
	float Value;
	
	bool Blocked;

	FFingerInfo()
	{
		Value = 0.f; Blocked = false;
	}

	FFingerInfo(const float Value, const bool Blocked)
	{
		this->Value = Value; this->Blocked = Blocked;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class VRPROJECT4_API UFingerCurlComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFingerCurlComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Hand reference
	UPROPERTY()
	class AGraspingHand* Hand;

	// Called from GraspingHand and gets timeline lerp value
	UFUNCTION(BlueprintCallable, Category="FingerCurlComponent")
	void HandleGrabFingerCurls(const float Value, AActor* OverlapActor = nullptr);

	UFUNCTION(BlueprintCallable, Category="FingerCurlComponent")
	void HandleOpenFingerCurls(const float Value, const TMap<EFinger, float> InitValues);

	UFUNCTION(BlueprintCallable, Category="FingerCurlComponent")
	void ResetFingersBlocked();
	
	// Finger enum and float curl value map
	UPROPERTY(BlueprintReadOnly, Category="FingerCurlComponent")
	TMap<EFinger, float> FingerCurlValues;

	TMap<EFinger, float> LastFingerCurlValues;
	TMap<EFinger, bool> FingersBlocked;
	
	UFUNCTION(BlueprintCallable, Category="FingerCurlComponent")
	class UCapsuleComponent* GetCollisionFromFinger(const EFinger Finger);
};
