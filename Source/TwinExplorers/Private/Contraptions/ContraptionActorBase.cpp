// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/ContraptionActorBase.h"

// Sets default values
AContraptionActorBase::AContraptionActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
    AActor::SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AContraptionActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AContraptionActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AContraptionActorBase::CanInteract_Implementation(const FItem& InHandItem) {
	return false;
}

void AContraptionActorBase::Interact_Implementation(APawn* FromPawn, const FItem& InHandItem) {
	
}

FString AContraptionActorBase::GetInteractString_Implementation() {
	return "";
}

UTexture2D* AContraptionActorBase::GetInteractIcon_Implementation() {
	return nullptr;
}

bool AContraptionActorBase::ShouldUpdate_Implementation() {
	return false;
}

void AContraptionActorBase::Updated_Implementation() {
	
}

void AContraptionActorBase::Focused_Implementation() {
	
}

void AContraptionActorBase::Unfocused_Implementation() {
	
}

