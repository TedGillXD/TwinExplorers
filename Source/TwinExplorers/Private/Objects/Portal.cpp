#include "Objects/Portal.h"

#include "DatasmithDefinitions.h"
#include "Camera/CameraComponent.h"
#include "Characters/MainCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Interfaces/TransportableInterface.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APortal::APortal()
{
    // Set this pawn to call Tick() every frame. You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    AsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(AsRoot);
    AsRoot->SetMobility(EComponentMobility::Type::Movable);

    DoorMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMeshComp"));
    DoorMeshComp->SetupAttachment(GetRootComponent());

    CameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot"));
    CameraRoot->SetupAttachment(GetRootComponent());
    CameraRoot->SetRelativeRotation({0.0, 180.0, 0.0});

    DoorCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("DoorCapture"));
    DoorCapture->SetupAttachment(CameraRoot);

    PlayerSimulator = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerSimulator"));
    PlayerSimulator->SetupAttachment(GetRootComponent());

    PortalBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalBox"));
    PortalBoxComp->SetupAttachment(GetRootComponent());
    PortalBoxComp->OnComponentBeginOverlap.AddDynamic(this, &APortal::PortalBoxOverlapBeginEvent);
    PortalBoxComp->OnComponentEndOverlap.AddDynamic(this, &APortal::PortalBoxOverlapEndEvent);

    OverlapDetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapDetectionBox"));
    OverlapDetectionBox->SetupAttachment(GetRootComponent());

    bReplicates = true;
    LinkedPortal = nullptr;

    bIsDoor1 = false;
    bIsEnabled = false;
}

void APortal::BeginPlay() {
    Super::BeginPlay();
    
    if(!HasAuthority()) {
        // 查看当前场景内有多少Portal
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), StaticClass(), FoundActors);
        if(FoundActors.Num() == 1) {            
            bIsDoor1 = true;
            DisableSceneCapture();      // RenderTarget2D应该延迟初始化
            Init(Door1MeshMaterial, Door2RenderTarget2D);
        } else {
            bIsDoor1 = false;
            DisableSceneCapture();      // RenderTarget2D应该延迟初始化
            Init(Door2MeshMaterial, Door1RenderTarget2D);
        }

        // 获取本地的角色
        const int32 NumLocalPlayer = UGameplayStatics::GetNumLocalPlayerControllers(GetWorld());
        for (int32 i = 0; i < NumLocalPlayer; ++i) {
            APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), i);
            if (PlayerController && PlayerController->IsLocalPlayerController()) {
                LocalPlayerController = PlayerController;
                AMainCharacterBase* Character = Cast<AMainCharacterBase>(PlayerController->GetPawn());
                if(Character && Character->GetLocalRole() == ROLE_AutonomousProxy) {
                    LocalCharacter = Character;
                }
            }
        }
    }
    
    
}

void APortal::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    // 在这里检测是否存在另外一个Portal，如果存在则绑定
    // 为什么不在BeginPlay执行呢？因为根据RPC规则，如果是通过通知的方式需要在Client1发送链接到服务器然后再从服务器复制到Client2，然后Client2得到通知再发送链接请求到服务器到Client1，总共需要Server <-> Client进行4次通信
    // 而改成在Tick中，就能减少为两次
    if(HasAuthority()) {
        if(!LinkedPortal) {
            // 尝试获取另外一个Portal
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), APortal::StaticClass(), FoundActors);
            for(AActor* Actor : FoundActors) {
                if(Actor != this) {
                    APortal* OtherPortal = Cast<APortal>(Actor);
                    LinkPortal(OtherPortal);       // 绑定传送门
                }
            }
        }
    }

    // 更新是放在本地的，并且是产生联系之后才会更新
    if(!HasAuthority()) {
        // for debug
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, GetName() + " " + (bIsEnabled ? "True" : "False"));
        
        // 在链接了的情况下进行更新
        if(LocalCharacter && LinkedPortal) {
            // 根据可见性来动态开启和关闭Portal，当能看到Portal1的时候，Portal2需要进行采集然后渲染到Portal1
            if(IsPortalOnScreen(LocalCharacter->GetCameraComponent())) {
                LinkedPortal->EnableSceneCapture();
            } else {
                LinkedPortal->DisableSceneCapture();
            }
            
            UpdateCaptureCameras();
        }
    }
}

void APortal::Init(UMaterialInterface* PortalDoorMaterial, UTextureRenderTarget2D* TextureTarget) {
    DoorMeshComp->SetMaterial(0, PortalDoorMaterial);
    DoorCapture->TextureTarget = TextureTarget;
}

void APortal::MarkExit() {
    if(HasAuthority()) {
        ServerMarkExit();
    } else {
        ServerMarkExit();
    }
}

