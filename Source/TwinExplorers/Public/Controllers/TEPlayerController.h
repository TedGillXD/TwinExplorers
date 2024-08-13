// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameModes/TEGameModeBase.h"
#include "TEPlayerController.generated.h"

UENUM(BlueprintType)
enum EInputDevice {
	Keyboard = 0,
	GamePad
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundCountDownChanged, int32, LeftTimeInSecond);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoundTitleChanged, FString, NewTitle, int32, StageTimeInSecond);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventCountDownChanged, int32, NextEventTimeLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEventTitleChanged, FString, NewTitle, int32, StageTimeInSecond);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundEnd, bool, bIsHumanWin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEscPressed);

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
/**
 * 
 */
UCLASS()
class TWINEXPLORERS_API ATEPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputMappingContext* PlayingMappingContext;		// 正常游玩状态下的MappingContext

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* MoveAction;				// 角色前后左右移动

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* LookAction;				// 鼠标四处看的Action

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* JumpAction;				// 跳跃Action

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* InteractAction;			// 和物体互动的Action

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* DragItemPressedAction;				// 使用物品的Action按下事件

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* DragItemReleaseAction;				// 使用物品的Action松开事件

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* UseSkillAction;		// 使用技能的Action事件

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* AttackAction;			// 鬼的攻击事件

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* EscAction;			// ESC按钮事件

	// 用来通知客户端进行倒计时更新的委托
	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="UI")
	FOnRoundCountDownChanged OnRoundCountDownChanged;

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="UI")
	FOnRoundTitleChanged OnRoundTitleChanged;

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="UI")
	FOnRoundEnd OnRoundEnd;

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="UI")
	FOnEscPressed OnEscPressed;

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="UI")
	FOnEventCountDownChanged OnEventCountDownChanged;

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="UI")
	FOnEventTitleChanged OnEventTitleChanged;
	
public:
	void DragItemPressed();
	void DragItemReleased();
	void UseSkillPressed();
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	void StartJump();
	void StopJump();
	void Attack();

	void OpenEscMenu();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION(Client, Reliable)
	void UpdateCountDown(int32 RoundTime);

	UFUNCTION(Client, Reliable)
	void UpdateCountDownTitle(const FString& String, int32 StageTime);

	UFUNCTION(Client, Reliable)
	void EndRound(bool bIsHumanWin);			// 回合结束

	UFUNCTION(BlueprintCallable, Client, Reliable)
	void PlaySoundOnClient(USoundBase* SoundBase);

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void PlaySoundAtLocationOnAllClient(USoundBase* SoundBase, FVector Location, FRotator Rotation);

	UFUNCTION(Client, Reliable)
	void BackToMainMenuLevel();

	// 事件倒计时
	UFUNCTION(Client, Reliable)
	void UpdateEventCountDown(int32 CurrentStartWaitTimeLeft);

	// 事件标题更新
	UFUNCTION(Client, Reliable)
	void UpdateEventCountDownTitle(const FString& String, int32 StageTime);
	
private:
	void BindInputContext() const;

public:
	UFUNCTION(BlueprintPure)
	TEnumAsByte<EInputDevice> GetCurrentInputDevice() const;
};
