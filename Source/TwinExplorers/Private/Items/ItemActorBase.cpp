// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemActorBase.h"

#include "Characters/MainCharacterBase.h"
#include "Components/InventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Controllers/TEPlayerController.h"

AItemActorBase::AItemActorBase() {
	AsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(AsRoot);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComp"));
	ItemMesh->SetupAttachment(AsRoot);

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(AsRoot);
	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AItemActorBase::PickupItem);

	// 开启网络同步
	this->bReplicates = true;
	ItemMesh->SetIsReplicated(true);

	NetUpdateFrequency = 10.f;
	MinNetUpdateFrequency = 2.f;
	RunningTime = 0.f;
}

void AItemActorBase::PickupItem(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	if(!HasAuthority()) { return; }
	if(!OtherActor->IsA<ACharacter>()) { return; }

	// 拾起道具
	Execute_Interact(this, Cast<ACharacter>(OtherActor), EmptyItem);
}

void AItemActorBase::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if(!HasAuthority()) {
		// 让物体旋转
		FRotator NewRotation = FRotator(0, RotationSpeed * DeltaSeconds, 0);
		ItemMesh->AddWorldRotation(NewRotation);

		// 让物体上下浮动
		FVector NewLocation = ItemMesh->GetRelativeLocation();
		float DeltaHeight = FMath::Sin(RunningTime + DeltaSeconds) - FMath::Sin(RunningTime);
		NewLocation.Z += DeltaHeight * FloatSpeed;  // 控制浮动速度
		ItemMesh->SetRelativeLocation(NewLocation);

		// 更新运行时间
		RunningTime += DeltaSeconds;
	}
}

bool AItemActorBase::CanInteract_Implementation(const FItem& InHandItem) {
	return false;
}

void AItemActorBase::Interact_Implementation(APawn* FromPawn, const FItem& InHandItem) {
	// 调用这个函数的地方理应永远是Server
	if(HasAuthority()) {
		// 检查这个角色是否是一个有背包的角色
		const AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(FromPawn);
		if(!CharacterBase) {
			return;
		}
		
		// 加入到服务器中对应角色的背包
		UInventoryComponent* InventoryComp = CharacterBase->GetInventoryComponent();
		if(!InventoryComp) {
			return;
		}
		InventoryComp->AddItem(ItemData);

		// 使用PC来放声音
		ATEPlayerController* Controller = Cast<ATEPlayerController>(FromPawn->GetController());
		if(Controller) {		// 进来的一定是玩家，AI拾取就不会发声了
			Controller->PlaySoundOnClient(PickupSound);
		}
	
		this->Destroy();
	}
}

FString AItemActorBase::GetInteractString_Implementation() {
	return "";
}

UTexture2D* AItemActorBase::GetInteractIcon_Implementation() {
	return nullptr;
}

bool AItemActorBase::ShouldUpdate_Implementation() {
	return false;
}

void AItemActorBase::Updated_Implementation() {  }
void AItemActorBase::Focused_Implementation() {  }
void AItemActorBase::Unfocused_Implementation() {  }
