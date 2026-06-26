#include "MyAIController.h"
#include "NavigationSystem.h"
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
    SightConfig->PeripheralVisionAngleDegrees = 360.f;
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

    if (!bReady)
    {
        StartDelay -= DeltaTime;
        if (StartDelay <= 0.f)
        {
            bReady = true;
        }
        return;
    }

    if (!GetPawn()) return;
    ACharacter* BombChar = Cast<ACharacter>(GetPawn());
    if (BombChar)
    {
        // Obtener el valor del enum EstadoBomba
        UFunction* GetStateFunc = BombChar->FindFunction(FName("GetEstadoBomba"));
        if (GetStateFunc)
        {
            // Si no está en Run, detener movimiento
            StopMovement();
            return;
        }
    }

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
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green,
        FString::Printf(TEXT("Dist: %.0f - Moving: %s"), 
            DistToClosest,
            MoveUpdateTimer >= 0.2f ? TEXT("YES") : TEXT("NO")
        )
    );
    if (DistToClosest < PanicRadius)
    {
    if (MoveUpdateTimer >= 0.2f)
    {
        MoveUpdateTimer = 0.f;

        FVector AILocation = GetPawn()->GetActorLocation();
        FVector PlayerLocation = Closest->GetActorLocation();
        FVector FleeDirection = (AILocation - PlayerLocation).GetSafeNormal();
        FVector FleeDestination = AILocation + (FleeDirection * 1200.f);

        // Proyectar el destino al NavMesh más cercano
        FNavLocation ProjectedLocation;
        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        if (NavSys && NavSys->ProjectPointToNavigation(FleeDestination, ProjectedLocation, FVector(500.f, 500.f, 500.f)))
        {
            MoveToLocation(ProjectedLocation.Location, 50.f);
        }
        else
        {
            // Si no encuentra punto válido, muévete en dirección opuesta corta
            MoveToLocation(AILocation + (FleeDirection * 300.f), 50.f);
        }
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