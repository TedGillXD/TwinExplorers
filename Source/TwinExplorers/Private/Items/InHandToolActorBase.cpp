// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InHandToolActorBase.h"

// Sets default values
AInHandToolActorBase::AInHandToolActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	this->SetReplicates(true);
}

// Called when the game starts or when spawned
void AInHandToolActorBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInHandToolActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInHandToolActorBase::UseInHandItemPressed_Implementation(AMainCharacterBase* FromCharacter) {
	// do nothing
}

void AInHandToolActorBase::UseInHandItemReleased_Implementation(AMainCharacterBase* FromCharacter) {
	// do nothing
}

void AInHandToolActorBase::CancelUseItemPressed_Implementation(AMainCharacterBase* FromCharacter) {
	// do nothing
}

void AInHandToolActorBase::CancelUseItemReleased_Implementation(AMainCharacterBase* FromCharacter) {
	// do nothing
}
