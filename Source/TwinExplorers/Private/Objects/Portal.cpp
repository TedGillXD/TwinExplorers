#include "Objects/Portal.h"
#include "Camera/CameraComponent.h"
#include "Characters/MainCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Interfaces/TransportableInterface.h"
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
}

void APortal::BeginPlay() {
    Super::BeginPlay();
    
    if(!HasAuthority()) {
        // 查看当前场景内有多少Portal
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), StaticClass(), FoundActors);
        if(FoundActors.Num() == 1) {            // TODO: RenderTarget2D应该延迟初始化
            bIsDoor1 = true;
            Init(Door1MeshMaterial, Door2RenderTarget2D);
        } else {
            bIsDoor1 = false;
            Init(Door2MeshMaterial, Door1RenderTarget2D);
        }

        // 获取本地的角色
        const int32 NumLocalPlayer = UGameplayStatics::GetNumLocalPlayerControllers(GetWorld());
        for (int32 i = 0; i < NumLocalPlayer; ++i) {
            APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), i);
            if (PlayerController && PlayerController->IsLocalPlayerController()) {
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
        if(LocalCharacter && LinkedPortal) {
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

void APortal::UpdateCaptureCameras() {
    UCameraComponent* CameraComp = LocalCharacter->GetCameraComponent();
    PlayerSimulator->SetWorldLocation(CameraComp->GetComponentLocation());

    // 处理Portal
    UpdatePortal(DoorCapture, LinkedPortal->PlayerSimulator);
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
