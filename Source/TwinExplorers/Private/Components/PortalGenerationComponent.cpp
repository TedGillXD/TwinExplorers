// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PortalGenerationComponent.h"

#include "Camera/CameraComponent.h"
#include "Characters/MainCharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Objects/Portal.h"

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

bool UPortalGenerationComponent::CheckRoom(const FHitResult& HitResult, FVector& ValidLocation, FRotator& ValidRotation) {
	// TODO: 检测目标位置是否上下左右都有足够的空间，如果没有，则找到一个，否则返回false
	return false;
}

bool UPortalGenerationComponent::CheckOverlap(const FVector& NewLocation, const FRotator& NewRotation) {
	// TODO: 检测是否存在传送门重叠的情况出现
	return false;
}

void UPortalGenerationComponent::SpawnPortalAtLocationAndRotation_Implementation(const FVector& NewLocation,
                                                                                 const FRotator& NewRotation) {
	if(bIsGenerating) { return; }
	bIsGenerating = true;
	FTransform Transform;
	Transform.SetLocation(NewLocation);
	Transform.SetRotation(NewRotation.Quaternion());
	Transform.SetScale3D({1.0, 1.0, 1.0});
	PortalRef = GetWorld()->SpawnActorDeferred<APortal>(PortalClass, Transform, Owner, Owner, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
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
