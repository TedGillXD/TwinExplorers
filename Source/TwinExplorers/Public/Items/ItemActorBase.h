// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DragableActorBase.h"
#include "Item.h"
#include "Interfaces/GrabableInterface.h"
#include "Interfaces/InteractableInterface.h"
#include "UObject/NoExportTypes.h"
#include "ItemActorBase.generated.h"

class USphereComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemBeingPicked, AActor*, SpawnLocationRef);

UCLASS()
class AItemActorBase : public ADragableActorBase, public IInteractableInterface {
	GENERATED_BODY()

public:
	AItemActorBase();

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="ItemActorBase Comps")
	USceneComponent* AsRoot;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="ItemActorBase Comp")
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="ItemActorBase Comp")
	USphereComponent* PickupSphere;			// 用来检测拾起Item的Sphere

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="ItemActorBase Comp")
	UDecalComponent* IndicatorDecal;
	
protected:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ItemActorBase Props")
	FItem ItemData;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ItemActorBase Props")
	float RotationSpeed;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ItemActorBase Props")
	float FloatSpeed;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ItemActorBase Props")
	UMaterialInterface* FocusedMaterial;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ItemActorBase Props")
	UMaterialInterface* UnfocusedMaterial;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ItemActorBase Props")
	USoundBase* PickupSound;		// 拾取的声音

	float RunningTime;
		
public:
	UPROPERTY()
	AActor* SpawnLocationActorRef;

	FOnItemBeingPicked OnItemBeingPicked;

public:
	UFUNCTION()
	void PickupItem(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void Tick(float DeltaSeconds) override;
	
	virtual bool CanInteract_Implementation(const FItem& InHandItem) override;
	virtual void Interact_Implementation(APawn* FromPawn, const FItem& InHandItem) override;
	virtual FString GetInteractString_Implementation() override;
	virtual UTexture2D* GetInteractIcon_Implementation() override;
	virtual bool ShouldUpdate_Implementation() override;
	virtual void Updated_Implementation() override;
	virtual void Focused_Implementation() override;
	virtual void Unfocused_Implementation() override;
};
