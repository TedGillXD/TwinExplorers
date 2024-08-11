// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Skill.h"

// Sets default values
ASkill::ASkill()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	this->SetReplicates(true);
}

// Called when the game starts or when spawned
void ASkill::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASkill::ActivateSkill_Implementation(AMainCharacterBase* CharacterBase) {
	
}

void ASkill::DeactivateSkill_Implementation(AMainCharacterBase* CharacterBase) {
	
}

