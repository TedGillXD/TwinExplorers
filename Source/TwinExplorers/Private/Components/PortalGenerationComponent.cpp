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
	PrimaryComponentTick.bCanEverTick = false;

	DetectionDistance = 2000.f;
	bIsInitialized = false;
	bIsGeneratingPortal1 = false;
	bIsGeneratingPortal2 = false;
}

// Called when the game starts
void UPortalGenerationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwnerRole() == ROLE_Authority) {
		Owner = Cast<AMainCharacterBase>(GetOwner());
		bIsInitialized = true;
	}
}

void UPortalGenerationComponent::ShootPortal1() {
	if (!bIsInitialized) { return; }

	FVector Start = Owner->GetCameraComponent()->GetComponentLocation();
	FVector End = Start + Owner->GetCameraComponent()->GetForwardVector() * DetectionDistance;
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, SurfaceType, Params)) {
		FVector NewLocation = HitResult.ImpactPoint + HitResult.ImpactNormal;
		FRotator NewRotation = UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal);

		if (!CheckRoom(HitResult, NewLocation, NewRotation) || !CheckOverlap(NewLocation, NewRotation)) {
			return;
		}

		if(bCanGeneratePortal1) {
			SpawnPortal1AtLocationAndRotation(NewLocation, NewRotation);
		}
	}
}

void UPortalGenerationComponent::ShootPortal2() {
	if (!bIsInitialized) { return; }

	FVector Start = Owner->GetCameraComponent()->GetComponentLocation();
	FVector End = Start + Owner->GetCameraComponent()->GetForwardVector() * DetectionDistance;
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, SurfaceType, Params)) {
		FVector NewLocation = HitResult.ImpactPoint + HitResult.ImpactNormal;
		FRotator NewRotation = UKismetMathLibrary::MakeRotFromX(HitResult.ImpactNormal);

		if (!CheckRoom(HitResult, NewLocation, NewRotation) || !CheckOverlap(NewLocation, NewRotation)) {
			return;
		}

		if (bCanGeneratePortal2) {
			SpawnPortal2AtLocationAndRotation(NewLocation, NewRotation);
		}
	}
}

void UPortalGenerationComponent::ResetOnServer_Implementation() {
	bCanGeneratePortal1 = true;
	bCanGeneratePortal2 = true;
	PortalRef1 = nullptr;
	PortalRef2 = nullptr;
}

void UPortalGenerationComponent::Reset() {
	// 在服务器上Reset
	ResetOnServer();
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

	if (RecursionDepth >= 10) { 
		return false; 
	}		// 最大递归深度，避免无限递归

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
	if (CheckIfIsARoom(UpHit, DownHit, RightHit, LeftHit)) {
		return true;
	}
	
	// 找到一个合适的位置并判断是否需要调整
	FVector Offset = FVector::ZeroVector;
	if (!bUpClear || (UpHit.GetActor() && UpHit.GetActor() != HitResult.GetActor())) { 
		Offset += Down * 0.1f; 
	}  // 如果上方空间不足或者命中的是不同的Actor，向下调整

	if (!bDownClear || (DownHit.GetActor() && DownHit.GetActor() != HitResult.GetActor())) { 
		Offset += Up * 0.1f; 
	}  // 如果下方空间不足或者命中的是不同的Actor，向上调整

	if (!bLeftClear || (LeftHit.GetActor() && LeftHit.GetActor() != HitResult.GetActor())) { 
		Offset += Right * 0.1f; 
	}  // 如果左方空间不足或者命中的是不同的Actor，向右调整

	if (!bRightClear || (RightHit.GetActor() && RightHit.GetActor() != HitResult.GetActor())) { 
		Offset += Left * 0.1f; 
	}  // 如果右方空间不足或者命中的是不同的Actor，向左调整

	// 更新有效位置并递归调用自身
	ValidLocation += Offset;
	return CheckRoom(HitResult, ValidLocation, RecursionDepth + 1, Up, Down, Left, Right);
}

bool UPortalGenerationComponent::CheckIfIsARoom(const FHitResult& Up, const FHitResult& Down, const FHitResult& Right,
	const FHitResult& Left) {

	// 判断四个是否都命中同一个Actor
	bool bIsSameSurface = Up.GetActor() != nullptr &&
                              Up.GetActor() == Down.GetActor() &&
                              Up.GetActor() == Right.GetActor() &&
                              Up.GetActor() == Left.GetActor();
	
	return Up.bBlockingHit && Down.bBlockingHit && Right.bBlockingHit && Left.bBlockingHit && bIsSameSurface;
}

bool UPortalGenerationComponent::CheckOverlap(const FVector& NewLocation, const FRotator& NewRotation) const {
	float BoxHeight = 141.f;
	float BoxWidth = 90.f;
	
	// 检测是否存在传送门重叠的情况出现
	FHitResult HitResult;
	const FCollisionShape Box = FCollisionShape::MakeBox(FVector{5 , BoxWidth, BoxHeight });
	return !GetWorld()->SweepSingleByChannel(HitResult, NewLocation, NewLocation, NewRotation.Quaternion(), PortalOverlapDetectionType, Box);
}

void UPortalGenerationComponent::SpawnPortal1AtLocationAndRotation_Implementation(const FVector& NewLocation, const FRotator& NewRotation) {
	if (!bCanGeneratePortal1) { return; }
	if (bIsGeneratingPortal1) { return; }
	bIsGeneratingPortal1 = true;

	FTransform Transform;
	Transform.SetLocation(NewLocation);
	Transform.SetRotation(NewRotation.Quaternion());
	Transform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

	PortalRef1 = GetWorld()->SpawnActorDeferred<APortalV2>(PortalClass, Transform, Owner, Owner, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	PortalRef1->FinishSpawning(Transform);

	bCanGeneratePortal1 = false;
	bIsGeneratingPortal1 = false;
}

void UPortalGenerationComponent::SpawnPortal2AtLocationAndRotation_Implementation(const FVector& NewLocation, const FRotator& NewRotation) {
	if (!bCanGeneratePortal2) { return; }
	if (bIsGeneratingPortal2) { return; }
	bIsGeneratingPortal2 = true;

	FTransform Transform;
	Transform.SetLocation(NewLocation);
	Transform.SetRotation(NewRotation.Quaternion());
	Transform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

	PortalRef2 = GetWorld()->SpawnActorDeferred<APortalV2>(PortalClass, Transform, Owner, Owner, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	PortalRef2->FinishSpawning(Transform);

	bCanGeneratePortal2 = false;
	bIsGeneratingPortal2 = false;
}

void UPortalGenerationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UPortalGenerationComponent, Owner, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPortalGenerationComponent, bIsInitialized, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPortalGenerationComponent, PortalRef1, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPortalGenerationComponent, PortalRef2, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPortalGenerationComponent, bCanGeneratePortal1, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UPortalGenerationComponent, bCanGeneratePortal2, COND_OwnerOnly);
}
