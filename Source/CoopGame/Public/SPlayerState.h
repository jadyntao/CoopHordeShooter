// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

//126  计算玩家得分，射挂一个球体得20分
//SPlayerState.h基于player state创建的C++文件，用于计算玩家得分
//父类PlayerState.h中已经有定义好的Score变量，但它是BlueprintReadOnly的
//在SGameMode.h中添加声明动态多播委托FOnActorKilled，killerController为了射中目标时发起者是手枪的bug
//在HealthComponent.cpp的HeadleTakeAnyDamage函数中增加关于Gamemode的广播，建立bIsDead

//127 BSP工具
//从Modes-Geometry 中拖出Box，在details-Brush Settings-Brush Type中可以设置为subtractive，默认是addtitive，减法类型跟顺序有关
//点击modes的小屋子图标Geometry Editing几何体编辑模式，或快捷键Shift+5，可以进行一些基础的几何体移动，包括顶点或面的移动
//光照
//点击Bulid旁边的三角，Lighting Quality选择光照质量为Preview预览，便于快速地烘焙出光影效果，点击Bulid生成光影
//点击Life，进入照明模式，可以查看真实光照效果
//在场景框中Life-Optimization Viewmodes - lightmap density可以查看光照贴图密度
//对光照不理想的面，在Details-Surface Properties-Lightmap Resolution设置更小的分辨率数值，默认值是32

//details-Scale U V apply 可以设置UV面的大小
//对于对齐不工整的立方体的平面，details-Geometry-选择Align surface planar使其UV映射均等排列（对于其他不规则的面要选择不同的对齐方式，视频中没细讲）


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
