#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Portal.generated.h"

class UCameraComponent;
class UBoxComponent;
class AMainCharacterBase;

UCLASS()
class TWINEXPLORERS_API APortal : public AActor
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditDefaultsOnly)
    USceneComponent* AsRoot;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Portal Comp")
    UStaticMeshComponent* DoorMeshComp;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Portal Comp")
    USceneCaptureComponent2D* DoorCapture;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal Comp")
    UBoxComponent* PortalBoxComp;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal Comp")
    UBoxComponent* OverlapDetectionBox;         // 用来检测是否存在Portal Overlapping的情况

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal Props")
    UMaterialInterface* DoorMeshMaterial;

    UPROPERTY(BlueprintReadOnly, Category="Portal Props")
    UTextureRenderTarget2D* DoorRenderTarget2D;

    UPROPERTY(Replicated, BlueprintReadOnly)
    APortal* LinkedPortal; // 链接的Portal

    UPROPERTY(Replicated)
    bool bIsTransporting; // 用于跟踪是否正在进行传送

    bool bIsDoor1;

    bool bIsEnabled;

    bool bIsInit = false;

private:
    UPROPERTY()
    AMainCharacterBase* LocalCharacter; // 这个不需要复制，而是运行在本地

    UPROPERTY()
    APlayerController* LocalPlayerController;       // 这个不需要复制，而是运行在本地

public:
    // Sets default values for this pawn's properties
    APortal();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    bool IsSceneCaptureActive(USceneCaptureComponent2D* SceneCapture);

    // 链接到另一个Portal
    UFUNCTION(Server, Reliable)
    void ServerLinkPortal(APortal* OtherPortal);
    void LinkPortal(APortal* OtherPortal);

    UFUNCTION(Server, Reliable)
    void ServerMarkExit();
    void MarkExit();

    UFUNCTION(BlueprintCallable)
    void EnableSceneCapture();

    UFUNCTION(BlueprintCallable)
    void DisableSceneCapture();

    UFUNCTION(BlueprintPure)
    bool IsPortalOnScreen(const UCameraComponent* CameraComponent) const;

private:
    void UpdateCaptureCameras();

    void UpdatePortal(UCameraComponent* CameraComp);

    FVector CalculateRotationAxes(const FVector& Axes, const FTransform& ActorTransform);
    
    UFUNCTION()
    void PortalBoxOverlapBeginEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void PortalBoxOverlapEndEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UTextureRenderTarget2D* CreateRenderTarget2D();
};
