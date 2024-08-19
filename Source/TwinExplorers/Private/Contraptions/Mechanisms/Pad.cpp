// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/Mechanisms/Pad.h"

#include "Components/BoxComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

void APad::SetMeshDisappear() {
	// 设置网格不可见
	Mesh->SetVisibility(false, true);
	// 禁用网格的碰撞
	Mesh->SetSimulatePhysics(false);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void APad::ResetMesh() {
	// 使网格再次可见
	Mesh->SetVisibility(true, true);
	// 重新启用碰撞
	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);

	// 重置触发状态
	bIsTriggered = false;
}


// Sets default values
APad::APad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	BoxComponent->SetupAttachment(Mesh);
	
	PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	PhysicsConstraint->SetupAttachment(GetRootComponent());

	DisappearTime = 3.f;
	ResetTime = 3.f;
	bIsTriggered = false;
}

// Called when the game starts or when spawned
void APad::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority()) {
		BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &APad::OnComponentOverlapped);
	}
}

// Called every frame
void APad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void APad::OnComponentOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if(!bIsTriggered) {
		bIsTriggered = true;

		// 设置消失的Countdown
		FTimerHandle DisappearTimerHandle;
		GetWorldTimerManager().SetTimer(DisappearTimerHandle, this, &APad::SetMeshDisappear, DisappearTime, false);

		// 设置重置的Countdown
		FTimerHandle ResetTimerHandle;
		GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &APad::ResetMesh, DisappearTime + ResetTime, false);
	}
}

