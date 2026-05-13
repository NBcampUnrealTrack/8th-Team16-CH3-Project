// SP_AbilityItem.cpp
#include "SP_AbilityItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ProjectTeam16/Character/SP_Character.h"
#include "ProjectTeam16/Weapons/SP_WeaponBase.h"

ASP_AbilityItem::ASP_AbilityItem()
{
    PrimaryActorTick.bCanEverTick = false;

    // 컴포넌트 생성 및 기본 세팅
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    RootComponent = CollisionSphere;
    CollisionSphere->SetSphereRadius(50.0f);

    // 콜리전 채널 설정 (오버랩만 감지하도록)
    CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));

    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    ItemMesh->SetupAttachment(RootComponent);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 메쉬는 충돌 제외

    // 기본값 설정
    AbilityToGrant = EWeaponSpecialAbility::Penetration;
    bTargetRightHand = true;
}

void ASP_AbilityItem::BeginPlay()
{
    Super::BeginPlay();

    // 충돌 델리게이트 바인딩
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASP_AbilityItem::OnOverlapBegin);
}

void ASP_AbilityItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // 부딪힌 대상이 플레이어 캐릭터인지 확인
    if (ASP_Character* PlayerChar = Cast<ASP_Character>(OtherActor))
    {
        // 캐릭터에 작성해둔 무기 능력 갱신 함수 호출 
        PlayerChar->ApplyAbilityToHandWeapon(AbilityToGrant, bTargetRightHand);

        // 아이템 획득 후 월드에서 제거
        Destroy();
    }
}

