// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/ThrowableObject.h"

#include "Components/SphereComponent.h"

// Sets default values
AThrowableObject::AThrowableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	this->AActor::SetReplicateMovement(true);

	AsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(AsRoot);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(GetRootComponent());
	StaticMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);			// 忽略角色碰撞
	StaticMeshComp->SetIsReplicated(true);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetupAttachment(StaticMeshComp);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	bCanTriggered = false;
}

void AThrowableObject::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if(HasAuthority() && bCanTriggered) {
		OnTriggered(OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	}
}

void AThrowableObject::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	if(HasAuthority()) {
		bCanTriggered = true;			// 当该物体离开当前扔出这个物体的角色后，开始可以触发
	}
}

void AThrowableObject::OnTriggered_Implementation(AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	// do nothing here
}

// Called when the game starts or when spawned
void AThrowableObject::BeginPlay()
{
	Super::BeginPlay();
	
	if(HasAuthority()) {
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AThrowableObject::OnOverlap);
		SphereComp->OnComponentEndOverlap.AddDynamic(this, &AThrowableObject::EndOverlap);
	}
}