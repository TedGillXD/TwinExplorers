// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MainCharacterBase.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/GrabComponent.h"
#include "Components/IceGenerationComponent.h"
#include "Components/InteractComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/PortalGenerationComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameModes/TEGameModeBase.h"
#include "Items/InHandToolActorBase.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

// Sets default values
AMainCharacterBase::AMainCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Character的基本设置
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Character的基本组件
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	FirstPersonCamera->SetupAttachment(SpringArm);
	FirstPersonCamera->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCamera->bUsePawnControlRotation = true;

	InteractComponent = CreateDefaultSubobject<UInteractComponent>(TEXT("InteractComp"));
	InteractComponent->SetIsReplicated(true);

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComp"));
	InventoryComponent->SetIsReplicated(true);

	GrabComponent = CreateDefaultSubobject<UGrabComponent>(TEXT("GrabComp"));
	GrabComponent->SetIsReplicated(true);

	InHandItemActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("InHandItemActor"));
	InHandItemActor->SetIsReplicated(true);
	InHandItemActor->SetupAttachment(FirstPersonCamera);

	IceGenerationComponent = CreateDefaultSubobject<UIceGenerationComponent>(TEXT("IceGenerationComp"));
	IceGenerationComponent->SetIsReplicated(true);

	PortalGenerationComponent = CreateDefaultSubobject<UPortalGenerationComponent>(TEXT("PortalGenerationComp"));
	PortalGenerationComponent->SetIsReplicated(true);

    PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	PhysicsConstraint->SetConstrainedComponents(GetCapsuleComponent(), NAME_None, nullptr, NAME_None);
	
    // 绑定更换手中道具的函数
    InventoryComponent->OnSelectedToolChanged.AddDynamic(this, &AMainCharacterBase::InHandItemChanged);
}

// Called when the game starts or when spawned
void AMainCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMainCharacterBase::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	
}

UCameraComponent* AMainCharacterBase::GetCameraComponent() const {
	return FirstPersonCamera;
}

UInteractComponent* AMainCharacterBase::GetInteractComponent() const {
	return InteractComponent;
}

UInventoryComponent* AMainCharacterBase::GetInventoryComponent() const {
	return InventoryComponent;
}

UGrabComponent* AMainCharacterBase::GetGrabComponent() const {
	return GrabComponent;
}

UIceGenerationComponent* AMainCharacterBase::GetIceGenerationComponent() const {
	return IceGenerationComponent;
}

void AMainCharacterBase::SetCharacterTeam(ECharacterTeam NewTeam) {
	CharacterTeam = NewTeam;
}

ECharacterTeam AMainCharacterBase::GetCharacterTeam() const {
	return CharacterTeam;
}

void AMainCharacterBase::GetInfectOnServer_Implementation() {
	SetCharacterTeam(ECharacterTeam::Enemy);
	
	// Only check game over condition from the server
	ATEGameModeBase* GameMode = Cast<ATEGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode) {
		GameMode->CheckGameOver();
	}
}

void AMainCharacterBase::GetInfect() {
	if (HasAuthority()) {
		SetCharacterTeam(ECharacterTeam::Enemy);
		AMainCharacterBase* Character = Cast<AMainCharacterBase>(GetWorld()->GetFirstPlayerController()->GetCharacter());
		ATEGameModeBase* GameMode = Cast<ATEGameModeBase>(GetWorld()->GetAuthGameMode());
		if (GameMode) {
			GameMode->CheckGameOver();
		}
	} else {
		GetInfectOnServer();
	}
}

void AMainCharacterBase::AddControllerPitchInput(float Val) {
	Super::AddControllerPitchInput(Val);

	// 更新Pitch，如果现在是服务器，则这个更新会通过OnRep函数复制到客户端
	CameraPitch = FirstPersonCamera->GetComponentRotation().Pitch;
	UpdateCameraPitchOnServer(CameraPitch);		// 否则，通过RPC更新角色在服务器中的Pitch
}

void AMainCharacterBase::DragItemPressed() {
	GrabComponent->GrabItem();
}

void AMainCharacterBase::DragItemReleased() {
	GrabComponent->DropItem();
}

void AMainCharacterBase::CancelUseItemPressed() {
	if(AInHandToolActorBase* InHandToolActor = Cast<AInHandToolActorBase>(InHandItemActor->GetChildActor())) {
		InHandToolActor->CancelUseItemPressed(this);
	}
}

void AMainCharacterBase::CancelUseItemReleased() {
	if(AInHandToolActorBase* InHandToolActor = Cast<AInHandToolActorBase>(InHandItemActor->GetChildActor())) {
		InHandToolActor->CancelUseItemReleased(this);
	}
}

void AMainCharacterBase::OnRep_CameraPitch() const {
	const FRotator Rotator = FirstPersonCamera->GetComponentRotation();
	const FRotator NewRotator{CameraPitch, Rotator.Yaw, Rotator.Roll};
	FirstPersonCamera->SetWorldRotation(NewRotator);
}

void AMainCharacterBase::OnRep_CharacterTeam() const {
	// TODO：根据队伍处理角色的外观
}

void AMainCharacterBase::InHandItemChanged(const FItem& Item) {
	if(HasAuthority()) {
		InHandItemActor->SetChildActorClass(Item.ItemActorClass);
	} else {
		InHandItemChangedOnServer(Item);
	}
}

void AMainCharacterBase::Transport_Implementation(const FVector& TargetLocation, const FRotator& TargetRotation, const FVector& TargetVelocity) {
	// 这个函数默认执行在服务器
	float Speed = GetVelocity().Length();
	SetActorLocationAndRotation(TargetLocation, { 0.0, TargetRotation.Yaw, 0.0 });
	if(GetController()) {		// 在客户端中设置这个ControlRotation才能生效，但是需要在服务器上做一次来保证得到正确的速度方向
		GetController()->SetControlRotation(TargetRotation);
	}

	// 速度要在服务端设置
	GetMovementComponent()->Velocity = Speed * GetActorForwardVector();
	SetControlRotationOnClient(TargetLocation, TargetRotation);
}

FVector AMainCharacterBase::GetOriginalVelocity_Implementation() {
	return GetVelocity();
}

void AMainCharacterBase::SetControlRotationOnClient_Implementation(const FVector& TargetLocation, const FRotator& TargetRotation) {
	SetActorLocationAndRotation(TargetLocation, { 0.0, TargetRotation.Yaw, 0.0 });
	if(GetController()) {		// 在客户端中设置这个ControlRotation才能生效，但是需要在服务器上做一次来保证得到正确的速度方向
		GetController()->SetControlRotation(TargetRotation);
	}
}

void AMainCharacterBase::InHandItemChangedOnServer_Implementation(const FItem& Item) {
	InHandItemChanged(Item);
}

void AMainCharacterBase::UpdateCameraPitchOnServer_Implementation(float NewCameraPitch) {
	CameraPitch = NewCameraPitch;
	// 因为OnRep_CameraPitch只会在Client被调用，所以要在这个RPC中进行设置
	const FRotator Rotator = FirstPersonCamera->GetComponentRotation();
	const FRotator NewRotator{CameraPitch, Rotator.Yaw, Rotator.Roll};
	FirstPersonCamera->SetWorldRotation(NewRotator);
}

void AMainCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMainCharacterBase, CameraPitch, COND_None);
	DOREPLIFETIME(AMainCharacterBase, CharacterTeam);
}