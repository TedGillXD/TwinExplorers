// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Contraptions/Switches/SwitchBase.h"
#include "PressurePlate.generated.h"

class UPhysicsConstraintComponent;
/**
 * 
 */
UCLASS()
class TWINEXPLORERS_API APressurePlate : public ASwitchBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	USceneComponent* AsRoot;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UStaticMeshComponent* BaseComp;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UStaticMeshComponent* PlateComp;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UPhysicsConstraintComponent* PhysicsConstraintComp;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="PressurePlate Props")
	float TriggeredOffset;		// 触发的偏移量

	bool bIsActivated;
	
private:
	float InitialZ;

public:
	APressurePlate();

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;

private:
	bool IsPlateStable() const;
};
