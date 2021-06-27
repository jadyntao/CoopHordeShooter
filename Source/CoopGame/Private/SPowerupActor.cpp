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
	//buff应用过的次数++
	TicksProcessed++;

	OnPowerupTicked();

	//buff的持续时长为TotalNrOfTicks*PowerupInterval
	//这样的写法主要方便于P111的生命值恢复道具，比如该道具一共恢复5格血，每秒恢复1格血，5秒恢复完
	if (TicksProcessed >= TotalNrOfTicks)
	{
		//buff过期
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

//在SPickupActor.cpp中，玩家与道具发生重叠时（即拾起道具时）触发
void ASPowerupActor::ActivatePowerup(AActor* ActiveFor)
{
	//激活buff状态
	OnActivated(ActiveFor);

	bIsPowerupActive = true;
	OnRep_PowerupActive();

	//若buff持续时长大于0，则经过持续时长后在通过OnTickPowerup函数更新buff状态
	if (PowerupInterval > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, this, &ASPowerupActor::OnTickPowerup, PowerupInterval, true);
	}
	else//否则，相当于buff持续时长为0，立即更新
	{
		OnTickPowerup();
	}
}

void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPowerupActor, bIsPowerupActive);
}