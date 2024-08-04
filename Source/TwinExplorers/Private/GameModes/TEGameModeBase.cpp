// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/TEGameModeBase.h"

#include "Controllers/TEPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ATEGameModeBase::ATEGameModeBase() {
	MaxPlayerCount = 2;
}

void ATEGameModeBase::BeginPlay() {
	Super::BeginPlay();
	
}

void ATEGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) {
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	ConnectedControllers.Add(Cast<ATEPlayerController>(NewPlayer));
}

AActor* ATEGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	// 获取所有可用的 PlayerStarts
	TArray<AActor*> AvailablePlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), AvailablePlayerStarts);

	// 根据玩家加入游戏的顺序选择 PlayerStart
	int32 PlayerIndex = GetNumPlayers() - 1; // 获取当前玩家的索引（假设 GetNumPlayers() 返回当前玩家数量）

	if (AvailablePlayerStarts.IsValidIndex(PlayerIndex)) {
		return AvailablePlayerStarts[PlayerIndex];
	}

	// 如果没有合适的 PlayerStart，返回父类的默认实现
	return Super::ChoosePlayerStart_Implementation(Player);
}