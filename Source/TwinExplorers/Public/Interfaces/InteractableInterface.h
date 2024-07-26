// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Items/Item.h"
#include "InteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TWINEXPLORERS_API IInteractableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanInteract(const FItem& InHandItem);
	virtual bool CanInteract_Implementation(const FItem& InHandItem) = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(APawn* FromPawn, const FItem& InHandItem);
	virtual void Interact_Implementation(APawn* FromPawn, const FItem& InHandItem) = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FString GetInteractString(const FItem& InHandItem);			// 返回互动的字符串
	virtual FString GetInteractString_Implementation(const FItem& InHandItem) = 0;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UTexture2D* GetInteractIcon(const FItem& InHandItem);			// 返回一个用什么键来互动的标签
	virtual UTexture2D* GetInteractIcon_Implementation(const FItem& InHandItem) = 0;

	// 互动更新
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ShouldUpdate();					// 返回物体是否需要更新状态
	virtual bool ShouldUpdate_Implementation() = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Updated();							// 标记为已更新
	virtual void Updated_Implementation() = 0;

	// 聚焦与失焦
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Focused();								// 当检测到物体时，调用这个函数
	virtual void Focused_Implementation() = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Unfocused();							// 当失去检测时，调用这个函数
	virtual void Unfocused_Implementation() = 0;
	
};
