// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/TEPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CommonInputSubsystem.h"
#include "InputActionValue.h"
#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#include "Characters/MainCharacterBase.h"
#include "Components/InteractComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


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

void ATEPlayerController::UseSkillPressed() {
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->UseSkillPressed();
	}
}

void ATEPlayerController::Move(const FInputActionValue& Value) {
	if(!GetPawn()) { return; }

	FVector2D MovementVector = Value.Get<FVector2D>();
	
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	// 移动
	GetPawn()->AddMovementInput(ForwardDirection, MovementVector.Y);
	GetPawn()->AddMovementInput(RightDirection, MovementVector.X);
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

void ATEPlayerController::Attack() {
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		CharacterBase->Attack();
	}
}

void ATEPlayerController::OpenEscMenu() {
	OnEscPressed.Broadcast();
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
		EnhancedInputComponent->BindAction(UseSkillAction, ETriggerEvent::Triggered, this, &ATEPlayerController::UseSkillPressed);

		// 攻击
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ATEPlayerController::Attack);

		// 打开菜单
		EnhancedInputComponent->BindAction(EscAction, ETriggerEvent::Triggered, this, &ATEPlayerController::OpenEscMenu);
	}
}

void ATEPlayerController::UpdateCountDown_Implementation(int32 RoundTime) {
	OnRoundCountDownChanged.Broadcast(RoundTime);
}

void ATEPlayerController::UpdateCountDownTitle_Implementation(const FString& String, int32 StageTime) {
	OnRoundTitleChanged.Broadcast(String, StageTime);
}

void ATEPlayerController::EndRound_Implementation(bool bIsHumanWin) {
	OnRoundEnd.Broadcast(bIsHumanWin);
	DisableInput(this);
}

void ATEPlayerController::PlaySoundOnClient_Implementation(USoundBase* SoundBase) {
	UGameplayStatics::PlaySound2D(GetWorld(), SoundBase);
}

void ATEPlayerController::PlaySoundAtLocationOnAllClient_Implementation(USoundBase* SoundBase, FVector Location, FRotator Rotation) {
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundBase, Location, Rotation);
}

void ATEPlayerController::BackToMainMenuLevel_Implementation() {
	UGameplayStatics::OpenLevel(this, FName("Level_StartupMenu"));
}

void ATEPlayerController::UpdateEventCountDown_Implementation(int32 CurrentStartWaitTimeLeft) {
	OnEventCountDownChanged.Broadcast(CurrentStartWaitTimeLeft);
}

void ATEPlayerController::UpdateEventCountDownTitle_Implementation(const FString& String, int32 StageTime) {
	OnEventTitleChanged.Broadcast(String, StageTime);
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
