// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/MainLevelGameState.h"

#include "GameModes/TEGameModeBase.h"
#include "Net/UnrealNetwork.h"

FLinearColor AMainLevelGameState::GetAvailableColor() {
	if(!AvailableColors.IsEmpty()) {
		FLinearColor Ret = AvailableColors.Last();
		AvailableColors.Pop();
		return Ret;
	}

	return FLinearColor(FMath::FRand(), FMath::FRand(), FMath::FRand(), 1.0f);		// 否则随机生成一个颜色返回回去
}

void AMainLevelGameState::NewPlayerJoin() {
	NumHumans++;
}

void AMainLevelGameState::OnRep_CountChanged() {
	OnPlayerStateChanged.Broadcast(NumHumans, NumGhosts);
}

void AMainLevelGameState::CharacterGetInfect() {
	NumHumans -= 1;
	NumGhosts += 1;

	// 做游戏结束检查
	if(NumHumans <= 0) {
		ATEGameModeBase* GameMode = Cast<ATEGameModeBase>(GetWorld()->GetAuthGameMode());
		if(!GameMode) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Failed to get game mode on server in game state function CharacterGetInfect()");
			return;
		}

		GameMode->CheckGameOver();
	}
}

void AMainLevelGameState::RefreshPlayerInfos() {
	NumHumans = 0;
	NumGhosts = 0;
}

void AMainLevelGameState::AddNewEnemy() {
	NumGhosts++;

	ATEGameModeBase* GameMode = Cast<ATEGameModeBase>(GetWorld()->GetAuthGameMode());
	if(!GameMode) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Failed to get game mode on server in game state function CharacterGetInfect()");
		return;
	}
	GameMode->CheckGameOver();
}

void AMainLevelGameState::AddNewHuman() {
	NumHumans++;

	ATEGameModeBase* GameMode = Cast<ATEGameModeBase>(GetWorld()->GetAuthGameMode());
	if(!GameMode) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Failed to get game mode on server in game state function CharacterGetInfect()");
		return;
	}
	GameMode->CheckGameOver();
}

void AMainLevelGameState::ReduceEnemy() {
	NumGhosts = FMath::Clamp(NumGhosts - 1, 0, NumGhosts);

	ATEGameModeBase* GameMode = Cast<ATEGameModeBase>(GetWorld()->GetAuthGameMode());
	if(!GameMode) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Failed to get game mode on server in game state function CharacterGetInfect()");
		return;
	}
	GameMode->CheckGameOver();
}

void AMainLevelGameState::ReduceHuman() {
	NumHumans = FMath::Clamp(NumHumans - 1, 0, NumHumans);

	ATEGameModeBase* GameMode = Cast<ATEGameModeBase>(GetWorld()->GetAuthGameMode());
	if(!GameMode) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Failed to get game mode on server in game state function CharacterGetInfect()");
		return;
	}
	GameMode->CheckGameOver();
}

void AMainLevelGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMainLevelGameState, AvailableColors);
	DOREPLIFETIME(AMainLevelGameState, NumHumans);
	DOREPLIFETIME(AMainLevelGameState, NumGhosts);
}
