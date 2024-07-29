// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/IcePillar.h"

// Sets default values
AIcePillar::AIcePillar()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PillarMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMeshComp"));
	PillarMeshComp->SetupAttachment(GetRootComponent());

	this->SetReplicates(true);
}

// Called when the game starts or when spawned
void AIcePillar::BeginPlay()
{
	Super::BeginPlay();

	FVector OriginalLocation = PillarMeshComp->GetComponentLocation();
	Current = OriginalLocation.Z - 100;
	Target = OriginalLocation.Z;
	PillarMeshComp->SetWorldLocation({ OriginalLocation.X, OriginalLocation.Y, OriginalLocation.Z - 100 });
}

// Called every frame
void AIcePillar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(HasAuthority()) {
		// 这个计算应该只在服务端进行
		if(!FMath::IsNearlyEqual(Current, Target)) {
			Current = FMath::FInterpTo(Current, Target, DeltaTime, GetInterpTime(Current, Target, 50));
			FVector OriginalLocation = PillarMeshComp->GetComponentLocation();
			PillarMeshComp->SetWorldLocation({ OriginalLocation.X, OriginalLocation.Y, Current });
		}
	}
}

void AIcePillar::DestroyOnServer_Implementation() {
	DestroyPillarOnServer();
}

void AIcePillar::DestroyPillarOnServer() {
	OnPillarDestroy.Broadcast();
	this->Destroy();
}

float AIcePillar::GetInterpTime(const float From, const float To, const float SpeedInConstant) const {
	float Distance = FMath::Abs(To - From);
	if(Distance == 0.f) { return 0.f; }

	return GetWorld()->GetDeltaSeconds() * SpeedInConstant * 1000.f / Distance;
}

bool AIcePillar::CanInteract_Implementation(const FItem& InHandItem) {
	return InHandItem.ItemName.IsEqual(InteractItemName, ENameCase::CaseSensitive);
}

void AIcePillar::Interact_Implementation(APawn* FromPawn, const FItem& InHandItem) {
	if(HasAuthority()) {
		DestroyPillarOnServer();
	} else {
		DestroyOnServer();	
	}
}

FString AIcePillar::GetInteractString_Implementation() {
	return InteractString;
}

UTexture2D* AIcePillar::GetInteractIcon_Implementation() {
	return nullptr;
}

bool AIcePillar::ShouldUpdate_Implementation() {
	return false;
}

void AIcePillar::Updated_Implementation() {
	// do nothing
}

void AIcePillar::Focused_Implementation() {
	
}

void AIcePillar::Unfocused_Implementation() {
	
}

