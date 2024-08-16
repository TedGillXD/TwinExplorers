// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MainLevelGameState.generated.h"

/**
 * 
 */
UCLASS()
class TWINEXPLORERS_API AMainLevelGameState : public AGameStateBase
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadOnly, Category="GameState Props")
	TArray<FLinearColor> UsedPortalColors;		// 使用过的颜色

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameState Props", Replicated)
	TArray<FLinearColor> AvailableColors;			// 未使用过的颜色

public:
	UFUNCTION(BlueprintCallable)
	FLinearColor GetAvailableColor();
};
