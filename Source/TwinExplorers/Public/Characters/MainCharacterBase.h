// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/MainCharacterInterface.h"
#include "Items/Item.h"
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

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UChildActorComponent* InHandItemActor;			// 握在手里的东西，开启复制这样就能把InHandItemActor的变化同步到客户端

protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CameraPitch)
	float CameraPitch;
	
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

	virtual void AddControllerPitchInput(float Val) override;
	
	UFUNCTION(BlueprintCallable)
	void UseInHandItemPressed();

	UFUNCTION(BlueprintCallable)
	void UseInHandItemReleased();

protected:
	UFUNCTION()
	void OnRep_CameraPitch() const;

	// 负责将客户端角色的CameraPitch更新到服务器
	UFUNCTION(Server, Unreliable)
	void UpdateCameraPitchOnServer(float NewCameraPitch);

	UFUNCTION(Server, Reliable)
	void InHandItemChangedOnServer(int32 NewIndex, const FItem& Item);

	UFUNCTION()
	void InHandItemChanged(int32 NewIndex, const FItem& Item);
};
