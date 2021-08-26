// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GraspingHand.h"
#include "Components/CapsuleComponent.h"

#include "LeftGraspingHand.generated.h"

/**
 * 
 */
UCLASS()
class VRPROJECT4_API ALeftGraspingHand : public AGraspingHand
{
	GENERATED_BODY()

	ALeftGraspingHand()
	{
		PrimaryActorTick.bCanEverTick = false;

		Laterality = ELaterality::Left;
		RootBone = FName("hand_l");
		
		
		for(UCapsuleComponent* Capsule : FingerCapsules)
		{
			if(!Capsule) continue;
			Capsule->ShapeColor = FColor::Green;
			Capsule->SetCapsuleRadius(0.4);
			Capsule->SetCapsuleHalfHeight(1.4);
			if(Mesh)
			{
				const FName AttachName = FName(Capsule->GetName() + "_l");
				Capsule->K2_AttachToComponent(Mesh, AttachName, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true);
			}
		}
	}
	/*
	virtual void OnConstruction(const FTransform& Transform) override
	{
		GrabSphere->K2_AttachToComponent(Mesh, FName("palm_l"), EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true);
		
		for(UCapsuleComponent* Capsule : FingerCapsules)
		{
			if(!Capsule) continue;
			Capsule->ShapeColor = FColor::Green;
			Capsule->SetCapsuleRadius(0.4);
			Capsule->SetCapsuleHalfHeight(1.4);
			if(Mesh)
			{
				const FName AttachName = FName(Capsule->GetName() + "_l");
				Capsule->K2_AttachToComponent(Mesh, AttachName, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true);
			}
		}
	}*/
};
