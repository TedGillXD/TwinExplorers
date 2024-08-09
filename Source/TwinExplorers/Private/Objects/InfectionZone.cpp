// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/InfectionZone.h"

#include "Characters/MainCharacterBase.h"
#include "Components/BillboardComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AInfectionZone::AInfectionZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	SetRootComponent(BoxComponent);

	TargetLocation = CreateDefaultSubobject<UBillboardComponent>(TEXT("TargetLocation"));
	TargetLocation->SetupAttachment(RootComponent);
	
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AInfectionZone::OnOverlapBegin);
}

void AInfectionZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {

	if(HasAuthority()) {
		AMainCharacterBase* OverlappingCharacter = Cast<AMainCharacterBase>(OtherActor);
		if (OverlappingCharacter) {
			// 将角色位置重置到 TargetLocation
			OverlappingCharacter->SetActorLocation(TargetLocation->GetComponentLocation());

			// 将角色的队伍从 Human 变为 Enemy
			OverlappingCharacter->GetInfect();  // 假设 GetInfect 函数已经实现了将队伍改为 Enemy 的逻辑
		} else {	// 对于其他的Actor，直接销毁
			OtherActor->Destroy();
		}
	}
}

// Called when the game starts or when spawned
void AInfectionZone::BeginPlay() {
	Super::BeginPlay();
	
}