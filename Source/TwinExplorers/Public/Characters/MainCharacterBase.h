// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/MainCharacterInterface.h"
#include "MainCharacterBase.generated.h"

class UGrabComponent;
class UInventoryComponent;
class UInteractComponent;
class UCameraComponent;

UCLASS()
class TWINEXPLORERS_API AMainCharacterBase : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UInteractComponent* InteractComponent;		// 提供与物体交互能力的Component

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UInventoryComponent* InventoryComponent;		// 提供存储拾取物品的功能

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UGrabComponent* GrabComponent;			// 用来提供抓取物品的功能
	
public:
	// Sets default values for this character's properties
	AMainCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UCameraComponent* GetCameraComponent() const;
	UInteractComponent* GetInteractComponent() const;
	UInventoryComponent* GetInventoryComponent() const;
	UGrabComponent* GetGrabComponent() const;
};
