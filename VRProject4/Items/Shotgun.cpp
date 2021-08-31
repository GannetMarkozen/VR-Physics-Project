// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "VRProject4/DebugMessage.h"

#include "VRProject4/Character/VRCharacterBase.h"

#include "Interactibles/VRSliderComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "VRProject4/VRHandSocketComponent.h"

AShotgun::AShotgun(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	FiringArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FiringArrow"));
	FiringArrow->SetupAttachment(RootComponent);

	EjectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("EjectionArrow"));
	EjectionArrow->SetupAttachment(RootComponent);

	Slider = CreateDefaultSubobject<UVRSliderComponent>(TEXT("Slider"));
	Slider->SetupAttachment(RootComponent);
}

void AShotgun::BeginPlay()
{
	Super::BeginPlay();
	
	this->OnGripped.AddDynamic(this, &AShotgun::OnActorGripped);
	Slider->OnSliderHitPoint.AddDynamic(this, &AShotgun::OnSliderHitPoint);

	TArray<UVRGripScriptBase*> GripScripts;
	if(IVRGripInterface::Execute_GetGripScripts(this, GripScripts))
	{
		for(UVRGripScriptBase* GripScript : GripScripts)
		{
			if(GripScript->GetClass() == UGS_GunTools::StaticClass())
			{
				GunTools = Cast<UGS_GunTools>(GripScript);
				UE_LOG(LogTemp, Warning, TEXT("Gun Tools initialized"));
				break;
			}
		}
		if(!GunTools) UE_LOG(LogTemp, Warning, TEXT("Gun Tools uninitialized on %s"), *GetName());
	}

	TArray<UVRHandSocketComponent*> HandSockets;
	GetComponents<UVRHandSocketComponent>(HandSockets);
	for(UVRHandSocketComponent* HandSocket : HandSockets)
	{
		if(HandSocket->SlotPrefix == FName("Secondary"))
		{
			SecondaryHandSocket = HandSocket;
			break;
		}
	}
	if(!SecondaryHandSocket) UE_LOG(LogTemp, Warning, TEXT("SecondaryHandSocket uninitialized on %s"), *GetName());
}

void AShotgun::OnActorGripped(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation)
{
	if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Actor Gripped"));
	if(SecondaryHandSocket && GripInformation.SecondaryGripInfo.bHasSecondaryAttachment == true)
	{
		
		FTransform SecondaryHandRelativeTransform = SecondaryHandSocket->GetComponentTransform().GetRelativeTransform(Slider->GetComponentTransform());
		if(GrippingController->MotionSource == FName("Left")) SecondaryHandRelativeTransform.Mirror(EAxis::Y, EAxis::Y);

		SecondaryHandRelativeTransform.SetScale3D(Slider->GetComponentScale());
		
		const bool bSuccessful = GrippingController->GripObjectByInterface(Slider, SecondaryHandRelativeTransform, true, NAME_None, NAME_None, true);
		if(bSuccessful) UE_LOG(LogTemp, Warning, TEXT("%s Gripped Slide"), *GrippingController->GetName());
	}
}

void AShotgun::OnSliderHitPoint(const float SliderProgressPoint)
{
	
}


void AShotgun::OnTriggerPressed()
{
	if(TryFire())
	{
		EjectShell();
	}
}

void AShotgun::OnTriggerReleased()
{
	
}

bool AShotgun::TryFire()
{
	if(bPrimed == false || Ammo <= 0) return false;

	if(MuzzleFlash)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, FTransform(FiringArrow->GetComponentRotation(), FiringArrow->GetComponentLocation(), FVector(1.f)));
	if(FiringSound)
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FiringSound, FiringArrow->GetComponentLocation());
	
	
	TArray<FHitResult*> HitResults;
	for(int32 i = 0; i < 9; i++)
	{
		FRotator AimRotation = FiringArrow->GetComponentRotation();
		if(i != 0)
		{
			for(float* Axis : {&AimRotation.Pitch, &AimRotation.Yaw})
			{
				*Axis += FMath::FRandRange(-Spread / 2.f, Spread / 2.f);
			}
		}
		
		FHitResult HitResult;
		const FVector Start = FiringArrow->GetComponentLocation();
		const FVector End = Start + AimRotation.Vector() * Range;
		
		const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
		DrawDebugLine(GetWorld(), Start, bHit ? HitResult.ImpactPoint : End, FColor::Red, false, 2.f);
		if(bHit)
		{
			if(HitParticle)
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, FTransform(HitResult.ImpactNormal.Rotation(), HitResult.ImpactPoint, FVector(1.f)));
			if(HitSound)
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, HitResult.ImpactPoint);
		}
	}
	
	
	return true;
}

void AShotgun::EjectShell()
{
	if(!AmmoClass) return;
	if(const AActor* ShotgunShell = GetWorld()->SpawnActor<AActor>(AmmoClass, EjectionArrow->GetComponentLocation(), EjectionArrow->GetComponentRotation()))
	{
		if(UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(ShotgunShell->GetRootComponent()))
		{
			Component->SetSimulatePhysics(true);
			
			const FVector Impulse = EjectionArrow->GetComponentRotation().Vector() * Component->GetMass() * EjectionSpeed * FMath::RandRange(0.9f, 1.1f);
			Component->AddImpulse(Impulse);

			FVector AngularImpulse;
			for(int32 i = 0; i < 2; i++) AngularImpulse[i] = FMath::RandRange(-EjectionSpeed, EjectionSpeed);
			Component->AddAngularImpulseInDegrees(AngularImpulse);
		}
	}
}



