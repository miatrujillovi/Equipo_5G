#include "MyAIController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

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
}

void AMyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!GetPawn()) return;

    const float PanicRadius = 1500.f;

    ACharacter* Closest = GetClosestPlayer();

    // Actualizar timer
    MoveUpdateTimer += DeltaTime;

    if (Closest)
    {
        float DistToClosest = FVector::Dist(
            GetPawn()->GetActorLocation(),
            Closest->GetActorLocation()
        );

        if (bCanSeePlayer || DistToClosest < PanicRadius)
        {
            // MODO HUIDA — recalcula cada 0.2 segundos
            if (MoveUpdateTimer >= 0.2f)
            {
                MoveUpdateTimer = 0.f;

                FVector AILocation = GetPawn()->GetActorLocation();
                FVector PlayerLocation = Closest->GetActorLocation();
                FVector FleeDirection = (AILocation - PlayerLocation).GetSafeNormal();
                FVector FleeDestination = AILocation + (FleeDirection * 800.f);

                MoveToLocation(FleeDestination, 50.f);
            }
            return;
        }
    }

    // MODO WANDERING — camina a un punto random cada 3 segundos
    if (MoveUpdateTimer >= 3.f)
    {
        MoveUpdateTimer = 0.f;

        FVector AILocation = GetPawn()->GetActorLocation();
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0.f;
        RandomDirection.Normalize();

        FVector WanderDestination = AILocation + (RandomDirection * 600.f);
        MoveToLocation(WanderDestination, 50.f);
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