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

//P066 �������Ա���
//����Ϸ�а�~�򿪿���̨��������~����ʾ�鿴��Ϣ�Ŀ���̨
//����COOP.DebugWeapons 1�ɽ�DebugWeaponDrawing=1��
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

	//088 ����������Ч�ĸ���Ƶ�ʣ�Ĭ��ֵΪ100��2����͵�2�ᵼ�¿ͻ���֮������������ʱ
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}


void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	//P073 ��С����ʱ����
	TimeBetweenShots = 60 / RateOfFire;
}


void ASWeapon::Fire()
{
	// Trace the world, from pawn eyes to crosshair location
	//P086 ���Role < ROLE_Authority���ڿͻ���������ʱ����ִ��ServerFire();
	//ʹ���ڷ�������ִ��serverFire�е�fire������������ڿͻ����ϵĿ�����Ч�����ڷ������ʾ������
	//���ǣ���Ȼ���ڷ���˵Ŀ�����Ч�ڿͻ��˲���ʾ�����⣨ʵ����Ϸ����Ҳ����ڷ������в��������Լ��ͻ�����ͻ���֮�俪����Ч����ʾ�����⣨P087�н����
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	//P056 ����ӵ����
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;//˫��λ��
		FRotator EyeRotation;//˫��ת��
		//P057 ��ȡActor˫���ӽǣ�Ĭ������������pawn�ĵ�һ�˳��ӽ�
		//ͨ��shift+alt+S ������Apawn::GetActorEyesViewPoint��GetPawnViewLocation()ʵ��������Actorλ�õĻ������������۵׸߶�
		//��SCharacter.cpp��APawn�еĺ���������д��ʹ���ӵ��켣��ʼλ��Ϊ�����˳��ӽǵ�����ͷ
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		// Bullet Spread
		float HalfRad = FMath::DegreesToRadians(BulletSpread);
		ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);//�����λ

		//׷����ֹλ��
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);//��������ӵ����
		QueryParams.AddIgnoredActor(this);//��������
		QueryParams.bTraceComplex = true;//����׷�٣���׷�ٶ�λ��Ŀ���������е�ÿ�������Σ���Ϊfalse�����Ϊ�򵥵ı߽����ײ
		QueryParams.bReturnPhysicalMaterial = true;//P070 ��Ϊtrue����������ʱ�Ż᷵���������

		// Particle "Target" parameter
		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		//����Ч����������ʲô���壬�����Զ�Լ����з���ȵȣ�
		FHitResult Hit;

		//��ȡ�������ͨ���Ļ����ϣ��������켣��
		//ָ������Ч��Hit��׷����ʼλ��Ϊ˫��λ��EyeLocation����ֹλ��TraceEndΪ����˫��λ�ú�ת�����һ���ܴ����������õ���
		//��ײͨ�����߽׵�����ѧϰ�½��У����ж�����ͨ���������Ż��������ƣ�������ʹ�õ��ǿɼ���ͨ��COLLISION_WEAPON�����������н���׷�ټ���
		//��ײ��ѯ����QueryParams
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams))
		//P071 ֮ǰ�����ɨ��׷����Ӧ�õ��ǡ��ɼ���ͨ���������ｫ�����Զ�����ײͨ��
		//��ԭ����ECc_Visiblity����COLLSION_WEAPOn����CoopGame.cpp�к궨���滻��ECC_gameTraceChannel1
		//��surfacetype�Ĵ���ʽ���ƣ��ڱ༭����Edit-Project settings - Engine - Collision- Trace Channels �½�׷��ͨ��������ΪWeapon��Ĭ����ӦΪBlock�谭��
		//C++�����е�����ΪECC_GameTraceChannel1������һ���Զ���ͨ��
		//��BP_targetdummy������ͼ�����У�details-Collision-��ײԤ��ֵ��ΪBlockAll����ײ��Ӧ�н�Weapon��
		{
			// Blocking hit! Process damage
			//if���Ϊtrue����ʾ�켣����ײ���裬�����������壬��Ҫ��Ӧ�����ݴ���

			//P058 ��ȡ���е�Actor
			AActor* HitActor = Hit.GetActor();

			//070 ��ȡ���е�������ʵı�������
			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			//P072 ��ӱ�ͷ�˺���������SURFACE_FLESHVULNERABLE�ı������ͣ����˺�Ϊԭ����4��
			float ActualDamage = BaseDamage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}
			//UGameplayStatics::���кܶ��ѡ�񣬰���ApplyDamage��ApplyPointDamage��Ӧ�õ��˺�����ApplyRadiaDamage��ApplyRadiaDamageWithFalloff
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), MyOwner, DamageType);

			//��������Ч��
			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);

			TracerEndPoint = Hit.ImpactPoint;

		}

		if (DebugWeaponDrawing > 0)
		{
			//����������
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		//P060 ����������Ч
		PlayFireEffects(TracerEndPoint);

		//P087 ���ڿͻ�����ҿ��𣬻ᷢ�͵���������ִ����Ӧ�Ŀ���Ч�����롣
		//���ｫ�ڷ�����ִ�еĿ������н��ж�HitScanTrace�ĸ��¡�
		//������.h��������HitScanTrace���и�������ʱ�ᴥ������OnRep_HitScanTrace��Ϊ�����ͻ��˴��������ſ���Ч����
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


//P073 ��ʼ����
//��SCharacter.cpp�а���������ʱ������õ��ú���
void ASWeapon::StartFire()
{   //���ö�ʱ��
	//FirstDelay��Ϊ�˱�֤�ڵ�������У��ӵ�Ƶ�ʲ��ᳬ���������ʱ��Ƶ��
	float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
	//TimeBetweenShots�����������ڳ�������������ʱ���ӵ������Ƶ��
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

//P073 ��������
//��SCharacter.cpp���ɿ�������ʱ������õ��ú���
void ASWeapon::StopFire()
{
	//��ն�ʱ��
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

//����������Ч
void ASWeapon::PlayFireEffects(FVector TraceEnd)
{
	//P060 ǹ�ڻ�����Ч
	if (MuzzleEffect)
	{
		//���ɷ��������ţ���Ч�����ǹ�ڣ�MuzzleEffectǹ���Ч��MeshComp���ŵ������MuzzleSocketName���ŵ����ƣ���ǹ�ڲ�����ƣ�
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	//P061 �ӵ��켣��Ч������Ч����
	if (TracerEffect)
	{
		//ǹ��λ��
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		//����ϵͳ��������硢����ϵͳ���ơ�������Ч��λ�ã�
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			//����������������Ч�ļ��в������ơ����ٽ�����(û�л���Ŀ���ʹ������Ĺ켣�����㣻�л���Ŀ���ʹ�û��е�)��
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	//P068 ������������
	//�½���ͼ��CameraShake������ΪCamShake_RifleFire��������ͼBP_Rifle�е�FireCamShakeѡ�и���ͼ
	//���Ե��������������FOVOscillation����Ұ��Χ��rotation��ת�ȡ�locationλ�á�duration����ʱ���ȵ�
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

//P070 ������Ч
void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	//�Բ�ͬ��SurfaceType�������ͣ�������ͬ�Ļ�����Ч
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
	//���ڿ�������Ŀͻ������Ѿ������Ŀ�����Ч�ˣ����ʹ��_CONDITION������COND_SkipOwner��ֻ���͵�������֮��������ͻ���
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}