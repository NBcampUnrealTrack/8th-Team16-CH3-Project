#include "ProjectTeam16/Enemy/ItemBox.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

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
}

float AItemBox::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (Health <= 0.f) return 0.f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    Health -= ActualDamage;

    if (Health <= 0.f)
    {
        DestroyBox();
    }

    return ActualDamage;
}

void AItemBox::DestroyBox()
{
    // 아이템 스폰
        if (DropItem)
        {
            GetWorld()->SpawnActor<AActor>(DropItem, GetActorLocation(), GetActorRotation());
        }

    // 파티클이나 사운드 여기서 재생 
  
    Destroy();
}



