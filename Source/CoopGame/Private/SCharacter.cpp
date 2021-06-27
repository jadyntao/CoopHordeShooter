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

	//P048 为弹簧臂组件创建实例，并运行使用pawn控制转向
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	//P050 启动对蹲伏功能的支持（getNavAgentPropertiesRef似乎只适用于AI，但虚幻引擎的底层编码仍会检查是否允许玩家角色蹲伏）
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	//P077 对胶囊组件的碰撞响应设为忽略，希望阻碍追踪的只有mesh网格体
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	//P079 生命组件（在078中用蓝图对BP_Targetdummy中实现，这里使用C++对SCharacter实现）
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

	//P079 绑定生命值改变时的响应函数
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);

	//P085 Role == ROLE_Authority检查是否在服务器中运行：
	//使得在玩家手上生成武器的代码只在服务器中运行，而在SWeapon.cpp中的武器构造函数中，SetReplicates(true)语句将武器设为复制品，使得服务器中生成的武器也能在客户端中复制显示出来，以保持相互同步
	//使得CurrentWeapon的赋值只在服务器中运行，而在SCharacter.cpp中开火函数是需要通过CurrentWeapon来调用SWeapon实现的，
	//因此，在.h中通过UPROPERTY(Replicated)将CurrentWeapon设为复制，并在.cpp中实现getLifetimeReplicatedProps函数来设置了复制生命周期规则。
	//这样，使得客户端和服务器的玩家可以在各自的窗口上实现开火操作，但客户端上开火效果（即轨迹线，开火粒子特效等）并不能在服务器上显示出来（在P086中在SWeapon中新加Serverfire来解决）
	if (Role == ROLE_Authority)
	{
		// Spawn a default weapon
		//P067 将原先在P054中用蓝图的BeginPlay事件实现的将武器与人物骨架手绑定并显示的节点图删除
		FActorSpawnParameters SpawnParams;
		//确保“总是生成”，即使是在碰撞某物的情况下
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			//附上人物骨架的WeaponAttachSocketName = "WeaponSocket"的插槽
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		}
	}
}


void ASCharacter::MoveForward(float Value)
{
	//getActorForwardVector获取Actor向右矢量
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

//P079 绑定生命值改变时的响应函数
void ASCharacter::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	//!bDied是防止生命值为零时，仍然被射击，生命值一直在改变（还是零），而不停触发响应函数
	//bDied在.h文件中是公开到蓝图的，在内容浏览器中找到动画蓝图UE4ASP_HeroTPP_AnimBlueprint - 
	//在其animGraph动画图，用bDied变量与布尔类型选择器Blend poses by bool 在原来的状态机上增加died动画播放的逻辑；
	//在其EventGraph事件图，从Character中牵出一根线，cast to SCharacter，获取到Died，在赋值到变量bDied
	if (Health <= 0.0f && !bDied)
	{
		// Die!
		bDied = true;
		//获取移动组件-》立即停止移动（但还是能够旋转）
		GetMovementComponent()->StopMovementImmediately();
		//设置胶囊组件无碰撞响应
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//等待销毁时从控制器接触控制
		DetachFromControllerPendingDestroy();
		//设置生命周期
		SetLifeSpan(10.0f);
	}
}


// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//P065变焦推进效果（开镜狙击机制），基本原理是调节摄像机最贱的视野范围设置
	//bWantsToZoom布尔类型，确定开镜还是关镜
	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	//如果直接使用TargetFOV至SetFieldOfView函数，开镜会比较慢；
	//而使用了FInterpTo插值函数，可以通过调节ZoomInterpSpeed来调节具体的开关镜速度
	//FInterpTo插值至（current当前，target目标，deltatime时间差量，interpspeed插值速度）
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	//默认值为90，数值越低，看得范围越少，相当于越高的倍镜
	CameraComp->SetFieldOfView(NewFOV);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//玩家输入组件-》添加轴
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);//前后轴
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);//左右轴

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);//仰角（内置函数）
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);//偏航角（内置函数）

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);//开始蹲伏
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);//结束蹲伏

	//PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ACharacter::Crouch);//开始蹲伏
	//PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ACharacter::UnCrouch);//结束蹲伏

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);//鼠标右键按下，开镜
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);//鼠标右键松开，关镜

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

	// CHALLENGE CODE
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);//跳跃（内置函数）
}

//P057 对APawn中的函数进行重写，使得子弹轨迹初始位置为第三人称视角的摄像头
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