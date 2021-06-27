// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameState.generated.h"

//124
//因为Gamemode只存在与服务器上，客户端上不存在GameMode的副本，因此无法在GameMode中存放EWaveState信息复制给客户端。
//基于Gamestatebase创建C++类GameState，用于承载所有与当前游戏相关的复制信息EWaveState。
//通过设置EWaveState类型的WaveState为ReplicatedUsing，来将状态复制给所有客户端
//C++实现了一些函数OnRep_WaveState、SetWaveState

//125蓝图中创建BP_GameState，并实现了一些逻辑 

//蓝图的枚举类仅适用于uint8
UENUM(BlueprintType)
enum class EWaveState : uint8
{
	WaitingToStart,

	WaveInProgress,

	// No longer spawning new bots, waiting for players to kill remaining bots
	WaitingToComplete,

	WaveComplete,

	GameOver,
};



/**
 * 
 */
UCLASS()
class COOPGAME_API ASGameState : public AGameStateBase
{
	GENERATED_BODY()

protected:

	UFUNCTION()
	void OnRep_WaveState(EWaveState OldState);

	UFUNCTION(BlueprintImplementableEvent, Category = "GameState")
	void WaveStateChanged(EWaveState NewState, EWaveState OldState);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = "GameState")
	EWaveState WaveState;

public:

	void SetWaveState(EWaveState NewState);
	
};
