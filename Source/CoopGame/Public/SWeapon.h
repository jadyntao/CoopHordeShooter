// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

//P059
//创建Actor类的蓝图BP_targetdummy，作为敌人，并在EventGraph中设置节点，被击中时绘制一个球体

//P062 添加十字准星
//创建蓝图控件WBP_Crosshair,添加image,并设置为中心位置
//在蓝图PlayPawn的BeginPlayEvent中，添加节点creat Widget与Add to viewport

//P069 添加表面类型sufacetype
//在编辑器中Edit-Project settings - Engine - Physics - Physical Surface 创建两个自定义表面类型：FleshDefault（默认肉体）、FleshVulnerable（易受伤肉体）
//在内容浏览器中Core创建两个Physics物理材质，分别命名为PhysMat_FleshDefualt、PhysMat_FleshVulnerable,并点击，在surface type 中选择上述表面类型
//在内容浏览器中Content-AnimStarterPack-UE4_Mannequin-Mesh - 物理资源文件SK_Mannequin_PhysicsAsset，选择头部，在details- Collision -Phys Material Override 中选择物理材质文件

// Contains information of a single hitscan weapon linetrace
//P087 自定义关于开火特效的结构体，用于客户端之间同步。
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaceType;

	UPROPERTY()
	FVector_NetQuantize TraceTo;
};


UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:

	virtual void BeginPlay() override;

	//P054 为了让武器显示化，添加虚幻骨骼网络体组件，命名为MeshComp
	//在编辑器中创建蓝图类BP_Rifle，在Event Graph中的Event BeginPlay中添加相关的逻辑（具体逻辑看视频P054），让游戏初始化时玩家就显示武器
	//在SK_Mannequin骨架网格物体，添加插槽，用于右手与武器相连
	//P055 在SK_Mannequin设置中调整武器的位姿，使得显示正确的握枪姿势
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	void PlayFireEffects(FVector TraceEnd);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	//P058 伤害类型
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	//P070 默认冲击效果
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;

	//P070 肉体冲击效果
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	//P056 开火函数
	virtual void Fire();

	//P086确保在代码中调用该函数时，该请求会发送至服务器上运行，并不是在客户端上运行
	//需要在.cpp中实现_Implementation与_validate函数，而在.h中不需要定义
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	/* RPM - Bullets per minute fired by weapon */
	//P073 每分钟的最大子弹发射数
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	/* Bullet Spread in Degrees */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin=0.0f))
	float BulletSpread;
	
	// Derived from RateOfFire
	float TimeBetweenShots;

	//服务端的HitScanTrace更新时，能够触发其他客户端执行onRep_HitScanTrace函数
	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

public:	
	//P073 添加自动开火功能（包括开始开火和结束开火功能）
	void StartFire();

	void StopFire();
	
};