void APortal::EnableSceneCapture() {
    if(bIsEnabled) { return; }
    DoorCapture->SetComponentTickEnabled(true);
    DoorCapture->SetVisibility(true);
    DoorCapture->bCaptureEveryFrame = true;
    DoorCapture->bCaptureOnMovement = true;
    DoorCapture->bAlwaysPersistRenderingState = true;
    DoorCapture->CaptureScene(); // 手动捕捉一次
    bIsEnabled = true;
}

void APortal::DisableSceneCapture() {
    if(!bIsEnabled) { return; }
    DoorCapture->SetComponentTickEnabled(false);
    DoorCapture->SetVisibility(false);
    DoorCapture->bCaptureEveryFrame = false;
    DoorCapture->bCaptureOnMovement = false;
    DoorCapture->bAlwaysPersistRenderingState = false;
    bIsEnabled = false;
}

bool APortal::IsPortalOnScreen(const UCameraComponent* CameraComponent) const {
    // 获取物体位置
    FVector ActorLocation = this->GetActorLocation();

    // 将世界坐标转换为屏幕坐标
    FVector2D ScreenLocation;

    // 首先通过看看当前Portal的位置是否能映射到屏幕上判断角色是否是正面面对着Portal
    if (LocalPlayerController->ProjectWorldLocationToScreen(ActorLocation, ScreenLocation)) {
        // 如果在正面，执行线性追踪，检查物体是否被遮挡
        FVector CameraLocation = CameraComponent->GetComponentLocation();
        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);  // 忽略传送门自身
        Params.AddIgnoredActor(LocalPlayerController->GetPawn());  // 忽略玩家自身

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            CameraLocation,
            ActorLocation,
            ECC_Visibility,
            Params
        );

        // 如果线性追踪没有命中或者命中的是该物体本身，说明物体在视锥内且没有被遮挡
        return !bHit || HitResult.GetActor() == this;
    }
    return false;
}

void APortal::UpdateCaptureCameras() {
    UCameraComponent* CameraComp = LocalCharacter->GetCameraComponent();
    PlayerSimulator->SetWorldLocation(CameraComp->GetComponentLocation());

    // 处理Portal，只有在Enable的时候才会进行处理
    if(bIsEnabled) {
        UpdatePortal(DoorCapture, LinkedPortal->PlayerSimulator);
    }
}

void APortal::UpdatePortal(USceneCaptureComponent2D* SceneCapture, const USceneComponent* TargetPlayerSimulator) {
    SceneCapture->SetRelativeLocation(TargetPlayerSimulator->GetRelativeLocation());
    FRotator DoorRotator = UKismetMathLibrary::FindLookAtRotation(SceneCapture->GetComponentLocation(), SceneCapture->GetAttachParent()->GetComponentLocation());
    SceneCapture->SetWorldRotation(DoorRotator);

    FVector Location = SceneCapture->GetRelativeLocation();
    float Degree = FMath::RadiansToDegrees(FMath::Atan(300.f / FMath::Max(Location.Size(), 1.f)));
    SceneCapture->FOVAngle = FMath::Clamp(Degree, 5.f, 180.f);
}

void APortal::PortalBoxOverlapBeginEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
    if(HasAuthority() && LinkedPortal && !bIsTransporting) {
        MarkExit();
        LinkedPortal->MarkExit();
        FVector TargetLocation = LinkedPortal->PortalBoxComp->GetComponentLocation();

        // TODO: 还是有问题 算出去的速度方向，先使用直勾勾的往外射
        // FVector VelocityDirection = -1 * OtherActor->GetVelocity().GetSafeNormal();
        // FQuat EntryToVelocityQuat = FQuat::FindBetweenNormals(CameraRoot->GetForwardVector(), VelocityDirection);
        // FVector RotatedVector = EntryToVelocityQuat.RotateVector(LinkedPortal->CameraRoot->GetForwardVector());
        // FRotator TargetRotation = RotatedVector.Rotation();
        
        if(!OtherActor->Implements<UTransportableInterface>()) { return; }
        ITransportableInterface::Execute_Transport(OtherActor, TargetLocation, LinkedPortal->PortalBoxComp->GetForwardVector().Rotation());
        // ITransportableInterface::Execute_Transport(OtherActor, TargetLocation, TargetRotation);
    }
}

void APortal::PortalBoxOverlapEndEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
    bIsTransporting = false;
}

void APortal::ServerLinkPortal_Implementation(APortal* OtherPortal) {
    if (OtherPortal) {
        LinkedPortal = OtherPortal;
        OtherPortal->LinkedPortal = this;
    }
}

void APortal::LinkPortal(APortal* OtherPortal) {
    if (HasAuthority()) {
        ServerLinkPortal(OtherPortal);
    } else {
        // 客户端请求服务器进行链接
        ServerLinkPortal(OtherPortal);
        EnableSceneCapture();
    }
}

void APortal::ServerMarkExit_Implementation() {
    bIsTransporting = true;
}

void APortal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APortal, LinkedPortal);
    DOREPLIFETIME(APortal, bIsTransporting);
}
