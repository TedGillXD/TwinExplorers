// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/Monopole.h"

#include "Components/SphereComponent.h"

AMonopole::AMonopole() {
	AsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(AsRoot);

	PoleMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PoleMeshComp"));
	PoleMeshComp->SetupAttachment(GetRootComponent());
	PoleMeshComp->SetCollisionObjectType(MagneticCollisionType);
	
	ActionRange = CreateDefaultSubobject<USphereComponent>(TEXT("ActionRange"));
	ActionRange->SetupAttachment(PoleMeshComp);
	ActionRange->SetSphereRadius(80.f);
	ActionRange->SetCollisionResponseToAllChannels(ECR_Ignore);

	MaxForce = 10000.f;
}

void AMonopole::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);
	 
	// 如果存在其他磁铁可以相互吸引或者排斥的话，每秒都对词条施加力；施加力只在服务端进行
	if(HasAuthority()) {
		ApplyInfluence();
	}
}

void AMonopole::ApplyInfluence() {
	TArray<AActor*> OverlappingActors;
	ActionRange->GetOverlappingActors(OverlappingActors, AMonopole::StaticClass());
	
	float MaxDistance = ActionRange->GetScaledSphereRadius();
	FVector CenterOfMassForThis = PoleMeshComp->GetCenterOfMass();
	for(AActor* OverlappingActor : OverlappingActors) {
		if(OverlappingActor == this) { continue; }

		// 下面处理重叠的其他Actor
		AMonopole* ForeignPole = Cast<AMonopole>(OverlappingActor);
		FVector CenterOfMassForForeign = ForeignPole->PoleMeshComp->GetCenterOfMass();		// 磁铁之间的吸引是在质量中心处产生
		FVector ClosetPointToForeign;
		PoleMeshComp->GetClosestPointOnCollision(CenterOfMassForForeign, ClosetPointToForeign);
		FVector FromForeignToThis = CenterOfMassForThis - CenterOfMassForForeign;

		// 如果磁铁是一个很大的一块板，那应该不是在质量中心而是在离碰撞最近的点为作为吸引
		if(!PoleMeshComp->IsSimulatingPhysics()) {
			FromForeignToThis = ClosetPointToForeign - CenterOfMassForForeign;
		}
		
		FVector AttractDir = FromForeignToThis.GetSafeNormal();
		float Distance = FMath::Clamp(FromForeignToThis.Length(), 0.f, MaxDistance);		// 保证得到的距离在范围内

		// 力度计算公式： F_Final = F_Max * [(d_Max - d_i) / d_Max]^2		其中d_Max是磁铁的最大影响范围，d_i是其他磁铁到本磁铁的距离
		float ScaledForce = MaxForce * FMath::Pow((MaxDistance - Distance) / MaxDistance, 2.f);
		AttractDir = ScaledForce * AttractDir;		// 计算出的最终Force
		
		// 应用这个力
		if(ForeignPole->PoleMeshComp->IsSimulatingPhysics()) {
			if(ForeignPole->MagneticPole == MagneticPole) {	// 排斥
				ForeignPole->PoleMeshComp->AddForceAtLocation(-1 * AttractDir, CenterOfMassForForeign);
			} else {	// 吸引
				ForeignPole->PoleMeshComp->AddForceAtLocation(AttractDir, CenterOfMassForForeign);
			}
		}
	}
}