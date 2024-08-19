// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MainCharacterBase.h"

#include "EnhancedInputSubsystems.h"
#include "KismetTraceUtils.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/GrabComponent.h"
#include "Components/IceGenerationComponent.h"
#include "Components/InteractComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/PortalGenerationComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameModes/TEGameModeBase.h"
#include "NiagaraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "GameStates/MainLevelGameState.h"
#include "Items/Skill.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Objects/ThrowableObject.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

class UEnhancedInputLocalPlayerSubsystem;
// Sets default values
AMainCharacterBase::AMainCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Character的基本设置
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Character的基本组件
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetCapsuleComponent());
	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	FirstPersonCamera->SetupAttachment(SpringArm);
	FirstPersonCamera->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCamera->bUsePawnControlRotation = true;

	InteractComponent = CreateDefaultSubobject<UInteractComponent>(TEXT("InteractComp"));
	InteractComponent->SetIsReplicated(true);

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComp"));
	InventoryComponent->SetIsReplicated(true);

	InHandItemActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("InHandItemActor"));
	InHandItemActor->SetIsReplicated(true);
	InHandItemActor->SetupAttachment(GetMesh(), FName("hand"));

	IceGenerationComponent = CreateDefaultSubobject<UIceGenerationComponent>(TEXT("IceGenerationComp"));
	IceGenerationComponent->SetIsReplicated(true);

	PortalGenerationComponent = CreateDefaultSubobject<UPortalGenerationComponent>(TEXT("PortalGenerationComp"));
	PortalGenerationComponent->SetIsReplicated(true);

    PhysicsConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraint"));
	PhysicsConstraint->SetConstrainedComponents(GetCapsuleComponent(), NAME_None, nullptr, NAME_None);

	NiagaraParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	NiagaraParticleComponent->SetupAttachment(GetRootComponent());

	DizzyParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DizzyNiagara"));
	DizzyParticleComponent->SetupAttachment(GetRootComponent());

	CharacterNameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("CharacterNameWidget"));
	CharacterNameWidget->SetupAttachment(GetRootComponent());

	ThrowDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("ThrowDirection"));
	ThrowDirection->SetupAttachment(GetMesh(), FName("hand"));
	
    // 绑定更换手中道具的函数
    InventoryComponent->OnSelectedToolChanged.AddDynamic(this, &AMainCharacterBase::InHandItemChanged);
	InventoryComponent->OnSkillDestroy.AddDynamic(this, &AMainCharacterBase::DeactivateSkill);

	bIsInAir = false;

	DefaultDetectionDistance = 300.f;
	ProbeRadius = 20.f;
	InterpSpeed = 5.f;
}

// Called when the game starts or when spawned
void AMainCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if(!HasAuthority()) {
		DefaultDetectionDistance = SpringArm->TargetArmLength;
	}
}

// Called every frame
void AMainCharacterBase::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

	if(!HasAuthority()) {
		CameraCollision();
	}
}

UCameraComponent* AMainCharacterBase::GetCameraComponent() const {
	return FirstPersonCamera;
}

UInteractComponent* AMainCharacterBase::GetInteractComponent() const {
	return InteractComponent;
}

UInventoryComponent* AMainCharacterBase::GetInventoryComponent() const {
	return InventoryComponent;
}

UIceGenerationComponent* AMainCharacterBase::GetIceGenerationComponent() const {
	return IceGenerationComponent;
}

void AMainCharacterBase::SetCharacterTeam(ECharacterTeam NewTeam) {
	CharacterTeam = NewTeam;
}

ECharacterTeam AMainCharacterBase::GetCharacterTeam() const {
	return CharacterTeam;
}

void AMainCharacterBase::GetInfectOnServer_Implementation() {
	SetCharacterTeam(ECharacterTeam::Enemy);
	
	// Only check game over condition from the server
	ATEGameModeBase* GameMode = Cast<ATEGameModeBase>(GetWorld()->GetAuthGameMode());
	if (GameMode) {
		GameMode->CheckGameOver();
	}
}

