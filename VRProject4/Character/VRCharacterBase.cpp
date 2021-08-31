// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacterBase.h"
#include "VRProject4/DebugMessage.h"

#include "GraspingHand.h"
#include "Grippables/GrippableStaticMeshComponent.h"
#include "Grippables/HandSocketComponent.h"
#include "Components/SphereComponent.h"
#include "Grippables/GrippableSkeletalMeshComponent.h"
#include "Misc/OptionalRepSkeletalMeshActor.h"
#include "Misc/VREPhysicsConstraintComponent.h"

// Sets default values
AVRCharacterBase::AVRCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	RGrabComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RGrabComponent"));
	RGrabComponent->ShapeColor = FColor::Red;
	RGrabComponent->SetupAttachment(RightMotionController);

	LGrabComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("LGrabComponent"));
	LGrabComponent->ShapeColor = FColor::Red;
	LGrabComponent->SetupAttachment(LeftMotionController);

	RightHandComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("RightHandChildActorComponent"));
	RightHandComponent->SetupAttachment(RightMotionController);

	LeftHandComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("LeftHandChildActorComponent"));
	LeftHandComponent->SetupAttachment(LeftMotionController);
}

void AVRCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Get references to the VRHands from the child actor components
	RHand = Cast<AGraspingHand>(RightHandComponent->GetChildActor());
	LHand = Cast<AGraspingHand>(LeftHandComponent->GetChildActor());
	
	if(RHand && LHand)
	{
		for(AGraspingHand* Hand : {RHand, LHand})
		{
			const bool bRightHand = Hand->Laterality == ELaterality::Right;

			// Init variables in GraspingHands
			Hand->OtherHand = bRightHand ? LHand : RHand;
			Hand->MotionController = bRightHand ? RightMotionController : LeftMotionController;
			Hand->GrabComponent = bRightHand ? RGrabComponent : LGrabComponent;
			Hand->ChildActorComponent = bRightHand ? RightHandComponent : LeftHandComponent;
			Hand->OwningCharacter = this;

			// Setup dynamic delegates
			Hand->MotionController->OnGrippedObject.AddDynamic(Hand, &AGraspingHand::OnGrippedObject);
			Hand->MotionController->OnDroppedObject.AddDynamic(Hand, &AGraspingHand::OnDroppedObject);
			Hand->GrabComponent->OnComponentBeginOverlap.AddDynamic(Hand, &AGraspingHand::GrabSphereBeginOverlap);
			Hand->GrabComponent->OnComponentEndOverlap.AddDynamic(Hand, &AGraspingHand::GrabSphereEndOverlap);

			
			// Attach the GraspingHand to the controller PhysicsConstraint
			Hand->Mesh->SetSimulatePhysics(true);
			Hand->Mesh->SetCenterOfMass(Hand->PhysicsConstraint->GetRelativeLocation());

			Hand->PhysicsRoot->SetRelativeLocation(FVector(0));
			FTransform RootRelativeTransform = Hand->ChildActorComponent->GetRelativeTransform();
			RootRelativeTransform.SetScale3D(RHand->PhysicsRoot->GetComponentScale());
			
			Hand->MotionController->GripObject(Hand->PhysicsRoot, RootRelativeTransform, true,NAME_None, NAME_None, EGripCollisionType::AttachmentGrip);
			Hand->AttachHandToMotionController();
		}
	}
	else
	{
		// If hands are uninitialized, display message of hand(s) that are missing
		FString Message;
		if(!RHand && !LHand) Message = "RightHand and LeftHand";
		else if(!RHand) Message = "RightHand";
		else if(!LHand) Message = "LeftHand";
		UE_LOG(LogTemp, Error, TEXT("%s is uninitialized"), *Message);
	}
}

void AVRCharacterBase::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If climbing and ClimbingHand is set, set location to the offset of the hand to MotionController
	if(ClimbingHand && VRMovementReference->IsClimbing())
	{
		const FVector ClimbingOffset = ClimbingHand->GetActorLocation() - ClimbingHand->PhysicsRoot->GetComponentLocation();//ClimbingHand->GetActorLocation() - ClimbingHand->MotionController->GetComponentLocation();
		VRMovementReference->AddCustomReplicatedMovement(ClimbingOffset);
	}
}

void AVRCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("L_Thumb_Y", this, &AVRCharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("L_Thumb_X", this, &AVRCharacterBase::MoveRight);
	PlayerInputComponent->BindAxis("R_Thumb_Y", this, &AVRCharacterBase::LookUp);
	PlayerInputComponent->BindAxis("R_Thumb_X", this, &AVRCharacterBase::LookRight);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("RightTrigger", IE_Pressed, this, &AVRCharacterBase::RightTriggerPressed);
	PlayerInputComponent->BindAction("RightTrigger", IE_Released, this, &AVRCharacterBase::RightTriggerReleased);
	PlayerInputComponent->BindAction("LeftTrigger", IE_Pressed, this, &AVRCharacterBase::LeftTriggerPressed);
	PlayerInputComponent->BindAction("LeftTrigger", IE_Released, this, &AVRCharacterBase::LeftTriggerReleased);

	PlayerInputComponent->BindAction("RightGrip", IE_Pressed, this, &AVRCharacterBase::RightGripPressed);
	PlayerInputComponent->BindAction("RightGrip", IE_Released, this, &AVRCharacterBase::RightGripReleased);
	PlayerInputComponent->BindAction("LeftGrip", IE_Pressed, this, &AVRCharacterBase::LeftGripPressed);
	PlayerInputComponent->BindAction("LeftGrip", IE_Released, this, &AVRCharacterBase::LeftGripReleased);
}

void AVRCharacterBase::MoveForward(const float Value)
{
	if(Controller && abs(Value) >= 0.15f)
	{
		const FVector Direction = FRotationMatrix(FRotator(0.f, VRReplicatedCamera->GetComponentRotation().Yaw, 0.f)).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AVRCharacterBase::MoveRight(const float Value)
{
	if(Controller && abs(Value) >= 0.15f)
	{
		const FVector Direction = FRotationMatrix(FRotator(0.f, VRReplicatedCamera->GetComponentRotation().Yaw, 0.f)).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AVRCharacterBase::LookUp(const float Value)
{
	if(TurnMode == ETurnMode::Mouse && Value != 0.f)
	{
		const float PitchValue = Value * MouseSensitivity * 2.f;
		VRReplicatedCamera->AddLocalRotation(FRotator(abs(VRReplicatedCamera->GetComponentRotation().Pitch + PitchValue) >= 89.f ? 0.f : PitchValue, 0.f, 0.f));
	}
}

void AVRCharacterBase::LookRight(const float Value)
{
	switch(TurnMode)
	{
		case ETurnMode::Constant:
			if(abs(Value) >= 0.15f) AddControllerYawInput(Value * TurnRate * GetWorld()->GetDeltaSeconds());
			break;
		
		case ETurnMode::Snap:
			if(bCanSnap && abs(Value) >= 0.85f)
			{
				AddControllerYawInput(Value > 0.f ? 15.f : -15.f);
				bCanSnap = false;
			}
			else if(!bCanSnap && abs(Value) <= 0.15f) bCanSnap = true;
			break;
		
		case ETurnMode::Mouse:
			if(Value != 0.f) AddControllerYawInput(Value * MouseSensitivity);
			break;
	}
}

void AVRCharacterBase::RightTriggerPressed()
{
	RHand->OnTriggerPressed();
}

void AVRCharacterBase::LeftTriggerPressed()
{
	LHand->OnTriggerPressed();
}

void AVRCharacterBase::RightTriggerReleased()
{
	
}

void AVRCharacterBase::LeftTriggerReleased()
{
	
}

void AVRCharacterBase::RightGripPressed()
{
	if(RHand) RHand->Grab();
}

void AVRCharacterBase::LeftGripPressed()
{
	if(LHand) LHand->Grab();
}

void AVRCharacterBase::RightGripReleased()
{
	if(RHand) RHand->ReleaseGrab();
}

void AVRCharacterBase::LeftGripReleased()
{
	if(LHand) LHand->ReleaseGrab();
}