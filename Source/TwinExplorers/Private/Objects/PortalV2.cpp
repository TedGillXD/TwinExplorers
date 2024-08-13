// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/PortalV2.h"

#include "Camera/CameraComponent.h"
#include "Characters/MainCharacterBase.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APortalV2::APortalV2()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(AsRoot);

	PortalPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalPlane"));
	PortalPlane->SetupAttachment(GetRootComponent());

	PortalCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("PortalCamera"));
	PortalCamera->SetupAttachment(GetRootComponent());

	ForwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardDirection"));
	ForwardDirection->SetupAttachment(GetRootComponent());

	OverlapDetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapDetectionBox"));
	OverlapDetectionBox->SetupAttachment(GetRootComponent());

	PlayerDetection = CreateDefaultSubobject<UBoxComponent>(TEXT("PlayerDetection"));
	PlayerDetection->SetupAttachment(GetRootComponent());
	PlayerDetection->OnComponentBeginOverlap.AddDynamic(this, &APortalV2::TriggerTeleport);
	PlayerDetection->OnComponentEndOverlap.AddDynamic(this, &APortalV2::LeavePortal);
	
	bIsInit = false;
	bIsEnabled = true;
}

// Called when the game starts or when spawned
void APortalV2::BeginPlay() {
	Super::BeginPlay();

	// 让这个Actor在最后更新，否则会出现抖动
	this->SetTickGroup(TG_PostUpdateWork);
	if(!HasAuthority()) {
		if(LinkedPortal) {
			Init();
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

// Called every frame
void APortalV2::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	// // 在这里检测是否存在另外一个Portal，如果存在则绑定
	// // 为什么不在BeginPlay执行呢？因为根据RPC规则，如果是通过通知的方式需要在Client1发送链接到服务器然后再从服务器复制到Client2，然后Client2得到通知再发送链接请求到服务器到Client1，总共需要Server <-> Client进行4次通信
	// // 而改成在Tick中，就能减少为两次
	// if(HasAuthority()) {
	// 	if(!LinkedPortal) {
	// 		// 尝试获取另外一个Portal
	// 		TArray<AActor*> FoundActors;
	// 		UGameplayStatics::GetAllActorsOfClass(GetWorld(), StaticClass(), FoundActors);
	// 		for(AActor* Actor : FoundActors) {
	// 			if(Actor != this) {
	// 				APortalV2* OtherPortal = Cast<APortalV2>(Actor);
	// 				LinkPortal(OtherPortal);       // 绑定传送门
	// 			}
	// 		}
	// 	}
	// }

	if(!HasAuthority()) {
		if(LocalCharacter && LinkedPortal && bIsInit) {
			// 获取当前优化级别
			EOptimizedLevel CurrentLevel = GetOptimizationLevel(LocalCharacter->GetCameraComponent()->GetComponentTransform(), LocalCharacter->GetCameraComponent()->GetForwardVector(), LocalCharacter->GetCameraComponent()->FieldOfView);
			// GEngine->AddOnScreenDebugMessage(-1, 0.0, FColor::Blue, GetName() + (bIsEnabled ? " : True" : " : False") + "  " + FString::FromInt(CurrentLevel));
			// 仅在优化级别变化时更新状态
			if (CurrentLevel != LastLevel) {
				SetCurrentOptimizationLevel(CurrentLevel);
			}
			
			if(bIsEnabled) {
				UpdateSceneCapture(LocalCharacter->GetCameraComponent()->GetComponentTransform());
			}
			
			FVector2D CurrentViewportSize;
			GEngine->GameViewport->GetViewportSize(CurrentViewportSize);

			if (!CurrentViewportSize.Equals(LastViewportSize)) {
				DoViewportResize();
				LastViewportSize = CurrentViewportSize;  // 更新缓存的视口大小
			}
		} 
	}
}

FVector APortalV2::CalculateTargetAxes(FVector X) const {
	FVector temp = UKismetMathLibrary::InverseTransformDirection(LinkedPortal->GetActorTransform(), X);
	temp = UKismetMathLibrary::MirrorVectorByNormal(temp, FVector{ 1.0, 0.0, 0.0 });
	return UKismetMathLibrary::MirrorVectorByNormal(temp, FVector{ 0.0, 1.0, 0.0 });
}

void APortalV2::TriggerTeleport(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	if(HasAuthority() && LinkedPortal && !bIsTeleporting) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, "Teleport!");
		if(!OtherActor->Implements<UTransportableInterface>()) { return; }
		LinkedPortal->MarkTeleporting();

		FVector TargetLocation = LinkedPortal->PlayerDetection->GetComponentLocation();
		FVector TargetVelocity = GetUpdatedVelocity(ITransportableInterface::Execute_GetOriginalVelocity(OtherActor));
		
		FRotator OriginalRotation = OtherActor->GetActorRotation();
		FRotator InverseRotation =	OriginalRotation.Vector().MirrorByVector(GetActorForwardVector()).MirrorByVector(GetActorRightVector()).Rotation();
		FRotator PortalRotationDelta = LinkedPortal->GetActorRotation() - GetActorRotation();
		FRotator TargetRotation = UKismetMathLibrary::ComposeRotators(PortalRotationDelta, InverseRotation);

		ITransportableInterface::Execute_Transport(OtherActor, TargetLocation, TargetRotation, TargetVelocity);
	}
}

