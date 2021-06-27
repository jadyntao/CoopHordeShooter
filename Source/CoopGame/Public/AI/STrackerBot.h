// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

//092 
//使用Pawn创建C++类，添加爆炸球体的静态网格体SM_TrackerBot 和 材质M_TrackerBot
//添加Nav Mesh Bounds Volume 寻路网格体边界体积，按下P键，可以显示其影响区域。
//在.cpp中添加MeshComp->SetCanEverAffectNavigation(false);消除爆炸球体的mesh对寻路区域的影响
//按Ins键可以使物体归于地面

//093 实现简单版的GetNextPathPoint，找到爆炸球体通向Actor的下一个路径点
//094 在093基础上给爆炸球体施加一个作用力，让其往下一个路径点方向移动

//095 添加生命值组件，绑定生命值改变时触发函数HandleTakeDamage，变量中将HealthComp改为OwningHealthComp，避免冲突
//096 添加受到伤害后脉冲闪烁的动态材质，材质M_TrackerBot的逻辑在蓝图中编辑（具体看视频）
//097 添加球体生命值为0时的爆炸功能
//098 添加SphereComp球体组件，通过重叠函数NotifyActorBeginoverlap与定时器函数实现靠近玩家时自己扣血，并借助097的生命值为0时爆炸函数实现自爆功能

//099 添加球体爆炸与球体感知范围内音效
	//在.wav文件上右键create cue生成cue文件，并在BP_TrackerBot的details中使用
	//在内容浏览器中新建蓝图类Sounds-Sound Attenuation 音量衰减，在其details - Attenuation Distance-Attenuation Funtion选择Natural Sound
	//在cue音效文件上details-Attenuation-Attenuation Settings选择上述的音量衰减文件，这样就会根据玩家与声源的距离进行衰减，避免没添加之前不同位置听到的音量都是100%的问题
//100 添加球体滚动音效  在蓝图BP_TrackerBot中添加Audio音效组件，并在蓝图中设置根据速度改变音量大小。 在Cue文件中details-勾选Looping
//100 在SM_TrackerBot静态网格体文件中，Collision-Remove Collision将原来的简单碰撞删除，Collision-Add Sphere Simplified Collision 添加球形简化碰撞

//P101 多人游戏模式下，在默认情况下，导航网格体在客户端中不加载，只在服务器中运行，在.cpp中需要加上if (Role == ROLE_Authority)来避免客户端访问到空的导航相关变量而程序崩溃
//P102 解决多人游戏模式下球体爆炸特效与脉冲闪烁等在客户端中不显示的问题：
	//1、
	//本文件中的HandleTakeDamage函数在生命值改变时运行，用于播放球体爆炸特效与脉冲闪烁，
	//而在SHealthComponent.cpp中生命值组件只在服务器中绑定伤害处理函数HandleTakeAnyDamage，伤害处理函数中将Health广播出去，这导致客户端的生命值一直不变，无法调到到HandleTakeDamage函数。
	//因此，在SHealthComponent.h中通过ReplicatedUsing=OnRep_Health绑定复制响应函数，并在在SHealthComponent.cpp中实现OnRep_Health，广播Health的更新
	//2、
	//在SelfDestruct()函数中添加if (Role == ROLE_Authority)，将爆炸伤害放到服务器中运行
	//将Destroy（）改成SetLifeSpan(2.0f);两秒后再销毁，让客户端有足够的反应时间来产生球体爆炸特效与脉冲闪烁效果，在前面使用MeshComp->SetVisibility(false, true)先让他看不见，但实际上未销毁
	//3、
	//在NotifyActorBeginOverlap函数的自我伤害也通过if (Role == ROLE_Authority)放到服务器中运行

//103 
	//实现了OnCheckNearbyBots函数，用于球体之间相互感应，感应到就脉冲闪烁；在beginPlay中设置定时器，每1秒触发OnCheckNearbyBots
	//在SelfDestruct函数增加一句，球体越多，伤害越高；
class USHealthComponent;
class USphereComponent;
class USoundCue;

UCLASS()
class COOPGAME_API ASTrackerBot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	USphereComponent* SphereComp;

	UFUNCTION()
	void HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, 
		class AController* InstigatedBy, AActor* DamageCauser);

	//获取爆炸球体下一个导航点
	FVector GetNextPathPoint();

	// Next point in navigation path
	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistanceToTarget;

	// Dynamic material to pulse on damage
	UMaterialInstanceDynamic* MatInst;

	//球体爆炸功能
	void SelfDestruct();

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	UParticleSystem* ExplosionEffect;

	bool bExploded;

	// Did we already kick off self destruct timer
	bool bStartedSelfDestruction;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionRadius;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float ExplosionDamage;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float SelfDamageInterval;

	FTimerHandle TimerHandle_SelfDamage;

	void DamageSelf();

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue* SelfDestructSound;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	USoundCue* ExplodeSound;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected:

	// CHALLENGE CODE	

	// Find nearby enemies and grow in 'power level' based on the amount.
	//103 球体之间相互感应，感应到就脉冲闪烁
	void OnCheckNearbyBots();

	// the power boost of the bot, affects damaged caused to enemies and color of the bot (range: 1 to 4)
	int32 PowerLevel;

	FTimerHandle TimerHandle_RefreshPath;

	void RefreshPath();
};
