// Fill out your copyright notice in the Description page of Project Settings.


#include "GraspingHand.h"
#include "VRProject4/DebugMessage.h"

#include "VRCharacterBase.h"

#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Misc/OptionalRepSkeletalMeshActor.h"
#include "Misc/VREPhysicalAnimationComponent.h"
#include "Misc/VREPhysicsConstraintComponent.h"

#include "Grippables/GrippableSkeletalMeshComponent.h"
#include "Grippables/HandSocketComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "VRProject4/VRHandSocketComponent.h"
#include "VRProject4/Items/InteractInterface.h"

AGraspingHand::AGraspingHand()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Mesh = CreateDefaultSubobject<UInversePhysicsSkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	PhysicsRoot = CreateDefaultSubobject<USphereComponent>(TEXT("PhysicsRoot"));
	PhysicsRoot->SetupAttachment(Mesh);

	PhysicsConstraint = CreateDefaultSubobject<UVREPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	PhysicsConstraint->SetupAttachment(Mesh);

	PhysicalAnimation = CreateDefaultSubobject<UVREPhysicalAnimationComponent>(TEXT("PhysicalAnimation"));

	index_02 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("index_02"));
	index_03 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("index_03"));
	middle_02 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("middle_02"));
	middle_03 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("middle_03"));
	ring_02 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ring_02"));
	ring_03 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ring_03"));
	pinky_02 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("pinky_02"));
	pinky_03 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("pinky_03"));
	thumb_02 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("thumb_02"));
	thumb_03 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("thumb_03"));

	FingerCapsules = {index_02, index_03, middle_02, middle_03, ring_02, ring_03, pinky_02, pinky_03, thumb_02, thumb_03};
	for(UCapsuleComponent* Capsule : FingerCapsules)
	{
		if(!Capsule) continue;
		Capsule->ShapeColor = FColor::Green;
		Capsule->SetCapsuleRadius(0.4);
		Capsule->SetCapsuleHalfHeight(1.4);
		if(Mesh)
		{
			const FName AttachName = FName(Capsule->GetName() + "_r");
			Capsule->SetupAttachment(Mesh, AttachName);
		}
	}
}

void AGraspingHand::BeginPlay()
{
	Super::BeginPlay();

	Mesh->OnComponentHit.AddDynamic(this, &AGraspingHand::OnMeshHit);

	// Init welded bone driver
	PhysicalAnimation->SetSkeletalMeshComponent(Mesh);
	PhysicalAnimation->SetupWeldedBoneDriver({RootBone});
}

void AGraspingHand::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGraspingHand::GrabSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, const int32 OtherBodyIndex, const bool bFromSweep, const FHitResult& SweepResult)
{
	// If the overlapped component implements the grip interface, set bOverlappingGrippableObject to true for use in animation
	/*
	if(OtherActor == this || OtherActor == GetOwner()) return;

	if(!GrabObject && HandAnimState != EHandAnimState::Custom && OtherComp->Implements<UVRGripInterface>() || OtherActor->Implements<UVRGripInterface>())
	{
		if(HandOverlapAnim)
		{
			UHandSocketComponent::GetAnimationSequenceAsPoseSnapShot(HandOverlapAnim, HandPose);
            HandAnimState = EHandAnimState::Custom;
		}
	}
	*/
}

void AGraspingHand::GrabSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, const int32 OtherBodyIndex)
{
	// If none of the overlapping components implement the grip interface, set bOverlappingGrippableObject to false for use in animation
	/*
	if(!GrabObject && HandAnimState == EHandAnimState::Custom)
	{
		TArray<UPrimitiveComponent*> OverlappedComponents;
		OverlappedComponent->GetOverlappingComponents(OverlappedComponents);
		bool bStillHasOverlappingGrippableObject = false;
		for(UPrimitiveComponent* Component : OverlappedComponents)
		{
			// If component is invalid or is a one of my components, continue
			if(!Component || Component->GetOwner() == this || Component->GetOwner() == OtherHand) continue;
			if(Component->Implements<UVRGripInterface>() || OtherActor->Implements<UVRGripInterface>())
			{
				bStillHasOverlappingGrippableObject = true;
				break;
			}
		}
		if(bStillHasOverlappingGrippableObject) HandAnimState = EHandAnimState::Custom;
	}*/
}

