// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractComponent.generated.h"

class AMainCharacterBase;
class UCameraComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectedActorChanged, AActor*, DetectedActor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TWINEXPLORERS_API UInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractComponent();

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bIsInitialized = false;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float InteractionDetectLength;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable)
	void InteractOnServer(AActor* InDetectedActor);
	
	UFUNCTION(BlueprintCallable)
	void Interact();

protected:
	UPROPERTY(Replicated)
	AMainCharacterBase* Owner;

	UPROPERTY()
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(BlueprintReadOnly)
	AActor* DetectedActor = nullptr;

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category="Interaction System")
	FOnDetectedActorChanged OnDetectedActorChanged;
	
	void DetectInteractions();		// 检测角色前面有什么物品
};
