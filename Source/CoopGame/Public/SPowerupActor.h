// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerupActor.generated.h"

//106 ʵ�ֵ���Ч����һЩ����
//108 ������������������Ĳ���M_PowerupDecal���ڻ���SPowerupActor��������ͼPowerup_SuperSpeed����ͼ��ʵ��OnActivated��OnExpired�߼���ʵ����Ҽ��ٺ�ȡ�����ٹ���
//111 
	//������ͼPowerup_HealthRegen����ͼ��ʵ��OnPowerupTicked�߼���ʵ����һظ�����ֵ����
	//��SHealthComponent.cpp��ʵ��Heal�ָ�����ֵ����

//113 ʹ�õ��߹��������ڶ�����Ϸ��������������ֵ�ָ�����Ϊ�������ٵ���������114���ø���̳���ʽ���Ϲ�ͬ�Ĳ��֣�
	//��SPickupActor.cpp�����SetReplicates(true);��if (Role == ROLE_Authority)��ʹ�ô���SPowerupActor.cpp�е�ActivatePowerup����ֻ�ڷ������н���
	//SPowerupActor.cpp�У����������ͱ���bIsPowerupActive���󶨰󶨸��ƴ�������OnRep_PowerupActive��OnRep_PowerupActiveʹ����OnRep_PowerupActive��ͼ�ɴ����¼�
	//����ͼPowerup_HealthRegen�У�OnRep_PowerupActive�¼����ڿ��Ƶ��ߵĳ��ֺ���ʧ��OnExpired�¼��������ٵ���

//114 Ϊ����ֵ�ָ���������ٵ��ߴ������ಢ�̳�
	//�Ҽ���ͼPowerup_HealthRegen��duplicate����һ�ݣ�����ΪBP_PowerupBase,��mesh�����ù�������reset��ȥ������OnRep_PowerupActive��OnExpired�������߹�ͬ�õ����¼���ɾ��OnPowerupTicked�¼��߼�
	//����ͼPowerup_HealthRegen�У�Class settings - details - Class Options - parent Class ѡ��BP_PowerupBase��Ϊ���࣬��������д�"_0"�����ɾ��

//115 
	//������ֵ�ָ���������������ҵģ��޸���Powerup_HealthRegen��ͼ��һЩ�ڵ�
	//�ü���Ч��ֻ������ʰ����ߵ���ң�ΪOnActivated�������βΣ��޸���Powerup_SuperSpeed��ͼ��һЩ�ڵ�
UCLASS()
class COOPGAME_API ASPowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPowerupActor();

protected:

	/* Time between powerup ticks */
	//������ǿ�͵���Ч����ʱ����
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerupInterval;

	/* Total times we apply the powerup effect */
	//Ӧ����ǿ�͵���Ч�����ܴ���
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 TotalNrOfTicks;

	FTimerHandle TimerHandle_PowerupTick;

	// Total number of ticks applied
	//Ӧ�ù����ܴ���
	int32 TicksProcessed;

	UFUNCTION()
	void OnTickPowerup();

	// Keeps state of the power-up
	UPROPERTY(ReplicatedUsing=OnRep_PowerupActive)
	bool bIsPowerupActive;

	UFUNCTION()
	void OnRep_PowerupActive();

	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupStateChanged(bool bNewIsActive);

public:	

	void ActivatePowerup(AActor* ActiveFor);

	//��ͼ��ʵ���¼�������ʱ��Powerup_SuperSpeed��ͼ�����ϣ�
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated(AActor* ActiveFor);

	//��ͼ��ʵ���¼�������ʱ��Powerup_HealthRegen��ͼ�����ϣ�
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupTicked();

	//��ͼ��ʵ���¼�������ʱ��Powerup_SuperSpeed��Powerup_HealthRegen��ͼ�����ϣ�
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();
	
};
