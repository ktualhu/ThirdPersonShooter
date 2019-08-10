// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CSZombieAIController.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ACSZombieAIController : public AAIController
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	class UBlackboardComponent* BlackboardComponent;

	UPROPERTY(Transient)
	class UBehaviorTreeComponent* BehaviorTreeComponent;

public:

	ACSZombieAIController();

	virtual void OnPossess(APawn* Pawn) override;

	virtual void OnUnPossess() override;

	void SetTargetEnemy(APawn* NewTarget);

	uint8 ZombieKeyID;
};
