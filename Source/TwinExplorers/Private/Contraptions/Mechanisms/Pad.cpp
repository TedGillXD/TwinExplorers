// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/Mechanisms/Pad.h"

// Sets default values
APad::APad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APad::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

