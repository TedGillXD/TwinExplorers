// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractableInterface.h"
#include "ContraptionActorBase.generated.h"

UCLASS()
class TWINEXPLORERS_API AContraptionActorBase : public AActor, public IInteractableInterface {
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AContraptionActorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	virtual bool CanInteract_Implementation(const FItem& InHandItem) override;
	virtual void Interact_Implementation(APawn* FromPawn, const FItem& InHandItem) override;
	virtual FString GetInteractString_Implementation() override;
	virtual UTexture2D* GetInteractIcon_Implementation() override;
	virtual bool ShouldUpdate_Implementation() override;
	virtual void Updated_Implementation() override;
	virtual void Focused_Implementation() override;
	virtual void Unfocused_Implementation() override;
};