void APortalV2::LeavePortal(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	bIsTeleporting = false;
}

void APortalV2::Relink(APortalV2* Portal1, APortalV2* Portal2) {
	// 将两个传送门进行链接
	if (!Portal1 || !Portal2 || Portal1 == Portal2) { return; }

	if(Portal1->HasAuthority() && Portal2->HasAuthority()) {
		Portal1->LinkPortal(Portal2);
		Portal2->LinkPortal(Portal1);
	}
}

void APortalV2::Init() {
	if(!PortalMatInstance) {
		PortalMatInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), PortalMat);
		if(!PortalMatInstance) { return; }
		PortalPlane->SetMaterial(0, PortalMatInstance);
	}

	if(!PortalRT) {
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		PortalRT = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), ViewportSize.X, ViewportSize.Y);
		if(!PortalRT) { return; }
	}

	PortalMatInstance->SetTextureParameterValue(FName("Texture"), PortalRT);
	// TODO: 修改门框颜色
	
	LinkedPortal->PortalCamera->TextureTarget = PortalRT;
	SetClipPlane();
	bIsInit = true;
}

void APortalV2::LinkPortalOnServer_Implementation(APortalV2* OtherPortal) {
	LinkPortal(OtherPortal);
}

void APortalV2::LinkPortal(APortalV2* OtherPortal) {
	if(HasAuthority()) {
		LinkedPortal = OtherPortal;
		OtherPortal->LinkedPortal = this;
	} else {
		LinkPortalOnServer(OtherPortal);
	}
}

void APortalV2::OnRep_LinkedPortal() {
	if(LinkedPortal) {
		this->Init();
	}
}

void APortalV2::SetClipPlane() const {
	if(!LinkedPortal) { return; }

	PortalCamera->bEnableClipPlane = true;
	PortalCamera->ClipPlaneBase = PortalPlane->GetComponentLocation() - ForwardDirection->GetForwardVector() * 0.f;
	PortalCamera->ClipPlaneNormal = ForwardDirection->GetForwardVector();
}

void APortalV2::UpdateSceneCapture(const FTransform& CameraTransform) const {
	// 计算目标位置
	FTransform Current = GetActorTransform();
	FTransform temp;
	temp.SetLocation(Current.GetLocation());
	temp.SetRotation(Current.GetRotation());
	temp.SetScale3D(FVector{ Current.GetScale3D().X * -1.f, Current.GetScale3D().Y * -1.f, Current.GetScale3D().Z });
	FVector InverseLocation = UKismetMathLibrary::InverseTransformLocation(temp, CameraTransform.GetLocation());
	FVector FinalLocation = UKismetMathLibrary::TransformLocation(LinkedPortal->GetActorTransform(), InverseLocation);

	// 计算目标旋转
	FRotator CharacterRotation = CameraTransform.GetRotation().Rotator();
	FVector X, Y, Z;
	UKismetMathLibrary::BreakRotIntoAxes(CharacterRotation, X, Y, Z);
	FRotator FinalRotation = UKismetMathLibrary::MakeRotationFromAxes(GetTargetRotationAxe(X), GetTargetRotationAxe(Y), GetTargetRotationAxe(Z));
	
	LinkedPortal->PortalCamera->SetWorldLocationAndRotation(FinalLocation, FinalRotation);
}

FVector APortalV2::GetTargetRotationAxe(const FVector& Axe) const {
	FTransform ThisPortalTransform = GetActorTransform(), LinkedPortalTransform = LinkedPortal->GetActorTransform();
	FVector temp = UKismetMathLibrary::InverseTransformDirection(ThisPortalTransform, Axe);
	temp = UKismetMathLibrary::MirrorVectorByNormal(temp, FVector{ 1.0, 0.0, 0.0 });
	temp = UKismetMathLibrary::MirrorVectorByNormal(temp, FVector{ 0.0, 1.0, 0.0 });
	return UKismetMathLibrary::TransformDirection(LinkedPortalTransform, temp);
}

void APortalV2::DoViewportResize() const {
	if (!bIsEnabled) { return; }
    
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);

	if (ViewportSize.X <= 0 || ViewportSize.Y <= 0) {
		return;
	}

	// 获取当前的缩放比例，这个缩放比例是基于距离动态调整的
	float Scale = GetScale();
	int32 TargetWidth = FMath::RoundToInt(ViewportSize.X * Scale);
	int32 TargetHeight = FMath::RoundToInt(ViewportSize.Y * Scale);

	// 如果大小不合法，返回
	if (TargetWidth <= 0 || TargetHeight <= 0) {
		return;
	}

	// 仅当目标尺寸与当前尺寸不同，且超过一定阈值时，才进行Resize
	constexpr int32 ResizeThreshold = 2; // 设置一个阈值，防止过于频繁的微小调整
	if (FMath::Abs(PortalRT->SizeX - TargetWidth) > ResizeThreshold || FMath::Abs(PortalRT->SizeY - TargetHeight) > ResizeThreshold) {
		PortalRT->ResizeTarget(TargetWidth, TargetHeight);
	}
}


