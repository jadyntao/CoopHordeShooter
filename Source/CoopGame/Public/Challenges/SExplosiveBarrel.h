// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

//P082 玩家射击红桶，红桶爆炸后，材质发生改变，并向上弹起，且将周围物体炸开
//P090 实现多人模式下的红桶爆炸：
	//在蓝图编辑器中，对蓝色方块勾选Static Mesh Replicate Movement 静态网格体复制移动属性，实现红桶爆炸时对蓝色方块作用力的同步
	//bExploded设为复制，更新时，所有客户端都执行OnRep_Exploded函数，执行爆炸时的粒子效果和改变材质，这里是视觉效果上的同步
	//而红桶向上移动通过在构造函数中SetReplicates(true);SetReplicateMovement(true);来实现同步
class USHealthComponent;
class UStaticMeshComponent;
class URadialForceComponent;
class UParticleSystem;


UCLASS()
class COOPGAME_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:

	//生命值组件
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USHealthComponent* HealthComp;

	//静态网格体组件
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;

	//径向力组件
	UPROPERTY(VisibleAnywhere, Category = "Components")
	URadialForceComponent* RadialForceComp;

	//生命值发生更改时的函数，用于绑定生命值组件
	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, 
		class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(ReplicatedUsing=OnRep_Exploded)
	bool bExploded;

	UFUNCTION()
	void OnRep_Exploded();

	/* Impulse applied to the barrel mesh when it explodes to boost it up a little */
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionImpulse;
	
	/* Particle to play when health reached zero */
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplosionEffect;

	/* The material to replace the original on the mesh once exploded (a blackened version) */
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UMaterialInterface* ExplodedMaterial;

};
