// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TEGameModeBase.generated.h"

class AItemActorBase;
class AMainCharacterBase;
class ATEPlayerController;
/**
 * 关卡中的GameMode
 */
UCLASS()
class TWINEXPLORERS_API ATEGameModeBase : public AGameModeBase {
	GENERATED_BODY()

public:
	ATEGameModeBase();
	
protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="GameRules")
	int32 MaxPlayerCount;			// 最大玩家数
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GameRules")
	int32 RoundPlayerCount;			// 本局的玩家数量

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="GameRules")
	int32 StartWaitTime;			// 玩家达到最小数量后等待的游戏开始时间，简单来说就是准备阶段

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="GameRules")
	int32 RoundTime;				// 回合时间

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="GameRules")
	int32 ItemSpawnInterval;		// 道具生成间隔时间

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="GameRules")
	TArray<TSubclassOf<AItemActorBase>> ItemClasses;		// 可以用来生成道具的Item Class

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="GameRules")
	TSubclassOf<AActor> SpawnItemClass;		// 用来标记生成物品位置的Class
	
	UPROPERTY(BlueprintReadOnly, Category="GameRules")
	TArray<AActor*> SpawnItemLocations;			// 用来生成道具的位置

	UPROPERTY(BlueprintReadOnly)
	TArray<ATEPlayerController*> ConnectedControllers;		// 所有连接上来的Controllers

	FTimerHandle TimerHandle_RoundStart;		// 回合开始倒计时，在玩家数量达到最小值的时候会开始
	FTimerHandle TimerHandle_ItemSpawn;	// 道具生成定时器
	FTimerHandle TimerHandle_RoundEnd;	// 回合结束定时器
	FTimerHandle TimerHandle_RoundCountDown;		// 回合中倒计时Handle
	FTimerHandle TimerHandle_PrepareCountDown;		// 准备阶段倒计时

	virtual void BeginPlay() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

private:
	void StartRoundPrepare();
	void StartRound();
	void SpawnItem();
	void EndRound();
	void SetPlayerRoles();
	void AssignCharacterTeam(AMainCharacterBase* Character, int32 Index);

	void IntoPrepareStage();
	void PrepareCountDown();
	void CountDown();

	bool AreAllPlayersInfected() const;

public:
	UFUNCTION(BlueprintCallable, Category="GameMode")
	void CheckGameOver();
};