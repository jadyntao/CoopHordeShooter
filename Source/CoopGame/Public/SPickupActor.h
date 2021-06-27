// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPickupActor.generated.h"

//105 建立球体与贴花组件，并创建实例，设置相关属性
//108 建立并设置贴花组件的材质M_PowerupDecal，在基于SPowerupActor创建的蓝图Powerup_SuperSpeed，蓝图中实现OnActivated和OnExpired功能
//109 将SpickupActor与SPowerupActor串联起来，实现Respawn与NotifyActorBeginOverlap函数，蓝图BP_TestPickup中将PowerUpClass选为SPowerupActor
//110 实现道具自发光效果，蓝图中实现M_Powerup和M_PowerupLightFunction两个材质，并生成材质实例MI_PowerupSpeed，作用于静态网格体文件SpeedIcon
//112 类似地，实现恢复生命值道具的自发光效果，增加RotaingMovement组件，实现道具旋转

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

	//球体组件
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* SphereComp;

	//贴花组件
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UDecalComponent* DecalComp;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	TSubclassOf<ASPowerupActor> PowerUpClass;

	ASPowerupActor* PowerUpInstance;

	UPROPERTY(EditInstanceOnly, Category = "PickupActor")
	float CooldownDuration;

	FTimerHandle TimerHandle_RespawnTimer;

	//道具重新生成
	void Respawn();

public:	

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	
};
