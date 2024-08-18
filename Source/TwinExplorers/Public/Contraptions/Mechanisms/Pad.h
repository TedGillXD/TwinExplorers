// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pad.generated.h"

UCLASS()
class TWINEXPLORERS_API APad : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APad();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
