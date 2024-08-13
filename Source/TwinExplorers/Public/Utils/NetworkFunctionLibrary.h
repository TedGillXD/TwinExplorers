// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NetworkFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TWINEXPLORERS_API UNetworkFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static bool GetIPAndPortFromString(const FString& Input, FString& OutIP, FString& OutPort);
};
