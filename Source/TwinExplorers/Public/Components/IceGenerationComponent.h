// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IceGenerationComponent.generated.h"


class AIcePillar;
class UCameraComponent;
class AMainCharacterBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TWINEXPLORERS_API UIceGenerationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UIceGenerationComponent();

private:
	UPROPERTY(Replicated)
	AIcePillar* GeneratedIce;		// 对生成出的Ice的一个引用，因为一次只允许一个冰柱的存在，所以在存在有冰柱的时候就不允许创建新的

	UPROPERTY(Replicated)
	AMainCharacterBase* Owner;
	
	UPROPERTY()
	UStaticMeshComponent* ForDetectionStaticMeshComp;			// 生成出来用来检测重叠的Actor
	
protected:
	UPROPERTY(Replicated)
	bool bIsInitialized;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Component Props")
	float DetectLength;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Component Props")
	TSubclassOf<AIcePillar> IceActorClass;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Component Props")
	UStaticMesh* CheckSpaceStaticMesh;		// 用来生成出来检查是否能创建的

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Component Props")
	UMaterialInterface* UnspawnableMaterial;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Component Props")
	UMaterialInterface* SpawnableMaterial;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Component Props")
	TEnumAsByte<ECollisionChannel> SurfaceTraceChannel;
	
	bool bIsChecking = false;

	UPROPERTY(BlueprintReadOnly)
	bool bCanSpawn = false;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// 开始检查是否能生成
	UFUNCTION(BlueprintCallable)
	void DoSpawnCheck();			

	// 取消检查
	UFUNCTION(BlueprintCallable)
	void CancelCheck();				

	// 在检查通过的情况下，生成Ice
	UFUNCTION(BlueprintCallable)
	void GenerateNewIce();

	UFUNCTION(Server, Reliable)
	void GenerateIceOnServer(bool bIsHit, const FHitResult& HitResult);

private:
	UFUNCTION()
	void OnPillarDestroy();
	
	void GenerateIceInternal(bool bIsHit, const FHitResult& HitResult);		// 这个函数负责生成Ice在指定位置，只会运行在服务器上

	void UpdateDetectorMaterial() const;
	void UpdateMaterial(UMaterialInterface* NewMaterial) const;
};
