// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controllers/TEPlayerController.h"
#include "GameFramework/Actor.h"
#include "Skill.generated.h"

class AMainCharacterBase;

UCLASS()
class TWINEXPLORERS_API ASkill : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASkill();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Skill IMC")
	UInputMappingContext* SkillIMC;		// 技能的IMC

	// 按下Q激活技能的时候进行使用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ActivateSkill(AMainCharacterBase* CharacterBase);
	virtual void ActivateSkill_Implementation(AMainCharacterBase* CharacterBase);

	// 技能结束或者主动取消的时候调用
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DeactivateSkill(AMainCharacterBase* CharacterBase);
	virtual void DeactivateSkill_Implementation(AMainCharacterBase* CharacterBase);
};