void AMainCharacterBase::GetInfect() {
	if (HasAuthority()) {
		SetCharacterTeam(ECharacterTeam::Enemy);
		AMainCharacterBase* Character = Cast<AMainCharacterBase>(GetWorld()->GetFirstPlayerController()->GetCharacter());
		ATEGameModeBase* GameMode = Cast<ATEGameModeBase>(GetWorld()->GetAuthGameMode());
		if (!GameMode) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Cannot get auth game mode in function GetInfect()");
			return;
		}
		
		AMainLevelGameState* GameState = GameMode->GetGameState<AMainLevelGameState>();
		if(!GameState) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Cannot get auth game state in function GetInfect()");
			return;
		}

		GameState->CharacterGetInfect();
	} else {
		GetInfectOnServer();
	}
}

void AMainCharacterBase::UseSkillPressed() {
	// 处理技能释放按下
	ASkill* Skill = Cast<ASkill>(InHandItemActor->GetChildActor());
	if(Skill) {
		Skill->ActivateSkill(this);
	}
}

void AMainCharacterBase::UseSkillReleased() {
	// TODO：处理技能释放松开
	
}

void AMainCharacterBase::DeactivateSkill() {
	ASkill* Skill = Cast<ASkill>(InHandItemActor->GetChildActor());
	if(Skill) {
		Skill->DeactivateSkill(this);
	}
}

void AMainCharacterBase::AttackOnServer_Implementation() {
	if(CharacterTeam == ECharacterTeam::Human) { return; }		// 人不会攻击
	if(bIsAttacking) { return; }
	bIsAttacking = true;
	
	MulticastPlayAttackMontage();
	
	FTimerHandle TimerHandle_Attack;
	float PlayTime = AttackMontage ? PlayAnimMontage(AttackMontage, AttackPlayRate) : 0.0f; // 计算动画播放时间，同时在服务器上也播放动画，使其能进行通知
	GetWorldTimerManager().SetTimer(TimerHandle_Attack, FTimerDelegate::CreateLambda([this]() -> void {
		this->bIsAttacking = false;
	}), PlayTime / AttackPlayRate, false);
}

void AMainCharacterBase::Attack() {
	// 客户端处理输入
	if(!HasAuthority()) {
		if(CharacterTeam == ECharacterTeam::Human) { return; }		// 人不会攻击
		if(bIsAttacking) { return; }
		AttackOnServer();
	}
}

void AMainCharacterBase::AttackDetection() {
	if(HasAuthority()) {
		TArray<FHitResult> Results;
		FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
		FVector End = Start + GetActorForwardVector() * 25.f;
		FCollisionShape Box = FCollisionShape::MakeBox(FVector{ 25.0, 25.0, 25.0 });
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		bool bIsHit = GetWorld()->SweepMultiByChannel(Results, Start, End, GetActorRotation().Quaternion(), ECC_Pawn, Box, Params);
		DrawDebugBoxTraceMulti(GetWorld(), Start, End, FVector{ 25.0, 25.0, 25.0 }, GetActorRotation(), EDrawDebugTrace::ForDuration, bIsHit, Results, FLinearColor::Green, FLinearColor::Red, 10.f);
		if(!bIsHit) { return; }
		
		for(FHitResult& Result : Results) {
			if(!Result.GetActor()->IsA<AMainCharacterBase>()) { continue; }

			AMainCharacterBase* OtherCharacter = Cast<AMainCharacterBase>(Result.GetActor());
			if(OtherCharacter->GetCharacterTeam() == ECharacterTeam::Human) {
				OtherCharacter->GetInfect();		// 处理感染
			}
		}
	}
}

void AMainCharacterBase::PlayMontageBroadcast_Implementation(UAnimMontage* Montage, float PlayRate) {
	PlayAnimMontage(Montage, PlayRate);
}

void AMainCharacterBase::PlayMontageOnAllClient_Implementation(UAnimMontage* Montage, float PlayRate) {
	PlayMontageBroadcast(Montage, PlayRate);
}

