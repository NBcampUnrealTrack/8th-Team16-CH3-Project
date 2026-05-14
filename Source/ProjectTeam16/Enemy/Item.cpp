#include "ProjectTeam16/Enemy/Item.h"
#include "Components\SphereComponent.h"
#include "ProjectTeam16/Character/SP_Character.h"
#include "ProjectTeam16/Data/ProjectDataStructs.h"
#include "Kismet/GameplayStatics.h"

AItem::AItem()
{
 	
	PrimaryActorTick.bCanEverTick = true;

    PickupRange = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRange"));
    SetRootComponent(PickupRange);
    PickupRange->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlap);

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(PickupRange);
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (bIsFollowingPlayer && TargetPlayer)
    {
        // 플레이어 방향으로 부드럽게 이동 (자석 효과)
        FVector CurrentLocation = GetActorLocation();
        FVector TargetLocation = TargetPlayer->GetActorLocation();

        FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);
        SetActorLocation(NewLocation);

        // 거리가 아주 가까워지면 획득 처리
        if (FVector::Dist(CurrentLocation, TargetLocation) < 50.0f)
        {
            ASP_Character* Player = Cast<ASP_Character>(TargetPlayer);
            if (Player)
            {
                if (Heal > 0.0f)
                {
                    // Player->AddHealth(Heal);
                    UE_LOG(LogTemp, Warning, TEXT("Heal HP"));
                }

                if (Exp > 0)
                {
                    //Player->AddExp(Exp);
                    UE_LOG(LogTemp, Warning, TEXT("Get Exp"));
                };
            }
           
            if (PickupSound)
            {
                UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
            }

            Destroy();
        }
    }
}

void AItem::OnOverlap(
    UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex,
    bool bFromSweep, 
    const FHitResult& SweepResult)
{
    ASP_Character* Player = Cast<ASP_Character>(OtherActor);
    if(Player)
    {
        TargetPlayer = Player;
        bIsFollowingPlayer = true;

        // 자석 효과가 시작되면 다른 충돌은 무시하도록 콜리전 끄기
        PickupRange->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    
}



