// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/MainCharacterInterface.h"
#include "Interfaces/TransportableInterface.h"
#include "Items/Item.h"
#include "MainCharacterBase.generated.h"

class AThrowableObject;
class UNiagaraComponent;
class UInputMappingContext;
class UPhysicsConstraintComponent;
class USpringArmComponent;
class UPortalGenerationComponent;
class UIceGenerationComponent;
class UGrabComponent;
class UInventoryComponent;
class UInteractComponent;
class UCameraComponent;

UENUM(BlueprintType)
enum class ECharacterTeam : uint8 {
	Human UMETA(DisplayName = "Human"),
	Enemy UMETA(DisplayName = "Enemy")
};

UCLASS()
class TWINEXPLORERS_API AMainCharacterBase : public ACharacter, public ITransportableInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	USpringArmComponent* SpringArm;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UInteractComponent* InteractComponent;		// 提供与物体交互能力的Component

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UInventoryComponent* InventoryComponent;		// 提供存储拾取物品的功能

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UGrabComponent* GrabComponent;			// 用来提供抓取物品的功能

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UPortalGenerationComponent* PortalGenerationComponent;		// 提供发射传送门的能力

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UChildActorComponent* InHandItemActor;			// 握在手里的东西，开启复制这样就能把InHandItemActor的变化同步到客户端

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UArrowComponent* ThrowDirection;			// 用来指示丢出物品方向的箭头

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UIceGenerationComponent* IceGenerationComponent;		// 用来在特定的表面上生成柱子的功能

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UPhysicsConstraintComponent* PhysicsConstraint;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Components)
	UNiagaraComponent* NiagaraParticleComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Skill Props")
	TArray<UMaterialInterface*> NormalStateCharacterMaterials;			// 正常状态下的角色材质

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Skill Props")
	TArray<UMaterialInterface*> InvisibleStateMaterials;				// 隐身状态下的角色材质
	
protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CameraPitch)
	float CameraPitch;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CharacterTeam)
	ECharacterTeam CharacterTeam;	// 角色队伍类型

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsAttacking;		// 是否正在攻击

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Team Props")
	UAnimMontage* AttackMontage;		// 攻击动画

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Team Props")
	float AttackPlayRate;				// 攻击动画的播放倍率

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Team Props")
	UMaterialInterface* EnemyShirtMaterial;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Team Props")
	UMaterialInterface* HumanShirtMaterial;

	bool bShouldPlayParticle;
	
public:
	bool bIsTeleporting;		// 是否正在传送
	
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
	UIceGenerationComponent* GetIceGenerationComponent() const;

	void SetCharacterTeam(ECharacterTeam NewTeam);
	ECharacterTeam GetCharacterTeam() const;

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void GetInfectOnServer();

	UFUNCTION(BlueprintCallable)
	void GetInfect();

	virtual void AddControllerPitchInput(float Val) override;
	
	UFUNCTION(BlueprintCallable)
	void DragItemPressed();

	UFUNCTION(BlueprintCallable)
	void DragItemReleased();

	UFUNCTION(BlueprintCallable)
	void UseSkillPressed();

	UFUNCTION(BlueprintCallable)
	void UseSkillReleased();

	UFUNCTION()
	void DeactivateSkill();

	UFUNCTION(Server, Reliable)
	void AttackOnServer();
	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackDetection();

	// 在服务器中用来通知其他所有的客户端中对应的角色播放攻击动画
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAttackMontage();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void PlayMontageOnAllClient(UAnimMontage* Montage, float PlayRate);
	UFUNCTION(NetMulticast, Reliable)
	void PlayMontageBroadcast(UAnimMontage* Montage, float PlayRate);

	// 传送技能
	UFUNCTION(Server, Reliable)
	void FinishTeleport();
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void FinishSpawningPortals(UInputMappingContext* MappingContext);

	// 丢香蕉皮技能
	UFUNCTION(BlueprintCallable)
	void ThrowObject(TSubclassOf<AThrowableObject> ActorClass);
	UFUNCTION(Server, Reliable)
	void SpawnThrowingObjectOnServer(TSubclassOf<AThrowableObject> ActorClass, const FVector& Direction);

	// 收到香蕉皮技能
	UFUNCTION(BlueprintCallable)
	void StepOnBananaPeel(float LastTime, AActor* PeelNeedToDestroy);
	UFUNCTION(Client, Reliable)
	void StepOnBananaPeelOnClient(float LastTime);

	// 造冰技能
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void FinishSpawningIcePillar(UInputMappingContext* MappingContext);

	// 隐身技能
	UFUNCTION(BlueprintCallable)
	void GettingInvisible(float LastTime);			// 开启隐身技能，并在LastTime后结束
	UFUNCTION(Server, Reliable)
	void SetCharacterVisibilityOnServer(bool NewVisibility);
	UFUNCTION(NetMulticast, Reliable)
	void SetCharacterVisibilityMulticast(bool NewVisibility);

	// 加速技能
	UFUNCTION(BlueprintCallable)
	void SpeedUp(float LastTime, float SpeedRatio);
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetWalkSpeedOnServer(float NewWalkSpeed);
	UFUNCTION(NetMulticast, Reliable)
	void SetWalkSpeedMulticast(float NewWalkSpeed);

	UFUNCTION(NetMulticast, Reliable)
	void GetHit();

	UFUNCTION(NetMulticast, Reliable)
	void RecoverFromHit();

protected:
	UFUNCTION()
	void OnRep_CameraPitch() const;

	UFUNCTION()
	void OnRep_CharacterTeam() const;

	// 负责将客户端角色的CameraPitch更新到服务器
	UFUNCTION(Server, Unreliable)
	void UpdateCameraPitchOnServer(float NewCameraPitch);

	UFUNCTION(Server, Reliable)
	void InHandItemChangedOnServer(const FItem& Item);

	UFUNCTION()
	void InHandItemChanged(const FItem& Item);

protected:
	virtual void Transport_Implementation(const FVector& TargetLocation, const FRotator& TargetRotation, const FVector& TargetVelocity) override;

	virtual FVector GetOriginalVelocity_Implementation() override;
	
	UFUNCTION(Client, Reliable)
	void SetControlRotationOnClient(const FVector& TargetLocation, const FRotator& TargetRotation);
	
};