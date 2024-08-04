// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/WindMachine.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

AWindMachine::AWindMachine() {
	HalfRangeX = 100.f;
	HalfRangeY = 100.f;
	HalfRangeZ = 300.f;
	DistanceBetweenForceLine = 15.f;
	MaxForce = 50000.f;
	bIsOn = false;
	
	WindRange = CreateDefaultSubobject<UBoxComponent>(TEXT("WindRange"));
	WindRange->SetupAttachment(GetRootComponent());
	WindRange->SetBoxExtent(FVector{ HalfRangeX, HalfRangeY, HalfRangeZ });
	WindRange->SetRelativeLocation(FVector{ 0.0, 0.0, WindRange->GetScaledBoxExtent().Z });		// 将这个BoxCollision向上移动一半

	PropellerMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PropellerMesh"));
	PropellerMeshComp->SetupAttachment(GetRootComponent());
	PropellerMeshComp->SetSimulatePhysics(true);

	PhysicsConstraintComp = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraintComp"));
	PhysicsConstraintComp->SetupAttachment(PropellerMeshComp);
	PhysicsConstraintComp->SetConstrainedComponents(nullptr, NAME_None, PropellerMeshComp, NAME_None);
	PhysicsConstraintComp->SetLinearXLimit(LCM_Locked, 0.f);
	PhysicsConstraintComp->SetLinearYLimit(LCM_Locked, 0.f);
	PhysicsConstraintComp->SetLinearZLimit(LCM_Locked, 0.f);
	PhysicsConstraintComp->SetAngularTwistLimit(ACM_Locked, 0.f);
	PhysicsConstraintComp->SetAngularSwing1Limit(ACM_Free, 0.f);
	PhysicsConstraintComp->SetAngularSwing2Limit(ACM_Locked, 0.f);
	PhysicsConstraintComp->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
	PhysicsConstraintComp->SetAngularVelocityDrive(true, false);
	PhysicsConstraintComp->SetAngularDriveParams(10.f, 500.f, 0.f);
}

void AWindMachine::BeginPlay() {
	Super::BeginPlay();

	// 在客户端做，不要在服务器上模拟
	if(!HasAuthority()) {
		if(bIsOn) {
			PhysicsConstraintComp->SetAngularVelocityTarget({ 0.0, 0.0, 1.0 });
			PropellerMeshComp->WakeRigidBody();
		} else {
			PhysicsConstraintComp->SetAngularVelocityTarget({ 0.0, 0.0, 0.0 });
			PropellerMeshComp->WakeRigidBody();
		}
	}
}

void AWindMachine::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if(HasAuthority() && bIsOn) {
		DoWindMachineLogic();
	}
}

void AWindMachine::Activate_Implementation() {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "WindMachine Activated!");
	bIsOn = true;
}

void AWindMachine::Deactivate_Implementation() {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "WindMachine Deactivated!");
	bIsOn = false;
}

void AWindMachine::DoWindMachineLogic() const {
	TArray<AActor*> OverlappingActors;
	WindRange->GetOverlappingActors(OverlappingActors);
	if(OverlappingActors.Num() > 0) {	// 只有大于0才会启动
		FHitResult HitResult;
	
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.bTraceComplex = true;
		
		int RowCount = HalfRangeX * 2.f / DistanceBetweenForceLine;
		int ColCount = HalfRangeY * 2.f / DistanceBetweenForceLine;
		for(int32 i = 0; i < RowCount; i++) {
			for(int32 j = 0; j < ColCount; j++) {
				// 计算在World中的Location
				float X = j * DistanceBetweenForceLine - HalfRangeX;
				float Y = i * DistanceBetweenForceLine - HalfRangeY;
				
				FVector Start = UKismetMathLibrary::TransformLocation(GetActorTransform(), FVector{ X, Y, 0.0 });
				FVector End = Start + GetActorUpVector() * HalfRangeZ * 2.f;
				
				bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_PhysicsBody, Params);
				if(bIsHit && HitResult.GetComponent()->IsSimulatingPhysics(HitResult.BoneName)) {
					// 命中并且模拟物理的情况下，对命中的点应用力
					FVector Force = (HitResult.ImpactNormal * -1) * FMath::Abs(FVector::DotProduct(HitResult.ImpactNormal * -1, GetActorUpVector())) * MaxForce;
					HitResult.GetComponent()->AddForceAtLocation(Force, HitResult.ImpactPoint, HitResult.BoneName);
				}
			}
		}
	}
}

void AWindMachine::OnRep_IsOn() {
	// 用OnRep保证这个物理的开启和关闭都是在客户端运行的，从而优化服务器性能
	if(bIsOn) {
		PhysicsConstraintComp->SetAngularVelocityTarget({ 0.0, 0.0, 1.0 });
		PropellerMeshComp->WakeRigidBody();
	} else {
		PhysicsConstraintComp->SetAngularVelocityTarget({ 0.0, 0.0, 0.0 });
		PropellerMeshComp->WakeRigidBody();
	}
}

void AWindMachine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWindMachine, bIsOn);
}
