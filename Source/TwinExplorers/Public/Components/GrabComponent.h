// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrabComponent.generated.h"


class UPhysicsConstraintComponent;
class AMainCharacterBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TWINEXPLORERS_API UGrabComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="GrabItem Props")
	UStaticMesh* VisibleMesh;			// 需要有一个能看到的Mesh才能正确绑定PhysicsConstraint

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="GrabItem Props")
	float DetectLength;			// 最远的检测距离

	UPROPERTY(Replicated)
	AMainCharacterBase* Owner;

	UPROPERTY(ReplicatedUsing=OnRep_GrabItemMeshComp)
	UStaticMeshComponent* GrabItemMeshComp;

	UPROPERTY(Replicated)
	UPhysicsConstraintComponent* GrabItemPhysicsConstraintComp;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsInitialized;

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsGrabbing;

	UPROPERTY(BlueprintReadOnly, Replicated)
	AActor* HeldObject;		// 正在被抓取的物品

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_HeldComponent)
	UPrimitiveComponent* HeldComponent;		//

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bIsOn;		// 目前是否能触发

public:	
	// Sets default values for this component's properties
	UGrabComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void AddRequireComponentsOnServer();

	UFUNCTION(Server, Reliable)
	void GrabItemOnServer(const bool bIsHit, const FHitResult& HitResult);

	UFUNCTION(Server, Reliable)
	void DropItemOnServer();

public:
	void GrabItemInternal(bool bIsHit, const FHitResult& HitResult);
	
	UFUNCTION(BlueprintCallable)
	void GrabItem();
	
	UFUNCTION(BlueprintCallable)
	void DropItem();

protected:
	UFUNCTION()
	void OnRep_HeldComponent();

	UFUNCTION()
	void OnRep_GrabItemMeshComp() const;
};
