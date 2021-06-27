// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

//P076 本文件是基于Actor Component创建的C++类
//在unreal编辑器中，在基于SCharacter的PlayerPawn蓝图文件中添加本组件SHealth

// OnHealthChanged event
//P078 创建自定义事件（如蓝图中的事件BeginPlay，tick等等），以响应生命值的更改，如玩家死亡时播放动画，或受到特定伤害类型（烈火烧身时玩家发出灼烧叫声）等等
//在.h文件中公开给蓝图，在.cpp中使用Broadcast广播出去，在蓝图PlayPawn的EventGraph中可拖出OnHealthChanged事件，实现一些响应
//声明_动态_组播_代理_六个参数（生命值更改时的签名(事件类型的名称)，生命值组件，生命值，生命值差量，伤害类型，，伤害发起者）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, OwningHealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

//P076 将Custom替换成COOP，以至于有了个人的类别，只是为了方便整理自定义组件，就想添加新组件时蓝图中的下拉列表
UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealthComponent")
	uint8 TeamNum;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool bIsDead;

	//P076 生命值 Replicated
	//102 ReplicatedUsing=OnRep_Health
	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
	float Health;

	//102 ReplicatedUsing=OnRep_Health默认可以将上一次的值作为参数输入float OldHealth
	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultHealth;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
public:

	float GetHealth() const;

	//P078 BlueprintAssignable可在蓝图中赋值，类别为事件
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HealthComponent")
	static bool IsFriendly(AActor* ActorA, AActor* ActorB);
};