void AGraspingHand::OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, const FVector NormalImpulse, const FHitResult& Hit)
{
	
}

void AGraspingHand::OnGrippedObject(const FBPActorGripInformation& GripInfo)
{
	if(GripInfo.bIsSlotGrip && GripSlotPrefix == FName("Primary"))
	{
		InteractInterface = Cast<IInteractInterface>(GrabObject);
		const FString Message = InteractInterface ? " has interact interface" : " does not have interact interface";
		if(GrabObject) PRINT("" + GrabObject->GetName() + Message);
	}
}

void AGraspingHand::OnDroppedObject(const FBPActorGripInformation& GripInfo, const bool bSlotted)
{
	ReleaseGrab();
}


bool AGraspingHand::Grab()
{
	if(QueryGrabObject() && GrabObject)
	{
		if(TryPrimaryGrab()) return true;
		if(TrySecondaryGrab()) return true;
		DefaultGrab();
	}
	else
	{
		if(TryClimbing() == false)
		{
			UHandSocketComponent::GetAnimationSequenceAsPoseSnapShot(FistAnim, HandPose);
			HandAnimState = EHandAnimState::Custom;
		}
	}
	return true;
}

void AGraspingHand::ReleaseGrab()
{
	if(OwningCharacter->ClimbingHand || GrabObject) AttachHandToMotionController();

	// Set HandAnimState to Default if not Dynamic to open hands in Animation
	if(HandAnimState != EHandAnimState::Dynamic) HandAnimState = EHandAnimState::Default;
	InitiateFingerCurls(true);

	// If this hand is the ClimbingHand, set climbing to false
	if(OwningCharacter->ClimbingHand == this)
	{
		OwningCharacter->ClimbingHand = nullptr;
		OwningCharacter->VRMovementReference->SetClimbingMode(false);
	}

	if(OtherHand->GrabObject == GrabObject && OtherHand->GripSlotPrefix == FName("Primary"))
		OtherHand->MotionController->RemoveSecondaryAttachmentPoint(GrabObject);
	
	// If nothing is currently grabbed return
	if(GrabObject) MotionController->DropObjectByInterface(GrabObject);

	// Init variables
	GripSlotPrefix = NAME_None;
	GrabObject = nullptr; 
	InteractInterface = nullptr;
}

bool AGraspingHand::OnTriggerPressed()
{
	if(!InteractInterface) return false;
	InteractInterface->OnTriggerPressed();
	return true;
}

FTransform AGraspingHand::GetSocketToHandRelativeTransform(const FTransform& SlotWorldTransform) const
{
	if(!GrabObject) return FTransform();
	
	FTransform SocketToObjectTransform;
	FVector ObjectScale;
	if(GripType == EGripTargetType::ActorGrip)
	{
		if(AActor* Actor = Cast<AActor>(GrabObject))
		{
			SocketToObjectTransform = Actor->GetActorTransform().GetRelativeTransform(SlotWorldTransform);
			ObjectScale = Actor->GetActorScale();
		}
	}
	else if(USceneComponent* SceneComponent = Cast<USceneComponent>(GrabObject))
	{
		// Get transform relative to MotionController to snap to hand
		SocketToObjectTransform = SceneComponent->GetComponentTransform().GetRelativeTransform(SlotWorldTransform);
		ObjectScale = SceneComponent->GetComponentScale();
	}
	if(Laterality == ELaterality::Left) SocketToObjectTransform.Mirror(EAxis::Y, EAxis::Y);
				                    
	FTransform ObjectRelativeTransform = SocketToObjectTransform * ChildActorComponent->GetRelativeTransform();
	ObjectRelativeTransform.SetScale3D(ObjectScale);
	return ObjectRelativeTransform;
}

void AGraspingHand::AttachHandToMotionController()
{
	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PhysicalAnimation->RefreshWeldedBoneDriver();
	
	// Constrain physics root to hand then using the physics root as a frame, get the relative constraint to mesh
	PhysicsConstraint->SetConstrainedComponents(PhysicsRoot, NAME_None, Mesh, RootBone);
	PhysicsConstraint->SetConstraintReferenceFrame(EConstraintFrame::Frame1, PhysicsConstraint->GetRelativeTransform());
}

