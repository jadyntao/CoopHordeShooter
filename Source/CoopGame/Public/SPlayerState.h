// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

//126  ������ҵ÷֣����һ�������20��
//SPlayerState.h����player state������C++�ļ������ڼ�����ҵ÷�
//����PlayerState.h���Ѿ��ж���õ�Score������������BlueprintReadOnly��
//��SGameMode.h�����������̬�ಥί��FOnActorKilled��killerControllerΪ������Ŀ��ʱ����������ǹ��bug
//��HealthComponent.cpp��HeadleTakeAnyDamage���������ӹ���Gamemode�Ĺ㲥������bIsDead

//127 BSP����
//��Modes-Geometry ���ϳ�Box����details-Brush Settings-Brush Type�п�������Ϊsubtractive��Ĭ����addtitive���������͸�˳���й�
//���modes��С����ͼ��Geometry Editing������༭ģʽ�����ݼ�Shift+5�����Խ���һЩ�����ļ������ƶ����������������ƶ�
//����
//���Bulid�Աߵ����ǣ�Lighting Qualityѡ���������ΪPreviewԤ�������ڿ��ٵغ決����ӰЧ�������Bulid���ɹ�Ӱ
//���Life����������ģʽ�����Բ鿴��ʵ����Ч��
//�ڳ�������Life-Optimization Viewmodes - lightmap density���Բ鿴������ͼ�ܶ�
//�Թ��ղ�������棬��Details-Surface Properties-Lightmap Resolution���ø�С�ķֱ�����ֵ��Ĭ��ֵ��32

//details-Scale U V apply ��������UV��Ĵ�С
//���ڶ��벻�������������ƽ�棬details-Geometry-ѡ��Align surface planarʹ��UVӳ��������У������������������Ҫѡ��ͬ�Ķ��뷽ʽ����Ƶ��ûϸ����


/**
 * 
 */
UCLASS()
class COOPGAME_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public: 

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void AddScore(float ScoreDelta);
	
	
};
