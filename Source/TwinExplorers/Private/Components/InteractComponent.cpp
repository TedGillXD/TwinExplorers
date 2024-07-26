// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InteractComponent.h"

#include "Camera/CameraComponent.h"
#include "Characters/MainCharacterBase.h"
#include "Interfaces/InteractableInterface.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UInteractComponent::UInteractComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	
	InteractionDetectLength = 1000.f;
}

// Called when the game starts
void UInteractComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<AMainCharacterBase>(GetOwner());
	if(Owner) {
		FirstPersonCamera = Owner->GetCameraComponent();
		if(FirstPersonCamera) {
			bIsInitialized = true;
		}
	}
}


// Called every frame
void UInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(bIsInitialized) {
		DetectInteractions();
	}
}

void UInteractComponent::InteractOnServer_Implementation(AActor* InDetectedActor) {
	FItem TempItem{};		// 用来测试的，这个InHandItem应该是从Inventory中得到的
	if(InDetectedActor && InDetectedActor->Implements<UInteractableInterface>() && IInteractableInterface::Execute_CanInteract(InDetectedActor, TempItem)) {
		IInteractableInterface::Execute_Interact(InDetectedActor, Owner, TempItem);
	}
}

void UInteractComponent::Interact() {
	if(GetOwnerRole() == ROLE_Authority) {
		FItem TempItem{};		// 用来测试的，这个InHandItem应该是从Inventory中得到的
		if(DetectedActor && DetectedActor->Implements<UInteractableInterface>() && IInteractableInterface::Execute_CanInteract(DetectedActor, TempItem)) {
			IInteractableInterface::Execute_Interact(DetectedActor, Owner, TempItem);
		}
	} else {		// 如果现在在客户端，则将其放到服务端上写
		InteractOnServer(DetectedActor);
	}
}

void UInteractComponent::DetectInteractions() {
	FVector Start = FirstPersonCamera->GetComponentLocation();
	FVector End = Start + FirstPersonCamera->GetForwardVector() * InteractionDetectLength;
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	bool IsHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
	if(IsHit && HitResult.GetActor()->Implements<UInteractableInterface>()) {		// 检测到物体了，并且是一个Interactable的物体
		// 当检测到的Actor需要更新状态的时候
		if(IInteractableInterface::Execute_ShouldUpdate(HitResult.GetActor())) {
			IInteractableInterface::Execute_Updated(HitResult.GetActor());
			OnDetectedActorChanged.Broadcast(HitResult.GetActor());
		}
		if(DetectedActor != HitResult.GetActor()) {		// 当检测到的Actor产生变化的时候
			OnDetectedActorChanged.Broadcast(HitResult.GetActor());
			// 处理聚焦和失焦
			IInteractableInterface::Execute_Focused(HitResult.GetActor());
			if(DetectedActor && DetectedActor->Implements<UInteractableInterface>()) {
				IInteractableInterface::Execute_Unfocused(DetectedActor);
			}
		}
		DetectedActor = HitResult.GetActor();
	} else {	// 检测到物体了，但不是一个Interactable的物体
		if(DetectedActor != HitResult.GetActor()) {		// 当没有检测到Actor的时候
			OnDetectedActorChanged.Broadcast(HitResult.GetActor());
			// 仅处理失焦，因为能进入这部分代码要么就是没检测到物体要么就是检测到的物体是不可互动的
			if(DetectedActor && DetectedActor->Implements<UInteractableInterface>()) {
				IInteractableInterface::Execute_Unfocused(DetectedActor);
			}
		}
		DetectedActor = nullptr;
	}
}