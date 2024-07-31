// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/Switches/Lever.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"

ALever::ALever() {
	PrimaryActorTick.bCanEverTick = true;
	
	AsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(AsRoot);

	BaseComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	BaseComp->SetupAttachment(GetRootComponent());

	LeverComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Lever"));
	LeverComp->SetupAttachment(GetRootComponent());
	LeverComp->SetSimulatePhysics(true);
	LeverComp->SetEnableGravity(false);
	LeverComp->SetCollisionObjectType(ECC_PhysicsBody);

	PhysicsConstraintComp = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("LeverAxis"));
	PhysicsConstraintComp->SetupAttachment(BaseComp);

	bIsOn = false;

	// 设置PhysicsConstraint
	PhysicsConstraintComp->SetConstrainedComponents(BaseComp, NAME_None, LeverComp, NAME_None);
	
	PhysicsConstraintComp->SetDisableCollision(true);
	PhysicsConstraintComp->SetLinearXLimit(LCM_Locked, 0.f);
	PhysicsConstraintComp->SetLinearYLimit(LCM_Locked, 0.f);
	PhysicsConstraintComp->SetLinearZLimit(LCM_Locked, 0.f);

	PhysicsConstraintComp->SetAngularSwing1Limit(ACM_Limited, 45.f);
	PhysicsConstraintComp->SetAngularSwing2Limit(ACM_Locked, 0.f);
	PhysicsConstraintComp->SetAngularTwistLimit(ACM_Locked, 0.f);
	PhysicsConstraintComp->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
	PhysicsConstraintComp->SetAngularVelocityDrive(true, false);		// 因为之开启了Swing
	PhysicsConstraintComp->SetAngularVelocityTarget({0.0, 0.0, 0.0});		// 由于摩擦力速度应该会降低

	PhysicsConstraintComp->SetAngularOrientationDrive(true, false);
	PhysicsConstraintComp->SetAngularDriveParams(1000.f, 50.f, 0.f);
	if(bIsOn) {
		SetToOnOrientationTarget();
	} else {
		SetToOnOrientationTarget();
	}

	bIsOn = false;
}

void ALever::BeginPlay() {
	Super::BeginPlay();
	
}

void ALever::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	// 当拉杆被拉超过一半后，切换状态
	if(LeverComp->GetComponentRotation().Pitch <= 0 && !bIsOn) {
		bIsOn = true;
		SetToOnOrientationTarget();
		OnSwitchActivated.Broadcast();
	} else if(LeverComp->GetComponentRotation().Pitch > 0 && bIsOn) {
		bIsOn = false;
		SetToOffOrientationTarget();
		OnSwitchDeactivated.Broadcast();
	}
}

void ALever::OnGrab_Implementation() {
	
}

void ALever::OnDrop_Implementation() {
	
}

void ALever::SetToOffOrientationTarget() const {
	PhysicsConstraintComp->SetAngularOrientationTarget({-45.0, 0.0, 0.0});
}

void ALever::SetToOnOrientationTarget() const {
	PhysicsConstraintComp->SetAngularOrientationTarget({45.0, 0.0, 0.0});
}


