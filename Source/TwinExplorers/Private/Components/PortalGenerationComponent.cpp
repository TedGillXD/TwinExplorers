// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PortalGenerationComponent.h"

#include "Camera/CameraComponent.h"
#include "Characters/MainCharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Objects/PortalV2.h"

// Sets default values for this component's properties
UPortalGenerationComponent::UPortalGenerationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
	DetectionDistance = 2000.f;

	bIsInitialized = false;
	bIsGenerating = false;
}


// Called when the game starts
void UPortalGenerationComponent::BeginPlay()
{
	Super::BeginPlay();

	if(GetOwnerRole() == ROLE_Authority) {
		Owner = Cast<AMainCharacterBase>(GetOwner());
		bIsInitialized = true;
	}
	
}

void UPortalGenerationComponent::Shoot() {
	// 客户端执行射线检测
	if(!bIsInitialized) { return; }	// 没初始化的情况下不执行
	
	FVector Start = Owner->GetCameraComponent()->GetComponentLocation();
	FVector End = Start + Owner->GetCameraComponent()->GetForwardVector() * DetectionDistance;
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	if(GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, SurfaceType, Params)) {		// 只有命中才会执行
		// 检测当前探测到的位置是否合法
		FVector NewLocation = HitResult.ImpactPoint + HitResult.ImpactNormal;		// 加上一个ImpactNormal是为了不要让Portal中的Plane紧挨着墙壁导致Z-fighting的问题
		FRotator NewRotation = UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal);
		if(!CheckRoom(HitResult, NewLocation, NewRotation)) {
			return;
		}
		
		if(!CheckOverlap(NewLocation, NewRotation)) {
			return;
		}
		
		// 生成或者改变Portal的位置，要在服务器上做
		if(PortalRef) {
			ChangePortalLocationAndRotation(NewLocation, NewRotation);
		} else {
			SpawnPortalAtLocationAndRotation(NewLocation, NewRotation);
		}
	}
}

bool UPortalGenerationComponent::CheckRoom(const FHitResult& HitResult, FVector& ValidLocation, const FRotator& ValidRotation, int RecursionDepth) {
	constexpr float PortalHeight = 300.f;
	constexpr float PortalWidth = 170.f;
	
	// 计算根据当前面的法线方向的旋转后的上下左右偏移量
	FQuat PortalQuat = ValidRotation.Quaternion();
	FVector Up = PortalQuat.RotateVector(FVector(0, 0, PortalHeight * 0.5f));
	FVector Down = PortalQuat.RotateVector(FVector(0, 0, -PortalHeight * 0.5f));
	FVector Left = PortalQuat.RotateVector(FVector(0, PortalWidth * 0.5f, 0));
	FVector Right = PortalQuat.RotateVector(FVector(0, -PortalWidth * 0.5f, 0));

	return CheckRoom(HitResult, ValidLocation, 0, Up, Down, Left, Right);
}

bool UPortalGenerationComponent::CheckRoom(const FHitResult& HitResult, FVector& ValidLocation,
	int RecursionDepth, const FVector& Up, const FVector& Down, const FVector& Left, const FVector& Right) {

	if(RecursionDepth >= 10) { return false; }		// 10 = 1 / 0.1(整体 / 每次的偏移量) 这样就能保证在玩家射到最边缘的时候能找到一个最极限的值

	// 检测目标位置是否上下左右都有足够的空间，如果没有，则找到一个，否则返回false
	FVector DetectionMiddle = ValidLocation + 25 * HitResult.Normal;		// 这个是要检测的中心点
	FVector DetectionDir = HitResult.Normal * -50.f;		// 检测的方向和距离

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	// 检查是否已经全部在范围内
	FHitResult UpHit, DownHit, LeftHit, RightHit;
	bool bUpClear = GetWorld()->LineTraceSingleByChannel(UpHit, DetectionMiddle + Up, DetectionMiddle + Up + DetectionDir, SurfaceType, Params);
	bool bDownClear = GetWorld()->LineTraceSingleByChannel(DownHit, DetectionMiddle + Down, DetectionMiddle + Down + DetectionDir, SurfaceType, Params);
	bool bLeftClear = GetWorld()->LineTraceSingleByChannel(LeftHit, DetectionMiddle + Left, DetectionMiddle + Left + DetectionDir, SurfaceType, Params);
	bool bRightClear = GetWorld()->LineTraceSingleByChannel(RightHit, DetectionMiddle + Right, DetectionMiddle + Right + DetectionDir, SurfaceType, Params);

	// 如果本轮检测通过，返回true
	if(bUpClear && bDownClear && bLeftClear && bRightClear) {
		return true;
	}
	
	// 找到一个合适的位置
	FVector Offset = FVector::ZeroVector;
	if (!bUpClear) { Offset += Down * 0.1f; }  // 如果上方空间不足，向下调整
	if (!bDownClear) { Offset += Up * 0.1f; }  // 如果下方空间不足，向上调整
	if (!bLeftClear) { Offset += Right * 0.1f; }  // 如果左方空间不足，向右调整
	if (!bRightClear) { Offset += Left * 0.1f; }  // 如果右方空间不足，向左调整

	ValidLocation += Offset;
	return CheckRoom(HitResult, ValidLocation, 0, Up, Down, Left, Right);
}

bool UPortalGenerationComponent::CheckOverlap(const FVector& NewLocation, const FRotator& NewRotation) const {
	float BoxHeight = 141.f;
	float BoxWidth = 90.f;
	
	// 检测是否存在传送门重叠的情况出现
	FHitResult HitResult;
	const FCollisionShape Box = FCollisionShape::MakeBox(FVector{5 , BoxWidth, BoxHeight });
	return !GetWorld()->SweepSingleByChannel(HitResult, NewLocation, NewLocation, NewRotation.Quaternion(), PortalOverlapDetectionType, Box);
}

void UPortalGenerationComponent::SpawnPortalAtLocationAndRotation_Implementation(const FVector& NewLocation,
                                                                                 const FRotator& NewRotation) {
	if(bIsGenerating) { return; }
	bIsGenerating = true;
	FTransform Transform;
	Transform.SetLocation(NewLocation);
	Transform.SetRotation(NewRotation.Quaternion());
	Transform.SetScale3D({1.0, 1.0, 1.0});
	PortalRef = GetWorld()->SpawnActorDeferred<APortalV2>(PortalClass, Transform, Owner, Owner, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	PortalRef->FinishSpawning(Transform);
	bIsGenerating = false;
}

void UPortalGenerationComponent::ChangePortalLocationAndRotation_Implementation(const FVector& NewLocation,
	const FRotator& NewRotation) {
	if(bIsGenerating) { return; }
	bIsGenerating = true;
	PortalRef->SetActorLocationAndRotation(NewLocation, NewRotation);
	bIsGenerating = false;
}

void UPortalGenerationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UPortalGenerationComponent, Owner, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPortalGenerationComponent, bIsInitialized, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPortalGenerationComponent, PortalRef, COND_OwnerOnly);
}
