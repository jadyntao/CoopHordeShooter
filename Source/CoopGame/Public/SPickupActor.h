// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

//105 �������������������������ʵ���������������
//108 ������������������Ĳ���M_PowerupDecal���ڻ���SPowerupActor��������ͼPowerup_SuperSpeed����ͼ��ʵ��OnActivated��OnExpired����
//109 ��SpickupActor��SPowerupActor����������ʵ��Respawn��NotifyActorBeginOverlap��������ͼBP_TestPickup�н�PowerUpClassѡΪSPowerupActor
//110 ʵ�ֵ����Է���Ч������ͼ��ʵ��M_Powerup��M_PowerupLightFunction�������ʣ������ɲ���ʵ��MI_PowerupSpeed�������ھ�̬�������ļ�SpeedIcon
//112 ���Ƶأ�ʵ�ָֻ�����ֵ���ߵ��Է���Ч��������RotaingMovement�����ʵ�ֵ�����ת

class USphereComponent;
class UDecalComponent;
class ASPowerupActor;

UCLASS()
class COOPGAME_API ASPickupActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPickupActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//�������
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	//�������
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	TSubclassOf<ASPowerupActor> PowerUpClass;

	ASPowerupActor* PowerUpInstance;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	float CooldownDuration;

	FTimerHandle TimerHandle_RespawnTimer;

	//������������
	void Respawn();

public:	

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	
};