void AMainCharacterBase::FinishTeleport_Implementation() {
	bIsTeleporting = false;
}

void AMainCharacterBase::FinishSpawningPortals_Implementation(UInputMappingContext* MappingContext) {
	// 在客户端中移除相关的输入
	ATEPlayerController* PlayerController = Cast<ATEPlayerController>(GetController());
	if(!PlayerController) { return; }
	if(auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
		Subsystem->RemoveMappingContext(MappingContext);
	}
}

void AMainCharacterBase::FinishSpawningIcePillar_Implementation(UInputMappingContext* MappingContext) {
	// 在客户端中移除相关的输入
	ATEPlayerController* PlayerController = Cast<ATEPlayerController>(GetController());
	if(!PlayerController) { return; }
	if(auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
		Subsystem->RemoveMappingContext(MappingContext);
	}
}

void AMainCharacterBase::ThrowObject(TSubclassOf<AThrowableObject> ActorClass) {
	FVector ActorForward = GetActorForwardVector();
	ActorForward.Z += ThrowDirection->GetForwardVector().Z;
	SpawnThrowingObjectOnServer(ActorClass, ActorForward);
}

void AMainCharacterBase::StepOnBananaPeel(float LastTime, AActor* PeelNeedToDestroy) {
	// 角色状态改变
	CharacterState = Dizzy;
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]() -> void {
		CharacterState = Normal;
	}), LastTime, false);
	PeelNeedToDestroy->Destroy();

	// 通知客户端完成按键调转
	StepOnBananaPeelOnClient(LastTime);
	PlayNiagaraOnAllClient(Dizzy, LastTime);
}

void AMainCharacterBase::StepOnBananaPeelOnClient_Implementation(float LastTime) {
	ATEPlayerController* PlayerController = Cast<ATEPlayerController>(GetController());
	// 设置输入反转
	PlayerController->InvertMovement();		// 反转操控

	this->DizzyParticleComponent->Deactivate();
	this->DizzyParticleComponent->Activate(true);
	FTimerHandle TimerHandle1;
	GetWorldTimerManager().SetTimer(TimerHandle1, FTimerDelegate::CreateLambda([this]() -> void {
		this->DizzyParticleComponent->Deactivate();
	}), LastTime, false);
	
	FTimerHandle TimerHandle2;
	GetWorldTimerManager().SetTimer(TimerHandle2, FTimerDelegate::CreateLambda([PlayerController]() -> void {
		PlayerController->RestoreMovement();		// 恢复操控
	}), LastTime, false);
}

void AMainCharacterBase::SpawnThrowingObjectOnServer_Implementation(TSubclassOf<AThrowableObject> ActorClass, const FVector& Direction) {
	// 1. 获取手部位置
	FVector HandLocation = GetMesh()->GetSocketLocation(FName("hand"));

	// 2. 在手部位置生成对象
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	AThrowableObject* Throwable = GetWorld()->SpawnActor<AThrowableObject>(ActorClass, HandLocation, {}, SpawnParams);

	// 3. 给予一个初始速度
	if (Throwable) {
		// 设置物理和初始速度
		if (Throwable->GetStaticMeshComp()) {
			Throwable->GetStaticMeshComp()->SetSimulatePhysics(true);
			Throwable->GetStaticMeshComp()->SetPhysicsLinearVelocity(Direction * 1000.f);
		}
	}
}

void AMainCharacterBase::GettingInvisible(float LastTime) {
	SetCharacterVisibilityOnServer(false);
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]() -> void {
		this->SetCharacterVisibilityOnServer(true);
	}), LastTime, false);
}

void AMainCharacterBase::SpeedUp(float LastTime, float SpeedRatio) {
	float OriginalWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	SetWalkSpeedOnServer(SpeedRatio * OriginalWalkSpeed);
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this, OriginalWalkSpeed]() -> void {
		this->SetWalkSpeedOnServer(OriginalWalkSpeed);
	}), LastTime, false);
}

