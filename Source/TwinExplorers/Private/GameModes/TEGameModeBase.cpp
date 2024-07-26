// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/TEGameModeBase.h"

#include "Controllers/TEPlayerController.h"

ATEGameModeBase::ATEGameModeBase() {
	MaxPlayerCount = 2;
}

void ATEGameModeBase::BeginPlay() {
	Super::BeginPlay();
	
}

void ATEGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) {
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	ConnectedControllers.Add(Cast<ATEPlayerController>(NewPlayer));

	// 重新设置到对应的PlayerStart
	RestartPlayerAtPlayerStart(NewPlayer, FindPlayerStart(nullptr, FString("Player") + FString::FromInt(ConnectedControllers.Num() - 1)));
}

void ATEGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage) {
	// 拦截更多的链接
	int32 CurrentPlayerCount = GetNumPlayers();
	if(CurrentPlayerCount >= MaxPlayerCount) {
		ErrorMessage = FString::Printf(TEXT("Server is full. Max players allowed is %d."), MaxPlayerCount);
	} else {
		Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	}
}
