// Fill out your copyright notice in the Description page of Project Settings.


#include "GunBase.h"
#include "Grippables/GrippableStaticMeshComponent.h"
#include "VRProject4/VRHandSocketComponent.h"

AGunBase::AGunBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) 
{
	PrimaryActorTick.bCanEverTick = false;

	
}

void AGunBase::BeginPlay()
{
	Super::BeginPlay();
}
	
void AGunBase::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

}