void AGraspingHand::AttachHandToComponent(USceneComponent* AttachComponent, const FTransform& HandAttachmentWorldTransform)
{
	PhysicsConstraint->BreakConstraint();
	
	Mesh->SetSimulatePhysics(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PhysicalAnimation->RefreshWeldedBoneDriver();
	
	const FTransform ScaledAttachWorldTransform = FTransform(HandAttachmentWorldTransform.GetRotation(), HandAttachmentWorldTransform.GetLocation(), Mesh->GetComponentScale());
	SetActorTransform(ScaledAttachWorldTransform, false, nullptr, ETeleportType::ResetPhysics);
	Mesh->K2_AttachToComponent(AttachComponent, NAME_None, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, true);
}

bool AGraspingHand::QueryGrabObject()
{
	if(GrabObject) return true;

	int32 HighestGripPriority = 0;
	int32 HighestGripPriorityIndex = -1;
	TArray<AActor*> Actors;
	if(GrabComponent) GrabComponent->GetOverlappingActors(Actors);
	for(int32 Index = 0; Index < Actors.Num(); Index++)
	{
		if(Actors[Index] == this || Actors[Index] == OwningCharacter) continue;
		if(Actors[Index]->Implements<UVRGripInterface>())
		{
			const int32 GripPriority = IVRGripInterface::Execute_AdvancedGripSettings(Actors[Index]).GripPriority;
			if(GripPriority > HighestGripPriority)
			{
				HighestGripPriority = GripPriority;
				GripType = EGripTargetType::ActorGrip;
				HighestGripPriorityIndex = Index;
			}
		}
	}

	TArray<UPrimitiveComponent*> Components;
	if(GrabComponent) GrabComponent->GetOverlappingComponents(Components);
	for(int32 Index = 0; Index < Components.Num(); Index++)
	{
		if(Components[Index]->GetOwner() == this || Components[Index]->GetOwner() == OwningCharacter) continue;
		if(Components[Index]->Implements<UVRGripInterface>())
		{
			const int32 GripPriority = IVRGripInterface::Execute_AdvancedGripSettings(Components[Index]).GripPriority;
			if(GripPriority > HighestGripPriority)
			{
				HighestGripPriority = GripPriority;
				GripType = EGripTargetType::ComponentGrip;
				HighestGripPriorityIndex = Index;
			}
		}
	}
	if(HighestGripPriorityIndex > -1)
	{
		if(GripType == EGripTargetType::ActorGrip)
		{
			GrabObject = Actors[HighestGripPriorityIndex];
		}
		else if(GripType == EGripTargetType::ComponentGrip)
		{
			GrabObject = Components[HighestGripPriorityIndex];
		}
	}
	if(GrabObject)
	{
		const FString Message = GripType == EGripTargetType::ActorGrip ? "Actor" : "Component";
		UE_LOG(LogTemp, Warning, TEXT("Grabbed type == %s"), *Message);
		return true;
	}
	
	return false;
}


bool AGraspingHand::TryPrimaryGrab()
{
	if(!GrabObject) return false;
	
	// Check if there is a HandSocketComponent in range with the SlotPrefix "Primary"
	bool bHadPrimarySlot; FTransform SlotWorldTransform; FName SlotName;
	IVRGripInterface::Execute_ClosestGripSlotInRange(GrabObject, MotionController->GetPivotLocation(), false, bHadPrimarySlot, SlotWorldTransform, SlotName, MotionController, FName("Primary"));
	if(bHadPrimarySlot)
	{
		// Get VRHandSocketComponent by SlotName
		if(UVRHandSocketComponent* HandSocket = Cast<UVRHandSocketComponent>(UHandSocketComponent::GetHandSocketComponentFromObject(GrabObject, SlotName)))
		{
			// If grip laterality is incompatible with hand laterality, return false
			if(IsCompatibleGrip(HandSocket->GetGripLaterality()) == false) return false;
			
			// If the other hand is holding the same object at the primary slot, swap hands
			if(OtherHand->GrabObject == GrabObject && OtherHand->GripSlotPrefix == FName("Primary"))
				OtherHand->ReleaseGrab();

			// Attach the hand to the component
			AttachHandToComponent(GripType == EGripTargetType::ActorGrip ? Cast<AActor>(GrabObject)->GetRootComponent() : Cast<USceneComponent>(GrabObject), SlotWorldTransform);
			
			GripSlotPrefix = HandSocket->SlotPrefix;
			
			FTransform GripOffset = HandSocket->GetGripOffset();
			if(Laterality == ELaterality::Left) GripOffset.Mirror(EAxis::Y, EAxis::Y);
			
			FTransform ObjectRelativeTransform = GetSocketToHandRelativeTransform(SlotWorldTransform);
			if(HandSocket->GetUseHandTargetTransform())
			{
				FTransform Offset = HandSocket->GetGripOffset();
				Offset.Mirror(EAxis::Y, EAxis::Y);
				ObjectRelativeTransform *= Offset;
			}
			
			// Grab object by interface and set custom pose if grab succeeds
			if(MotionController->GripObjectByInterface(GrabObject, ObjectRelativeTransform, true, NAME_None, SlotName, true))
			{
				if(HandSocket->GetBlendedPoseSnapShot(HandPose)) HandAnimState = EHandAnimState::Custom;

				UE_LOG(LogTemp, Warning, TEXT("Primary gripped on %s"), *GrabObject->GetName());
				return true;
			}
		}
	}

	return false;
}

bool AGraspingHand::TrySecondaryGrab()
{
	const ESecondaryGripType SecondaryGripType = IVRGripInterface::Execute_SecondaryGripType(GripType == EGripTargetType::ActorGrip ? GrabObject : Cast<UActorComponent>(GrabObject)->GetOwner());
	if(SecondaryGripType == ESecondaryGripType::SG_None)
	{
		UE_LOG(LogTemp, Error, TEXT("Secondary Grip Type == SG_None"));
		return false;
	}
	
	// If there is no primary socket in range, query whether the other hand is holding this object in the Primary slot and whether this object allows multiple grips
	if(OtherHand->GrabObject == GrabObject || OtherHand->GrabObject == Cast<UActorComponent>(GrabObject)->GetOwner() && OtherHand->GripSlotPrefix == FName("Primary"))
	{
		// Check if there is a HandSocketComponent in range with the SlotPrefix "Secondary"
		bool bHadSecondarySlot; FTransform SlotWorldTransform; FName SlotName;
		IVRGripInterface::Execute_ClosestGripSlotInRange(GrabObject, MotionController->GetPivotLocation(), true, bHadSecondarySlot, SlotWorldTransform, SlotName, MotionController, FName("Secondary"));
		if(bHadSecondarySlot)
		{
			// Get VRHandSocketComponent by SlotName
			if(UVRHandSocketComponent* HandSocket = Cast<UVRHandSocketComponent>(UHandSocketComponent::GetHandSocketComponentFromObject(GrabObject, SlotName)))
			{
				// If grip laterality is incompatible with hand laterality, return false
				if(IsCompatibleGrip(HandSocket->GetGripLaterality()) == false) return false;
				
				GripSlotPrefix = HandSocket->SlotPrefix;
				
				// Set hand pose as secondary grip on GrabObject, if secondary grip type is slot only or if adding secondary attach point succeeds
				if(SecondaryGripType == ESecondaryGripType::SG_SlotOnly ||
					OtherHand->MotionController->AddSecondaryAttachmentPoint(GrabObject, MotionController, FTransform(),true, 0.25f, true, SlotName))
				{
					AttachHandToComponent(GripType == EGripTargetType::ActorGrip ? Cast<AActor>(GrabObject)->GetRootComponent() : Cast<USceneComponent>(GrabObject), SlotWorldTransform);
					
					if(HandSocket->GetBlendedPoseSnapShot(HandPose)) HandAnimState = EHandAnimState::Custom;

					UE_LOG(LogTemp, Warning, TEXT("Secondary gripped on %s"), *GrabObject->GetName());
					return true;
				}
			} 
			else UE_LOG(LogTemp, Warning, TEXT("Failed to cast HandSocket from secondary slot"));
		}
		else UE_LOG(LogTemp, Warning, TEXT("No secondary slot in range"));
	}
	else if(OtherHand->GrabObject) UE_LOG(LogTemp, Warning, TEXT("Failed first if. OtherHand GrabObject == %s && OtherHand GripSlotPrefix == %s"), *OtherHand->GrabObject->GetName(), *OtherHand->GripSlotPrefix.ToString());
	
	return false;
}

bool AGraspingHand::TryComponentGrab()
{
	// Check if there is a HandSocketComponent in range with the SlotPrefix "Component"
	bool bHadSlot; FTransform SlotWorldTransform; FName SlotName;
	IVRGripInterface::Execute_ClosestGripSlotInRange(GrabObject, MotionController->GetPivotLocation(), false, bHadSlot, SlotWorldTransform, SlotName, MotionController, FName("Component"));
	if(bHadSlot)
	{
		// Get HandSocketComponent by SlotName
		if(UVRHandSocketComponent* HandSocket = Cast<UVRHandSocketComponent>(UHandSocketComponent::GetHandSocketComponentFromObject(GrabObject, SlotName)))
		{
			// If grip laterality is incompatible with hand laterality, return false
			if(IsCompatibleGrip(HandSocket->GetGripLaterality()) == false) return false;

			// Get the attach parent component to the HandSocket
			if(USceneComponent* AttachComponent = HandSocket->GetAttachParent())
			{
				// Grab AttachComponent by grip interface. If successful, continue
				if(MotionController->GripObjectByInterface(AttachComponent, GetSocketToHandRelativeTransform(SlotWorldTransform), true, NAME_None, SlotName, true))
				{
					AttachHandToComponent(AttachComponent, SlotWorldTransform);

					// Set hand animation to the hand socket pose
					if(HandSocket->GetBlendedPoseSnapShot(HandPose)) HandAnimState = EHandAnimState::Custom;

					return true;
				}
			}
		}
	}

	return false;
}

void AGraspingHand::DefaultGrab()
{
	if(!GrabObject) return;
	if(GripType == EGripTargetType::ActorGrip)
	{
		if(AActor* Actor = Cast<AActor>(GrabObject))
		{
			const FTransform RelativeTransform = Actor->GetTransform().GetRelativeTransform(GetTransform()) * ChildActorComponent->GetRelativeTransform();
			if(MotionController->GripObjectByInterface(Actor, RelativeTransform, true))
			{
				HandAnimState = EHandAnimState::Dynamic;
				AttachHandToComponent(Actor->GetRootComponent(), GetActorTransform());
				InitiateFingerCurls(false, Actor);
			}
		}
	}
	else if(USceneComponent* Component = Cast<USceneComponent>(GrabObject))
	{
		const FTransform RelativeTransform = Component->GetComponentTransform().GetRelativeTransform(GetTransform()) * ChildActorComponent->GetRelativeTransform();
		if(MotionController->GripObjectByInterface(Component, RelativeTransform, true))
		{
			HandAnimState = EHandAnimState::Dynamic;
			AttachHandToComponent(Component, GetActorTransform());
			InitiateFingerCurls(false, Component->GetOwner());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Default gripped on %s"), *GrabObject->GetName());
}

bool AGraspingHand::TryClimbing()
{
	TArray<AActor*> OverlappingActors;
	GrabComponent->GetOverlappingActors(OverlappingActors);
	for(AActor* Actor : OverlappingActors)
	{
		if(Actor->ActorHasTag(FName("Climbable")))
		{
			HandAnimState = EHandAnimState::Dynamic;
			AttachHandToComponent(Actor->GetRootComponent(), GetActorTransform());
			InitiateFingerCurls(false, Actor);
			
			OwningCharacter->VRMovementReference->SetClimbingMode(true);

			const bool bOtherHandIsClimbingHand = OwningCharacter->ClimbingHand == OtherHand;
			OwningCharacter->ClimbingHand = this;
			if(bOtherHandIsClimbingHand) OtherHand->ReleaseGrab();
			
			return true;
		}
	}
	return false;
}




