// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameMode.generated.h"

//P084 �ڱ༭��������playstart���½�Gamemodebase����ͼ�࣬
//��details-classes�н�default pawn class��Ϊ�Զ����PlayerPawn�������Ϳ����ڶ���ģʽ����ҵ���ʾ��

//118 ɾ����ԭ���������Զ����ɵ�CoopGameGameModeBase�����ļ�SGameMode�ǻ���GameModeBase������
//unreal�Դ���GameMode��ͬ���Ǽ̳���GameModeBase�࣬����ͨ��shift+alt+o ���������зֱ����������
//gameMode���ж�����PlayerState��GameState���̳����½������࣬��˱��ļ�SGameMode�̳���GameModeBase��������
//GameState�������ڳ��������뵱ǰ��Ϸ��صĸ�����Ϣ����ΪGamemodeֻ������������ϣ��ͻ����ϲ�����GameMode�ĸ���������޷���GameMode�д�Ÿ�����Ϣ��
//PlayerState��������ҵ�ȫ���־���Ϣ����Ϊ�����ͻ�����Ҳ��������ҿ�������ֻ�������������������

//119-121 ʵ���ñ�ը����������ھ�����ҽ�Զ��λ������
//119 EQS���
//1��������ѯģ��
	//�༭��-Editor Preferences -����environment - ��ѡEnvironment Querying System - �����༭��
	//���ݱ༭���У��Ҽ�-Artificial Intelligence - Environment Query ����ΪEQS_FindSpawnLocation
	//��EQS_FindSpawnLocation�У���ROOT�ڵ��������ߣ�ѡ��Points Grid ����״���磩��������ò�ѯ����Χ���ɼ򵥵�����
	//���SimpleGrid����details-Projection Data-Post Projection Vertical Offset��Ϊ50���������������ƫ�Ƴ̶ȣ���ֹ���ɵ�����һ��ʼ��Ƕ��ذ�
	//��EQS_FindSpawnLocation�У��Ҽ�SimpleGrid��generate around Querier - Add Test - Distance

//2��������ѯ����������
	//���ݱ༭���У��Ҽ�-�½���ͼ��-EnvQueryContext_BlueprintBase,����ΪEnvQueryContext_BotSpawns��˫����ȥ���������-Functions- Override -Provide Actor Set
	//�������ڵ�Provide Actors Set -�� Return Node֮�����Get All Actors Of Class��Actor Classѡ��TargetPoint��
	//�ص�EQS_FindSpawnLocation�У���Points Grid - details��Generate Around ѡ�� EnvQueryContext_BotSpawns

	//���ݱ༭���У��Ҽ�-�½���ͼ��-EnvQueryContext_BlueprintBase,����ΪEnvQueryContext_AllPlayers��˫����ȥ���������-Functions- Override -Provide Actor Set
	//�������ڵ�Provide Actors Set -�� Return Node֮�����Get All Actors Of Class��Actor Classѡ��SCharacter��
	//�ص�EQS_FindSpawnLocation�У����Distance��to Querier - Details - Distance - Distance To ѡ��EnvQueryContext_AllPlayers
	//�ص�EQS_FindSpawnLocation�У����Distance��to Querier - Details - Filter -Filter Type ѡ��Minimum ��Float Value Min ��Ϊ200����ʾ�ڴ���200����λ���ȵľ������ɻ����� 

//3�����ӻ�
	//���ݱ༭���У��Ҽ�-�½���ͼ��-EQSTestingPawn ����ΪBP_EQSTestingPawn�����볡���У���details-EQS-Query Template��ѡ��EQS_FindSpawnLocation�����ڿ��ӻ����ֳ�����Ч��
	//�ڳ���������TargetPoint�����Կ�������Χ������״����
	//�ڳ���������һ�����PlayerPawn���ϵ���״��������Կ����������Χ�ı����ɫDistance(0),������ɺ쵽�Ƶ��̽��䣬��ʾ����Խ��Խ��

//120 ��119�����Ͻ�����ͼ���ã�������ɻ�����
	//�½���ͼBP_TestGameMode����Event BeginPlay�¼�����Run EQSQuery��Query Templateѡ��EQS_FindSpawnLocation��Querierѡ��Self��Run Modeѡ��Random Item From Bset 25%��
	//����AssignOnQueryFinished�¼��ڵ㣨����EQS��ѯ������Query Status ��������ӵ� switch on EEnvQuesyStatus�����ز�ѯ�Ƿ�ɹ�����
	//��Query Instance���ӵ�Get Result as Locations����ȡҪ���ɵĻ�����λ�ã�
	//����ڵ㣬�����ɵ������塢����λ�����ɻ����˵ȹ��ܣ�����ͼ�ļ�BP_TestGameMode

//121 ���ɻ����˵�һЩ�߼�����Ҫ�Ƕ�ʱ��
//122 �������ɻ������߼����ȴ���һ���Ļ����˶��ҵ����ٿ�ʼ��һ��
//123 �����߼���CheckAnyPlayerAlive��GameOver��������

//130 RestartDeadPlayers���������������
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
	//�ڵ�ǰ������Ҫ���ɵĻ�������
	int32 NrOfBotsToSpawn;

	//��WaveCount�����ɻ�����
	int32 WaveCount;

	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	float TimeBetweenWaves;
	
protected:

	// Hook for BP to spawn a single bot
	//������ͼ�����ɵ���������
	UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnNewBot();

	void SpawnBotTimerElapsed();

	// Start Spawning Bots
	//��ʼ��һ�����ɻ�����
	void StartWave();

	// Stop Spawning Bots
	//������һ���Ļ���������
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
