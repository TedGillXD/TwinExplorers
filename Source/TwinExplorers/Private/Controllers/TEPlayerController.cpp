// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/TEPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CommonInputSubsystem.h"
#include "InputActionValue.h"
#include "Characters/MainCharacterBase.h"
#include "Components/InteractComponent.h"
#include "GameFramework/Character.h"


void ATEPlayerController::UseItemPressed() {
	// for test now
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->UseInHandItemPressed();
	}
	
}

void ATEPlayerController::UseItemReleased() {
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->UseInHandItemReleased();
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

void ATEPlayerController::FirstTool() {
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->SelectTool(0);
	}
}

void ATEPlayerController::SecondTool() {
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->SelectTool(1);
	}
}

void ATEPlayerController::ThirdTool() {
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->SelectTool(2);
	}
}

void ATEPlayerController::NextTool() {
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->NextTool();
	}
}

void ATEPlayerController::PreviousTool() {
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->PreviousTool();
	}
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
		EnhancedInputComponent->BindAction(UseItemButtonPressedAction, ETriggerEvent::Triggered, this, &ATEPlayerController::UseItemPressed);
		EnhancedInputComponent->BindAction(UseItemButtonReleasedAction, ETriggerEvent::Triggered, this, &ATEPlayerController::UseItemReleased);
		EnhancedInputComponent->BindAction(CancelUseItemBottomPressedAction, ETriggerEvent::Triggered, this, &ATEPlayerController::CancelUseItemPressed);
		EnhancedInputComponent->BindAction(CancelUseItemBottomReleasedAction, ETriggerEvent::Triggered, this, &ATEPlayerController::CancelUseItemReleased);

		// 切换工具的事件
		EnhancedInputComponent->BindAction(FirstToolAction, ETriggerEvent::Triggered, this, &ATEPlayerController::FirstTool);
		EnhancedInputComponent->BindAction(SecondToolAction, ETriggerEvent::Triggered, this, &ATEPlayerController::SecondTool);
		EnhancedInputComponent->BindAction(ThirdToolAction, ETriggerEvent::Triggered, this, &ATEPlayerController::ThirdTool);
		EnhancedInputComponent->BindAction(NextToolAction, ETriggerEvent::Triggered, this, &ATEPlayerController::NextTool);
		EnhancedInputComponent->BindAction(PreviousToolAction, ETriggerEvent::Triggered, this, &ATEPlayerController::PreviousTool);
	}
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
