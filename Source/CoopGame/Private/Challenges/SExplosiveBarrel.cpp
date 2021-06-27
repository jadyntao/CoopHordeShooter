// Fill out your copyright notice in the Description page of Project Settings.

#include "SExplosiveBarrel.h"
#include "SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComp->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	// Set to physics body to let radial component affect us (eg. when a nearby barrel explodes)
	//����ײ����������Ϊ����ʵ�壨Ĭ������£��������������Ӱ�켸��ͨ����������ʵ��Ϊ����һ����
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;
	 
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 250;
	RadialForceComp->bImpulseVelChange = true;
	//�Զ�������Ϊfalse��ȷ����������ܵ�Tick������Ӱ�죬����Ŀ��ֻϣ��FireImpulse�������ܴ�����һЧ��������ϣ��ÿһ֡������
	//��URadialForceComponent��Alt+G���鿴��TickComponent������bIsActiveΪtrueʱ����ִ��һ�δ��룬�����ﲻϣ���ظ�ִ�����Ƶ��߼�
	RadialForceComp->bAutoActivate = false; // Prevent component from ticking, and only use FireImpulse() instead
	//bIgnoreOwningActor = true ���Ժ�������������壬�Ӷ�ֻ����������ʩ�Ӿ�����
	RadialForceComp->bIgnoreOwningActor = true; // ignore self

	ExplosionImpulse = 400;

	SetReplicates(true);
	SetReplicateMovement(true);
}


void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	if (bExploded)
	{
		// Nothing left to do, already exploded.
		return;
	}

	if (Health <= 0.0f)
	{
		// Explode!
		bExploded = true;
		OnRep_Exploded();

		// Boost the barrel upwards
		//�Ժ�Ͱ���������ʩ���������������ܹ����ϵ���
		FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

		// Blast away nearby physics actors
		//����������������ͷ���������������Χ����
		RadialForceComp->FireImpulse();

		// @TODO: Apply radial damage
	}
}


void ASExplosiveBarrel::OnRep_Exploded()
{
	// Play FX and change self material to black
	//��ըʱ������Ч��
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	// Override material on mesh with blackened version
	//��ը��ı����
	MeshComp->SetMaterial(0, ExplodedMaterial);
	UE_LOG(LogTemp, Warning, TEXT("OnRep_Exploded"));
}


void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}