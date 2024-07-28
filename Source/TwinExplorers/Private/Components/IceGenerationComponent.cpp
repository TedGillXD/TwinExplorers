// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/IceGenerationComponent.h"

#include "Camera/CameraComponent.h"
#include "Characters/MainCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UIceGenerationComponent::UIceGenerationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	bIsInitialized = false;
	bIsChecking = true;
	bCanSpawn = true;		// 暂时设置为true用来测试
	DetectLength = 1000.f;
}


// Called when the game starts
void UIceGenerationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if(GetOwnerRole() == ROLE_Authority) {
		Owner = Cast<AMainCharacterBase>(GetOwner());
		if(Owner) {
			bIsInitialized = true;
		}
	}
}

void UIceGenerationComponent::DoSpawnCheck() {
	bIsChecking = true;

	// TODO: 生成检测的Actor，然后查看是否有overlap的情况
}

void UIceGenerationComponent::CancelCheck() {
	bIsChecking = false;

	// TODO: 移除因为需要Checking而生成的Actor
}

void UIceGenerationComponent::GenerateNewIce() {
	if(!bIsInitialized) { return; }
	if(!bCanSpawn) { return; }				// 目前检测到不能生成，直接返回

	FHitResult HitResult;
	FVector Start = Owner->GetCameraComponent()->GetComponentLocation();
	FVector End = Start + Owner->GetCameraComponent()->GetForwardVector() * DetectLength;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, SurfaceTraceChannel, Params);

	if(GetOwnerRole() == ROLE_Authority) {		// 在服务器
		GenerateIceInternal(bIsHit, HitResult);
	} else {			// 在客户端
		GenerateIceOnServer(bIsHit, HitResult);
	}
}

void UIceGenerationComponent::GenerateIceOnServer_Implementation(bool bIsHit, const FHitResult& HitResult) {
	GenerateIceInternal(bIsHit, HitResult);
}

void UIceGenerationComponent::GenerateIceInternal(bool bIsHit, const FHitResult& HitResult) {
	// 如果现在已经有了，那就返回，破坏需要另一个工具
	if(GeneratedIce) { return; }

	// 没命中，直接返回
	if(!bIsHit) { return; }

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(HitResult.ImpactPoint);
	SpawnTransform.SetRotation(Owner->GetCapsuleComponent()->GetComponentQuat());
	SpawnTransform.SetScale3D({1.f, 1.f, 1.f});
	GeneratedIce = GetWorld()->SpawnActorDeferred<AActor>(IceActorClass, SpawnTransform, Owner, Owner, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	// do some settings
	
	GeneratedIce->FinishSpawning(SpawnTransform);
}

void UIceGenerationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UIceGenerationComponent, GeneratedIce, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UIceGenerationComponent, Owner, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UIceGenerationComponent, bIsInitialized, COND_OwnerOnly);
}