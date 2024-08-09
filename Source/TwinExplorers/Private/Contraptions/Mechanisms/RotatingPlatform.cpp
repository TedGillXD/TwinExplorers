// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/Mechanisms/RotatingPlatform.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"

// Sets default values
ARotatingPlatform::ARotatingPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 初始化根组件
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// 初始化平台网格
	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlatformMesh"));
	PlatformMesh->SetupAttachment(Root);
	PlatformMesh->SetSimulatePhysics(true);
	PlatformMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(Root);

	// 初始化物理约束组件
	PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	PhysicsConstraint->SetupAttachment(Root);

	// 设定旋转速度
	RotationSpeed = 30.0f; // 每秒旋转的角度

	// 设置物理约束
	PhysicsConstraint->SetConstrainedComponents(nullptr, NAME_None, PlatformMesh, NAME_None);
	PhysicsConstraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Free, 0.f);
	PhysicsConstraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	PhysicsConstraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	PhysicsConstraint->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
	PhysicsConstraint->SetAngularDriveParams(50.f, 100.f, 0.f);
	PhysicsConstraint->SetAngularVelocityTarget({ 0.0, 0.0, 5.0 });
	PhysicsConstraint->SetAngularVelocityDrive(true, false);
}

// Called when the game starts or when spawned
void ARotatingPlatform::BeginPlay() {
	Super::BeginPlay();

	
}
