// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MainCharacterBase.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/GrabComponent.h"
#include "Components/IceGenerationComponent.h"
#include "Components/InteractComponent.h"
#include "Components/InventoryComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Items/InHandToolActorBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AMainCharacterBase::AMainCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Character的基本设置
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Character的基本组件
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
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

void AMainCharacterBase::AddControllerPitchInput(float Val) {
	Super::AddControllerPitchInput(Val);

	// 更新Pitch，如果现在是服务器，则这个更新会通过OnRep函数复制到客户端
	CameraPitch = FirstPersonCamera->GetComponentRotation().Pitch;
	UpdateCameraPitchOnServer(CameraPitch);		// 否则，通过RPC更新角色在服务器中的Pitch
}

void AMainCharacterBase::UseInHandItemPressed() {
	if(AInHandToolActorBase* InHandToolActor = Cast<AInHandToolActorBase>(InHandItemActor->GetChildActor())) {
		InHandToolActor->UseInHandItemPressed(this);
	}
}

void AMainCharacterBase::UseInHandItemReleased() {
	if(AInHandToolActorBase* InHandToolActor = Cast<AInHandToolActorBase>(InHandItemActor->GetChildActor())) {
		InHandToolActor->UseInHandItemReleased(this);
	}
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

void AMainCharacterBase::InHandItemChanged(int32 NewIndex, const FItem& Item) {
	if(HasAuthority()) {
		InHandItemActor->SetChildActorClass(Item.ItemActorClass);
	} else {
		InHandItemChangedOnServer(NewIndex, Item);
	}
}

void AMainCharacterBase::Transport_Implementation(const FVector& TargetLocation, const FRotator& TargetRotation) {
	// 这个函数默认执行在客户端
	SetActorLocationAndRotation(TargetLocation, TargetRotation);		// 在客户端执行一次使其能计算出准确的Velocity
	if(GetController()) {		// 在客户端中设置这个ControlRotation才能生效
		GetController()->SetControlRotation(TargetRotation);
	}
	SetSpeedAndTransformOnServer(TargetLocation, TargetRotation, GetActorForwardVector() * GetVelocity().Length());
}

void AMainCharacterBase::SetSpeedAndTransformOnServer_Implementation(const FVector TargetLocation,
	const FRotator& TargetRotation, const FVector& NewVelocity) {
	// 下面的设置Location和Rotation需要在Server做
	SetActorLocationAndRotation(TargetLocation, TargetRotation);
	LaunchCharacter(NewVelocity, true, true);
}

void AMainCharacterBase::InHandItemChangedOnServer_Implementation(int32 NewIndex, const FItem& Item) {
	InHandItemChanged(NewIndex, Item);
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
}