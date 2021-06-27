// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

//P066 创建调试变量
//在游戏中按~打开控制台，按两次~可显示查看信息的控制台
//输入COOP.DebugWeapons 1可将DebugWeaponDrawing=1；
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw Debug Lines for Weapons"), 
	ECVF_Cheat);


// Sets default values
ASWeapon::ASWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	BaseDamage = 20.0f;
	BulletSpread = 2.0f;
	RateOfFire = 600;

	SetReplicates(true);

	//088 设置武器特效的更新频率，默认值为100和2，最低的2会导致客户端之间会出现明显延时
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}


void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	//P073 最小发射时间间隔
	TimeBetweenShots = 60 / RateOfFire;
}


void ASWeapon::Fire()
{
	// Trace the world, from pawn eyes to crosshair location
	//P086 如果Role < ROLE_Authority即在客户端上运行时，再执行ServerFire();
	//使得在服务器上执行serverFire中的fire函数，解决了在客户端上的开火特效不能在服务端显示的问题
	//但是，仍然存在服务端的开火特效在客户端不显示的问题（实际游戏中玩家不会在服务器中操作），以及客户端与客户端之间开火特效不显示的问题（P087中解决）
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	//P056 武器拥有者
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;//双眼位置
		FRotator EyeRotation;//双眼转向
		//P057 获取Actor双眼视角，默认情况下是玩家pawn的第一人称视角
		//通过shift+alt+S 搜索在Apawn::GetActorEyesViewPoint中GetPawnViewLocation()实际上是在Actor位置的基础上增加了眼底高度
		//在SCharacter.cpp对APawn中的函数进行重写，使得子弹轨迹初始位置为第三人称视角的摄像头
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		// Bullet Spread
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);//射击方位

		//追踪终止位置
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);//忽略所有拥有者
		QueryParams.AddIgnoredActor(this);//忽略武器
		QueryParams.bTraceComplex = true;//复合追踪，能追踪定位到目标网格体中的每块三角形；若为false，则仅为简单的边界盒碰撞
		QueryParams.bReturnPhysicalMaterial = true;//P070 设为true，这样击中时才会返回物理材质

		// Particle "Target" parameter
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		//击中效果（击中了什么物体，距离多远以及击中方向等等）
		FHitResult Hit;

		//获取世界后，在通道的基础上，创建单轨迹线
		//指定击中效果Hit，追踪起始位置为双眼位置EyeLocation，终止位置TraceEnd为利用双眼位置和转向乘以一个很大的数来计算得到，
		//碰撞通道（高阶的武器学习章节中，会有独立的通道，便于优化武器控制），这里使用的是可见性通道COLLISION_WEAPON，能在世界中进行追踪即可
		//碰撞查询参数QueryParams
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		//P071 之前在射击扫描追踪中应用的是“可见性通道”，这里将设置自定义碰撞通道
		//将原来的ECc_Visiblity换成COLLSION_WEAPOn，在CoopGame.cpp中宏定义替换是ECC_gameTraceChannel1
		//与surfacetype的处理方式类似，在编辑器中Edit-Project settings - Engine - Collision- Trace Channels 新建追踪通道，命名为Weapon，默认响应为Block阻碍。
		//C++代码中的名字为ECC_GameTraceChannel1，即第一个自定义通道
		//在BP_targetdummy敌人蓝图对象中，details-Collision-碰撞预设值设为BlockAll，碰撞响应中将Weapon打钩
		{
			// Blocking hit! Process damage
			//if语句为true，表示轨迹线碰撞受阻，即击中了物体，需要对应的数据处理

			//P058 获取击中的Actor
			AActor* HitActor = Hit.GetActor();

			//070 获取击中的物理材质的表面类型
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			//P072 添加爆头伤害，若击中SURFACE_FLESHVULNERABLE的表面类型，则伤害为原来的4倍
			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}
			//UGameplayStatics::下有很多个选择，包括ApplyDamage、ApplyPointDamage（应用点伤害）、ApplyRadiaDamage、ApplyRadiaDamageWithFalloff
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

			//击中粒子效果
			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;

		}

		if (DebugWeaponDrawing > 0)
		{
			//画出弹道线
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		//P060 画出开火特效
		PlayFireEffects(TracerEndPoint);

		//P087 由于客户端玩家开火，会发送到服务器，执行相应的开火效果代码。
		//这里将在服务器执行的开火函数中进行对HitScanTrace的更新。
		//由于在.h中设置了HitScanTrace进行复制属性时会触发函数OnRep_HitScanTrace，为其他客户端触发，播放开火效果。
		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceTo = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}

		LastFireTime = GetWorld()->TimeSeconds;
	}
}


void ASWeapon::OnRep_HitScanTrace()
{
	// Play cosmetic FX
	PlayFireEffects(HitScanTrace.TraceTo);
	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}


void ASWeapon::ServerFire_Implementation()
{
	Fire();
}


bool ASWeapon::ServerFire_Validate()
{
	return true;
}


//P073 开始开火
//在SCharacter.cpp中按下鼠标左键时，会调用到该函数
void ASWeapon::StartFire()
{   //设置定时器
	//FirstDelay是为了保证在点射操作中，子弹频率不会超过连射操作时的频率
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	//TimeBetweenShots是用于设置在长按鼠标进行连射时，子弹反射的频率
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

//P073 结束开火
//在SCharacter.cpp中松开鼠标左键时，会调用到该函数
void ASWeapon::StopFire()
{
	//清空定时器
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

//画出开火特效
void ASWeapon::PlayFireEffects(FVector TraceEnd)
{
	//P060 枪口火焰特效
	if (MuzzleEffect)
	{
		//生成发射器附着，特效会跟着枪口，MuzzleEffect枪火光效、MeshComp附着的组件、MuzzleSocketName附着点名称（即枪口插槽名称）
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	//P061 子弹轨迹特效（烟雾效果）
	if (TracerEffect)
	{
		//枪口位置
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		//粒子系统组件（世界、粒子系统名称、产生特效的位置）
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			//设置向量参数（特效文件中参数名称、跟踪结束点(没有击中目标就使用最初的轨迹结束点；有击中目标就使用击中点)）
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	//P068 添加摄像机抖动
	//新建蓝图类CameraShake，命名为CamShake_RifleFire，并在蓝图BP_Rifle中的FireCamShake选中该蓝图
	//可以调节摄像机抖动的FOVOscillation振动视野范围、rotation旋转度、location位置、duration持续时长等等
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());
		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

//P070 击中特效
void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	//对不同的SurfaceType表面类型，产生不同的击中特效
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}


void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//由于开火操作的客户端上已经触发的开火特效了，因此使用_CONDITION（，，COND_SkipOwner）只发送到除本身之外的其他客户端
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}