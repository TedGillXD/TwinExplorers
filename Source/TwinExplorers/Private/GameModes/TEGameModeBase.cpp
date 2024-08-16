// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/TEGameModeBase.h"

#include "EngineUtils.h"
#include "Characters/MainCharacterBase.h"
#include "Controllers/TEPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Items/ItemActorBase.h"
#include "Kismet/GameplayStatics.h"
#include "Objects/PortalV2.h"

ATEGameModeBase::ATEGameModeBase() {
	MaxPlayerCount = 5;
	StartWaitTime = 5.f;
	RoundTime = 120.f;	// 2分钟，先用20秒做测试
	PortalRelinkInterval = 20.f;
	ItemSpawnInterval = 30.0f;	// 每分钟生成道具，先用10秒做测试
}

void ATEGameModeBase::BeginPlay() {
	Super::BeginPlay();

	// 获取所有的能生成道具的位置
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpawnItemClass, SpawnItemLocations);

	// 初始化 SpawnLocationStatusMap
	for (AActor* SpawnLocation : SpawnItemLocations) {
		SpawnLocationStatusMap.Add(SpawnLocation, false);
	}
}

void ATEGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) {
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	ATEPlayerController* PlayerController = Cast<ATEPlayerController>(NewPlayer);
	if(PlayerController) {
		PlayerController->FocusOnGame();
		ConnectedControllers.Add(PlayerController);
	}

	// 当加入到足够多的人数之后，开始进入准备阶段，此时玩家仍然可以加入
	if (ConnectedControllers.Num() >= RoundPlayerCount) {
		IntoPrepareStage();
	}
}

void ATEGameModeBase::Logout(AController* Exiting) {
	Super::Logout(Exiting);

	// 当有玩家登出的时候移除服务器上的Controller
	ConnectedControllers.Remove(Cast<ATEPlayerController>(Exiting));
}

AActor* ATEGameModeBase::ChoosePlayerStart_Implementation(AController* Player) {
	TArray<AActor*> AvailablePlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), AvailablePlayerStarts);

	int32 PlayerIndex = GetNumPlayers() - 1;

	if (AvailablePlayerStarts.IsValidIndex(PlayerIndex)) {
		return AvailablePlayerStarts[PlayerIndex];
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

void ATEGameModeBase::StartRoundPrepare() {
	GetWorldTimerManager().ClearTimer(TimerHandle_PrepareCountDown);
	SetPlayerRoles();
	StartRound();
}

void ATEGameModeBase::StartRound() {
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("Round Start!"));
	// 更新HUD上方的倒计时
	for(ATEPlayerController* Controller : ConnectedControllers) {
		Controller->StartRound(RoundTime, FString(TEXT("距离本回合结束还有")), ItemSpawnInterval, FString(TEXT("距离下一轮道具生成还有")));
	}
	
	GetWorldTimerManager().SetTimer(TimerHandle_ItemSpawn, this, &ATEGameModeBase::SpawnItem, ItemSpawnInterval, true);		// 开始生成道具倒计时
	EventTimeLeft = ItemSpawnInterval;		// 重置EventTimeLeft
	GetWorldTimerManager().SetTimer(TimerHandle_ItemSpawnCountDown, this, &ATEGameModeBase::ItemSpawnCountDown, 1.f, true);

	CurrentRoundTimeLeft = RoundTime;			// 重置回合时间
	GetWorldTimerManager().SetTimer(TimerHandle_RoundEnd, this, &ATEGameModeBase::EndRound, RoundTime, false);			// 回合结束倒计时
	GetWorldTimerManager().SetTimer(TimerHandle_RoundCountDown, this, &ATEGameModeBase::CountDown, 1.f, true);			// 关卡结束倒计时
}

