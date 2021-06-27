// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

//092 
//ʹ��Pawn����C++�࣬��ӱ�ը����ľ�̬������SM_TrackerBot �� ����M_TrackerBot
//���Nav Mesh Bounds Volume Ѱ·������߽����������P����������ʾ��Ӱ������
//��.cpp�����MeshComp->SetCanEverAffectNavigation(false);������ը�����mesh��Ѱ·�����Ӱ��
//��Ins������ʹ������ڵ���

//093 ʵ�ּ򵥰��GetNextPathPoint���ҵ���ը����ͨ��Actor����һ��·����
//094 ��093�����ϸ���ը����ʩ��һ������������������һ��·���㷽���ƶ�

//095 �������ֵ�����������ֵ�ı�ʱ��������HandleTakeDamage�������н�HealthComp��ΪOwningHealthComp�������ͻ
//096 ����ܵ��˺���������˸�Ķ�̬���ʣ�����M_TrackerBot���߼�����ͼ�б༭�����忴��Ƶ��
//097 �����������ֵΪ0ʱ�ı�ը����
//098 ���SphereComp���������ͨ���ص�����NotifyActorBeginoverlap�붨ʱ������ʵ�ֿ������ʱ�Լ���Ѫ��������097������ֵΪ0ʱ��ը����ʵ���Ա�����

//099 ������屬ը�������֪��Χ����Ч
	//��.wav�ļ����Ҽ�create cue����cue�ļ�������BP_TrackerBot��details��ʹ��
	//��������������½���ͼ��Sounds-Sound Attenuation ����˥��������details - Attenuation Distance-Attenuation Funtionѡ��Natural Sound
	//��cue��Ч�ļ���details-Attenuation-Attenuation Settingsѡ������������˥���ļ��������ͻ�����������Դ�ľ������˥��������û���֮ǰ��ͬλ����������������100%������
//100 ������������Ч  ����ͼBP_TrackerBot�����Audio��Ч�����������ͼ�����ø����ٶȸı�������С�� ��Cue�ļ���details-��ѡLooping
//100 ��SM_TrackerBot��̬�������ļ��У�Collision-Remove Collision��ԭ���ļ���ײɾ����Collision-Add Sphere Simplified Collision ������μ���ײ

//P101 ������Ϸģʽ�£���Ĭ������£������������ڿͻ����в����أ�ֻ�ڷ����������У���.cpp����Ҫ����if (Role == ROLE_Authority)������ͻ��˷��ʵ��յĵ�����ر������������
//P102 ���������Ϸģʽ�����屬ը��Ч��������˸���ڿͻ����в���ʾ�����⣺
	//1��
	//���ļ��е�HandleTakeDamage����������ֵ�ı�ʱ���У����ڲ������屬ը��Ч��������˸��
	//����SHealthComponent.cpp������ֵ���ֻ�ڷ������а��˺�������HandleTakeAnyDamage���˺��������н�Health�㲥��ȥ���⵼�¿ͻ��˵�����ֵһֱ���䣬�޷�������HandleTakeDamage������
	//��ˣ���SHealthComponent.h��ͨ��ReplicatedUsing=OnRep_Health�󶨸�����Ӧ������������SHealthComponent.cpp��ʵ��OnRep_Health���㲥Health�ĸ���
	//2��
	//��SelfDestruct()���������if (Role == ROLE_Authority)������ը�˺��ŵ�������������
	//��Destroy�����ĳ�SetLifeSpan(2.0f);����������٣��ÿͻ������㹻�ķ�Ӧʱ�����������屬ը��Ч��������˸Ч������ǰ��ʹ��MeshComp->SetVisibility(false, true)����������������ʵ����δ����
	//3��
	//��NotifyActorBeginOverlap�����������˺�Ҳͨ��if (Role == ROLE_Authority)�ŵ�������������

//103 
	//ʵ����OnCheckNearbyBots��������������֮���໥��Ӧ����Ӧ����������˸����beginPlay�����ö�ʱ����ÿ1�봥��OnCheckNearbyBots
	//��SelfDestruct��������һ�䣬����Խ�࣬�˺�Խ�ߣ�
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

	//��ȡ��ը������һ��������
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

	//���屬ը����
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
	//103 ����֮���໥��Ӧ����Ӧ����������˸
	void OnCheckNearbyBots();

	// the power boost of the bot, affects damaged caused to enemies and color of the bot (range: 1 to 4)
	int32 PowerLevel;

	FTimerHandle TimerHandle_RefreshPath;

	void RefreshPath();
};
