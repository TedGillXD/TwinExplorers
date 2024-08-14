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
	int32 PortalRelinkInterval;			// 传送门重新链接倒计时

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="GameRules")
	TArray<TSubclassOf<AItemActorBase>> ItemClasses;		// 可以用来生成道具的Item Class

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="GameRules")
	TSubclassOf<AActor> SpawnItemClass;		// 用来标记生成物品位置的Class
	
	UPROPERTY(BlueprintReadOnly, Category="GameRules")
	TArray<AActor*> SpawnItemLocations;			// 用来生成道具的位置

	UPROPERTY(BlueprintReadOnly, Category="GameRules")
	TMap<AActor*, bool> SpawnLocationStatusMap;  // 存储 SpawnLocation 的状态，如果目前生成了道具然后还没捡起来，就是true，其他的为false，表示能生成

	UPROPERTY(BlueprintReadOnly)
	TArray<ATEPlayerController*> ConnectedControllers;		// 所有连接上来的Controllers

	FTimerHandle TimerHandle_RoundStart;		// 回合开始倒计时，在玩家数量达到最小值的时候会开始
	FTimerHandle TimerHandle_ItemSpawn;	// 道具生成定时器
	FTimerHandle TimerHandle_RoundEnd;	// 回合结束定时器
	FTimerHandle TimerHandle_RoundCountDown;		// 回合中倒计时Handle
	FTimerHandle TimerHandle_PrepareCountDown;		// 准备阶段倒计时
	FTimerHandle TimerHandle_ItemSpawnCountDown;	// 道具生成倒计时

	float CurrentRoundTimeLeft;			// 当前回合剩余时间
	float CurrentStartWaitTimeLeft;		// 当前准备阶段剩余时间
	float EventTimeLeft;			// 道具生成倒数剩余时间

	virtual void BeginPlay() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

private:
	// 回合开始
	void StartRoundPrepare();
	void StartRound();
	void SetPlayerRoles();
	void AssignCharacterTeam(AMainCharacterBase* Character, int32 Index);
	
	// 回合中事件
	void SpawnItem();
	void RelinkPortals() const;
	void ItemSpawnCountDown();

	// 回合结束事件
	void EndRound();
	void KickAllPlayer();

	void IntoPrepareStage();
	void PrepareCountDown();
	void CountDown();

	void ResetGame();

	bool AreAllPlayersInfected() const;

	UFUNCTION()
	void PickedItem(AActor* SpawnLocationRef);

public:
	UFUNCTION(BlueprintCallable, Category="GameMode")
	void CheckGameOver();
};