void ATEGameModeBase::SpawnItem() {
	// 如果没有指定生成的道具并且场景中没有标记位置，直接返回
	if (SpawnItemLocations.Num() == 0 || ItemClasses.Num() == 0) { return; }

	// 获取所有未生成道具的SpawnLocation
	TArray<AActor*> AvailableLocations;
	for (AActor* Location : SpawnItemLocations) {
		if (!SpawnLocationStatusMap[Location]) {
			AvailableLocations.Add(Location);
		}
	}

	if (AvailableLocations.Num() == 0) {
		// 如果没有可用的SpawnLocation，直接返回
		return;
	}
	
	// 随机选择一个数量n，n的范围为1到3的数量
	int32 NumToSpawn = FMath::RandRange(1, FMath::Min(3, AvailableLocations.Num()));

	// 从AvailableLocations中随机选择n个位置
	TArray<AActor*> SelectedLocations;
	for (int32 i = 0; i < NumToSpawn; i++) {
		int32 RandomIndex = FMath::RandRange(0, AvailableLocations.Num() - 1);
		SelectedLocations.Add(AvailableLocations[RandomIndex]);
		AvailableLocations.RemoveAt(RandomIndex); // 确保每个位置只能选择一次
	}

	// 对每个选择的位置生成道具
	for (AActor* SpawnLocationActor : SelectedLocations) {
		FVector SpawnLocation = SpawnLocationActor->GetActorLocation();
		FRotator SpawnRotation = FRotator::ZeroRotator;

		int32 RandomItemIndex = FMath::RandRange(0, ItemClasses.Num() - 1);
		TSubclassOf<AItemActorBase> SelectedItemClass = ItemClasses[RandomItemIndex];
		if (!SelectedItemClass) { continue; }

		AItemActorBase* ItemActor = GetWorld()->SpawnActor<AItemActorBase>(SelectedItemClass, SpawnLocation, SpawnRotation);
		ItemActor->SpawnLocationActorRef = SpawnLocationActor;
		ItemActor->OnItemBeingPicked.AddDynamic(this, &ATEGameModeBase::PickedItem);
		SpawnedItems.Add(ItemActor);

		// 更新SpawnLocation的状态为true，表示已生成物品
		SpawnLocationStatusMap[SpawnLocationActor] = true;
	}
	
	EventTimeLeft = ItemSpawnInterval;		// 重置倒计时
}

void ATEGameModeBase::RelinkPortals() const {
	TArray<AActor*> Portals;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APortalV2::StaticClass(), Portals);
	
	// 打乱传送门数组
	for (int32 i = Portals.Num() - 1; i > 0; --i) {
		int32 j = FMath::RandRange(0, i);
		Portals.Swap(i, j);
	}

	// 两两配对传送门
	for (int32 i = 0; i < Portals.Num(); i += 2) {
		APortalV2* Portal1 = Cast<APortalV2>(Portals[i]);
		APortalV2* Portal2 = Cast<APortalV2>(Portals[i + 1]);

		// if (Portal1 && Portal2) {
		// 	APortalV2::Relink(Portal1, Portal2, );
		// }
	}
}

void ATEGameModeBase::ItemSpawnCountDown() {
	if(EventTimeLeft < 0) { return; }
	
	EventTimeLeft--;
	for(ATEPlayerController* Controller : ConnectedControllers) {
		Controller->UpdateEventCountDown(EventTimeLeft);
	}
}

void ATEGameModeBase::EndRound() {
	// 清除道具生成定时器
	GetWorldTimerManager().ClearTimer(TimerHandle_ItemSpawn);
	GetWorldTimerManager().ClearTimer(TimerHandle_RoundCountDown);

	// 检查场上是否还有人类角色
	bool bHumanRemaining = false;
	for (AActor* Actor : TActorRange<AActor>(GetWorld())) {
		AMainCharacterBase* Character = Cast<AMainCharacterBase>(Actor);
		if (Character && Character->GetCharacterTeam() == ECharacterTeam::Human) {
			bHumanRemaining = true;
			break;
		}
	}

	if (bHumanRemaining) {
		// 如果还有人类，则人类获胜
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("Humans Win!"));
	} else {
		// 否则鬼获胜
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Enemies Win!"));
	}

	// 可以在这里添加其他回合结束的逻辑
	// 显示结束画面
	for(ATEPlayerController* Controller : ConnectedControllers) {
		Controller->EndRound(bHumanRemaining);
	}

	// 5秒后重置游戏，并踢掉所有玩家
	FTimerHandle TimerHandle_Reset;
	GetWorldTimerManager().SetTimer(TimerHandle_Reset, this, &ATEGameModeBase::ResetGame, 5.f, false);
}

