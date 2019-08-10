// Fill out your copyright notice in the Description page of Project Settings.


#include "CSSniperRifle.h"

ACSSniperRifle::ACSSniperRifle()
{
	BaseDamage = 2000.0f;

	MagazineCapacity = 5;
	MaxBullets = 20;

	CountOfBulletsInMagazine = MagazineCapacity;
	CountOfBulletsOnCharacter = MaxBullets;

	FiringRange = 9000.0f;

	FireDelay = 1.2f;
}
