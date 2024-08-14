// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CustomRateCaptureComponent2D.h"

UCustomRateCaptureComponent2D::UCustomRateCaptureComponent2D() {
	// Setup tick to enable manual control
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f / 40.0f;  // 默认40帧一秒
	
	// 默认关闭每游戏帧采集
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;

	bIsEnabled = true;
}

void UCustomRateCaptureComponent2D::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(bIsEnabled) {
		CaptureScene();
	}
}

void UCustomRateCaptureComponent2D::ChangeFramePerSec(int32 FramePerSec) {
	if (FramePerSec > 0 && FramePerSec <= 60) {
		this->SetComponentTickInterval(1.f / static_cast<float>(FramePerSec));
	} else {
		UE_LOG(LogTemp, Warning, TEXT("FramePerSec value out of range: %d"), FramePerSec);
	}
}

void UCustomRateCaptureComponent2D::Enable() {
	if(bIsEnabled) { return; }
	this->SetActive(true);
	bIsEnabled = true;
}

void UCustomRateCaptureComponent2D::Disable() {
	if(!bIsEnabled) { return; }
	this->SetActive(false);
	bIsEnabled = false;
}