void ATEGameModeBase::SetPlayerRoles() {
	if (ConnectedControllers.Num() > 0) {
		// 随机选择一个玩家作为鬼
		int32 RandomIndex = FMath::RandRange(0, ConnectedControllers.Num() - 1);

		for (int32 i = 0; i < ConnectedControllers.Num(); ++i) {
			ATEPlayerController* PlayerController = ConnectedControllers[i];
			if (PlayerController) {
				AMainCharacterBase* Character = Cast<AMainCharacterBase>(PlayerController->GetCharacter());
				if (Character) {
					if (i == RandomIndex) {
						AssignCharacterTeam(Character, 0);  // 将随机选中的玩家设为鬼
					} else {
						AssignCharacterTeam(Character, 1);  // 其他玩家设为人
					}
				}
			}
		}
	}
}

void ATEGameModeBase::AssignCharacterTeam(AMainCharacterBase* Character, int32 Index) {
	if (Character) {
		if (Index == 0) {
			Character->SetCharacterTeam(ECharacterTeam::Enemy); // 第一个玩家是鬼
		} else {
			Character->SetCharacterTeam(ECharacterTeam::Human); // 其他玩家是人
		}
	}
}

void ATEGameModeBase::IntoPrepareStage() {
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("Waiting for rest of the players!"));
	
	// 更新HUD上方的倒计时
	for(ATEPlayerController* Controller : ConnectedControllers) {
		Controller->EnterPrepareStage(RoundTime, FString(TEXT("距离正式开始还有")));
	}

	CurrentStartWaitTimeLeft = StartWaitTime;			// 重置准备时间
	GetWorldTimerManager().SetTimer(TimerHandle_RoundStart, this, &ATEGameModeBase::StartRoundPrepare, StartWaitTime, false);
	GetWorldTimerManager().SetTimer(TimerHandle_PrepareCountDown, this, &ATEGameModeBase::PrepareCountDown, 1.f, true);
}

void ATEGameModeBase::PrepareCountDown() {
	if(CurrentStartWaitTimeLeft <= 0) { return; }
	
	CurrentStartWaitTimeLeft--;
	for(ATEPlayerController* Controller : ConnectedControllers) {
		Controller->UpdateCountDown(CurrentStartWaitTimeLeft);
	}
}

void ATEGameModeBase::CountDown() {
	if(CurrentRoundTimeLeft <= 0) { return; }
	
	CurrentRoundTimeLeft--;
	for(ATEPlayerController* Controller : ConnectedControllers) {
		Controller->UpdateCountDown(CurrentRoundTimeLeft);
	}
}

void ATEGameModeBase::ResetGame() {
	// 重置整个游戏，重新等待玩家的加入
	// 停止所有相关的计时器
	GetWorldTimerManager().ClearAllTimersForObject(this);

	// 重置游戏状态
	CurrentRoundTimeLeft = RoundTime;  // 重置回合时间
	EventTimeLeft = ItemSpawnInterval; // 重置道具生成时间

	// 清除场地上的所有道具
	for (TPair<AActor*, bool>& Pair : SpawnLocationStatusMap) {
		Pair.Value = false; // 标记所有生成点为未生成状态
	}
	for (AActor* Item : SpawnedItems) {
		Item->Destroy();
	}
	SpawnedItems.Empty();

	// 踢掉所有玩家
	for(ATEPlayerController* Controller : ConnectedControllers) {
		Controller->BackToMainMenuLevel();
	}
	ConnectedControllers.Empty();

	// TODO: 重置其他必要的游戏逻辑或变量
	
}

bool ATEGameModeBase::AreAllPlayersInfected() const {
	for (AActor* Actor : TActorRange<AActor>(GetWorld())) {
		AMainCharacterBase* Character = Cast<AMainCharacterBase>(Actor);
		if (Character && Character->GetCharacterTeam() == ECharacterTeam::Human) {
			return false; // 如果还有人类角色，游戏未结束
		}
	}
	return true; // 所有玩家均已被感染
}

void ATEGameModeBase::PickedItem(AActor* SpawnLocationRef, AItemActorBase* Self) {
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, SpawnLocationRef->GetName() + " Reset!");
	SpawnLocationStatusMap[SpawnLocationRef] = false;
	SpawnedItems.Remove(Self);
}

void ATEGameModeBase::CheckGameOver() {
	if (AreAllPlayersInfected()) {
		// 如果所有玩家都已被感染，结束游戏
		EndRound();
	}
}
