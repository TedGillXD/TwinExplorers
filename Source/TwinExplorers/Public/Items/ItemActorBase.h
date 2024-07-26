// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Interfaces/InteractableInterface.h"
#include "UObject/NoExportTypes.h"
#include "ItemActorBase.generated.h"

UCLASS()
class AItemActorBase : public AActor, public IInteractableInterface {
	GENERATED_BODY()

public:
	AItemActorBase();

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="ItemActorBase Comps")
	USceneComponent* AsRoot;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="ItemActorBase Comp")
	UStaticMeshComponent* ItemMesh;
	
protected:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ItemActorBase Props")
	FItem ItemData;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ItemActorBase Props")
	UMaterialInterface* FocusedMaterial;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ItemActorBase Props")
	UMaterialInterface* UnfocusedMaterial;

public:
	virtual bool CanInteract_Implementation(const FItem& InHandItem) override;
	virtual void Interact_Implementation(APawn* FromPawn, const FItem& InHandItem) override;
	virtual FString GetInteractString_Implementation(const FItem& InHandItem) override;
	virtual UTexture2D* GetInteractIcon_Implementation(const FItem& InHandItem) override;
	virtual bool ShouldUpdate_Implementation() override;
	virtual void Updated_Implementation() override;
	virtual void Focused_Implementation() override;
	virtual void Unfocused_Implementation() override;
};
