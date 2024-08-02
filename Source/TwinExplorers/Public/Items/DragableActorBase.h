// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/GrabableInterface.h"
#include "Interfaces/TransportableInterface.h"
#include "DragableActorBase.generated.h"

UCLASS()
class TWINEXPLORERS_API ADragableActorBase : public AActor, public IGrabableInterface, public ITransportableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADragableActorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnGrab_Implementation() override;
	virtual void OnDrop_Implementation() override;

	virtual void Transport_Implementation(const FVector& TargetLocation, const FRotator& TargetRotation) override;
};
