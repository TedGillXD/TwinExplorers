// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Contraptions/ContraptionActorBase.h"
#include "Interfaces/GrabableInterface.h"
#include "Monopole.generated.h"

// 表示磁铁的磁极
UENUM(BlueprintType)
enum EMagneticPole {
	South = 0,
	North = 1,
};

class USphereComponent;
/**
 * 
 */
UCLASS()
class TWINEXPLORERS_API AMonopole : public AContraptionActorBase
{
	GENERATED_BODY()

public:
	AMonopole();
	
protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Monopole Props")
	TEnumAsByte<ECollisionChannel> MagneticCollisionType;			// 磁铁的碰撞类型
	
	UPROPERTY()
	USceneComponent* AsRoot;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Monopole Comps")
	UStaticMeshComponent* PoleMeshComp;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Monopole Comps")
	USphereComponent* ActionRange;		// 磁铁的影响范围

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Monopole Props")
	TEnumAsByte<EMagneticPole> MagneticPole;		// 本磁铁的磁极

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Monopole Props")
	float MaxForce;		// 最大的吸力和排斥力

public:
	virtual void Tick(float DeltaSeconds) override;
	
private:
	void ApplyInfluence();
};
