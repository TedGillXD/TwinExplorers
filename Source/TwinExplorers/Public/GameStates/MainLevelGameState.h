// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MainLevelGameState.generated.h"

class AMainCharacterBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStateChanged, int32, HumanCount, int32, EnemyCount);

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

	UPROPERTY(BlueprintReadWrite)
	TMap<AMainCharacterBase*, FString> NameMapping;			// 用来记录所有加入服务器的玩家的名字

	UPROPERTY(ReplicatedUsing=OnRep_CountChanged)
	int32 NumHumans;
	
	UPROPERTY(ReplicatedUsing=OnRep_CountChanged)
	int32 NumGhosts;

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable)
	FOnPlayerStateChanged OnPlayerStateChanged;				// 当鬼或者人的数量发生变化的时候调用

public:
	UFUNCTION(BlueprintCallable)
	FLinearColor GetAvailableColor();

	UFUNCTION(BlueprintCallable)
	void NewPlayerJoin();

	UFUNCTION()
	void OnRep_CountChanged();

	UFUNCTION()
	void CharacterGetInfect();
	
	void RefreshPlayerInfos();
	void AddNewEnemy();
	void AddNewHuman();
	void ReduceEnemy();
	void ReduceHuman();
};
