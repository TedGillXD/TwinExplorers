// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Item.h"
#include "InventoryComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, const FItem&, Tool, const TArray<FItem>&, Props);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedToolChanged, const FItem&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillDestroy);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TWINEXPLORERS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	UPROPERTY(BlueprintReadOnly, Category="Inventory Props", ReplicatedUsing=OnRep_Tools)
	FItem Tools;		// 能拿在手上的道具

	UPROPERTY(BlueprintReadOnly, Category="Inventory Props", ReplicatedUsing=OnRep_Props)
	TArray<FItem> Props;		// 不能拿在手上的道具
	
public:
	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="Inventory Props")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="Inventory Props")
	FOnSelectedToolChanged OnSelectedToolChanged;

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="Inventory Props")
	FOnSkillDestroy OnSkillDestroy;		// 技能已经用完了，通知角色调用Deactivate
	
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

	UFUNCTION(BlueprintCallable)
	void UseInHandItem();
	
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
