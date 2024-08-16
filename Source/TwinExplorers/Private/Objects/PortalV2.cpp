// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/PortalV2.h"

#include "Camera/CameraComponent.h"
#include "Characters/MainCharacterBase.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CustomRateCaptureComponent2D.h"
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

	PortalCamera = CreateDefaultSubobject<UCustomRateCaptureComponent2D>(TEXT("PortalCamera"));
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
	bCanBeOptimized = false;
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

	if(!HasAuthority()) {
		if(LocalCharacter && LinkedPortal && bIsInit && LinkedPortal->bIsInit) {
			// 获取当前优化级别
			EOptimizedLevel CurrentLevel = GetOptimizationLevel(LocalCharacter->GetCameraComponent()->GetComponentTransform(), LocalCharacter->GetCameraComponent()->GetForwardVector(), LocalCharacter->GetCameraComponent()->FieldOfView);
			GEngine->AddOnScreenDebugMessage(-1, 0.0, FColor::Blue, GetName() + (bIsEnabled ? " : True" : " : False") + "  " + FString::FromInt(CurrentLevel));
			// 仅在优化级别变化时更新状态
			if (CurrentLevel != LastLevel && bCanBeOptimized) {
				SetCurrentOptimizationLevel(CurrentLevel);
			}
			GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([this]() -> void {
				this->bCanBeOptimized = true;
			}));
			
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
		if(!OtherActor->IsA<AMainCharacterBase>()) { return; }			// 现在只有角色能传送了
		AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(OtherActor);
		if(CharacterBase->bIsTeleporting) { return; }
		LinkedPortal->MarkTeleporting();

		FVector TargetLocation = LinkedPortal->PlayerDetection->GetComponentLocation();
		FVector TargetVelocity = GetUpdatedVelocity(ITransportableInterface::Execute_GetOriginalVelocity(OtherActor));
		
		FRotator OriginalRotation = CharacterBase->GetActorRotation();
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

void APortalV2::SetRingColor(const FLinearColor& Color) {
	// 这个函数运行在服务器
	RingColor = Color;
}

void APortalV2::UpdateRingColor(const FLinearColor& Color) {
	UMaterialInterface* Material = PortalPlane->GetMaterial(0);
	if (!Material) {
		UE_LOG(LogTemp, Error, TEXT("Material of PortalPlane is nullptr"));
		return;
	}

	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(Material);
	if (!MaterialInstance) {
		// 如果当前材质不是UMaterialInstanceDynamic，则创建一个新的动态实例
		MaterialInstance = UMaterialInstanceDynamic::Create(Material, this);
		if (MaterialInstance) {
			// 将新创建的动态实例应用到网格上
			PortalPlane->SetMaterial(0, MaterialInstance);
		} else {
			UE_LOG(LogTemp, Error, TEXT("Failed to create a dynamic material instance"));
			return;
		}
	}

	// 设置RingColor参数
	MaterialInstance->SetVectorParameterValue(FName("RingColor"), Color);
}

void APortalV2::Relink(APortalV2* Portal1, APortalV2* Portal2, FLinearColor NewColor) {
	// 将两个传送门进行链接
	if (!Portal1 || !Portal2 || Portal1 == Portal2) { return; }

	if(Portal1->HasAuthority() && Portal2->HasAuthority()) {
		Portal1->LinkPortal(Portal2);
		Portal1->SetRingColor(NewColor);
		Portal2->SetRingColor(NewColor);
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
	PortalMatInstance->SetVectorParameterValue(FName("RingColor"), RingColor);
	
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
		UE_LOG(LogTemp, Warning, TEXT("LinkPortal on Server"));
	} else {
		LinkPortalOnServer(OtherPortal);
		UE_LOG(LogTemp, Warning, TEXT("LinkPortal on Client"));
	}
}

void APortalV2::OnRep_LinkedPortal() {
	if(LinkedPortal) {
		this->Init();
	}
}

void APortalV2::OnRep_RingColor() {
	UpdateRingColor(RingColor);
	UE_LOG(LogTemp, Warning, TEXT("Update ring color"));
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
	// if(bIsEnabled) { return; }
	// // PortalCamera->Enable();
	// bIsEnabled = true;
}

void APortalV2::DisableSceneCapture() {
	// if(!bIsEnabled) { return; }
	// // PortalCamera->Disable();
	// bIsEnabled = false;
}

void APortalV2::SetCurrentOptimizationLevel(EOptimizedLevel OptimizedLevel) {
	if(!LinkedPortal) { return; }
	
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

void APortalV2::SetToLevel0Resolution() const {
	FVector2D Size;
	GEngine->GameViewport->GetViewportSize(Size);
	ResizeTextureToMatchViewport(Size);
	LinkedPortal->PortalCamera->ChangeFramePerSec(40);
}

void APortalV2::SetToLevel1Resolution() const {
	FVector2D Size;
	GEngine->GameViewport->GetViewportSize(Size);
	ResizeTextureToMatchViewport(Size / 2);
	LinkedPortal->PortalCamera->ChangeFramePerSec(30);
}

void APortalV2::SetToLevel2Resolution() const {
	FVector2D Size;
	GEngine->GameViewport->GetViewportSize(Size);
	ResizeTextureToMatchViewport(Size / 2);
	LinkedPortal->PortalCamera->ChangeFramePerSec(25);
}

void APortalV2::ResizeTextureToMatchViewport(const FVector2D& DesiredSize) const {
	if (!LinkedPortal || !LinkedPortal->PortalRT) return;

	int32 CurrentWidth = LinkedPortal->PortalRT->SizeX;
	int32 CurrentHeight = LinkedPortal->PortalRT->SizeY;
	
	if (DesiredSize.X != CurrentWidth || DesiredSize.Y != CurrentHeight) {
		LinkedPortal->PortalRT->ResizeTarget(DesiredSize.X, DesiredSize.Y);
		LinkedPortal->PortalCamera->CaptureSceneDeferred();
	}
}

float APortalV2::GetScale() const {
	if(LastLevel == Level0) { return 1; }
	if(LastLevel == Level1) { return 0.5; }
	if(LastLevel == Level2) { return 0.5; }
	return 0.f;
}

void APortalV2::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APortalV2, LinkedPortal);
	DOREPLIFETIME(APortalV2, bIsTeleporting);
	DOREPLIFETIME(APortalV2, RingColor);
}
