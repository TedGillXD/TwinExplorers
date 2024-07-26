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
	for(const auto& [Name, bIsTool, ItemActorClass, Icon] : Tools) {
		if(Name.IsEqual(ItemName, ENameCase::CaseSensitive)) {
			return true;
		}
	}

	for(const auto& [Name, bIsTool, ItemActorClass, Icon] : Props) {
		if(Name.IsEqual(ItemName, ENameCase::CaseSensitive)) {
			return true;
		}
	}
	
	return false;
}

FItem UInventoryComponent::GetItemByName(const FName ItemName) {
	for(int32 Index = 0; Index < Tools.Num(); Index++) {
		if(Tools[Index].ItemName.IsEqual(ItemName, ENameCase::CaseSensitive)) {
			const FItem Item = Tools[Index];
			return Item;
		}
	}

	for(int32 Index = 0; Index < Props.Num(); Index++) {
		if(Props[Index].ItemName.IsEqual(ItemName, ENameCase::CaseSensitive)) {
			const FItem Item = Tools[Index];
			return Item;
		}
	}
	return {};
}

void UInventoryComponent::AddItem(const FItem& Item) {
	if(GetOwnerRole() == ROLE_Authority) {		// 服务器端中直接进行加入
		if(Item.bIsTool) {
			Tools.Add(Item);
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
		for(int32 Index = 0; Index < Tools.Num(); Index++) {
			if(Tools[Index].ItemName.IsEqual(ItemName, ENameCase::CaseSensitive)) {
				const FItem Item = Tools[Index];
				Tools.RemoveAt(Index);
				OnInventoryChanged.Broadcast(Tools, Props);
				return Item;
			}
		}

		for(int32 Index = 0; Index < Props.Num(); Index++) {
			if(Props[Index].ItemName.IsEqual(ItemName, ENameCase::CaseSensitive)) {
				const FItem Item = Tools[Index];
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

void UInventoryComponent::OnRep_Tools() const {
	OnInventoryChanged.Broadcast(Tools, Props);
}

void UInventoryComponent::OnRep_Props() const {
	OnInventoryChanged.Broadcast(Tools, Props);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Tools);
	DOREPLIFETIME(UInventoryComponent, Props);
}
