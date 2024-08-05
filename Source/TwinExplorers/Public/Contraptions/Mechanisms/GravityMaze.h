// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Contraptions/Mechanisms/MechanismBase.h"
#include "GravityMaze.generated.h"

class UBoxComponent;
class UPhysicsConstraintComponent;
/**
 * 
 */
UCLASS()
class TWINEXPLORERS_API AGravityMaze : public AMechanismBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Maze Comp")
	UStaticMeshComponent* BaseComp;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Maze Comp")
	UStaticMeshComponent* MazeFloorComp;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Maze Comp")
	UStaticMeshComponent* BallGeneratorComp;		// 用来表示一个球体生成器的StaticMesh

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Maze Comp")
	UBoxComponent* BallDestroyBoxComp;		// 用来将掉出Maze的球体给销毁的Box

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Maze Comp")
	USceneComponent* BallSpawnLocation;		// 用来标记生成球的位置
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Maze Comp")
	UPhysicsConstraintComponent* PhysicsConstraintComp;

	UPROPERTY(BlueprintReadOnly)
	FVector CurrentTargetVelocity;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Maze Props")
	float VelocityDelta;		// 每次改变的Velocity的大小

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Maze Props")
	TSubclassOf<AActor> BallClass;			// 生成的球的类型

	UPROPERTY(ReplicatedUsing=OnRep_Sphere)
	AActor* Sphere;		// 生成的球的指针

public:
	AGravityMaze();

	virtual void BeginPlay() override;

protected:
	UFUNCTION()
	void ForwardActivated();

	UFUNCTION()
	void ForwardDeactivated();

	UFUNCTION()
	void BackwardActivated();

	UFUNCTION()
	void BackwardDeactivated();

	UFUNCTION()
	void LeftwardActivated();

	UFUNCTION()
	void LeftwardDeactivated();

	UFUNCTION()
	void RightwardActivated();

	UFUNCTION()
	void RightwardDeactivated();

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRep_Sphere();
	
private:
	void UpdateAngularVelocityTarget() const;
};
