// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "CoopGame.h"
#include "SHealthComponent.h"
#include "SWeapon.h"
#include "Net/UnrealNetwork.h"



// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//P048 Ϊ���ɱ��������ʵ����������ʹ��pawn����ת��
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	//P050 �����Զ׷����ܵ�֧�֣�getNavAgentPropertiesRef�ƺ�ֻ������AI�����������ĵײ�����Ի����Ƿ�������ҽ�ɫ�׷���
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	//P077 �Խ����������ײ��Ӧ��Ϊ���ԣ�ϣ���谭׷�ٵ�ֻ��mesh������
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	//P079 �����������078������ͼ��BP_Targetdummy��ʵ�֣�����ʹ��C++��SCharacterʵ�֣�
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	ZoomedFOV = 45.0f;
	ZoomInterpSpeed = 20;

	WeaponAttachSocketName = "WeaponSocket";
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultFOV = CameraComp->FieldOfView;

	//P079 ������ֵ�ı�ʱ����Ӧ����
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	//P085 Role == ROLE_Authority����Ƿ��ڷ����������У�
	//ʹ��������������������Ĵ���ֻ�ڷ����������У�����SWeapon.cpp�е��������캯���У�SetReplicates(true)��佫������Ϊ����Ʒ��ʹ�÷����������ɵ�����Ҳ���ڿͻ����и�����ʾ�������Ա����໥ͬ��
	//ʹ��CurrentWeapon�ĸ�ֵֻ�ڷ����������У�����SCharacter.cpp�п���������Ҫͨ��CurrentWeapon������SWeaponʵ�ֵģ�
	//��ˣ���.h��ͨ��UPROPERTY(Replicated)��CurrentWeapon��Ϊ���ƣ�����.cpp��ʵ��getLifetimeReplicatedProps�����������˸����������ڹ���
	//������ʹ�ÿͻ��˺ͷ���������ҿ����ڸ��ԵĴ�����ʵ�ֿ�����������ͻ����Ͽ���Ч�������켣�ߣ�����������Ч�ȣ��������ڷ���������ʾ��������P086����SWeapon���¼�Serverfire�������
	if (Role == ROLE_Authority)
	{
		// Spawn a default weapon
		//P067 ��ԭ����P054������ͼ��BeginPlay�¼�ʵ�ֵĽ�����������Ǽ��ְ󶨲���ʾ�Ľڵ�ͼɾ��
		FActorSpawnParameters SpawnParams;
		//ȷ�����������ɡ�����ʹ������ײĳ��������
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			//��������Ǽܵ�WeaponAttachSocketName = "WeaponSocket"�Ĳ��
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		}
	}
}


void ASCharacter::MoveForward(float Value)
{
	//getActorForwardVector��ȡActor����ʸ��
	AddMovementInput(GetActorForwardVector() * Value);
}


void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}


void ASCharacter::BeginCrouch()
{
	Crouch();
}


void ASCharacter::EndCrouch()
{
	UnCrouch();
}


void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
}


void ASCharacter::EndZoom()
{
	bWantsToZoom = false;
}


void ASCharacter::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}


void ASCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

//P079 ������ֵ�ı�ʱ����Ӧ����
void ASCharacter::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	//!bDied�Ƿ�ֹ����ֵΪ��ʱ����Ȼ�����������ֵһֱ�ڸı䣨�����㣩������ͣ������Ӧ����
	//bDied��.h�ļ����ǹ�������ͼ�ģ���������������ҵ�������ͼUE4ASP_HeroTPP_AnimBlueprint - 
	//����animGraph����ͼ����bDied�����벼������ѡ����Blend poses by bool ��ԭ����״̬��������died�������ŵ��߼���
	//����EventGraph�¼�ͼ����Character��ǣ��һ���ߣ�cast to SCharacter����ȡ��Died���ڸ�ֵ������bDied
	if (Health <= 0.0f && !bDied)
	{
		// Die!
		bDied = true;
		//��ȡ�ƶ����-������ֹͣ�ƶ����������ܹ���ת��
		GetMovementComponent()->StopMovementImmediately();
		//���ý����������ײ��Ӧ
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//�ȴ�����ʱ�ӿ������Ӵ�����
		DetachFromControllerPendingDestroy();
		//������������
		SetLifeSpan(10.0f);
	}
}


// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//P065�佹�ƽ�Ч���������ѻ����ƣ�������ԭ���ǵ���������������Ұ��Χ����
	//bWantsToZoom�������ͣ�ȷ���������ǹؾ�
	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	//���ֱ��ʹ��TargetFOV��SetFieldOfView������������Ƚ�����
	//��ʹ����FInterpTo��ֵ����������ͨ������ZoomInterpSpeed�����ھ���Ŀ��ؾ��ٶ�
	//FInterpTo��ֵ����current��ǰ��targetĿ�꣬deltatimeʱ�������interpspeed��ֵ�ٶȣ�
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	//Ĭ��ֵΪ90����ֵԽ�ͣ����÷�ΧԽ�٣��൱��Խ�ߵı���
	CameraComp->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//����������-�������
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);//ǰ����
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);//������

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);//���ǣ����ú�����
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);//ƫ���ǣ����ú�����

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);//��ʼ�׷�
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);//�����׷�

	//PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ACharacter::Crouch);//��ʼ�׷�
	//PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ACharacter::UnCrouch);//�����׷�

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);//����Ҽ����£�����
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);//����Ҽ��ɿ����ؾ�

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

	// CHALLENGE CODE
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);//��Ծ�����ú�����
}

//P057 ��APawn�еĺ���������д��ʹ���ӵ��켣��ʼλ��Ϊ�����˳��ӽǵ�����ͷ
FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}


void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
}