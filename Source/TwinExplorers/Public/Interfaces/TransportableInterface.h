// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TransportableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTransportableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TWINEXPLORERS_API ITransportableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 调用这个函数触发传送
	// 需要实现者自己实现网络同步相关的处理，比如哪部分需要在客户端那部分需要在服务器
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Transport(const FVector& TargetLocation, const FRotator& TargetRotation);
	virtual void Transport_Implementation(const FVector& TargetLocation, const FRotator& TargetRotation) = 0;
};
