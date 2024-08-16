// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PortalGenerationComponent.generated.h"

class APortalV2;
class AMainCharacterBase;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TWINEXPLORERS_API UPortalGenerationComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    UPROPERTY(BlueprintReadWrite, Category = "PortalGen Props", Replicated)
    APortalV2* PortalRef1; // Portal1的引用

    UPROPERTY(BlueprintReadWrite, Category = "PortalGen Props", Replicated)
    APortalV2* PortalRef2; // Portal2的引用

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "PortalGen Props")
    TEnumAsByte<ECollisionChannel> SurfaceType; // 用来检测能生成Portal的表面类型

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "PortalGen Props")
    TEnumAsByte<ECollisionChannel> PortalOverlapDetectionType; // 用来检测传送门重叠的类型

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "PortalGen Props")
    TSubclassOf<APortalV2> PortalClass; // 传送门类

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "PortalGen Props")
    float DetectionDistance; // 检测距离

    UPROPERTY(Replicated)
    AMainCharacterBase* Owner; // 拥有当前组件的角色

    UPROPERTY(Replicated)
    bool bIsInitialized; // 是否已经初始化了

    UPROPERTY(Replicated)
    bool bCanGeneratePortal1;

    UPROPERTY(Replicated)
    bool bCanGeneratePortal2;

    bool bIsGeneratingPortal1; // 是否正在生成Portal1
    bool bIsGeneratingPortal2; // 是否正在生成Portal2

    FLinearColor PortalRingColor;

public:
    UPortalGenerationComponent();

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintCallable)
    bool ShootPortal1();

    UFUNCTION(BlueprintCallable)
    bool ShootPortal2();

private:
    UFUNCTION(Server, Reliable)
    void SpawnPortal1AtLocationAndRotation(const FVector& NewLocation, const FRotator& NewRotation);

    UFUNCTION(Server, Reliable)
    void SpawnPortal2AtLocationAndRotation(const FVector& NewLocation, const FRotator& NewRotation);

    UFUNCTION(BlueprintPure)
    bool IsPortal1Exist() const;

    UFUNCTION(BlueprintPure)
    bool IsPortal2Exist() const;

    UFUNCTION(Server, Reliable)
    void ResetOnServer();

    UFUNCTION(BlueprintCallable)
    void Reset();

    bool CheckRoom(const FHitResult& HitResult, FVector& ValidLocation, const FRotator& ValidRotation, int RecursionDepth = 0);
    bool CheckRoom(const FHitResult& HitResult, FVector& ValidLocation, int RecursionDepth, const FVector& Up, const FVector& Down, const FVector& Left, const FVector& Right);
    static bool CheckIfIsARoom(const FHitResult& Up, const FHitResult& Down, const FHitResult& Right, const FHitResult& Left);
    bool CheckOverlap(const FVector& NewLocation, const FRotator& NewRotation) const;
};