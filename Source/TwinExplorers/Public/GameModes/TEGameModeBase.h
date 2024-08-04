// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TEGameModeBase.generated.h"

class ATEPlayerController;
/**
 * 关卡中的GameMode
 */
UCLASS()
class TWINEXPLORERS_API ATEGameModeBase : public AGameModeBase {
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="GameRules")
	int32 MaxPlayerCount;			// 最大玩家数

public:
	ATEGameModeBase();

	virtual void BeginPlay() override;

	// 当一个新玩家加入的时候就会调用这个函数
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	
	// 用来选择PlayerStart
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	
protected:
	UPROPERTY(BlueprintReadOnly)
	TArray<ATEPlayerController*> ConnectedControllers;		// 所有连接上来的Controllers
	
};