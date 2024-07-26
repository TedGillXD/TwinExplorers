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
	
	AddRequireComponentsOnServer();
}

void UGrabComponent::GrabItem() {
	if(GetOwnerRole() == ROLE_Authority) {
		FHitResult HitResult;
		FVector Start = Owner->GetCameraComponent()->GetComponentLocation();
		FVector End = Start + Owner->GetCameraComponent()->GetForwardVector() * DetectLength;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Owner);
		if(GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_PhysicsBody, Params)) {
			// 0. for debug
			DrawDebugLineTraceSingle(GetWorld(), Start, End, EDrawDebugTrace::ForDuration, true, HitResult, FLinearColor::Red, FLinearColor::Green, 5.f);
		
			// 1. 检查Actor是否能被Grab
			if(!HitResult.GetActor()->Implements<UGrabableInterface>()) {		// 实现了Grabbable这个接口的才能被抓取
				bIsGrabbing = false;
				return;
			}

			// 2. 将Mesh移动过去
			GrabItemMeshComp->SetWorldLocation(HitResult.ImpactPoint);

			// 3. 设置物理约束
			GrabItemPhysicsConstraintComp->SetConstrainedComponents(GrabItemMeshComp, NAME_None, HitResult.GetComponent(), HitResult.BoneName);

			// 4. 设置已经抓起物体
			HeldObject = HitResult.GetActor();
			bIsGrabbing = true;

			// 5. 调用抓取事件
			IGrabableInterface::Execute_OnGrab(HeldObject);
			return;
		}

		HeldObject = nullptr;
		bIsGrabbing = false;
	} else {
		GrabItemOnServer();
	}
}

void UGrabComponent::DropItemOnServer_Implementation() {
	DropItem();
}

void UGrabComponent::DropItem() {
	if(GetOwnerRole() == ROLE_Authority) {
		if(!bIsGrabbing) { return; }

		GrabItemPhysicsConstraintComp->BreakConstraint();
		HeldObject = nullptr;
		bIsGrabbing = false;
		IGrabableInterface::Execute_OnDrop(HeldObject);
	} else {
		DropItemOnServer();
	}
}

void UGrabComponent::GrabItemOnServer_Implementation() {
	GrabItem();
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
		GrabItemPhysicsConstraintComp->SetLinearDriveParams(1000.f, 100.f, 0.f);

		// 设置Angular Limits
		GrabItemPhysicsConstraintComp->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
		GrabItemPhysicsConstraintComp->SetAngularOrientationDrive(true, true);
		GrabItemPhysicsConstraintComp->SetAngularVelocityDrive(true, true);
		GrabItemPhysicsConstraintComp->SetAngularOrientationTarget({0.0, 0.0, 0.0});
		GrabItemPhysicsConstraintComp->SetAngularVelocityTarget({0.0, 0.0, 0.0});
		GrabItemPhysicsConstraintComp->SetAngularDriveParams(1000.f, 100.f, 0.f);

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

	DOREPLIFETIME(UGrabComponent, Owner);
	DOREPLIFETIME(UGrabComponent, bIsInitialized);
	DOREPLIFETIME(UGrabComponent, bIsGrabbing);
	DOREPLIFETIME(UGrabComponent, HeldObject);
	DOREPLIFETIME(UGrabComponent, bIsOn);
	DOREPLIFETIME(UGrabComponent, GrabItemMeshComp);
	DOREPLIFETIME(UGrabComponent, GrabItemPhysicsConstraintComp);
}