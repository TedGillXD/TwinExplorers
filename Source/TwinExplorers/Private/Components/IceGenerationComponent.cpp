// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/IceGenerationComponent.h"

#include "KismetTraceUtils.h"
#include "Camera/CameraComponent.h"
#include "Characters/MainCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Objects/IcePillar.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UIceGenerationComponent::UIceGenerationComponent()
{
		// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bIsInitialized = false;
	bIsChecking = false;
	bCanSpawn = false;
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

void UIceGenerationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// 只有在检查中才会运行
	if(bIsChecking) {
		bool OldCanSpawn = bCanSpawn;
		FHitResult HitResult;
		FVector Start = Owner->GetCameraComponent()->GetComponentLocation();
		FVector End = Start + Owner->GetCameraComponent()->GetForwardVector() * DetectLength;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Owner);
		bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, SurfaceTraceChannel, Params);
		
		if(!ForDetectionStaticMeshComp) {
			UE_LOG(LogTemp, Error, TEXT("Create mesh for detection FAILED!"));
			return;
		}

		// 判断本轮是否能生成
		if(bIsHit) {
			ForDetectionStaticMeshComp->SetVisibility(true);
			ForDetectionStaticMeshComp->SetWorldLocation(HitResult.Location);
			ForDetectionStaticMeshComp->SetWorldRotation(Owner->GetCapsuleComponent()->GetComponentQuat());
		
			// 在对应位置执行Overlap检测
			TArray<AActor*> OverlappingActors;
			ForDetectionStaticMeshComp->GetOverlappingActors(OverlappingActors);
			// 根据规则清除Overlapping中的Actors
			OverlappingActors.RemoveAllSwap([](const AActor* Actor) -> bool {
				return Actor->IsA<AMainCharacterBase>();		// TODO: 还要去掉水体Actor，这个暂时还没做
			});
			
			// 2. 如果合法，则将bCanSpawn设置为true，如果非法，则将bCanSpawn设置为false
			if(OverlappingActors.IsEmpty()) {
				bCanSpawn = true;
			} else {
				bCanSpawn = false;
			}
		} else { // 没命中，不能生成
			bCanSpawn = false;
			ForDetectionStaticMeshComp->SetVisibility(false);
		}

		// 在发生变化时更换材质
		if(OldCanSpawn != bCanSpawn) { UpdateDetectorMaterial(); }
	}
}

void UIceGenerationComponent::DoSpawnCheck() {
	// 生成检测的StaticMeshComp，绑定到角色的摄像机，然后查看是否有overlap的情况
	ForDetectionStaticMeshComp = NewObject<UStaticMeshComponent>(Owner, TEXT("ForDetectionStaticMesh"));
	if(!ForDetectionStaticMeshComp) {
		UE_LOG(LogTemp, Error, TEXT("Create mesh for detection FAILED!"));
		return;
	}

	// 设置MeshComp
	ForDetectionStaticMeshComp->SetStaticMesh(CheckSpaceStaticMesh);
	ForDetectionStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ForDetectionStaticMeshComp->SetCollisionResponseToAllChannels(ECR_Overlap);
	ForDetectionStaticMeshComp->SetGenerateOverlapEvents(true);

	// 设置材质
	UpdateDetectorMaterial();

	// 注册组件到角色身上并将其绑定到摄像机上
	ForDetectionStaticMeshComp->RegisterComponent();
	ForDetectionStaticMeshComp->AttachToComponent(Owner->GetCameraComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	bIsChecking = true;
}

void UIceGenerationComponent::CancelCheck() {
	bIsChecking = false;
	bCanSpawn = false;

	// 移除因为需要Checking而生成的Component
	if(ForDetectionStaticMeshComp) {
		ForDetectionStaticMeshComp->DestroyComponent();
	}
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
	
	// 销毁用来检测的Comp
	CancelCheck();
}

void UIceGenerationComponent::GenerateIceOnServer_Implementation(bool bIsHit, const FHitResult& HitResult) {
	GenerateIceInternal(bIsHit, HitResult);
}

void UIceGenerationComponent::OnPillarDestroy() {
	GeneratedIce = nullptr;
}

void UIceGenerationComponent::GenerateIceInternal(bool bIsHit, const FHitResult& HitResult) {
	// 如果现在已经有了，破坏上一个
	if(GeneratedIce) {
		GeneratedIce->Destroy();
		GeneratedIce = nullptr;
	}

	// 没命中，直接返回
	if(!bIsHit) { return; }

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(HitResult.ImpactPoint);
	SpawnTransform.SetRotation(Owner->GetCapsuleComponent()->GetComponentQuat());
	SpawnTransform.SetScale3D({1.f, 1.f, 1.f});
	GeneratedIce = GetWorld()->SpawnActorDeferred<AIcePillar>(IceActorClass, SpawnTransform, Owner, Owner, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	// do some settings
	if(GeneratedIce) {
		GeneratedIce->OnPillarDestroy.AddDynamic(this, &UIceGenerationComponent::OnPillarDestroy);
		GeneratedIce->FinishSpawning(SpawnTransform);
	}
}


void UIceGenerationComponent::UpdateDetectorMaterial() const {
	if(bCanSpawn) {
		UpdateMaterial(SpawnableMaterial);
	} else {
		UpdateMaterial(UnspawnableMaterial);
	}
}

void UIceGenerationComponent::UpdateMaterial(UMaterialInterface* NewMaterial) const {
	const int32 MaterialCount = ForDetectionStaticMeshComp->GetNumMaterials();
	for(int32 Index = 0; Index < MaterialCount; ++Index) {
		ForDetectionStaticMeshComp->SetMaterial(0, NewMaterial);
	}
}

void UIceGenerationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UIceGenerationComponent, GeneratedIce, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UIceGenerationComponent, Owner, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UIceGenerationComponent, bIsInitialized, COND_OwnerOnly);
}
