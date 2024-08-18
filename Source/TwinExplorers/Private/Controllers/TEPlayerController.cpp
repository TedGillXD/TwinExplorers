// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/TEPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CommonInputSubsystem.h"
#include "InputActionValue.h"
#include "Characters/MainCharacterBase.h"
#include "Components/InteractComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


void ATEPlayerController::DragItemPressed() {
	// for test now
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		
	}
	
}

void ATEPlayerController::DragItemReleased() {
	if(!GetCharacter()) { return; }

	if(AMainCharacterBase* CharacterBase = Cast<AMainCharacterBase>(GetCharacter())) {
		
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

void ATEPlayerController::InvertMovement() {
	if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		Subsystem->AddMappingContext(InvertedPlayingMappingContext, 1);
	}
}

void ATEPlayerController::RestoreMovement() {
	if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
		FModifyContextOptions Options{};
		Options.bIgnoreAllPressedKeysUntilRelease = false;
		Subsystem->RemoveMappingContext(InvertedPlayingMappingContext, Options);
	}
}

void ATEPlayerController::FocusOnGame_Implementation() {
	if(UGameViewportClient* GameViewportClient = GetWorld()->GetGameViewport()) {
		GameViewportClient->Viewport->CaptureMouse(true);
	}
	
	FInputModeGameOnly InputMode;
	this->SetInputMode(InputMode);
	this->bShowMouseCursor = false;
}

void ATEPlayerController::UpdateCountDown_Implementation(int32 RoundTime) {
	OnRoundCountDownChanged.Broadcast(RoundTime);
}

void ATEPlayerController::UpdateCountDownTitle_Implementation(const FString& String, int32 StageTime, bool bShouldPlaySound) {
	OnRoundTitleChanged.Broadcast(String, StageTime, bShouldPlaySound);
}

void ATEPlayerController::EndRound_Implementation(bool bIsHumanWin) {
	RoundStage = ERoundStage::EndRound;
	OnStageChanged.Broadcast(RoundStage);
	
	// 1. 播报比赛结束
	OnRoundEnd.Broadcast(bIsHumanWin);

	// 2. 禁止玩家继续进行输入
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

void ATEPlayerController::EnterPrepareStage_Implementation(int32 StageTime, const FString& StageTitle) {
	RoundStage = Preparing;
	
	// 1. 设置倒计时标题
	OnRoundTitleChanged.Broadcast(StageTitle, StageTime, false);
	
	// 2. 隐藏事件倒计时
	OnStageChanged.Broadcast(RoundStage);
}

void ATEPlayerController::StartRound_Implementation(int32 StageTime, const FString& StageTitle, int32 EventTime, const FString& EventTitle) {
	RoundStage = InRound;
	
	// 1. 设置阶段倒计时
	OnRoundTitleChanged.Broadcast(StageTitle, StageTime, true);

	// 2. 设置事件倒计时并接触事件倒计时隐藏
	OnEventTitleChanged.Broadcast(EventTitle, EventTime);
	OnStageChanged.Broadcast(RoundStage);
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
