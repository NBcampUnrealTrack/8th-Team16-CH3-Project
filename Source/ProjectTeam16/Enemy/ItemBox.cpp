#include "ProjectTeam16/Enemy/ItemBox.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Item.h"
#include "ProjectTeam16/Data/ProjectDataStructs.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

AItemBox::AItemBox()
{
	PrimaryActorTick.bCanEverTick = false;

    BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
    RootComponent = BoxComp;
    BoxComp->SetCollisionProfileName(TEXT("BlockAll")); // 총알에 맞아야 하므로 Block 설정

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);

    MaxHealth = 30.0f;
    Health = MaxHealth;
}


void AItemBox::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;
    OriginalScale = GetActorScale3D();
    OriginalRotation = GetActorRotation();
}

float AItemBox::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (Health <= 0.f) return 0.f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    //사운드
    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
    }

    // 이펙트
    if (HitEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, GetActorLocation());
    }

    Health -= ActualDamage;

    if (Health <= 0.f)
    {
        DestroyBox();
    }
    else
    {
        PlayHitReaction(); //타격시 움찔거리게 해줌
    }
    return ActualDamage;
}

void AItemBox::DestroyBox()
{
    // 아이템 스폰
        if (DropTable)
        {
            TArray<FBoxDropData*>AllRows;
         
            DropTable->GetAllRows<FBoxDropData>(TEXT(""), AllRows);

            float RandomRoll = FMath::FRand();
            float CumulativeChance = 0.0f;

            for (auto Row : AllRows)
            {
                CumulativeChance += Row->DropChance;
                if (RandomRoll <= CumulativeChance)
                {
                    AItem* SpawnedItem = GetWorld()->SpawnActor<AItem>(Row->ItemClass, GetActorLocation(), GetActorRotation());
                    if (SpawnedItem)
                    {
                        // 데이터 테이블에 적힌 수치를 아이템에게 전달
                        SpawnedItem->Heal = Row->HealingAmount;
                        SpawnedItem->Exp = Row->ExpAmount;
                    }
                    break;
                }
            }
        }
    Destroy();
}

void AItemBox::PlayHitReaction()
{
    // 이미 애니메이션 중이라면 초기화해서 다시 시작
    GetWorldTimerManager().ClearTimer(HitReactionTimer);

    // 크기를 순간적으로 크게
    //SetActorScale3D(OriginalScale * HitScaleMultiplier);
    SetActorScale3D(FVector(OriginalScale.X * 1.2f, OriginalScale.Y * 1.2f, OriginalScale.Z * 0.8f));

    FRotator RandomShake = GetActorRotation();
    RandomShake.Yaw += FMath::FRandRange(-20.0f, 20.0f);
    SetActorRotation(RandomShake);

    // 일정 시간 후에 원래 크기로 되돌리는 함수 호출
    GetWorldTimerManager().SetTimer(HitReactionTimer, this, &AItemBox::ResetScale, ReactionDuration, false);
}

void AItemBox::ResetScale()
{
    SetActorScale3D(OriginalScale);
    SetActorRotation(OriginalRotation);
}



