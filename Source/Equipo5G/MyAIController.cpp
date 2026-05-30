#include "MyAIController.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AMyAIController::AMyAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*AIPerceptionComp);

    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.f;
    SightConfig->LoseSightRadius = 2000.f;
    SightConfig->PeripheralVisionAngleDegrees = 180.f;
    SightConfig->SetMaxAge(0.f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    AIPerceptionComp->ConfigureSense(*SightConfig);
    AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
        this, &AMyAIController::OnTargetPerceptionUpdated
    );
}

void AMyAIController::BeginPlay()
{
    Super::BeginPlay();
    // Ya no usamos Behavior Tree — la lógica es directo en Tick
}

void AMyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!GetPawn()) return;
    // DEBUG temporal
    GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red,
        FString::Printf(TEXT("AI Tick - CanSee: %s"), bCanSeePlayer ? TEXT("true") : TEXT("false"))
    );
    const float PanicRadius = 1500.f;

    ACharacter* Closest = GetClosestPlayer();
    if (!Closest) return;

    float DistToClosest = FVector::Dist(
        GetPawn()->GetActorLocation(),
        Closest->GetActorLocation()
    );

    if (bCanSeePlayer || DistToClosest < PanicRadius)
    {
        FVector AILocation = GetPawn()->GetActorLocation();
        FVector PlayerLocation = Closest->GetActorLocation();
        FVector FleeDirection = (AILocation - PlayerLocation).GetSafeNormal();
        FVector FleeDestination = AILocation + (FleeDirection * 800.f);

        MoveToLocation(FleeDestination, 50.f);
    }
    else
    {
        StopMovement();
    }
}

void AMyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    ACharacter* PlayerChar = Cast<ACharacter>(Actor);
    if (!PlayerChar) return;

    bCanSeePlayer = Stimulus.WasSuccessfullySensed();
}

ACharacter* AMyAIController::GetClosestPlayer() const
{
    ACharacter* Closest = nullptr;
    float MinDist = MAX_FLT;

    for (int32 i = 0; i < 2; i++)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), i);
        if (!PC) continue;

        ACharacter* Player = Cast<ACharacter>(PC->GetPawn());
        if (!Player) continue;

        float Dist = FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());
        if (Dist < MinDist)
        {
            MinDist = Dist;
            Closest = Player;
        }
    }

    return Closest;
}