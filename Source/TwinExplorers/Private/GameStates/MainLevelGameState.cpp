// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/MainLevelGameState.h"

#include "Net/UnrealNetwork.h"

FLinearColor AMainLevelGameState::GetAvailableColor() {
	if(!AvailableColors.IsEmpty()) {
		FLinearColor Ret = AvailableColors.Last();
		AvailableColors.Pop();
		return Ret;
	}

	return FLinearColor(FMath::FRand(), FMath::FRand(), FMath::FRand(), 1.0f);		// 否则随机生成一个颜色返回回去
}

void AMainLevelGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMainLevelGameState, AvailableColors);
}
