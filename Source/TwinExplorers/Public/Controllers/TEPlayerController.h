// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TEPlayerController.generated.h"

UENUM(BlueprintType)
enum EInputDevice {
	Keyboard = 0,
	GamePad
};

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
	UInputAction* UseItemButtonPressedAction;				// 使用物品的Action按下事件

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* UseItemButtonReleasedAction;				// 使用物品的Action松开事件

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* CancelUseItemBottomPressedAction;		// 取消使用物品的Action按下事件

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* CancelUseItemBottomReleasedAction;	// 取消使用物品的Action松开事件

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* FirstToolAction;		// 第一个工具的事件

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* SecondToolAction;		// 第二个工具的事件

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* ThirdToolAction;		// 第三个工具的事件

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* NextToolAction;		// 下一个工具的事件

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Input|Playing")
	UInputAction* PreviousToolAction;		// 上一个工具的事件

public:
	void UseItemPressed();
	void UseItemReleased();
	void CancelUseItemPressed();
	void CancelUseItemReleased();
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact();
	void StartJump();
	void StopJump();

	void FirstTool();
	void SecondTool();
	void ThirdTool();
	void NextTool();
	void PreviousTool();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void BindInputContext() const;

public:
	UFUNCTION(BlueprintPure)
	TEnumAsByte<EInputDevice> GetCurrentInputDevice() const;
};
