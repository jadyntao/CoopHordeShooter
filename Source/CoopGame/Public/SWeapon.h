// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;

//P059
//����Actor�����ͼBP_targetdummy����Ϊ���ˣ�����EventGraph�����ýڵ㣬������ʱ����һ������

//P062 ���ʮ��׼��
//������ͼ�ؼ�WBP_Crosshair,���image,������Ϊ����λ��
//����ͼPlayPawn��BeginPlayEvent�У���ӽڵ�creat Widget��Add to viewport

//P069 ��ӱ�������sufacetype
//�ڱ༭����Edit-Project settings - Engine - Physics - Physical Surface ���������Զ���������ͣ�FleshDefault��Ĭ�����壩��FleshVulnerable�����������壩
//�������������Core��������Physics������ʣ��ֱ�����ΪPhysMat_FleshDefualt��PhysMat_FleshVulnerable,���������surface type ��ѡ��������������
//�������������Content-AnimStarterPack-UE4_Mannequin-Mesh - ������Դ�ļ�SK_Mannequin_PhysicsAsset��ѡ��ͷ������details- Collision -Phys Material Override ��ѡ����������ļ�

// Contains information of a single hitscan weapon linetrace
//P087 �Զ�����ڿ�����Ч�Ľṹ�壬���ڿͻ���֮��ͬ����
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

	//P054 Ϊ����������ʾ���������ù������������������ΪMeshComp
	//�ڱ༭���д�����ͼ��BP_Rifle����Event Graph�е�Event BeginPlay�������ص��߼��������߼�����ƵP054��������Ϸ��ʼ��ʱ��Ҿ���ʾ����
	//��SK_Mannequin�Ǽ��������壬��Ӳ�ۣ�������������������
	//P055 ��SK_Mannequin�����е���������λ�ˣ�ʹ����ʾ��ȷ����ǹ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	void PlayFireEffects(FVector TraceEnd);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	//P058 �˺�����
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TracerTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	//P070 Ĭ�ϳ��Ч��
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* DefaultImpactEffect;

	//P070 ������Ч��
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* FleshImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TracerEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShake> FireCamShake;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	//P056 ������
	virtual void Fire();

	//P086ȷ���ڴ����е��øú���ʱ��������ᷢ���������������У��������ڿͻ���������
	//��Ҫ��.cpp��ʵ��_Implementation��_validate����������.h�в���Ҫ����
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire();

	FTimerHandle TimerHandle_TimeBetweenShots;

	float LastFireTime;

	/* RPM - Bullets per minute fired by weapon */
	//P073 ÿ���ӵ�����ӵ�������
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;

	/* Bullet Spread in Degrees */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon", meta = (ClampMin=0.0f))
	float BulletSpread;
	
	// Derived from RateOfFire
	float TimeBetweenShots;

	//����˵�HitScanTrace����ʱ���ܹ����������ͻ���ִ��onRep_HitScanTrace����
	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

public:	
	//P073 ����Զ������ܣ�������ʼ����ͽ��������ܣ�
	void StartFire();

	void StopFire();
	
};
