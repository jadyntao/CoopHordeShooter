// Fill out your copyright notice in the Description page of Project Settings.

#include "STrackerBot.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Navigation/NavigationSystem.h"
#include "GameFramework/Character.h"
#include "AI/Navigation/NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "SHealthComponent.h"
#include "SCharacter.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"

static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(
	TEXT("COOP.DebugTrackerBot"),
	DebugTrackerBotDrawing,
	TEXT("Draw Debug Lines for TrackerBot"),
	ECVF_Cheat);


// Sets default values
ASTrackerBot::ASTrackerBot()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetCanEverAffectNavigation(false);
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASTrackerBot::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;

	ExplosionDamage = 60;
	ExplosionRadius = 350;

	SelfDamageInterval = 0.25f;
}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();

	//P101 在默认情况下，导航网格体在客户端中不加载，只在服务器中运行
	//这导致若在客户端的BeginPlay中执行GetNextPathPoint函数时，NavPath为空，执行到NavPath->PathPoints.Num()时会出现程序崩溃
	//因此，加上if (Role == ROLE_Authority)保证在服务器中执行NextPathPoint = GetNextPathPoint();
	//在Tick函数中也加上if (Role == ROLE_Authority)这样的语句
	if (Role == ROLE_Authority)
	{
		// Find initial move-to
		NextPathPoint = GetNextPathPoint();

		// Every second we update our power-level based on nearby bots (CHALLENGE CODE)
		// 设置定时器，每1秒触发OnCheckNearbyBots函数
		FTimerHandle TimerHandle_CheckPowerLevel;
		GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ASTrackerBot::OnCheckNearbyBots, 1.0f, true);
	}
}


void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	//096 如果MatLinst未赋值
	if (MatInst == nullptr)
	{
		//096 创建并设置新动态示例（希望在运行时更改参数的话，则需要创建某种动态材质示例，这样就会只改变对应的参数，而不应用到关卡中的所有对象）
		//ElementIndex=0，因为它是网格体上第一个也是唯一的材质；
		//UMaterialInterface材质界面
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		//设置动态材质中的LastTimeDamageTaken变量，形成脉冲闪烁的动态效果
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	// Explode on hitpoints == 0
	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

//获取爆炸球体下一个导航点
FVector ASTrackerBot::GetNextPathPoint()
{
	AActor* BestTarget = nullptr;
	float NearestTargetDistance = FLT_MAX;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* TestPawn = It->Get();
		if (TestPawn == nullptr || USHealthComponent::IsFriendly(TestPawn, this))
		{
			continue;
		}

		USHealthComponent* TestPawnHealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
		if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.0f)
		{
			float Distance = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

			if (Distance < NearestTargetDistance)
			{
				BestTarget = TestPawn;
				NearestTargetDistance = Distance;
			}
		}
	}

	if (BestTarget)
	{
		//P093 FindPathToActorSynchronously同步找出通向Actor的路径
		UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath , 5.0f, false);

		//P093 路径的数量大于1，默认第一个路径点是爆炸球体本身的位置
		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			// Return next point in the path
			////P093 取导航路径的下一个路径点
			return NavPath->PathPoints[1];
		}
	}

	// Failed to find path
	return GetActorLocation();
}


void ASTrackerBot::SelfDestruct()
{
	if (bExploded)
	{
		return;
	}

	bExploded = true;
	//097爆炸特效
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	//099 爆炸音效（这里不必使用SpawnSoundAttached，因为球体爆炸后也无法移动了，原来发出音效就行）
	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	//102
	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		//097 爆炸伤害时需要忽略自身
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		// Increase damage based on the power level (challenge code)
		// 周围的球体越多，PowerLevel值越大，伤害越高
		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);

		// Apply Damage!
		//097 爆炸伤害设置
		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		if (DebugTrackerBotDrawing)
		{
			//097 绘制调试球体，爆炸范围
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);
		}
		SetLifeSpan(2.0f);
	}
}

//098 自扣20滴血
void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}


// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role == ROLE_Authority && !bExploded)
	{
		//094 距离导航点的距离
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			//094 距离小于阈值，则获取继续获取下一个目标点
			NextPathPoint = GetNextPathPoint();

			if (DebugTrackerBotDrawing)
			{
				//094 在GetActorLocation()位置画出字符串
				DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
			}
		}
		else
		{
			// Keep moving towards next target
			//094 距离大于阈值，则施加作用力，让球体移动

			//094 作用力方向
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			//094 作用力方向乘以一个大值
			ForceDirection *= MovementForce;
			//094 施加作用力，是否使用速率变化=false
			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			if (DebugTrackerBotDrawing)
			{
				//094 绘制箭头
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
			}
		}

		if (DebugTrackerBotDrawing)
		{
			//094 绘制调试球体
			DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
		}
	}
}


void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// 098 bStartedSelfDestruction保证定时器只开启一次
	if (!bStartedSelfDestruction && !bExploded)
	{
		//098 类型转换，检查是否是玩家
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn && !USHealthComponent::IsFriendly(OtherActor, this))
		{
			// We overlapped with a player!

			if (Role == ROLE_Authority)
			{
				// Start self destruction sequence
				//设置定时器，运行DamageSelf函数（自扣20滴血）
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}

			bStartedSelfDestruction = true;
			//099 播放音效附着在RootComponent组件上
			UGameplayStatics::SpawnSoundAttached(SelfDestructSound, RootComponent);
		}
	}
}

// CHALLENGE CODE
// 103
void ASTrackerBot::OnCheckNearbyBots()
{
	// distance to check for nearby bots
	const float Radius = 600;

	// Create temporary collision shape for overlaps
	//建立球体碰撞形状，用于OverlapMultiByObjectType函数
	FCollisionShape CollShape;
	CollShape.SetSphere(Radius);

	// Only find Pawns (eg. players and AI bots)
	//设置可碰撞类型为PhysicsBody和Pawn，爆炸球体的网格体组件设置的是PhysicsBody
	FCollisionObjectQueryParams QueryParams;
	// Our tracker bot's mesh component is set to Physics Body in Blueprint (default profile of physics simulated actors)
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	//获取重叠的对象，结果存于Overlaps
	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

	if (DebugTrackerBotDrawing)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.0f);
	}

	//重叠次数
	int32 NrOfBots = 0;
	// loop over the results using a "range based for loop"
	for (FOverlapResult Result : Overlaps)
	{
		//检查对象与该球体类型匹配的个数
		// Check if we overlapped with another tracker bot (ignoring players and other bot types)
		ASTrackerBot* Bot = Cast<ASTrackerBot>(Result.GetActor());
		// Ignore this trackerbot instance
		if (Bot && Bot != this)
		{
			NrOfBots++;
		}
	}

	const int32 MaxPowerLevel = 4;
	// Clamp between min=0 and max=4
	//限制发生重叠的球体数量为0-4
	PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);

	// Update the material color
	// 更新对应的脉冲闪烁显示材质
	if (MatInst == nullptr)
	{
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}
	if (MatInst)
	{
		// Convert to a float between 0 and 1 just like an 'Alpha' value of a texture. Now the material can be set up without having to know the max power level 
		// which can be tweaked many times by gameplay decisions (would mean we need to keep 2 places up to date)
		float Alpha = PowerLevel / (float)MaxPowerLevel;
		// Note: (float)MaxPowerLevel converts the int32 to a float, 
		//	otherwise the following happens when dealing when dividing integers: 1 / 4 = 0 ('PowerLevel' int / 'MaxPowerLevel' int = 0 int)
		//	this is a common programming problem and can be fixed by 'casting' the int (MaxPowerLevel) to a float before dividing.
		//更新材质蓝图中PowerLevelAlpha变量
		MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
	}

	if (DebugTrackerBotDrawing)
	{
		// Draw on the bot location
		DrawDebugString(GetWorld(), FVector(0, 0, 0), FString::FromInt(PowerLevel), this, FColor::White, 1.0f, true);
	}
}

void ASTrackerBot::RefreshPath()
{
	NextPathPoint = GetNextPathPoint();
}

