// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Item.h"
#include "InventoryComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, const TArray<FItem>&, Tools, const TArray<FItem>&, Props);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSelectedToolChanged, int32, NewIndex, const FItem&, Item);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TWINEXPLORERS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	UPROPERTY(BlueprintReadOnly, Category="Inventory Props", ReplicatedUsing=OnRep_Tools)
	TArray<FItem> Tools;		// 能拿在手上的道具

	UPROPERTY(BlueprintReadOnly, Category="Inventory Props", ReplicatedUsing=OnRep_Props)
	TArray<FItem> Props;		// 不能拿在手上的道具

	UPROPERTY(ReplicatedUsing=OnRep_SelectedToolIndex)
	int32 SelectedToolIndex;

public:
	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="Inventory Props")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="Inventory Props")
	FOnSelectedToolChanged OnSelectedToolChanged;
	
	UFUNCTION(Server, Reliable)
	void ServerAddItem(const FItem& Item);

	UFUNCTION(Server, Reliable)
	void ServerRemoveItemByName(FName ItemName);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UFUNCTION(BlueprintCallable)
	bool IsContain(const FName ItemName);

	UFUNCTION(BlueprintCallable)
	FItem GetItemByName(const FName ItemName);

	UFUNCTION(BlueprintCallable)
	void AddItem(const FItem& Item);

	// 移除从背包中移除一个同名的物品，如果背包中存在多个同名的话，只会移除其中的一个
	UFUNCTION(BlueprintCallable)
	FItem RemoveItemByName(FName ItemName);

	UFUNCTION(BlueprintCallable)
	static bool IsItemValid(const FItem& Item);

public:
	UFUNCTION(Server, Reliable)
	void ChangeInHandItemOnServer(int32 NewIndex);
	
	UFUNCTION(BlueprintCallable)
	void ChangeInHandItem(int32 NewIndex);

	UFUNCTION(BlueprintCallable)
	void NextTool();

	UFUNCTION(BlueprintCallable)
	void PreviousTool();

	UFUNCTION(BlueprintCallable)
	const FItem& GetInHandItem();

protected:
	UFUNCTION()
	void OnRep_Tools() const;

	UFUNCTION()
	void OnRep_Props() const;

	UFUNCTION()
	void OnRep_SelectedToolIndex() const;
};
