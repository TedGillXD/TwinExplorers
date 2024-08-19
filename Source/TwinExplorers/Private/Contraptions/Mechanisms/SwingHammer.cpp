// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/Mechanisms/SwingHammer.h"

#include "Characters/MainCharacterBase.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "GameFramework/Character.h"

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

	LeftBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftBox"));
	LeftBox->SetupAttachment(HammerMesh);
	RightBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightBox"));
	RightBox->SetupAttachment(HammerMesh);

	Arrow1 = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow1"));
	Arrow1->SetupAttachment(HammerMesh);
	Arrow2 = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow2"));
	Arrow2->SetupAttachment(HammerMesh);

	// 初始化物理约束组件
	PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	PhysicsConstraint->SetupAttachment(Root);

	// 设置摆动速度
	SwingSpeed = 2.0f;
	SwingAmplitude = 45.0f; // 摆动幅度为45度
	CurrentSwingTime = 0.0f;

	// 设置物理约束
	PhysicsConstraint->SetConstrainedComponents(HammerMesh, NAME_None, nullptr, NAME_None);
	PhysicsConstraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.0f); // 允许围绕 Swing1 轴自由旋转
	PhysicsConstraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 50.f); // 锁定 Swing2 轴
	PhysicsConstraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.0f);  // 锁定 Twist 轴
	PhysicsConstraint->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
	PhysicsConstraint->SetAngularVelocityDrive(true, false);
	PhysicsConstraint->SetAngularDriveParams(50.f, 100.f, 0.f);

	bMovingToEnd = true;

	LaunchForce = 200.f;
}

void ASwingHammer::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if(HasAuthority()) {
		CurrentSwingTime += DeltaSeconds;
		float Angle = SwingAmplitude * FMath::Sin(CurrentSwingTime * SwingSpeed);
		FRotator NewRotation = FRotator(0, 0, Angle);
		HammerMesh->SetRelativeRotation(NewRotation);
	}
}

// void ASwingHammer::Hit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
//                        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
// 	
// 	if (HasAuthority() && OtherActor && OtherActor->IsA<AMainCharacterBase>()) {
// 		AMainCharacterBase* Character = Cast<AMainCharacterBase>(OtherActor);
// 		Character->GetCapsuleComponent()->SetIsReplicated(true);
// 		Character->GetHit();
//
// 		FTimerHandle TimerHandle_Recovery;
// 		GetWorldTimerManager().SetTimer(TimerHandle_Recovery, FTimerDelegate::CreateLambda([Character]() -> void {
// 			Character->GetCapsuleComponent()->SetIsReplicated(false);
// 			Character->RecoverFromHit();
// 		}), 1.f, false);
// 	}
// }

// Called when the game starts or when spawned
void ASwingHammer::BeginPlay()
{
	Super::BeginPlay();

	// 把客户端的模拟物理关掉防止jittery
	if(!HasAuthority()) {
		HammerMesh->SetSimulatePhysics(false);
	}
	
	// 初始方向
	if(HasAuthority()) {
		// ToggleSwingDirection();
		LeftBox->OnComponentBeginOverlap.AddDynamic(this, &ASwingHammer::Hit1);
		RightBox->OnComponentBeginOverlap.AddDynamic(this, &ASwingHammer::Hit2);
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

void ASwingHammer::Hit1(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (HasAuthority() && OtherActor && OtherActor->IsA<AMainCharacterBase>()) {
		AMainCharacterBase* Character = Cast<AMainCharacterBase>(OtherActor);
		FVector Dir = Arrow1->GetForwardVector();
		Dir.Z = FMath::Max(Dir.Z, 0.5f);
		Character->LaunchCharacter(Dir * LaunchForce, false, false);
	}
}

void ASwingHammer::Hit2(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if (HasAuthority() && OtherActor && OtherActor->IsA<AMainCharacterBase>()) {
		AMainCharacterBase* Character = Cast<AMainCharacterBase>(OtherActor);
		FVector Dir = Arrow2->GetForwardVector();
		Dir.Z = FMath::Max(Dir.Z, 0.5f);
		Character->LaunchCharacter(Dir * LaunchForce, false, false);
	}
}
