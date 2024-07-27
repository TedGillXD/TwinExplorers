// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemActorBase.h"

#include "Characters/MainCharacterBase.h"
#include "Components/InventoryComponent.h"

AItemActorBase::AItemActorBase() {
	AsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(AsRoot);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComp"));
	ItemMesh->SetupAttachment(AsRoot);

	// 开启网络同步
	this->bReplicates = true;
	ItemMesh->SetIsReplicated(true);

	NetUpdateFrequency = 10.f;
	MinNetUpdateFrequency = 2.f;
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
