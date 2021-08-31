// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "VRBPDatatypes.h"
#include "GameFramework/Actor.h"
#include "Misc/OptionalRepSkeletalMeshActor.h"
#include "VRProject4/VRHandSocketComponent.h"

#include "GraspingHand.generated.h"


UENUM(BlueprintType)
enum class ELaterality : uint8
{
	Right, Left
};


UENUM(BlueprintType)
enum class EHandAnimState : uint8
{
	Default, Custom, Dynamic
};

/**
 * 
 */
UCLASS()
class VRPROJECT4_API AGraspingHand : public AActor
{
	GENERATED_BODY()
public:
	AGraspingHand();
	virtual void Tick(const float DeltaTime) override;
	virtual void BeginPlay() override;

	// Mesh reference
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UInversePhysicsSkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class USphereComponent* PhysicsRoot;

	UPROPERTY(BlueprintReadWrite)
	class UCapsuleComponent* GrabComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UVREPhysicsConstraintComponent* PhysicsConstraint;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fingers")
	class UVREPhysicalAnimationComponent* PhysicalAnimation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCapsuleComponent* pinky_03;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fingers")
	class UCapsuleComponent* index_03;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fingers")
	class UCapsuleComponent* middle_03;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fingers")
	class UCapsuleComponent* ring_03;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fingers")
	class UCapsuleComponent* thumb_03;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fingers")
	class UCapsuleComponent* thumb_02;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fingers")
	class UCapsuleComponent* index_02;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fingers")
	class UCapsuleComponent* middle_02;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fingers")
	class UCapsuleComponent* ring_02;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Fingers")
	class UCapsuleComponent* pinky_02;

	UPROPERTY(BlueprintReadOnly)
	TArray<class UCapsuleComponent*> FingerCapsules;
	
	// Initialized variables within VRCharacterBase
	UPROPERTY(BlueprintReadWrite, Category="Character References")
	class AGraspingHand* OtherHand;

	UPROPERTY(BlueprintReadWrite, Category="Character References")
	class AVRCharacterBase* OwningCharacter;

	UPROPERTY(BlueprintReadWrite, Category="Character References")
	class UGripMotionControllerComponent* MotionController;

	UPROPERTY(BlueprintReadWrite, Category="Character References")
	class UChildActorComponent* ChildActorComponent;

	// Query grab by querying grabbable object and then calling different grab functions
	bool Grab();

	// Releases grab
	void ReleaseGrab();

	// Try to interact with grabbed item with InteractInterface, if there is any
	bool OnTriggerPressed();

	// Attaches hand to motion controller
	void AttachHandToMotionController();

	// Attaches hand to component
	void AttachHandToComponent(class USceneComponent* AttachComponent, const FTransform& HandAttachmentWorldTransform);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Configurations")
	ELaterality Laterality;
	
	// Animation variables
	UPROPERTY(EditAnywhere)
	class UAnimSequence* HandOverlapAnim;

	UPROPERTY(EditAnywhere)
	class UAnimSequence* FistAnim;
	
	UPROPERTY(BlueprintReadOnly, Category="Animation")
	FPoseSnapshot HandPose;

	UPROPERTY(BlueprintReadWrite, Category="Animation")
	EHandAnimState HandAnimState;

	UPROPERTY(BlueprintReadOnly, Category="Animation")
	bool bOverlappingGrippableObject = false;

	// Public variables
	UPROPERTY(BlueprintReadWrite)
	//class UPrimitiveComponent* GrabComponent;
	class UObject* GrabObject;

	UPROPERTY(BlueprintReadWrite)
	EGripTargetType GripType;

	// Grabbed item interface used for interactions
	class IInteractInterface* InteractInterface;

	// Gripped slot name
	FName GripSlotPrefix = NAME_None;

	UFUNCTION()
	void OnGrippedObject(const FBPActorGripInformation& GripInfo);

	UFUNCTION()
	void OnDroppedObject(const FBPActorGripInformation& GripInfo, const bool bSlotted);

	UFUNCTION()
	void GrabSphereBeginOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp,
		const int32 OtherBodyIndex, const bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, const FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void GrabSphereEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, const int32 OtherBodyIndex);

	// Calls a timeline that initiates finger curling in the FingerCurlComponent
	UFUNCTION(BlueprintImplementableEvent, Category="FingerCurlComponent")
	void InitiateFingerCurls(const bool bReversed = false, AActor* OverlapActor = nullptr);

protected:		
	UPROPERTY(EditDefaultsOnly, Category="Configurations")
	FName RootBone;

private:
	// Helper functions
	// Gets the relative transform from the component to hand in MotionController space
	FTransform GetSocketToHandRelativeTransform(const FTransform& SlotWorldTransform) const;
	
    // Gets grippable object in GrabSphere if there is one
	bool QueryGrabObject();
    
    // Tries to grab grabbable component by hand socket component if it has one
    bool TryPrimaryGrab();

	// If TryGrabBySocket fails and the primary grip is held, query secondary grip
	bool TrySecondaryGrab();

	// If Primary and Secondary grips fail, query component grip. Component grip grabs onto whatever component the HandSocketComponent is attached to
	bool TryComponentGrab();

	// Default grip when all other grip implementations are fail
	void DefaultGrab();

	// If query grab fails, try climbing
	bool TryClimbing();

	bool IsCompatibleGrip(const EGripLaterality GripLaterality) const
	{
		return GripLaterality == EGripLaterality::Ambidextrous || GripLaterality == EGripLaterality::LeftOnly && Laterality == ELaterality::Left ||
			GripLaterality == EGripLaterality::RightOnly && Laterality == ELaterality::Right;
	}
};
