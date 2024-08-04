// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Contraptions/MechanismBase.h"
#include "WindMachine.generated.h"

class UPhysicsConstraintComponent;
class UBoxComponent;
/**
 * 
 */
UCLASS()
class TWINEXPLORERS_API AWindMachine : public AMechanismBase
{
	GENERATED_BODY()
public:
	AWindMachine();

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="WindMachine Comp")
	UBoxComponent* WindRange;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="WindMachine Comp")
	UStaticMeshComponent* PropellerMeshComp;		// 风扇的模型

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="WindMachine Comp")
	UPhysicsConstraintComponent* PhysicsConstraintComp;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="WindMachine Props")
	float HalfRangeX;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="WindMachine Props")
	float HalfRangeY;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="WindMachine Props")
	float HalfRangeZ;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="WindMachine Props")
	float DistanceBetweenForceLine;		// LineTrace之间的距离，越大性能越好，越小效果越精准

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="WindMachine Props")
	float MaxForce;				// 风扇能提供的最大的力，离风扇越远则力越小

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_IsOn)
	bool bIsOn;

protected:
	virtual void BeginPlay() override;
	
public:
	virtual void Tick(float DeltaSeconds) override;

	virtual void Activate_Implementation() override;
	virtual void Deactivate_Implementation() override;

private:
	void DoWindMachineLogic() const;

	UFUNCTION()
	void OnRep_IsOn();
};
