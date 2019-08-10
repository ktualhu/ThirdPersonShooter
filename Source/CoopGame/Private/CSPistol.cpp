// Fill out your copyright notice in the Description page of Project Settings.


#include "CSPistol.h"

ACSPistol::ACSPistol()
{
	BaseDamage = 250.0f;

	FiringRange = 3500.0f;

	MagazineCapacity = 8;
	MaxBullets = 100;

	CountOfBulletsInMagazine = MagazineCapacity;
	CountOfBulletsOnCharacter = MaxBullets;
	
	MinRandomDeviation = 0;
	MaxRandomDeviation = 0;
}
