// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

//P076 ���ļ��ǻ���Actor Component������C++��
//��unreal�༭���У��ڻ���SCharacter��PlayerPawn��ͼ�ļ�����ӱ����SHealth

// OnHealthChanged event
//P078 �����Զ����¼�������ͼ�е��¼�BeginPlay��tick�ȵȣ�������Ӧ����ֵ�ĸ��ģ����������ʱ���Ŷ��������ܵ��ض��˺����ͣ��һ�����ʱ��ҷ������ս������ȵ�
//��.h�ļ��й�������ͼ����.cpp��ʹ��Broadcast�㲥��ȥ������ͼPlayPawn��EventGraph�п��ϳ�OnHealthChanged�¼���ʵ��һЩ��Ӧ
//����_��̬_�鲥_����_��������������ֵ����ʱ��ǩ��(�¼����͵�����)������ֵ���������ֵ������ֵ�������˺����ͣ����˺������ߣ�
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, OwningHealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

//P076 ��Custom�滻��COOP�����������˸��˵����ֻ��Ϊ�˷��������Զ��������������������ʱ��ͼ�е������б�
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

	//P076 ����ֵ Replicated
	//102 ReplicatedUsing=OnRep_Health
	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
	float Health;

	//102 ReplicatedUsing=OnRep_HealthĬ�Ͽ��Խ���һ�ε�ֵ��Ϊ��������float OldHealth
	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultHealth;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
public:

	float GetHealth() const;

	//P078 BlueprintAssignable������ͼ�и�ֵ�����Ϊ�¼�
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HealthComponent")
	static bool IsFriendly(AActor* ActorA, AActor* ActorB);
};
