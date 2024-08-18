// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pad.generated.h"

class UPhysicsConstraintComponent;

UCLASS()
class TWINEXPLORERS_API APad : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components")
	USceneComponent* Root;
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components")
    UStaticMeshComponent* Mesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components")
    UPhysicsConstraintComponent* PhysicsConstraint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Props")
	float TriggerThreshold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Props")
	float DisappearTime;		// 触发后消失的时间

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Props")
	float ResetTime;			// 重置需要的时间

	float OriginalZ;
	bool bIsTriggered;

private:
	void SetMeshDisappear();

	void ResetMesh();
	
public:	
	// Sets default values for this actor's properties
	APad();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
