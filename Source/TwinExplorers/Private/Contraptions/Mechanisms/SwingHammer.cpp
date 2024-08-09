// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/Mechanisms/SwingHammer.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"

// Sets default values
ASwingHammer::ASwingHammer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 初始化根组件
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// 初始化锤子网格
	HammerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HammerMesh"));
	HammerMesh->SetupAttachment(Root);
	HammerMesh->SetSimulatePhysics(true);
	HammerMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

	// 初始化物理约束组件
	PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	PhysicsConstraint->SetupAttachment(Root);

	// 设置摆动速度
	SwingSpeed = 2.0f; // 每次摆动的时间（秒）

	// 设置物理约束
	PhysicsConstraint->SetConstrainedComponents(HammerMesh, NAME_None, nullptr, NAME_None);
	PhysicsConstraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.0f); // 允许围绕 Swing1 轴自由旋转
	PhysicsConstraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 50.f); // 锁定 Swing2 轴
	PhysicsConstraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.0f);  // 锁定 Twist 轴
	PhysicsConstraint->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
	PhysicsConstraint->SetAngularVelocityDrive(true, false);
	PhysicsConstraint->SetAngularDriveParams(50.f, 100.f, 0.f);

	bMovingToEnd = true;
}

// Called when the game starts or when spawned
void ASwingHammer::BeginPlay()
{
	Super::BeginPlay();

	// 把客户端的模拟物理关掉方式jittery
	if(!HasAuthority()) {
		HammerMesh->SetSimulatePhysics(false);
	}
	
	// 初始方向
	if(HasAuthority()) {
		ToggleSwingDirection();
	}

}

void ASwingHammer::ToggleSwingDirection() {
	if (bMovingToEnd) {
		PhysicsConstraint->SetAngularVelocityTarget(FVector{ 0.0, -0.5, 0.0 });
		HammerMesh->WakeRigidBody();
	} else {
		PhysicsConstraint->SetAngularVelocityTarget(FVector{ 0.0, 0.5, 0.0 });
		HammerMesh->WakeRigidBody();
	}

	bMovingToEnd = !bMovingToEnd;

	// 在指定的时间后切换方向
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASwingHammer::ToggleSwingDirection, SwingSpeed, false);
}