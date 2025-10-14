// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OFProjectileWeapon.h"


// Sets default values
AOFProjectileWeapon::AOFProjectileWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AOFProjectileWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOFProjectileWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

