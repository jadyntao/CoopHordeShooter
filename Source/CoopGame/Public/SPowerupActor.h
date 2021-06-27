// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPowerupActor.generated.h"

//106 实现道具效果的一些函数
//108 建立并设置贴花组件的材质M_PowerupDecal，在基于SPowerupActor创建的蓝图Powerup_SuperSpeed，蓝图中实现OnActivated和OnExpired逻辑，实现玩家加速和取消加速功能
//111 
	//创建蓝图Powerup_HealthRegen，蓝图中实现OnPowerupTicked逻辑，实现玩家回复生命值功能
	//在SHealthComponent.cpp中实现Heal恢复生命值函数

//113 使得道具功能适用于多人游戏（下面是以生命值恢复道具为例，加速道具则是在114中用父类继承形式用上共同的部分）
	//在SPickupActor.cpp中添加SetReplicates(true);与if (Role == ROLE_Authority)，使得触发SPowerupActor.cpp中的ActivatePowerup函数只在服务器中进行
	//SPowerupActor.cpp中，创建布尔型变量bIsPowerupActive并绑定绑定复制触发函数OnRep_PowerupActive，OnRep_PowerupActive使用了OnRep_PowerupActive蓝图可触发事件
	//在蓝图Powerup_HealthRegen中，OnRep_PowerupActive事件用于控制道具的出现和消失，OnExpired事件用于销毁道具

//114 为生命值恢复道具与加速道具创建父类并继承
	//右键蓝图Powerup_HealthRegen，duplicate复制一份，命名为BP_PowerupBase,将mesh等设置过的属性reset回去，保留OnRep_PowerupActive与OnExpired两个道具共同用到的事件，删除OnPowerupTicked事件逻辑
	//在蓝图Powerup_HealthRegen中，Class settings - details - Class Options - parent Class 选择BP_PowerupBase作为父类，把组件栏中带"_0"的组件删除

//115 
	//让生命值恢复是作用于所有玩家的，修改了Powerup_HealthRegen蓝图中一些节点
	//让加速效果只作用于拾起道具的玩家，为OnActivated增加了形参，修改了Powerup_SuperSpeed蓝图中一些节点
UCLASS()
class COOPGAME_API ASPowerupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPowerupActor();

protected:

	/* Time between powerup ticks */
	//更新增强型道具效果的时间间隔
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float PowerupInterval;

	/* Total times we apply the powerup effect */
	//应用增强型道具效果的总次数
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 TotalNrOfTicks;

	FTimerHandle TimerHandle_PowerupTick;

	// Total number of ticks applied
	//应用过的总次数
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

	//蓝图可实现事件：激活时（Powerup_SuperSpeed蓝图中用上）
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnActivated(AActor* ActiveFor);

	//蓝图可实现事件：更新时（Powerup_HealthRegen蓝图中用上）
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnPowerupTicked();

	//蓝图可实现事件：过期时（Powerup_SuperSpeed、Powerup_HealthRegen蓝图中用上）
	UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
	void OnExpired();
	
};
