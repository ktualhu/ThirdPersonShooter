// Fill out your copyright notice in the Description page of Project Settings.


#include "CSPistol.h"

ACSPistol::ACSPistol()
{
	MagazineCapacity = 8;
	MaxBullets = 64;

	CountOfBulletsInMagazine = MagazineCapacity;
	CountOfBulletsOnCharacter = MaxBullets;
}
