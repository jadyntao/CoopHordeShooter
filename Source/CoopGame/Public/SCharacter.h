// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;

//P049 �ڻ���SCharacter����ΪPlayerPawn����ͼ���У�meshѡ����ΪSK_Mannequin�Ĺ�����������

//P067 ��ͼ��C++�Ĺ���ͬ����ʹ��������Ϊ��˽��״̬����������˵���Ļ������ܴ���ͼ�е��øú���
//ֻ��UFUNCTION(BlueprintProtected)������ֹ��ͼ���ʸú���

//P070 �������
//VS�е���-���ӵ�����-UE4editor-���öϵ㣬unreal�п�ʼ��ִ�е���Ӧ�����Զ���ת��vsҳ��

//P080��081������Ѫ��ԽС����ʾһ��ͼ��Խ���ԣ������������Ƶ��������ͼ�ϵĲ���
//P080 ����Ҫ��ʾ��ͼ�� ��������� - ����M_HealthIndicator
//P081 ������ͼ�ؼ�WBP_HealthIndicator

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

	//P047 ��������ͷ��������ڵ����˳��ӽǣ���BlueprintReadOnlyʹ���������ڱ༭���в��ɸ��ģ�ֻ��
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	//P048 ���ɱ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	//P079 �����������078������ͼ��BP_Targetdummy��ʵ�֣�����ʹ��C++��SCharacterʵ�֣�
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
	//���������������ĺ���

	//P045������ҵ�WSADǰ�������ƶ���
	//��.cpp��ͨ��playerInputComponent->bindAxis()���������������󶨵ĺ�����
	//��.cpp��ʵ�����󶨺������Զ����β�����float��ֻ��Ҫʵ��һ��ǰ�����һ����������ƶ��������ɣ�
	//�ڱ�������Edit-Project Settings-Input-Axis Mappings ���������밴����Ȩֵ��Wѡ1��sѡ-1����������ʵ��ǰ���ƶ��ˣ�
	
	//P046������ҵ�����ƶ��ӽ�(���Ǻ�ƫ����)
	//�󶨵ĺ������ڽ�������AddControllerPitchinput��AddControllerYawinput
	//�ڱ�����������inputʱ��lookup��ӦMouse Y ��-1��Turn��ӦMouse X��1

	//P050������ҵĿ�ʼ�׷��ͽ����׷��Ķ�������
	//P051�ڱ༭����������Ҷ׷��Ķ�������UE4ASP_HeroTPP_AnimBlueprint������ͼ���¼�ͼ�������ýڵ㣬���䶯��ͼ�����ж�Ӧ��״̬����
	//P052���������Ծ����UE4ASP_HeroTPP_AnimBlueprint������ͼ���¼�ͼ�������ýڵ�ͼ��
	
	//065 ����Ҽ���������ҿ������ǹؾ�
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetPawnViewLocation() const override;

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
	void StopFire();
};
