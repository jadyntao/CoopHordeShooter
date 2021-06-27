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
	//将碰撞对象类型设为物理实体（默认情况下，径向力组件仅会影响几条通道，而物理实物为其中一条）
	MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = MeshComp;
	 
	RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForceComp->SetupAttachment(MeshComp);
	RadialForceComp->Radius = 250;
	RadialForceComp->bImpulseVelChange = true;
	//自动激活设为false，确保组件不会受到Tick函数的影响，该项目中只希望FireImpulse函数才能触发这一效果，而不希望每一帧都触发
	//对URadialForceComponent按Alt+G，查看到TickComponent函数，bIsActive为true时，会执行一段代码，而这里不希望重复执行类似的逻辑
	RadialForceComp->bAutoActivate = false; // Prevent component from ticking, and only use FireImpulse() instead
	//bIgnoreOwningActor = true 可以忽略自身的网格体，从而只对其他物体施加径向力
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
		//对红桶网格体组件施加作用力，让其能够向上弹起
		FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
		MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

		// Blast away nearby physics actors
		//触发径向力组件，释放作用力，弹开周围物体
		RadialForceComp->FireImpulse();

		// @TODO: Apply radial damage
	}
}


void ASExplosiveBarrel::OnRep_Exploded()
{
	// Play FX and change self material to black
	//爆炸时的粒子效果
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	// Override material on mesh with blackened version
	//爆炸后改变材质
	MeshComp->SetMaterial(0, ExplodedMaterial);
	UE_LOG(LogTemp, Warning, TEXT("OnRep_Exploded"));
}


void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}