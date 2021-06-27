// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;

//P049 在基于SCharacter的名为PlayerPawn的蓝图类中，mesh选中名为SK_Mannequin的骨骼网络物体

//P067 蓝图和C++的规则不同，即使将函数设为了私有状态，不作特殊说明的话，仍能从蓝图中调用该函数
//只有UFUNCTION(BlueprintProtected)才能阻止蓝图访问该函数

//P070 代码调试
//VS中调试-附加到进程-UE4editor-设置断点，unreal中开始，执行到对应语句会自动跳转到vs页面

//P080和081，根据血量越小，显示一个图案越明显，具体操作看视频，都是蓝图上的操作
//P080 设置要显示的图案 内容浏览器 - 材质M_HealthIndicator
//P081 设置蓝图控件WBP_HealthIndicator

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void BeginCrouch();

	void EndCrouch();

	//P047 创建摄像头组件（用于第三人称视角），BlueprintReadOnly使得其属性在编辑器中不可更改，只读
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	//P048 弹簧臂组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	//P079 生命组件（在078中用蓝图对BP_Targetdummy中实现，这里使用C++对SCharacter实现）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;

	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;

	/* Default FOV set during begin play */
	float DefaultFOV;

	void BeginZoom();

	void EndZoom();

	UPROPERTY(Replicated)
	ASWeapon* CurrentWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/* Pawn died previously */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	//设置玩家输入组件的函数

	//P045设置玩家的WSAD前后左右移动：
	//在.cpp中通过playerInputComponent->bindAxis()来设置名字与所绑定的函数名
	//在.cpp中实现所绑定函数，自定义形参类型float（只需要实现一个前后轴和一个左右轴的移动函数即可）
	//在编译器中Edit-Project Settings-Input-Axis Mappings 来设置输入按键与权值（W选1，s选-1，这样即可实现前后移动了）
	
	//P046设置玩家的鼠标移动视角(仰角和偏航角)
	//绑定的函数是内建函数，AddControllerPitchinput，AddControllerYawinput
	//在编译器中设置input时，lookup对应Mouse Y ，-1；Turn对应Mouse X，1

	//P050设置玩家的开始蹲伏和结束蹲伏的动作输入
	//P051在编辑器中设置玩家蹲伏的动画（在UE4ASP_HeroTPP_AnimBlueprint动画蓝图中事件图表中设置节点，在其动画图表中有对应的状态机）
	//P052设置玩家跳跃（在UE4ASP_HeroTPP_AnimBlueprint动画蓝图中事件图表中设置节点图）
	
	//065 鼠标右键来设置玩家开镜还是关镜
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire();
};
