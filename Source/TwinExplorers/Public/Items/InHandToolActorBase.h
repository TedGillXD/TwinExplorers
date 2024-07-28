// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InHandToolActorBase.generated.h"

class AMainCharacterBase;

UCLASS()
class TWINEXPLORERS_API AInHandToolActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInHandToolActorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// 按下使用物品按钮的时候调用这个函数
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void UseInHandItemPressed(AMainCharacterBase* FromCharacter);
	virtual void UseInHandItemPressed_Implementation(AMainCharacterBase* FromCharacter);

	// 松开使用物品按钮的时候调用这个函数
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void UseInHandItemReleased(AMainCharacterBase* FromCharacter);
	virtual void UseInHandItemReleased_Implementation(AMainCharacterBase* FromCharacter);

	// 按下取消使用物品按钮的时候调用这个函数
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void CancelUseItemPressed(AMainCharacterBase* FromCharacter);
	virtual void CancelUseItemPressed_Implementation(AMainCharacterBase* FromCharacter);

	// 松开取消使用物品按钮的时候调用这个函数
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void CancelUseItemReleased(AMainCharacterBase* FromCharacter);
	virtual void CancelUseItemReleased_Implementation(AMainCharacterBase* FromCharacter);
};
