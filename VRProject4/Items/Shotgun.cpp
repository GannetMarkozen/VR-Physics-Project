// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Interactibles/VRSliderComponent.h"
#include "Components/ArrowComponent.h"

AShotgun::AShotgun(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	EjectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("EjectionArrow"));
	EjectionArrow->SetupAttachment(RootComponent);

	Slider = CreateDefaultSubobject<UVRSliderComponent>(TEXT("Slider"));
	Slider->SetupAttachment(RootComponent);
}

void AShotgun::BeginPlay()
{
	Super::BeginPlay();

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
}
