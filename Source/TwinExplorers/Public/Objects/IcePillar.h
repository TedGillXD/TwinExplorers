// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractableInterface.h"
#include "IcePillar.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPillarDestroy);

UCLASS()
class TWINEXPLORERS_API AIcePillar : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UStaticMeshComponent* PillarMeshComp;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="IcePillar Props")
	FName InteractItemName;			//能和这个物体进行互动的物品名字

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="IcePillar Props")
	FString InteractString;

	float Current;
	float Target;

public:
	UPROPERTY()
	FOnPillarDestroy OnPillarDestroy;		// 当柱子被销毁的时候调用
	
public:	
	// Sets default values for this actor's properties
	AIcePillar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, Reliable)
	void DestroyOnServer();

	void DestroyPillarOnServer();

	float GetInterpTime(float From, float To, float SpeedInConstant) const;

public:
	virtual bool CanInteract_Implementation(const FItem& InHandItem) override;
	virtual void Interact_Implementation(APawn* FromPawn, const FItem& InHandItem) override;
	virtual FString GetInteractString_Implementation() override;
	virtual UTexture2D* GetInteractIcon_Implementation() override;
	virtual bool ShouldUpdate_Implementation() override;
	virtual void Updated_Implementation() override;
	virtual void Focused_Implementation() override;
	virtual void Unfocused_Implementation() override;
};
