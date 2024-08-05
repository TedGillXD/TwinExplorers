// Fill out your copyright notice in the Description page of Project Settings.


#include "Contraptions/Mechanisms/MechanismBase.h"

#include "Contraptions/Switches/SwitchBase.h"

AMechanismBase::AMechanismBase() {
	PrimaryActorTick.bCanEverTick = true;
	
	AsRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(AsRoot);

	bUseDefaultBehavior = true;
}

void AMechanismBase::BeginPlay() {
	Super::BeginPlay();

	if(bUseDefaultBehavior) {
		for (ASwitchBase* CurrentSwitch : RelatedSwitches) {
			CurrentSwitch->OnSwitchActivated.AddDynamic(this, &AMechanismBase::SwitchOn);
			CurrentSwitch->OnSwitchDeactivated.AddDynamic(this, &AMechanismBase::SwitchOff);
		}
	}
}

void AMechanismBase::SwitchOn() {
	bool CurrentIsActive = true;
	for(const ASwitchBase* CurrentSwitch : RelatedSwitches) {
		CurrentIsActive &= CurrentSwitch->bIsOn;
	}
	
	// 检查是否所有的switch都已经是On的状态了
	if(CurrentIsActive) { Activate(); }
}

void AMechanismBase::SwitchOff() {
	Deactivate();		// 只要有其中一个开关关闭了，整个机器就会切换到关闭状态
}

void AMechanismBase::Activate_Implementation() {
	
}

void AMechanismBase::Deactivate_Implementation() {
	
}
