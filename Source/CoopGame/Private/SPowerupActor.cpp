// Fill out your copyright notice in the Description page of Project Settings.

#include "SPowerupActor.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ASPowerupActor::ASPowerupActor()
{
	PowerupInterval = 0.0f;//10
	TotalNrOfTicks = 0;//1

	bIsPowerupActive = false;

	SetReplicates(true);
}


void ASPowerupActor::OnTickPowerup()
{
	//buffӦ�ù��Ĵ���++
	TicksProcessed++;

	OnPowerupTicked();

	//buff�ĳ���ʱ��ΪTotalNrOfTicks*PowerupInterval
	//������д����Ҫ������P111������ֵ�ָ����ߣ�����õ���һ���ָ�5��Ѫ��ÿ��ָ�1��Ѫ��5��ָ���
	if (TicksProcessed >= TotalNrOfTicks)
	{
		//buff����
		OnExpired();

		bIsPowerupActive = false;
		OnRep_PowerupActive();

		// Delete timer
		GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
	}
}


void ASPowerupActor::OnRep_PowerupActive()
{
	OnPowerupStateChanged(bIsPowerupActive);
}

//��SPickupActor.cpp�У��������߷����ص�ʱ����ʰ�����ʱ������
void ASPowerupActor::ActivatePowerup(AActor* ActiveFor)
{
	//����buff״̬
	OnActivated(ActiveFor);

	bIsPowerupActive = true;
	OnRep_PowerupActive();

	//��buff����ʱ������0���򾭹�����ʱ������ͨ��OnTickPowerup��������buff״̬
	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true);
	}
	else//�����൱��buff����ʱ��Ϊ0����������
	{
		OnTickPowerup();
	}
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}