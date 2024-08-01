// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/Portal.h"

#include "Camera/CameraComponent.h"
#include "Characters/MainCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Interfaces/TransportableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APortal::APortal()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(AsRoot);

	Door1Root = CreateDefaultSubobject<USceneComponent>(TEXT("Door1Root"));
	Door1Root->SetupAttachment(GetRootComponent());

	Door1MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door1MeshComp"));
	Door1MeshComp->SetupAttachment(Door1Root);

	Door2Root = CreateDefaultSubobject<USceneComponent>(TEXT("Door2Root"));
	Door2Root->SetupAttachment(GetRootComponent());

	Door2MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Door2MeshComp"));
	Door2MeshComp->SetupAttachment(Door2Root);

	CameraRoot1 = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot1"));
	CameraRoot1->SetupAttachment(Door1Root);
	CameraRoot1->SetRelativeRotation({0.0, 180.0, 0.0});

	CameraRoot2 = CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot2"));
	CameraRoot2->SetupAttachment(Door2Root);
	CameraRoot2->SetRelativeRotation({0.0, 180.0, 0.0});

	Door1Capture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Door1Capture"));
	Door1Capture->SetupAttachment(CameraRoot1);

	Door2Capture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Door2Capture"));
	Door2Capture->SetupAttachment(CameraRoot2);

	PlayerSimulatorDoor1 = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerSimDoor1"));
	PlayerSimulatorDoor1->SetupAttachment(Door1Root);
	
	PlayerSimulatorDoor2 = CreateDefaultSubobject<USceneComponent>(TEXT("PlayerSimDoor2"));
	PlayerSimulatorDoor2->SetupAttachment(Door2Root);

	Portal1BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Portal1Box"));
	Portal1BoxComp->SetupAttachment(Door1Root);
	Portal1BoxComp->OnComponentBeginOverlap.AddDynamic(this, &APortal::Portal1BoxOverlapBeginEvent);
	Portal1BoxComp->OnComponentEndOverlap.AddDynamic(this, &APortal::PortalBoxOverlapEndEvent);

	Portal2BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Portal2Box"));
	Portal2BoxComp->SetupAttachment(Door2Root);
	Portal2BoxComp->OnComponentBeginOverlap.AddDynamic(this, &APortal::Portal2BoxOverlapBeginEvent);
	Portal2BoxComp->OnComponentEndOverlap.AddDynamic(this, &APortal::PortalBoxOverlapEndEvent);
	
}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();

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

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 更新是放在本地的
	if(LocalCharacter) {
		UpdateCaptureCameras();
	}
}

void APortal::UpdateCaptureCameras() {
	UCameraComponent* CameraComp = LocalCharacter->GetCameraComponent();
	PlayerSimulatorDoor1->SetWorldLocation(CameraComp->GetComponentLocation());
	PlayerSimulatorDoor2->SetWorldLocation(CameraComp->GetComponentLocation());

	// 处理Portal 1
	UpdatePortal(Door1Capture, PlayerSimulatorDoor2);

	// 处理Portal 2
	UpdatePortal(Door2Capture, PlayerSimulatorDoor1);
}

void APortal::UpdatePortal(USceneCaptureComponent2D* SceneCapture, const USceneComponent* PlayerSimulator) {
	SceneCapture->SetRelativeLocation(PlayerSimulator->GetRelativeLocation());
	FRotator&& Door1Rotator = UKismetMathLibrary::FindLookAtRotation(SceneCapture->GetComponentLocation(), SceneCapture->GetAttachParent()->GetComponentLocation());
	SceneCapture->SetWorldRotation(Door1Rotator);

	FVector&& Location = SceneCapture->GetRelativeLocation();
	// Location.Size()表示从SceneCapture到CameraRoot的距离，因为是RelativeLocation
	// Clamp将角度限制为5.f到180.f，因为FOV的最小值是5，超过180就看不到了
	float Degree = FMath::RadiansToDegrees(FMath::Atan(300.f / FMath::Max(Location.Size(), 1.f)));
	SceneCapture->FOVAngle = FMath::Clamp(Degree, 5.f, 180.f);
}

void APortal::TransportInternal(UPrimitiveComponent* Trigger, AActor* OtherActor, UBoxComponent* TargetBoxComponent, const FRotator& TargetRotation) {
	if(!OtherActor->Implements<UTransportableInterface>()) {
		return;
	}

	if(Trigger == ExitCollider) {
		return;
	}
	
	ExitCollider = TargetBoxComponent;		// 设置出口组件
	ITransportableInterface::Execute_Transport(OtherActor, TargetBoxComponent->GetComponentLocation(), TargetRotation);
}

void APortal::Portal1BoxOverlapBeginEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if(!HasAuthority()) {
		TransportInternal(OverlappedComponent, OtherActor, Portal2BoxComp, Door2Capture->GetComponentRotation());
	}
}

void APortal::Portal2BoxOverlapBeginEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if(!HasAuthority()) {
		TransportInternal(OverlappedComponent, OtherActor, Portal1BoxComp, Door1Capture->GetComponentRotation());
	}
}

void APortal::PortalBoxOverlapEndEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if(OverlappedComponent == ExitCollider) {
		ExitCollider = nullptr;
	}
}