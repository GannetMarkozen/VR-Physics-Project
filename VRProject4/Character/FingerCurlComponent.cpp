// Fill out your copyright notice in the Description page of Project Settings.


#include "FingerCurlComponent.h"
#include "GraspingHand.h"
#include "GripMotionControllerComponent.h"
#include "Chaos/AABBTree.h"
#include "Components/CapsuleComponent.h"
#include "Components/TimelineComponent.h"
#include "Kismet/KismetMathLibrary.h"


UFingerCurlComponent::UFingerCurlComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UFingerCurlComponent::BeginPlay()
{
	Super::BeginPlay();

	Hand = Cast<AGraspingHand>(GetOwner());
	if(Hand)
	{
		for(EFinger Finger : {EFinger::Index_02, EFinger::Index_03, EFinger::Middle_02, EFinger::Middle_03, EFinger::Ring_02, EFinger::Ring_03, EFinger::Pinky_02, EFinger::Pinky_03, EFinger::Thumb_02, EFinger::Thumb_03})
		{
			// Init finger maps
			FingerCurlValues.Add(Finger, 0.f);
			FingersBlocked.Add(Finger, false);
			LastFingerCurlValues.Add(Finger, 0.f);
		}
	}
	else UE_LOG(LogTemp, Warning, TEXT("Hand uninitialized"));
}


void UFingerCurlComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

UCapsuleComponent* UFingerCurlComponent::GetCollisionFromFinger(const EFinger Finger)
{
	UCapsuleComponent* Capsule = nullptr;
	switch(Finger)
	{
	case EFinger::Index_02:
		Capsule = Hand->index_02;
		break;
	case EFinger::Index_03:
		Capsule = Hand->index_03;
		break;
	case EFinger::Middle_02:
		Capsule = Hand->middle_02;
		break;
	case EFinger::Middle_03:
		Capsule = Hand->middle_03;
		break;
	case EFinger::Pinky_02:
		Capsule = Hand->pinky_02;
		break;
	case EFinger::Pinky_03:
		Capsule = Hand->pinky_03;
		break;
	case EFinger::Ring_02:
		Capsule = Hand->ring_02;
		break;
	case EFinger::Ring_03:
		Capsule = Hand->ring_03;
		break;
	case EFinger::Thumb_02:
		Capsule = Hand->thumb_02;
		break;
	case EFinger::Thumb_03:
		Capsule = Hand->thumb_03;
		break;
	}
	return Capsule;
}


void UFingerCurlComponent::ResetFingersBlocked()
{
	// When uncurling fingers, reset all fingers to not blocked
	TArray<EFinger> Fingers;
	FingersBlocked.GetKeys(Fingers);
	for(const EFinger Finger : Fingers)
	{
		*FingersBlocked.Find(Finger) = false;
	}
}

// Handles curling fingers until the finger capsule detects an overlap against the given Actor
void UFingerCurlComponent::HandleGrabFingerCurls(const float Value, AActor* OverlapActor)
{
	if(!Hand) return;
	
	// Get all finger enums
	TArray<EFinger> Fingers;
	FingerCurlValues.GetKeys(Fingers);
	for(const EFinger Finger : Fingers)
	{
		// Finger curl Blocked reference
		bool& bBlocked = *FingersBlocked.Find(Finger);
		if(OverlapActor && bBlocked == false)
		{
			// If finger capsule is overlapping the OverlapActor, set blocked to true and stop assigning curl values
			if(OverlapActor && GetCollisionFromFinger(Finger)->IsOverlappingActor(OverlapActor))
			{
				bBlocked = true;
				*FingerCurlValues.Find(Finger) = *LastFingerCurlValues.Find(Finger);

				// If fingertip is blocked, set all base-fingers to blocked as well
				const FString FingerName = GetCollisionFromFinger(Finger)->GetName();
				if(FingerName[FingerName.Len() - 1] == '3')
				{
					const int32 Index = Fingers.IndexOfByKey(Finger) - 1;
					if(Fingers.IsValidIndex(Index))
					{
						*FingersBlocked.Find(Fingers[Index]) = true;
						*FingerCurlValues.Find(Fingers[Index]) = *LastFingerCurlValues.Find(Fingers[Index]);
					}
				}
			}
			// If finger isn't blocked, assign finger curl value to Value
			else
			{
				float& FingerValue = *FingerCurlValues.Find(Finger);
				*LastFingerCurlValues.Find(Finger) = FingerValue;
				FingerValue = Value;
			}
		}
	}
}

// Uncurls fingers from their original position
void UFingerCurlComponent::HandleOpenFingerCurls(const float Value, const TMap<EFinger, float> InitValues)
{
	TArray<EFinger> Fingers;
	InitValues.GetKeys(Fingers);
	for(const EFinger Finger : Fingers)
	{
		*FingerCurlValues.Find(Finger) = *InitValues.Find(Finger) * Value;
	}
}


