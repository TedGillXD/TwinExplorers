// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Contraptions/ContraptionActorBase.h"
#include "SwitchBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSwitchActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSwitchDeactivated);

/**
 * 
 */
UCLASS()
class TWINEXPLORERS_API ASwitchBase : public AContraptionActorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, BlueprintAssignable)
	FOnSwitchActivated OnSwitchActivated;			// 当按键 未激活 -> 激活 时调用

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable)
	FOnSwitchDeactivated OnSwitchDeactivated;		// 当按键 激活 -> 未激活 时调用

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Switch Props")
	bool bIsOn = false;			// 当前的状态
};
