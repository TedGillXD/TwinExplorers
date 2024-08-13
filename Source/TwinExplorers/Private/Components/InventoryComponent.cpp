// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
}


void UInventoryComponent::ServerAddItem_Implementation(const FItem& Item) {
	AddItem(Item);
}

void UInventoryComponent::ServerRemoveItemByName_Implementation(FName ItemName) {
	RemoveItemByName(ItemName);
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

bool UInventoryComponent::IsContain(const FName ItemName) {
	if(Tools.ItemName.IsEqual(ItemName, ENameCase::CaseSensitive)) {
		return true;
	}

	for(const auto& [Name, bIsTool, ItemActorClass, Icon, MaxUsageCount, UsageCount] : Props) {
		if(Name.IsEqual(ItemName, ENameCase::CaseSensitive)) {
			return true;
		}
	}
	
	return false;
}

FItem UInventoryComponent::GetItemByName(const FName ItemName) {
	if(Tools.ItemName.IsEqual(ItemName, ENameCase::CaseSensitive)) {
		return Tools;
	}

	for(int32 Index = 0; Index < Props.Num(); Index++) {
		if(Props[Index].ItemName.IsEqual(ItemName, ENameCase::CaseSensitive)) {
			const FItem Item = Props[Index];
			return Item;
		}
	}
	return {};
}

void UInventoryComponent::AddItem(const FItem& Item) {
	if(GetOwnerRole() == ROLE_Authority) {		// 服务器端中直接进行加入
		if(Item.bIsTool) {
			// TODO: 将背包中的道具Drop出来
			Tools = Item;		// 直接替换Tool
			OnSelectedToolChanged.Broadcast(Tools); 	// 手中的技能发生变化，更新
		} else {
			Props.Add(Item);
		}
		OnInventoryChanged.Broadcast(Tools, Props);
	} else {	// 对于客户端来说，这个拾取物品需要发送到服务器中进行存入
		ServerAddItem(Item);
	}
}

FItem UInventoryComponent::RemoveItemByName(FName ItemName) {
	if(GetOwnerRole() == ROLE_Authority) {
		if(Tools.ItemName.IsEqual(ItemName, ENameCase::CaseSensitive)) {
			FItem Ret = Tools;
			Tools = {};
			OnInventoryChanged.Broadcast(Tools, Props);
			OnSelectedToolChanged.Broadcast(Tools);
			return Ret;
		}

		for(int32 Index = 0; Index < Props.Num(); Index++) {
			if(Props[Index].ItemName.IsEqual(ItemName, ENameCase::CaseSensitive)) {
				const FItem Item = Props[Index];
				Props.RemoveAt(Index);
				OnInventoryChanged.Broadcast(Tools, Props);
				return Item;
			}
		}
	} else {
		FItem ItemWillBeRemoved = GetItemByName(ItemName);
		ServerRemoveItemByName(ItemName);
		return ItemWillBeRemoved;
	}
	
	return {};
}

bool UInventoryComponent::IsItemValid(const FItem& Item) {
	return !Item.ItemName.IsNone();
}

void UInventoryComponent::UseInHandItem() {
	if(!IsItemValid(Tools)) { return; }

	Tools.UsageCount++;
	if(Tools.UsageCount == Tools.MaxUsageCount) {
		// 销毁这个Tool
		Tools = EmptyItem;
		OnSkillDestroy.Broadcast();
		OnSelectedToolChanged.Broadcast(Tools);
		OnInventoryChanged.Broadcast(Tools, Props);
	}
}

const FItem& UInventoryComponent::GetInHandItem() {
	return Tools;
}

void UInventoryComponent::OnRep_Tools() const {
	OnInventoryChanged.Broadcast(Tools, Props);
}

void UInventoryComponent::OnRep_Props() const {
	OnInventoryChanged.Broadcast(Tools, Props);
}

void UInventoryComponent::OnRep_SelectedToolIndex() const {
	// 当客户端更新的时候调用这个函数
	OnSelectedToolChanged.Broadcast(Tools);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Tools);
	DOREPLIFETIME(UInventoryComponent, Props);
}
