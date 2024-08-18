// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SettingSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class TWINEXPLORERS_API USettingSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	USettingSaveGame();

	// 主音量
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Audio Settings")
	float MasterVolume;

	// 效果音量
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Audio Settings")
	float EffectVolume;

	// 背景音乐音量
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Audio Settings")
	float BGMVolume;

	// 角色设置
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Character Settings")
	FString CharacterName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Network Settings")
	FString IpAndPort;				// 服务器地址
};
