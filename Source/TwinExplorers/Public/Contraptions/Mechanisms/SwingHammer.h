// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SwingHammer.generated.h"

class UBoxComponent;
class UPhysicsConstraintComponent;

UCLASS()
class TWINEXPLORERS_API ASwingHammer : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* HammerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UPhysicsConstraintComponent* PhysicsConstraint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Components")
	UBoxComponent* LeftBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UBoxComponent* RightBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Hammer Settings")
	float SwingSpeed; // 摆动的速度
	
public:	
	// Sets default values for this actor's properties
	ASwingHammer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	FTimerHandle TimerHandle;
	bool bMovingToEnd;

	void ToggleSwingDirection();

	UFUNCTION()
	void Hit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
