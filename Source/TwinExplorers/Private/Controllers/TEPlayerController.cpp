// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/TEPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CommonInputSubsystem.h"
#include "InputActionValue.h"
#include "Characters/MainCharacterBase.h"
#include "Components/InteractComponent.h"
#include "GameFramework/Character.h"


void ATEPlayerController::DragItemPressed() {
	// for test now
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->DragItemPressed();
	}
	
}

void ATEPlayerController::DragItemReleased() {
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->DragItemReleased();
	}
}

void ATEPlayerController::CancelUseItemPressed() {
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->CancelUseItemPressed();
	}
}

void ATEPlayerController::CancelUseItemReleased() {
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->CancelUseItemReleased();
	}
}

void ATEPlayerController::Move(const FInputActionValue& Value) {
	if(!GetPawn()) { return; }

	FVector2D MovementVector = Value.Get<FVector2D>();
	
	// 移动
	GetPawn()->AddMovementInput(GetPawn()->GetActorForwardVector(), MovementVector.Y);
	GetPawn()->AddMovementInput(GetPawn()->GetActorRightVector(), MovementVector.X);
}
	
void ATEPlayerController::Look(const FInputActionValue& Value) {
	if(!GetPawn()) { return; }

	FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	// 移动摄像机
	GetPawn()->AddControllerYawInput(LookAxisVector.X);
	GetPawn()->AddControllerPitchInput(LookAxisVector.Y);
}

void ATEPlayerController::Interact() {
	if(!GetCharacter()) { return; }

	AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter());
	UInteractComponent* InteractComp = CharacterBase->GetInteractComponent();
	if(InteractComp) {
		InteractComp->Interact();
	}
}

void ATEPlayerController::StartJump() {
	if(!GetCharacter()) { return; }
	GetCharacter()->Jump();
}

void ATEPlayerController::StopJump() {
	if(!GetCharacter()) { return; }
	GetCharacter()->StopJumping();
}

void ATEPlayerController::BeginPlay() {
	Super::BeginPlay();

	BindInputContext();
}

void ATEPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();

	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) {
		// 基础移动
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATEPlayerController::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATEPlayerController::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATEPlayerController::StartJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATEPlayerController::StopJump);

		// 互动事件
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ATEPlayerController::Interact);
		EnhancedInputComponent->BindAction(DragItemPressedAction, ETriggerEvent::Triggered, this, &ATEPlayerController::DragItemPressed);
		EnhancedInputComponent->BindAction(DragItemReleaseAction, ETriggerEvent::Triggered, this, &ATEPlayerController::DragItemReleased);
		EnhancedInputComponent->BindAction(UseToolPressedAction, ETriggerEvent::Triggered, this, &ATEPlayerController::CancelUseItemPressed);
		EnhancedInputComponent->BindAction(UseToolReleaseAction, ETriggerEvent::Triggered, this, &ATEPlayerController::CancelUseItemReleased);
	}
}

void ATEPlayerController::UpdateCountDown_Implementation(int32 RoundTime) {
	OnRoundCountDownChanged.Broadcast(RoundTime);
}

void ATEPlayerController::UpdateCountDownTitle_Implementation(const FString& String, int32 StageTime) {
	OnRoundTitleChanged.Broadcast(String, StageTime);
}

void ATEPlayerController::BindInputContext() const {
	if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		Subsystem->AddMappingContext(PlayingMappingContext, 0);
	}
}

TEnumAsByte<EInputDevice> ATEPlayerController::GetCurrentInputDevice() const {
	if(UCommonInputSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UCommonInputSubsystem>(GetLocalPlayer())) {
		ECommonInputType InputType = InputSubsystem->GetCurrentInputType();
		if(InputType == ECommonInputType::MouseAndKeyboard) {
			return Keyboard;
		}
		if(InputType == ECommonInputType::Gamepad) {
			return GamePad;
		}
	}

	return Keyboard;		// 默认返回Keyboard
}
