// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PortalGenerationComponent.generated.h"

class APortal;
class AMainCharacterBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TWINEXPLORERS_API UPortalGenerationComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Category="PortalGen Props", Replicated)
	APortal* PortalRef;			// 当前Portal的引用
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="PortalGen Props")
	TEnumAsByte<ECollisionChannel> SurfaceType;		// 用来检测能生成Portal的表面类型

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="PortalGen Props")
	TSubclassOf<APortal> PortalClass;		// 传送门类

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="PortalGen Props")
	float DetectionDistance;			// 检测距离
	
	UPROPERTY(Replicated)
	AMainCharacterBase* Owner;		// 拥有当前组件的角色

	UPROPERTY(Replicated)
	bool bIsInitialized;			// 是否已经初始化了

	bool bIsGenerating;			// 是否正在执行RPC的途中

public:	
	// Sets default values for this component's properties
	UPortalGenerationComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	void Shoot();

private:
	UFUNCTION(Server, Reliable)
	void ChangePortalLocationAndRotation(const FVector& NewLocation, const FRotator& NewRotation);

	UFUNCTION(Server, Reliable)
	void SpawnPortalAtLocationAndRotation(const FVector& NewLocation, const FRotator& NewRotation);

	// 检查当前的表面是否有足够的空间容纳下这个传送门
	bool CheckRoom(const FHitResult& HitResult, FVector& ValidLocation, FRotator& ValidRotation, int RecursionDepth = 0);
	bool CheckRoom(const FHitResult& HitResult, FVector& ValidLocation, int RecursionDepth, const FVector& Up, const FVector& Down, const FVector& Left, const FVector& Right);
	bool CheckOverlap(const FVector& NewLocation, const FRotator& NewRotation);
};


class AMainCharacterBase;