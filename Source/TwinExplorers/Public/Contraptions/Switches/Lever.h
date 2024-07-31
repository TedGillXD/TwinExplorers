// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Contraptions/Switches/SwitchBase.h"
#include "Interfaces/GrabableInterface.h"
#include "Lever.generated.h"

class UPhysicsConstraintComponent;
/**
 * 
 */
UCLASS()
class TWINEXPLORERS_API ALever : public ASwitchBase, public IGrabableInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	USceneComponent* AsRoot;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Leverl Props")
	UStaticMeshComponent* BaseComp;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Leverl Props")
	UStaticMeshComponent* LeverComp;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Leverl Props")
	UPhysicsConstraintComponent* PhysicsConstraintComp;

public:
	ALever();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

public:
	virtual void OnGrab_Implementation() override;
	virtual void OnDrop_Implementation() override;

private:
	void SetToOffOrientationTarget() const;
	void SetToOnOrientationTarget() const;
};
