// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Portal.generated.h"

class UBoxComponent;
class AMainCharacterBase;

UCLASS()
class TWINEXPLORERS_API APortal : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	USceneComponent* AsRoot;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 1 Comp")
	USceneComponent* Door1Root;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 1 Comp")
	UStaticMeshComponent* Door1MeshComp;			// 应该是一个Plane	

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 1 Comp")
	USceneCaptureComponent2D* Door1Capture;			// 用来捕获当前门的景像

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 1 Comp")
	USceneComponent* CameraRoot1;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 1 Comp")
	USceneComponent* PlayerSimulatorDoor1;			// 对于Portal1关于角色相对于Portal2的位置

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 1 Comp")
	UBoxComponent* Portal1BoxComp;

protected:

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 2 Comp")
	USceneComponent* Door2Root;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 2 Comp")
	UStaticMeshComponent* Door2MeshComp;			// 应该是一个Plane

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 2 Comp")
	USceneComponent* CameraRoot2;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 2 Comp")
	USceneCaptureComponent2D* Door2Capture;			// 用来捕获当前门的景像

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 2 Comp")
	USceneComponent* PlayerSimulatorDoor2;			// 对于Portal2关于角色相对于Portal1的位置

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 2 Comp")
	UBoxComponent* Portal2BoxComp;

	UPROPERTY()
	UPrimitiveComponent* ExitCollider;

private:
	UPROPERTY()
	AMainCharacterBase* LocalCharacter;		// 这个不需要复制，而是运行在本地
	
public:
	// Sets default values for this pawn's properties
	APortal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateCaptureCameras();

	void UpdatePortal(USceneCaptureComponent2D* SceneCapture, const USceneComponent* PlayerSimulator);
	
	// 处理传送的函数
	void TransportInternal(UPrimitiveComponent* Trigger, AActor* OtherActor, UBoxComponent* TargetBoxComponent, const FRotator& TargetRotation);
	
	UFUNCTION()
	void Portal1BoxOverlapBeginEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void Portal2BoxOverlapBeginEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void PortalBoxOverlapEndEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
