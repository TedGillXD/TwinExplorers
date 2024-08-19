// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalV2.generated.h"

class UCustomRateCaptureComponent2D;
class UBoxComponent;
class AMainCharacterBase;
class UArrowComponent;

UENUM()
enum EOptimizedLevel {
	Level0,			// ViewportSize
	Level1,			// 0.5 * ViewportSize
	Level2,			// 0.25 * ViewportSize
	Level3,			// 不渲染
};

UCLASS()
class TWINEXPLORERS_API APortalV2 : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	USceneComponent* AsRoot;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UStaticMeshComponent* PortalPlane;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UCustomRateCaptureComponent2D* PortalCamera;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UArrowComponent* ForwardDirection;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UBoxComponent* OverlapDetectionBox;         // 用来检测是否存在Portal Overlapping的情况

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UBoxComponent* PlayerDetection;			// 用来检测角色的BoxCollision

	// TODO： 下面这部分只是预先定义好，还没有做完
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UBoxComponent* DisableWallCollisionBox;			// 用来检测角色是否已经进入了Portal区域，用来关闭墙体与需要传送的物体之间的碰撞

	EOptimizedLevel LastLevel;
	
protected:

	// 传送门的材质
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal Props")
	UMaterialInterface* PortalMat;

	UPROPERTY(BlueprintReadOnly, Category="Portal Props")
	UTextureRenderTarget2D* PortalRT;		// PortalCamera的RenderTarget
	
	UPROPERTY(BlueprintReadOnly, Category="Portal Props")
	UMaterialInstanceDynamic* PortalMatInstance;

	UPROPERTY(BlueprintReadOnly, Category="Portal Props", EditAnywhere, ReplicatedUsing=OnRep_LinkedPortal)
	APortalV2* LinkedPortal;

	UPROPERTY(BlueprintReadOnly, Category="Portal Props")
	int32 MaxRecursions;			// 最大递归渲染

	int32 CurrentRecursion;			// 当前的渲染递归次数

	UPROPERTY(ReplicatedUsing=OnRep_RingColor)
	FLinearColor RingColor;

	UPROPERTY()
	AMainCharacterBase* LocalCharacter;

	UPROPERTY()
	APlayerController* LocalPlayerController;

	UPROPERTY(Replicated)
	bool bIsTeleporting;

	bool bIsInit;

	bool bIsEnabled;		// 目前是否开启更新
	FVector2D LastViewportSize;
	bool bCanBeOptimized;		// 能否开始进行优化

public:
	APortalV2();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void TriggerTeleport(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void LeavePortal(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void SetRingColor(const FLinearColor& Color);
	void UpdateRingColor(const FLinearColor& Color);

	UFUNCTION(BlueprintCallable)
	static void Relink(APortalV2* Portal1, APortalV2* Portal2, FLinearColor NewColor);
	
private:
	UFUNCTION(Server, Reliable)
	void LinkPortalOnServer(APortalV2* OtherPortal);
	void LinkPortal(APortalV2* OtherPortal);

	UFUNCTION()
	void OnRep_LinkedPortal();

	UFUNCTION()
	void OnRep_RingColor();

	// PortalPlane的设置
	void Init();
	void SetClipPlane() const;	// 设置裁切平面
	void UpdateSceneCapture(const FTransform& CameraTransform) const;
	void UpdateSceneCaptureRecursive(const FVector& Location, const FRotator& Rotation);
	FVector GetTargetRotationAxe(const FVector& Axe) const;
	void DoViewportResize() const;		// 处理视口大小变化

	// 传送相关
	TArray<AActor*> BoxOverlappingActors;
	FVector GetUpdatedVelocity(const FVector& OriginalVelocity);
	FVector CalculateTargetAxes(FVector X) const;

	UFUNCTION(Server, Reliable)
	void MarkTeleportingOnServer();
	void MarkTeleporting();

	// Portal优化相关
	EOptimizedLevel GetOptimizationLevel(const FTransform& CameraTransform, const FVector& CameraForward, int CameraFOV) const;
	void EnableSceneCapture();
	void DisableSceneCapture();
	void SetCurrentOptimizationLevel(EOptimizedLevel OptimizedLevel);
	void SetToLevel0Resolution() const;
	void SetToLevel1Resolution() const;
	void SetToLevel2Resolution() const;
	void ResizeTextureToMatchViewport(const FVector2D& DesiredSize) const;
	float GetScale() const;
};