void AMainCharacterBase::SetWalkSpeedOnServer_Implementation(float NewWalkSpeed) {
	GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;
	SetWalkSpeedMulticast(NewWalkSpeed);
}

void AMainCharacterBase::SetCharacterVisibilityMulticast_Implementation(bool NewVisibility) {
	if(NewVisibility) {	// 不隐身
		int32 NumMat = NormalStateCharacterMaterials.Num();
		for(int32 Index = 0; Index < NumMat; Index++) {
			GetMesh()->SetMaterial(Index, NormalStateCharacterMaterials[Index]);
		}
		if(CharacterTeam == ECharacterTeam::Enemy) {
			GetMesh()->SetMaterial(0, EnemyShirtMaterial);
		}
	} else {		// 隐身
		int32 NumMat = InvisibleStateMaterials.Num();
		for(int32 Index = 0; Index < NumMat; Index++) {
			GetMesh()->SetMaterial(Index, InvisibleStateMaterials[Index]);
		}
		if(CharacterTeam == ECharacterTeam::Enemy) {
			GetMesh()->SetMaterial(0, InvisibleEnemyShirt);
		}
	}
}

void AMainCharacterBase::SetCharacterVisibilityOnServer_Implementation(bool NewVisibility) {
	SetCharacterVisibilityMulticast(NewVisibility);
}

void AMainCharacterBase::GetHit_Implementation() {
	GetCapsuleComponent()->SetSimulatePhysics(true);
	bIsInAir = true;
}

void AMainCharacterBase::MulticastPlayAttackMontage_Implementation() {
	// 播放攻击的蒙太奇
	if(AttackMontage) {
		PlayAnimMontage(AttackMontage, AttackPlayRate);
	}
}

void AMainCharacterBase::RecoverFromHit_Implementation() {
	GetCapsuleComponent()->SetSimulatePhysics(false);
	bIsInAir = false;
}

void AMainCharacterBase::SetWalkSpeedMulticast_Implementation(float NewWalkSpeed) {
	GetCharacterMovement()->MaxWalkSpeed = NewWalkSpeed;
}

void AMainCharacterBase::UnPossessed() {
	Super::UnPossessed();

	// 清除和角色有关的数据
	if(HasAuthority()) {
		AMainLevelGameState* MainLevelGameState = Cast<AMainLevelGameState>(UGameplayStatics::GetGameState(GetWorld()));
		if(MainLevelGameState) {
			if(CharacterTeam == ECharacterTeam::Enemy) {
				MainLevelGameState->ReduceEnemy();
			} else {
				MainLevelGameState->ReduceHuman();				
			}
		}
	}
}

void AMainCharacterBase::PlayNiagaraOnAllClient_Implementation(ECharacterState State, float LastTime) {
	if(State == Dizzy) {
		this->DizzyParticleComponent->Deactivate();
		this->DizzyParticleComponent->Activate(true);
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]() -> void {
			this->DizzyParticleComponent->Deactivate();
		}), LastTime, false);
	} else {
		
	}
}

void AMainCharacterBase::OnRep_CameraPitch() const {
	const FRotator Rotator = FirstPersonCamera->GetComponentRotation();
	const FRotator NewRotator{CameraPitch, Rotator.Yaw, Rotator.Roll};
	FirstPersonCamera->SetWorldRotation(NewRotator);
}

void AMainCharacterBase::OnRep_CharacterTeam() const {
	// 根据角色所属队伍处理角色的外观
	if(CharacterTeam == ECharacterTeam::Human) {
		GetMesh()->SetMaterial(0, HumanShirtMaterial);
	} else {
		GetMesh()->SetMaterial(0, EnemyShirtMaterial);
		NiagaraParticleComponent->Deactivate();
		NiagaraParticleComponent->Activate(true);
	}
}

void AMainCharacterBase::OnRep_CharacterState() const {
	
}

