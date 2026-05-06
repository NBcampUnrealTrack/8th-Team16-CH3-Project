#include "ProjectTeam16/Enemy/Zombie.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components\SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AZombie::AZombie()
{

    PrimaryActorTick.bCanEverTick = false;

    MaxHealth = 100.0f;
    Health = MaxHealth;

    // 플레이어 감지 센서 설정
    PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
    PawnSensing->SightRadius = 2000.0f; // 감지 거리
    PawnSensing->SetPeripheralVisionAngle(45.0f); // 시야각

    AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
    AttackRangeSphere->SetupAttachment(RootComponent);
    AttackRangeSphere->SetSphereRadius(150.0f); // 공격 사거리

    // 충돌 이벤트 연결
    AttackRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AZombie::OnAttackOverlapBegin);
    AttackRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AZombie::OnAttackOverlapEnd);

}

void AZombie::BeginPlay()
{
    Super::BeginPlay();

    if (PawnSensing)
    {
        // OnSeePlayer 함수를 플레이어 발견 이벤트에 등록
        PawnSensing->OnSeePawn.AddDynamic(this, &AZombie::OnSeePlayer);
    }
}

float AZombie::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

    Health -= ActualDamage;

    if (Health <= 0.0f)
    {
        // 경험치 변수 상승 로직을 여기에 추가 
        Destroy(); // 일단 즉시 삭제
    }

    return ActualDamage;
}

void AZombie::OnSeePlayer(APawn* SeenPawn)
{
    // 발견한 대상이 플레이어인지 확인
    if (SeenPawn)
    {
        TargetPlayer = SeenPawn;

        AAIController* AIController = Cast<AAIController>(GetController());
        if (AIController)
        {
            // 플레이어에게 이동 (NavMesh 기반 장애물 회피 포함)
            AIController->MoveToActor(SeenPawn, 5.0f); // 5.0f는 정지 거리

        }
    }
}

void AZombie::OnAttackOverlapBegin(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)

{
    if (OtherActor && OtherActor == TargetPlayer)
    {
        // 1.5초마다 반복 공격 타이머 시작
        GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AZombie::AttackLoop, 1.5f, true, 0.0f);
    }
}

void AZombie::OnAttackOverlapEnd(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)

{
    if (OtherActor && OtherActor == TargetPlayer)
    {
        // 범위를 벗어나면 타이머 초기화
        GetWorldTimerManager().ClearTimer(AttackTimerHandle);
    }
}

void AZombie::AttackLoop()
{
    if (IsValid(TargetPlayer))
    {
        UGameplayStatics::ApplyDamage(TargetPlayer, DamageAmount, GetController(), this, nullptr);
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Zombie Attack!"));
    }
}

