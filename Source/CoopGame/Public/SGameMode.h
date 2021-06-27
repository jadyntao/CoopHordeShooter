// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

//P084 在编辑器中拖入playstart，新建Gamemodebase的蓝图类，
//在details-classes中将default pawn class设为自定义的PlayerPawn，这样就可以在多人模式中玩家的显示了

//118 删除了原来工程中自动生成的CoopGameGameModeBase，本文件SGameMode是基于GameModeBase构建的
//unreal自带的GameMode类同样是继承于GameModeBase类，可以通过shift+alt+o 看看两者中分别包含的内容
//gameMode类中定义了PlayerState、GameState，教程中新建这两类，因此本文件SGameMode继承于GameModeBase类来创建
//GameState：可用于承载所有与当前游戏相关的复制信息，因为Gamemode只存在与服务器上，客户端上不存在GameMode的副本，因此无法在GameMode中存放该类信息。
//PlayerState：包含玩家的全部持久信息，因为其他客户端上也不存在玩家控制器，只存在于主机或服务器上

//119-121 实现让爆炸球体机器人在距离玩家较远的位置生成
//119 EQS相关
//1、建立查询模板
	//编辑器-Editor Preferences -搜索environment - 勾选Environment Querying System - 重启编辑器
	//内容编辑器中，右键-Artificial Intelligence - Environment Query 命名为EQS_FindSpawnLocation
	//在EQS_FindSpawnLocation中，从ROOT节点引出连线，选择Points Grid （点状网络），这可以让查询器周围生成简单的网格
	//点击SimpleGrid，在details-Projection Data-Post Projection Vertical Offset设为50，设置网格的上下偏移程度，防止生成的球体一开始就嵌入地板
	//在EQS_FindSpawnLocation中，右键SimpleGrid：generate around Querier - Add Test - Distance

//2、建立查询上下文内容
	//内容编辑器中，右键-新建蓝图类-EnvQueryContext_BlueprintBase,命名为EnvQueryContext_BotSpawns，双击进去，在组件栏-Functions- Override -Provide Actor Set
	//在两个节点Provide Actors Set -》 Return Node之间加入Get All Actors Of Class（Actor Class选择TargetPoint）
	//回到EQS_FindSpawnLocation中，在Points Grid - details中Generate Around 选择 EnvQueryContext_BotSpawns

	//内容编辑器中，右键-新建蓝图类-EnvQueryContext_BlueprintBase,命名为EnvQueryContext_AllPlayers，双击进去，在组件栏-Functions- Override -Provide Actor Set
	//在两个节点Provide Actors Set -》 Return Node之间加入Get All Actors Of Class（Actor Class选择SCharacter）
	//回到EQS_FindSpawnLocation中，点击Distance：to Querier - Details - Distance - Distance To 选择EnvQueryContext_AllPlayers
	//回到EQS_FindSpawnLocation中，点击Distance：to Querier - Details - Filter -Filter Type 选择Minimum ；Float Value Min 设为200，表示在大于200个单位长度的距离生成机器人 

//3、可视化
	//内容编辑器中，右键-新建蓝图类-EQSTestingPawn 命名为BP_EQSTestingPawn，拖入场景中，在details-EQS-Query Template中选择EQS_FindSpawnLocation，用于可视化呈现出设置效果
	//在场景中拖入TargetPoint，可以看到其周围产生点状网格
	//在场景中拖入一个玩家PlayerPawn，拖到点状网络里，可以看到，玩家周围的变成蓝色Distance(0),其余的由红到黄到绿渐变，表示距离越来越大；

//120 在119基础上进行蓝图设置，随机生成机器人
	//新建蓝图BP_TestGameMode，在Event BeginPlay事件连接Run EQSQuery（Query Template选择EQS_FindSpawnLocation，Querier选择Self，Run Mode选择Random Item From Bset 25%）
	//插入AssignOnQueryFinished事件节点（用于EQS查询），在Query Status 输出点连接到 switch on EEnvQuesyStatus（返回查询是否成功），
	//在Query Instance连接到Get Result as Locations（获取要生成的机器人位置）
	//其余节点，如生成调试球体、根据位置生成机器人等功能，见蓝图文件BP_TestGameMode

//121 生成机器人的一些逻辑，主要是定时器
//122 完善生成机器人逻辑，等待上一波的机器人都挂掉了再开始下一波
//123 完善逻辑，CheckAnyPlayerAlive和GameOver两个函数

//130 RestartDeadPlayers复活已死亡的玩家
enum class EWaveState : uint8;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor, AController*, KillerController);


/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	FTimerHandle TimerHandle_BotSpawner;

	FTimerHandle TimerHandle_NextWaveStart;

	// Bots to spawn in current wave
	//在当前波次中要生成的机器人数
	int32 NrOfBotsToSpawn;

	//第WaveCount波生成机器人
	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;
	
protected:

	// Hook for BP to spawn a single bot
	//关联蓝图以生成单个机器人
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBotTimerElapsed();

	// Start Spawning Bots
	//开始新一波生成机器人
	void StartWave();

	// Stop Spawning Bots
	//结束这一波的机器人生成
	void EndWave();

	// Set timer for next startwave
	void PrepareForNextWave();

	void CheckWaveState();

	void CheckAnyPlayerAlive();

	void GameOver();

	void SetWaveState(EWaveState NewState);

	void RestartDeadPlayers();

public:

	ASGameMode();

	virtual void StartPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilled OnActorKilled;
};