void AMainCharacterBase::InHandItemChanged(const FItem& Item) {
	if(HasAuthority()) {
		InHandItemActor->SetChildActorClass(Item.ItemActorClass);
	} else {
		InHandItemChangedOnServer(Item);
	}
}

void AMainCharacterBase::Transport_Implementation(const FVector& TargetLocation, const FRotator& TargetRotation, const FVector& TargetVelocity) {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, "Server Set Transport");

	bIsTeleporting = true;
	
	// 这个函数默认执行在服务器
	float Speed = GetVelocity().Length();
	SetActorLocationAndRotation(TargetLocation, { 0.0, TargetRotation.Yaw, 0.0 });
	if(GetController()) {		// 在客户端中设置这个ControlRotation才能生效，但是需要在服务器上做一次来保证得到正确的速度方向
		GetController()->SetControlRotation(TargetRotation);
	}

	// 速度要在服务端设置
	GetMovementComponent()->Velocity = Speed * GetActorForwardVector();
	SetControlRotationOnClient(TargetLocation, TargetRotation);
}

FVector AMainCharacterBase::GetOriginalVelocity_Implementation() {
	return GetVelocity();
}

void AMainCharacterBase::CameraCollision() {
	FHitResult HitResult;
	FVector Start = SpringArm->GetComponentLocation();
	FVector Dir = FirstPersonCamera->GetComponentLocation() - SpringArm->GetComponentLocation();
	Dir.Normalize();
	FVector End = Start + Dir * DefaultDetectionDistance;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool bIsHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, Dir.ToOrientationQuat(), ECC_Camera, FCollisionShape::MakeSphere(ProbeRadius), Params);

	if(bIsHit) {
		ImpactPoint = HitResult.ImpactPoint;
		float Distance = FVector::Distance(SpringArm->GetComponentLocation(), HitResult.ImpactPoint) - ProbeRadius;
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, Distance, GetWorld()->GetDeltaSeconds(), GetInterpSpeed());
	} else {
		if(!FMath::IsNearlyEqual(SpringArm->TargetArmLength, DefaultDetectionDistance, 1.f)) {
			SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, DefaultDetectionDistance, GetWorld()->GetDeltaSeconds(), GetInterpSpeed());
		}
	}
}

float AMainCharacterBase::GetInterpSpeed() const {
	float Distance = FVector::Distance(FirstPersonCamera->GetComponentLocation(), ImpactPoint);
	return UKismetMathLibrary::MapRangeClamped(Distance, 50.f, DefaultDetectionDistance, InterpSpeed, InterpSpeed * 10.f);
}

void AMainCharacterBase::SetAllPlayersName_Implementation(const TArray<AMainCharacterBase*>& Characters,
                                                          const TArray<FString>& Names) {
	if(Characters.Num() != Names.Num()) {
		UE_LOG(LogTemp, Error, TEXT("The size of Controllers and Names array are not the same!"));
		return;
	}
	
	// 从所有Controller里面获取到角色然后设置名字
	for(int32 Index = 0; Index < Characters.Num(); Index++) {
		if(!Characters[Index]) { continue; }
		Characters[Index]->SetCharacterName(Names[Index]);
	}
}

void AMainCharacterBase::SetControlRotationOnClient_Implementation(const FVector& TargetLocation, const FRotator& TargetRotation) {
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, "Client Set Transport");
	if(GetController()) {		// 在客户端中设置这个ControlRotation才能生效，但是需要在服务器上做一次来保证得到正确的速度方向
		GetController()->SetControlRotation(TargetRotation);
	}

	FinishTeleport();
}

void AMainCharacterBase::InHandItemChangedOnServer_Implementation(const FItem& Item) {
	InHandItemChanged(Item);
}

void AMainCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMainCharacterBase, CameraPitch, COND_None);
	DOREPLIFETIME(AMainCharacterBase, CharacterTeam);
	DOREPLIFETIME_CONDITION(AMainCharacterBase, bIsAttacking, COND_OwnerOnly);
	DOREPLIFETIME(AMainCharacterBase, CharacterState);
}