#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "MyAIController.generated.h"

UCLASS()
class EQUIPO5G_API AMyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AMyAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;


    float StartDelay = 2.f;
    bool bReady = false;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree* BehaviorTree;

protected:
    UPROPERTY(VisibleAnywhere, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComp;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

private:
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    ACharacter* GetClosestPlayer() const;

    bool bCanSeePlayer = false;
    float MoveUpdateTimer = 0.2f;
};