// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ThrowableObject.generated.h"

class USphereComponent;

UCLASS()
class TWINEXPLORERS_API AThrowableObject : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	USceneComponent* AsRoot;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UStaticMeshComponent* StaticMeshComp;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	USphereComponent* SphereComp;		// 用来检测是否碰到角色的Comp
	
	bool bCanTriggered;			// 是否可以触发重叠事件了，这个只需要在服务器上存储就可以了，因为所有的重叠判断都是在服务器上进行的
	
public:	
	// Sets default values for this actor's properties
	AThrowableObject();

	UStaticMeshComponent* GetStaticMeshComp() const { return StaticMeshComp; }

protected:
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintNativeEvent)
	void OnTriggered(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
