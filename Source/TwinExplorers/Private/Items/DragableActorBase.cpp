// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/DragableActorBase.h"

// Sets default values
ADragableActorBase::ADragableActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
}

// Called when the game starts or when spawned
void ADragableActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADragableActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADragableActorBase::OnGrab_Implementation() {
	// do nothing
}

void ADragableActorBase::OnDrop_Implementation() {
	// do nothing
}

void ADragableActorBase::Transport_Implementation(const FVector& TargetLocation, const FRotator& TargetRotation,
	const FVector& TargetVelocity) {
	
	SetActorLocationAndRotation(TargetLocation, TargetRotation);
}

FVector ADragableActorBase::GetOriginalVelocity_Implementation() {
	return GetVelocity();
}

