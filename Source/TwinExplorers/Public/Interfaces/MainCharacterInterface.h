// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MainCharacterInterface.generated.h"

class UInventoryComponent;
class UInteractComponent;
class UCameraComponent;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMainCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 这个接口负责定义一些MainCharacter应该有的函数，比如获取组件的getter，或者是接受输入的函数
 */
class TWINEXPLORERS_API IMainCharacterInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// Getters
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Main Character Funcs")
	UCameraComponent* GetCameraComponent() const;
	virtual UCameraComponent* GetCameraComponent_Implementation() const = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Main Character Funcs")
	UInteractComponent* GetInteractComponent() const;
	virtual UInteractComponent* GetInteractComponent_Implementation() const = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Main Character Funcs")
	UInventoryComponent* GetInventoryComponent() const;
	virtual UInventoryComponent* GetInventoryComponent_Implementation() const = 0;
};
