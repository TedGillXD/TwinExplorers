// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/Mechanisms/GravityMaze.h"

#include "Components/BoxComponent.h"
#include "Contraptions/Switches/SwitchBase.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

AGravityMaze::AGravityMaze() {
	BaseComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
	BaseComp->SetupAttachment(GetRootComponent());

	MazeFloorComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Maze Floor"));
	MazeFloorComp->SetupAttachment(GetRootComponent());
	MazeFloorComp->SetSimulatePhysics(true);
	
	BallGeneratorComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ball Generator"));
	BallGeneratorComp->SetupAttachment(GetRootComponent());

	BallDestroyBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Ball Destroy Box"));
	BallDestroyBoxComp->SetupAttachment(GetRootComponent());
	BallDestroyBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGravityMaze::OnBoxBeginOverlap);

	BallSpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Ball Spawn Location"));
	BallSpawnLocation->SetupAttachment(BallGeneratorComp);

	PhysicsConstraintComp = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	PhysicsConstraintComp->SetupAttachment(GetRootComponent());

	CurrentTargetVelocity = {0.0, 0.0, 0.0};
	
	// 设置PhysicsConstraint
	PhysicsConstraintComp->SetDisableCollision(true);
	PhysicsConstraintComp->SetAngularSwing1Limit(ACM_Limited, 45.f);
	PhysicsConstraintComp->SetAngularSwing2Limit(ACM_Limited, 45.f);
	PhysicsConstraintComp->SetAngularTwistLimit(ACM_Locked, 0.f);
	
	PhysicsConstraintComp->SetLinearXLimit(LCM_Locked, 0.f);
	PhysicsConstraintComp->SetLinearYLimit(LCM_Locked, 0.f);
	PhysicsConstraintComp->SetLinearZLimit(LCM_Locked, 0.f);

	PhysicsConstraintComp->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
	PhysicsConstraintComp->SetAngularVelocityDrive(true, false);
	PhysicsConstraintComp->SetAngularVelocityTarget(CurrentTargetVelocity);
	PhysicsConstraintComp->SetAngularDriveParams(10.f, 40.f, 0.f);

	PhysicsConstraintComp->SetConstrainedComponents(BaseComp, NAME_None, MazeFloorComp, NAME_None);

	bUseDefaultBehavior = false;		// 禁用默认行为
	VelocityDelta = 0.05;		// 默认旋转速度
}

void AGravityMaze::BeginPlay() {
	Super::BeginPlay();

	// 绑定按钮事件
	if(RelatedSwitches.Num() < 4) {
		UE_LOG(LogTemp, Error, TEXT("Don't have enough related switches, need 4 with the order of forward, backward, leftward and rightward!"));
	}

	ASwitchBase* CurrentSwitch = RelatedSwitches[0];
	CurrentSwitch->OnSwitchActivated.AddDynamic(this, &AGravityMaze::ForwardActivated);
	CurrentSwitch->OnSwitchDeactivated.AddDynamic(this, &AGravityMaze::ForwardDeactivated);
	
	CurrentSwitch = RelatedSwitches[1];
	CurrentSwitch->OnSwitchActivated.AddDynamic(this, &AGravityMaze::BackwardActivated);
	CurrentSwitch->OnSwitchDeactivated.AddDynamic(this, &AGravityMaze::BackwardDeactivated);
	
	CurrentSwitch = RelatedSwitches[2];
	CurrentSwitch->OnSwitchActivated.AddDynamic(this, &AGravityMaze::LeftwardActivated);
	CurrentSwitch->OnSwitchDeactivated.AddDynamic(this, &AGravityMaze::LeftwardDeactivated);
	
	CurrentSwitch = RelatedSwitches[3];
	CurrentSwitch->OnSwitchActivated.AddDynamic(this, &AGravityMaze::RightwardActivated);
	CurrentSwitch->OnSwitchDeactivated.AddDynamic(this, &AGravityMaze::RightwardDeactivated);

	// 生成一个新的球
	Sphere = GetWorld()->SpawnActor(BallClass, &BallSpawnLocation->GetComponentTransform());
}

void AGravityMaze::ForwardActivated() {
	CurrentTargetVelocity.Set(CurrentTargetVelocity.X, CurrentTargetVelocity.Y, CurrentTargetVelocity.Z + VelocityDelta);
	UpdateAngularVelocityTarget();
}

void AGravityMaze::ForwardDeactivated() {
	CurrentTargetVelocity.Set(CurrentTargetVelocity.X, CurrentTargetVelocity.Y, CurrentTargetVelocity.Z - VelocityDelta);
	UpdateAngularVelocityTarget();
}

void AGravityMaze::BackwardActivated() {
	CurrentTargetVelocity.Set(CurrentTargetVelocity.X, CurrentTargetVelocity.Y, CurrentTargetVelocity.Z - VelocityDelta);
	UpdateAngularVelocityTarget();
}

void AGravityMaze::BackwardDeactivated() {
	CurrentTargetVelocity.Set(CurrentTargetVelocity.X, CurrentTargetVelocity.Y, CurrentTargetVelocity.Z + VelocityDelta);
	UpdateAngularVelocityTarget();
}

void AGravityMaze::LeftwardActivated() {
	CurrentTargetVelocity.Set(CurrentTargetVelocity.X, CurrentTargetVelocity.Y + VelocityDelta, CurrentTargetVelocity.Z);
	UpdateAngularVelocityTarget();
}

void AGravityMaze::LeftwardDeactivated() {
	CurrentTargetVelocity.Set(CurrentTargetVelocity.X, CurrentTargetVelocity.Y - VelocityDelta, CurrentTargetVelocity.Z);
	UpdateAngularVelocityTarget();
}

void AGravityMaze::RightwardActivated() {
	CurrentTargetVelocity.Set(CurrentTargetVelocity.X, CurrentTargetVelocity.Y - VelocityDelta, CurrentTargetVelocity.Z);
	UpdateAngularVelocityTarget();
}

void AGravityMaze::RightwardDeactivated() {
	CurrentTargetVelocity.Set(CurrentTargetVelocity.X, CurrentTargetVelocity.Y + VelocityDelta, CurrentTargetVelocity.Z);
	UpdateAngularVelocityTarget();
}

void AGravityMaze::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	// 1. 判断是不是一个球
	if(!OtherActor->IsA(BallClass)) {	// 如果不是球，忽略
		return;
	}

	// 2. 如果是，删除；
	if(Sphere) {
		Sphere->Destroy();
	}
	Sphere = nullptr;

	// 3. 删除完成后生成一个新的
	Sphere = GetWorld()->SpawnActor(BallClass, &BallSpawnLocation->GetComponentTransform());
}

void AGravityMaze::OnRep_Sphere() {
	
}

void AGravityMaze::UpdateAngularVelocityTarget() const {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Angular velocity updated!");
	MazeFloorComp->WakeRigidBody();
	PhysicsConstraintComp->SetAngularVelocityTarget(CurrentTargetVelocity);
}

void AGravityMaze::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGravityMaze, Sphere, COND_None);
}