FVector APortalV2::GetUpdatedVelocity(const FVector& OriginalVelocity) {
	return GetTargetRotationAxe(OriginalVelocity.GetSafeNormal()) * OriginalVelocity.Length();
}

void APortalV2::MarkTeleportingOnServer_Implementation() {
	bIsTeleporting = true;
}

void APortalV2::MarkTeleporting() {
    if(HasAuthority()) {
        MarkTeleportingOnServer();
    } else {
    	MarkTeleportingOnServer();
    }
}

EOptimizedLevel APortalV2::GetOptimizationLevel(const FTransform& CameraTransform, const FVector& CameraForward, int CameraFOV) const {
	// 可见性检查
	FVector CameraToPortal = (this->GetActorLocation() - CameraTransform.GetLocation());
	CameraToPortal.Normalize();
	const float HalfFOVRadians = FMath::DegreesToRadians(CameraFOV + 20.f / 2.0f);
	const float CosHalfFOV = FMath::Cos(HalfFOVRadians);
	const float ViewDotProduct = FVector::DotProduct(CameraForward, CameraToPortal);
	if (ViewDotProduct < CosHalfFOV) {
		return Level3;  // Portal不在视野范围内
	}
	
	// 判断Portal到角色摄像机和Portal法线之间的夹角
	FVector PortalToCamera = CameraTransform.GetLocation() - this->GetActorLocation();
	PortalToCamera.Z = 0.0;
	PortalToCamera.Normalize();
	const float DotProduct = FVector::DotProduct(PortalToCamera, this->GetActorForwardVector());
	const float Distance = (CameraTransform.GetLocation() - this->GetActorLocation()).Length();

	// 大于等于90°
	if (DotProduct < 0) {
		return Level3;
	}

	// 大于45°
	if (DotProduct <= 0.707) {
		if (Distance < 1000.f) { return Level1; }  // 中高精度
		return Level2;
		// return Level3;  // 低精度
	}

	// 小于45°
	if (Distance < 1000.f) { return Level0; }  // 高精度
	if (Distance < 2000.f) { return Level1; }  // 中高精度
	return Level2;
}

void APortalV2::EnableSceneCapture() {
	if(bIsEnabled) { return; }
	// PortalCamera->SetComponentTickEnabled(true);
	// PortalCamera->SetVisibility(true);
	// PortalCamera->bCaptureEveryFrame = true;
	// PortalCamera->bCaptureOnMovement = true;
	// PortalCamera->bAlwaysPersistRenderingState = true;
	// PortalCamera->CaptureScene(); // 手动捕捉一次
	bIsEnabled = true;
}

void APortalV2::DisableSceneCapture() {
	if(LinkedPortal) {
		LinkedPortal->PortalRT->ResizeTarget(1.f, 1.f);  // 设置为1.f
	}
}

void APortalV2::SetCurrentOptimizationLevel(EOptimizedLevel OptimizedLevel) {
	switch (OptimizedLevel) {
	case Level0:
		LinkedPortal->EnableSceneCapture();
		LinkedPortal->SetToLevel0Resolution();
		break;
	case Level1:
		LinkedPortal->EnableSceneCapture();
		LinkedPortal->SetToLevel1Resolution();
		break;
	case Level2:
		LinkedPortal->EnableSceneCapture();
		LinkedPortal->SetToLevel2Resolution();
		break;
	case Level3:  // 不渲染
	default:
		LinkedPortal->DisableSceneCapture();
		break;
	}

	LastLevel = OptimizedLevel;
}

void APortalV2::SetToLevel0Resolution() {
	FVector2D Size;
	GEngine->GameViewport->GetViewportSize(Size);
	if(LinkedPortal) {
		LinkedPortal->PortalRT->ResizeTarget(Size.X, Size.Y);  // 使用原始分辨率
	}
}

void APortalV2::SetToLevel1Resolution() {
	FVector2D Size;
	GEngine->GameViewport->GetViewportSize(Size);
	if(LinkedPortal) {
		LinkedPortal->PortalRT->ResizeTarget(Size.X * 0.5f, Size.Y * 0.5f);  // 原始分辨率的0.5
	}
}

void APortalV2::SetToLevel2Resolution() {
	FVector2D Size;
	GEngine->GameViewport->GetViewportSize(Size);
	if(LinkedPortal) {
		LinkedPortal->PortalRT->ResizeTarget(Size.X * 0.5f, Size.Y * 0.5f);  // 分辨率调整为原始的0.5，并降低采样帧率
	}
}

float APortalV2::GetScale() const {
	if(LastLevel == Level0) { return 1; }
	if(LastLevel == Level1) { return 0.5; }
	if(LastLevel == Level2) { return 0.25; }
	return 0.f;
}

void APortalV2::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APortalV2, LinkedPortal);
	DOREPLIFETIME(APortalV2, bIsTeleporting);
}
