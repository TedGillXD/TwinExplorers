// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/Switches/PressurePlate.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"

APressurePlate::APressurePlate() {
	// 开启Tick
	PrimaryActorTick.bCanEverTick = true;
	
	AsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(AsRoot);

	BaseComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	BaseComp->SetupAttachment(AsRoot);

	PlateComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plate"));
	PlateComp->SetSimulatePhysics(true);
	PlateComp->SetupAttachment(AsRoot);
	PlateComp->SetEnableGravity(false);

	PhysicsConstraintComp = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	PhysicsConstraintComp->SetupAttachment(BaseComp);

	// 设置PhysicsConstraint的参数
	PhysicsConstraintComp->SetDisableCollision(true);
	PhysicsConstraintComp->SetAngularSwing1Limit(ACM_Locked, 0.f);
	PhysicsConstraintComp->SetAngularSwing2Limit(ACM_Locked, 0.f);
	PhysicsConstraintComp->SetAngularTwistLimit(ACM_Locked, 0.f);

	PhysicsConstraintComp->SetLinearXLimit(LCM_Locked, 0.f);
	PhysicsConstraintComp->SetLinearYLimit(LCM_Locked, 0.f);
	PhysicsConstraintComp->SetLinearZLimit(LCM_Limited, 8.f);

	PhysicsConstraintComp->SetLinearPositionDrive(true, true, true);
	PhysicsConstraintComp->SetLinearVelocityDrive(true, true, true);
	PhysicsConstraintComp->SetLinearPositionTarget({ 0.0, 0.0, 0.0 });
	PhysicsConstraintComp->SetLinearVelocityTarget({ 0.0, 0.0, 0.0 });
	PhysicsConstraintComp->SetLinearDriveParams(50.f, 10.f, 0.f);

	// 绑定两个组件
	PhysicsConstraintComp->SetConstrainedComponents(BaseComp, NAME_None, PlateComp, NAME_None);

	bIsOn = false;
}

void APressurePlate::BeginPlay() {
	Super::BeginPlay();

	if(HasAuthority()) {
		InitialZ = PlateComp->GetComponentLocation().Z;
	}
}

void APressurePlate::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if(HasAuthority()) {
		const float CurrentZ = PlateComp->GetComponentLocation().Z;
		if (InitialZ - CurrentZ >= TriggeredOffset && !bIsOn) {
			bIsOn = true;
			OnSwitchActivated.Broadcast();
		} else if (InitialZ - CurrentZ < TriggeredOffset && bIsOn) {
			bIsOn = false;
			OnSwitchDeactivated.Broadcast();
		}
	}
}

bool APressurePlate::IsPlateStable() const {
	return FMath::IsNearlyEqual(PlateComp->GetComponentVelocity().Z, 0.0);
}
