#include "Weapons/OFBulletShell.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AOFBulletShell::AOFBulletShell()
{
	PrimaryActorTick.bCanEverTick = false;
	ShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell Mesh"));
	SetRootComponent(ShellMesh);
	ShellMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	ShellMesh->SetSimulatePhysics(true);
	ShellMesh->SetEnableGravity(true);
	ShellMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;
}

void AOFBulletShell::BeginPlay()
{
	Super::BeginPlay();
	
	ShellMesh->OnComponentHit.AddDynamic(this, &AOFBulletShell::OnHit);
	ShellMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
	
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [this]() { Destroy(); }, 5.0f, false);
	ShellMesh->OnComponentHit.RemoveDynamic(this, &AOFBulletShell::OnHit);
}

void AOFBulletShell::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& HitResult)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
}

