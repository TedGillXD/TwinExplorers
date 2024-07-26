// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GrabableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGrabableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TWINEXPLORERS_API IGrabableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnGrab();									// 在物体被Grab的时候会调用
	virtual void OnGrab_Implementation() = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDrop();									// 在物体给Drop的时候会调用
	virtual void OnDrop_Implementation() = 0;
};
