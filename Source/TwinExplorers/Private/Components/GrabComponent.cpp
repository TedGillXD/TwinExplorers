// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GrabComponent.h"

#include "KismetTraceUtils.h"
#include "Characters/MainCharacterBase.h"
#include "Camera/CameraComponent.h"
#include "Interfaces/GrabableInterface.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

// Sets default values for this component's properties
UGrabComponent::UGrabComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
	bIsOn = false;
	bIsInitialized = false;
	DetectLength = 1000.f;
}


// Called when the game starts
void UGrabComponent::BeginPlay()
{
	Super::BeginPlay();

	if(GetOwnerRole() == ROLE_Authority) {
		AddRequireComponentsOnServer();
	}
}

void UGrabComponent::GrabItemInternal(const bool bIsHit, const FHitResult& HitResult) {
	if(bIsHit) {
		// 1. 检查Actor是否能被Grab
		if(!HitResult.GetActor()->Implements<UGrabableInterface>()) {		// 实现了Grabable这个接口的才能被抓取
			bIsGrabbing = false;
			return;
		}

		// 2. 将Mesh移动过去
		HeldComponent = HitResult.GetComponent();
		if(!HeldComponent && HeldComponent->GetCollisionObjectType() != ECC_PhysicsBody) {
			return;
		}
		GrabItemMeshComp->SetWorldLocation(HitResult.ImpactPoint);

		// 3. 设置物理约束
		GrabItemPhysicsConstraintComp->SetConstrainedComponents(GrabItemMeshComp, NAME_None, HeldComponent, HitResult.BoneName);

		// 4. 设置已经抓起物体
		HeldObject = HitResult.GetActor();
		bIsGrabbing = true;

		// 5. 调用抓取事件
		IGrabableInterface::Execute_OnGrab(HeldObject);
		return;
	}
	
	HeldObject = nullptr;
	bIsGrabbing = false;
}

void UGrabComponent::GrabItem() {
	FHitResult HitResult;
	FVector Start = Owner->GetCameraComponent()->GetComponentLocation();
	FVector End = Start + Owner->GetCameraComponent()->GetForwardVector() * DetectLength;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_PhysicsBody, Params);
	
	if(GetOwnerRole() == ROLE_Authority) {
		GrabItemInternal(bIsHit, HitResult);
	} else {
		GrabItemOnServer(bIsHit, HitResult);
	}
}

void UGrabComponent::DropItemOnServer_Implementation() {
	DropItem();
}

void UGrabComponent::DropItem() {
	if(GetOwnerRole() == ROLE_Authority) {
		if(!bIsGrabbing) { return; }
		
		HeldComponent->WakeRigidBody();
		HeldComponent->SetSimulatePhysics(true);		// 重新将客户端的SimulatePhysics设置为true
		GrabItemPhysicsConstraintComp->BreakConstraint();
		IGrabableInterface::Execute_OnDrop(HeldObject);
		HeldObject = nullptr;
		bIsGrabbing = false;
	} else {
		DropItemOnServer();
	}
}

void UGrabComponent::OnRep_HeldComponent() {
	if(!HeldComponent) {
		return;
	}

	// 当物体被拖拽的时候就将客户端的物体查询设置为NoCollision
	HeldComponent->SetSimulatePhysics(false);
}

void UGrabComponent::OnRep_GrabItemMeshComp() const {
	GrabItemMeshComp->bHiddenInGame = true;
}

void UGrabComponent::GrabItemOnServer_Implementation(const bool bIsHit, const FHitResult& HitResult) {
	GrabItemInternal(bIsHit, HitResult);
}

void UGrabComponent::AddRequireComponentsOnServer_Implementation() {
	if(GetOwnerRole() == ROLE_Authority) {
		Owner = Cast<AMainCharacterBase>(GetOwner());
		if(!Owner) {
			return;
		}
		
		GrabItemMeshComp = NewObject<UStaticMeshComponent>(Owner, TEXT("GrabItemMeshComp"));
		GrabItemPhysicsConstraintComp = NewObject<UPhysicsConstraintComponent>(Owner, TEXT("GrabItemPhysicsConstraintComp"));
		
		if(!GrabItemMeshComp || !GrabItemMeshComp) {
			return;
		}

		GrabItemMeshComp->bHiddenInGame = true;
		GrabItemMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		if(VisibleMesh) {
			GrabItemMeshComp->SetStaticMesh(VisibleMesh);
		}

		// 设置物理约束的一些基本属性
		GrabItemPhysicsConstraintComp->SetDisableCollision(true);
		GrabItemPhysicsConstraintComp->SetLinearXLimit(LCM_Free, 0.f);
		GrabItemPhysicsConstraintComp->SetLinearYLimit(LCM_Free, 0.f);
		GrabItemPhysicsConstraintComp->SetLinearYLimit(LCM_Free, 0.f);
		GrabItemPhysicsConstraintComp->SetAngularSwing1Limit(ACM_Free, 0.f);
		GrabItemPhysicsConstraintComp->SetAngularSwing2Limit(ACM_Free, 0.f);
		GrabItemPhysicsConstraintComp->SetAngularTwistLimit(ACM_Free, 0.f);

		// 设置Linear Limits
		GrabItemPhysicsConstraintComp->SetLinearPositionDrive(true, true, true);
		GrabItemPhysicsConstraintComp->SetLinearVelocityDrive(true, true, true);
		GrabItemPhysicsConstraintComp->SetLinearPositionTarget({0.0, 0.0, 0.0});
		GrabItemPhysicsConstraintComp->SetLinearVelocityTarget({0.0, 0.0, 0.0});
		GrabItemPhysicsConstraintComp->SetLinearDriveParams(500.f, 100.f, 0.f);

		// 设置Angular Limits
		GrabItemPhysicsConstraintComp->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
		GrabItemPhysicsConstraintComp->SetAngularOrientationDrive(true, true);
		GrabItemPhysicsConstraintComp->SetAngularVelocityDrive(true, true);
		GrabItemPhysicsConstraintComp->SetAngularOrientationTarget({0.0, 0.0, 0.0});
		GrabItemPhysicsConstraintComp->SetAngularVelocityTarget({0.0, 0.0, 0.0});
		GrabItemPhysicsConstraintComp->SetAngularDriveParams(500.f, 100.f, 0.f);

		GrabItemMeshComp->RegisterComponent();
		GrabItemPhysicsConstraintComp->RegisterComponent();

		GrabItemMeshComp->AttachToComponent(Owner->GetCameraComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		GrabItemPhysicsConstraintComp->AttachToComponent(GrabItemMeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale);

		// 设置两个组件的网络复制
		GrabItemMeshComp->SetIsReplicated(true);
		GrabItemPhysicsConstraintComp->SetIsReplicated(true);
		
		bIsInitialized = true;
	}
}

void UGrabComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UGrabComponent, Owner, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UGrabComponent, bIsInitialized, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UGrabComponent, bIsGrabbing, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UGrabComponent, HeldObject, COND_None);				
	DOREPLIFETIME_CONDITION(UGrabComponent, HeldComponent, COND_None);		// 这个需要所有客户端同步用来防止物体抽搐，主要是用来将所有客户端中对应的物体的模拟物理给关闭掉
	DOREPLIFETIME_CONDITION(UGrabComponent, bIsOn, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UGrabComponent, GrabItemMeshComp, COND_None);
	DOREPLIFETIME_CONDITION(UGrabComponent, GrabItemPhysicsConstraintComp, COND_OwnerOnly);
}