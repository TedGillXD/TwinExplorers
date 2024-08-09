// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item.generated.h"

class AInHandToolActorBase;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FItem {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="ItemProps")
	FName ItemName;			// 物品的名字

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="ItemProps")
	bool bIsTool;			// 是否是能拿在手里的道具，true为可以拿在手上，false为诸如钥匙一样的道具
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="ItemProps")
	TSubclassOf<AInHandToolActorBase> ItemActorClass;		// 实际在游戏场景中的表现类

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="ItemProps")
	UTexture2D* Icon;		// 图标

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="ItemProps")
	uint8 MaxUsageCount;		// 最大使用次数

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="ItemProps")
	uint8 UsageCount;			// 当前的使用次数
};

static FItem EmptyItem = {};