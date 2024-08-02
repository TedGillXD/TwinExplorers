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
    UPROPERTY(EditDefaultsOnly)
    USceneComponent* AsRoot;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Portal Comp")
    UStaticMeshComponent* DoorMeshComp;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Portal Comp")
    USceneCaptureComponent2D* DoorCapture;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal Comp")
    USceneComponent* CameraRoot;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal Comp")
    USceneComponent* PlayerSimulator;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal Comp")
    UBoxComponent* PortalBoxComp;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 1 Props")
    UMaterialInterface* Door1MeshMaterial;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 1 Props")
    UTextureRenderTarget2D* Door1RenderTarget2D;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 2 Props")
    UMaterialInterface* Door2MeshMaterial;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Portal 2 Props")
    UTextureRenderTarget2D* Door2RenderTarget2D;

    UPROPERTY(Replicated, BlueprintReadOnly)
    APortal* LinkedPortal; // 链接的Portal

    UPROPERTY(Replicated)
    bool bIsTransporting; // 用于跟踪是否正在进行传送

    bool bIsDoor1;

private:
    UPROPERTY()
    AMainCharacterBase* LocalCharacter; // 这个不需要复制，而是运行在本地

public:
    // Sets default values for this pawn's properties
    APortal();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable)
    void Init(UMaterialInterface* PortalDoorMaterial, UTextureRenderTarget2D* TextureTarget);

    // 链接到另一个Portal
    UFUNCTION(Server, Reliable)
    void ServerLinkPortal(APortal* OtherPortal);
    void LinkPortal(APortal* OtherPortal);

    UFUNCTION(Server, Reliable)
    void ServerMarkExit();
    void MarkExit();

private:
    void UpdateCaptureCameras();

    void UpdatePortal(USceneCaptureComponent2D* SceneCapture, const USceneComponent* TargetPlayerSimulator);
    
    UFUNCTION()
    void PortalBoxOverlapBeginEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void PortalBoxOverlapEndEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
