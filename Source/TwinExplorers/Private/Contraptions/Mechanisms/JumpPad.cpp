// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/Mechanisms/JumpPad.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"

// Sets default values
AJumpPad::AJumpPad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	JumpPadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JumpPadMesh"));
	RootComponent = JumpPadMesh;

	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->SetupAttachment(JumpPadMesh);
	OverlapBox->SetCollisionProfileName(TEXT("Trigger"));
	OverlapBox->OnComponentBeginOverlap.AddDynamic(this, &AJumpPad::HandleOverlap);
}

// Called when the game starts or when spawned
void AJumpPad::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AJumpPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AJumpPad::HandleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if(HasAuthority()) {
		ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
		if (PlayerCharacter) {
			// 给射出方向增加一些随机偏移
			FVector LaunchVelocity = FMath::VRandCone(GetActorUpVector(), FMath::DegreesToRadians(5.0f)) * JumpForce;
			PlayerCharacter->LaunchCharacter(LaunchVelocity, true, true);
		}
	}
	
}