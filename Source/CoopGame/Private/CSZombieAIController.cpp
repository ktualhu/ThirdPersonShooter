// Fill out your copyright notice in the Description page of Project Settings.


#include "CSZombieAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "..\Public\CSZombieCharacter.h"

ACSZombieAIController::ACSZombieAIController()
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("behaviorTreeComponent");
}

void ACSZombieAIController::OnPossess(APawn* Pawn)
{
	Super::OnPossess(Pawn);

	ACSZombieCharacter* ZombieCharacter = Cast<ACSZombieCharacter>(Pawn);

	if (ZombieCharacter && ZombieCharacter->BotBehavior)
	{
		BlackboardComponent->InitializeBlackboard(*ZombieCharacter->BotBehavior->BlackboardAsset);

		ZombieKeyID = BlackboardComponent->GetKeyID("Target");

		BehaviorTreeComponent->StartTree(*ZombieCharacter->BotBehavior);
	}
}

void ACSZombieAIController::OnUnPossess()
{
	Super::OnUnPossess();

	if (BlackboardComponent)
	{
		BehaviorTreeComponent->StopTree();
	}
}

void ACSZombieAIController::SetTargetEnemy(APawn* NewTarget)
{
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValue<UBlackboardKeyType_Object>(ZombieKeyID, NewTarget);
	}
}
