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

	//P101 ��Ĭ������£������������ڿͻ����в����أ�ֻ�ڷ�����������
	//�⵼�����ڿͻ��˵�BeginPlay��ִ��GetNextPathPoint����ʱ��NavPathΪ�գ�ִ�е�NavPath->PathPoints.Num()ʱ����ֳ������
	//��ˣ�����if (Role == ROLE_Authority)��֤�ڷ�������ִ��NextPathPoint = GetNextPathPoint();
	//��Tick������Ҳ����if (Role == ROLE_Authority)���������
	if (Role == ROLE_Authority)
	{
		// Find initial move-to
		NextPathPoint = GetNextPathPoint();

		// Every second we update our power-level based on nearby bots (CHALLENGE CODE)
		// ���ö�ʱ����ÿ1�봥��OnCheckNearbyBots����
		FTimerHandle TimerHandle_CheckPowerLevel;
		GetWorldTimerManager().SetTimer(TimerHandle_CheckPowerLevel, this, &ASTrackerBot::OnCheckNearbyBots, 1.0f, true);
	}
}


void ASTrackerBot::HandleTakeDamage(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	//096 ���MatLinstδ��ֵ
	if (MatInst == nullptr)
	{
		//096 �����������¶�̬ʾ����ϣ��������ʱ���Ĳ����Ļ�������Ҫ����ĳ�ֶ�̬����ʾ���������ͻ�ֻ�ı��Ӧ�Ĳ���������Ӧ�õ��ؿ��е����ж���
		//ElementIndex=0����Ϊ�����������ϵ�һ��Ҳ��Ψһ�Ĳ��ʣ�
		//UMaterialInterface���ʽ���
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
	}

	if (MatInst)
	{
		//���ö�̬�����е�LastTimeDamageTaken�������γ�������˸�Ķ�̬Ч��
		MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	}

	// Explode on hitpoints == 0
	if (Health <= 0.0f)
	{
		SelfDestruct();
	}
}

//��ȡ��ը������һ��������
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
		//P093 FindPathToActorSynchronouslyͬ���ҳ�ͨ��Actor��·��
		UNavigationPath* NavPath = UNavigationSystem::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

		GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
		GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ASTrackerBot::RefreshPath , 5.0f, false);

		//P093 ·������������1��Ĭ�ϵ�һ��·�����Ǳ�ը���屾���λ��
		if (NavPath && NavPath->PathPoints.Num() > 1)
		{
			// Return next point in the path
			////P093 ȡ����·������һ��·����
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
	//097��ը��Ч
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	//099 ��ը��Ч�����ﲻ��ʹ��SpawnSoundAttached����Ϊ���屬ը��Ҳ�޷��ƶ��ˣ�ԭ��������Ч���У�
	UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	//102
	MeshComp->SetVisibility(false, true);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Role == ROLE_Authority)
	{
		//097 ��ը�˺�ʱ��Ҫ��������
		TArray<AActor*> IgnoredActors;
		IgnoredActors.Add(this);

		// Increase damage based on the power level (challenge code)
		// ��Χ������Խ�࣬PowerLevelֵԽ���˺�Խ��
		float ActualDamage = ExplosionDamage + (ExplosionDamage * PowerLevel);

		// Apply Damage!
		//097 ��ը�˺�����
		UGameplayStatics::ApplyRadialDamage(this, ActualDamage, GetActorLocation(), ExplosionRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

		if (DebugTrackerBotDrawing)
		{
			//097 ���Ƶ������壬��ը��Χ
			DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Red, false, 2.0f, 0, 1.0f);
		}
		SetLifeSpan(2.0f);
	}
}

//098 �Կ�20��Ѫ
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
		//094 ���뵼����ľ���
		float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

		if (DistanceToTarget <= RequiredDistanceToTarget)
		{
			//094 ����С����ֵ�����ȡ������ȡ��һ��Ŀ���
			NextPathPoint = GetNextPathPoint();

			if (DebugTrackerBotDrawing)
			{
				//094 ��GetActorLocation()λ�û����ַ���
				DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
			}
		}
		else
		{
			// Keep moving towards next target
			//094 ���������ֵ����ʩ�����������������ƶ�

			//094 ����������
			FVector ForceDirection = NextPathPoint - GetActorLocation();
			ForceDirection.Normalize();
			//094 �������������һ����ֵ
			ForceDirection *= MovementForce;
			//094 ʩ�����������Ƿ�ʹ�����ʱ仯=false
			MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

			if (DebugTrackerBotDrawing)
			{
				//094 ���Ƽ�ͷ
				DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
			}
		}

		if (DebugTrackerBotDrawing)
		{
			//094 ���Ƶ�������
			DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
		}
	}
}


void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// 098 bStartedSelfDestruction��֤��ʱ��ֻ����һ��
	if (!bStartedSelfDestruction && !bExploded)
	{
		//098 ����ת��������Ƿ������
		ASCharacter* PlayerPawn = Cast<ASCharacter>(OtherActor);
		if (PlayerPawn && !USHealthComponent::IsFriendly(OtherActor, this))
		{
			// We overlapped with a player!

			if (Role == ROLE_Authority)
			{
				// Start self destruction sequence
				//���ö�ʱ��������DamageSelf�������Կ�20��Ѫ��
				GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ASTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
			}

			bStartedSelfDestruction = true;
			//099 ������Ч������RootComponent�����
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
	//����������ײ��״������OverlapMultiByObjectType����
	FCollisionShape CollShape;
	CollShape.SetSphere(Radius);

	// Only find Pawns (eg. players and AI bots)
	//���ÿ���ײ����ΪPhysicsBody��Pawn����ը�����������������õ���PhysicsBody
	FCollisionObjectQueryParams QueryParams;
	// Our tracker bot's mesh component is set to Physics Body in Blueprint (default profile of physics simulated actors)
	QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	QueryParams.AddObjectTypesToQuery(ECC_Pawn);

	//��ȡ�ص��Ķ��󣬽������Overlaps
	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

	if (DebugTrackerBotDrawing)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.0f);
	}

	//�ص�����
	int32 NrOfBots = 0;
	// loop over the results using a "range based for loop"
	for (FOverlapResult Result : Overlaps)
	{
		//�����������������ƥ��ĸ���
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
	//���Ʒ����ص�����������Ϊ0-4
	PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);

	// Update the material color
	// ���¶�Ӧ��������˸��ʾ����
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
		//���²�����ͼ��PowerLevelAlpha����
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

