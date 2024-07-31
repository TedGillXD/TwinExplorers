// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Contraptions/ContraptionActorBase.h"
#include "MechanismBase.generated.h"

class ASwitchBase;
/**
 * 
 */
UCLASS()
class TWINEXPLORERS_API AMechanismBase : public AContraptionActorBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category="Mechanism Props")
	TArray<ASwitchBase*> RelatedSwitches;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Mechanism Comps")
	USceneComponent* AsRoot;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Mechanism Props")
	bool bUseDefaultBehavior;		// 使用默认的行为，也就是只有开关两种形态
	
public:
	AMechanismBase();

	virtual void BeginPlay() override;

protected:
	// UFUNCTION(BlueprintNativeEvent)
	UFUNCTION()
	void SwitchOn();

	// UFUNCTION(BlueprintNativeEvent)
	UFUNCTION()
	void SwitchOff();

	UFUNCTION(BlueprintImplementableEvent)
	void Activate();

	UFUNCTION(BlueprintImplementableEvent)
	void Deactivate();
};
