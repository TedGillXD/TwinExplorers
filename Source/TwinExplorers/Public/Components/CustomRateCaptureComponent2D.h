// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "CustomRateCaptureComponent2D.generated.h"

class AMainCharacterBase;
/**
 * 
 */
UCLASS()
class TWINEXPLORERS_API UCustomRateCaptureComponent2D : public USceneCaptureComponent2D
{
	GENERATED_BODY()
protected:
	UPROPERTY(BlueprintReadOnly)
	bool bIsEnabled;			// 是否被开启了


public:
	UCustomRateCaptureComponent2D();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// 修改帧数
	UFUNCTION(BlueprintCallable)
	void ChangeFramePerSec(int32 FramePerSec);

	UFUNCTION(BlueprintCallable)
	void Enable();

	UFUNCTION(BlueprintCallable)
	void Disable();
};
