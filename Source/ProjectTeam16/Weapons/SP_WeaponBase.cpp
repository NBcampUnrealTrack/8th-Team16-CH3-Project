//SP_WeaponBase.cpp 

#include "SP_WeaponBase.h"
#include "ProjectTeam16/Weapons/SP_WeaponType.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ASP_WeaponBase::ASP_WeaponBase()
{
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

    // 기본 스탯 초기화
    CurrentStats.Damage = 10.0f;  //데미지
    CurrentStats.Range = 5000.0f; //사거리
}

void ASP_WeaponBase::Fire(FVector ForwardVector)
{
    if (!WeaponMesh) return;

    // 총구 소켓 위치 가져오기
    FName MuzzleSocketName = TEXT("Muzzle");
    FVector MuzzleLocation = WeaponMesh->DoesSocketExist(MuzzleSocketName)
        ? WeaponMesh->GetSocketLocation(MuzzleSocketName)
        : GetActorLocation();

    // 사거리 적용
    float FireRange = (CurrentStats.Range > 0.0f) ? CurrentStats.Range : 5000.0f;
    FVector EndLocation = MuzzleLocation + (ForwardVector * FireRange);


    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(GetOwner());

    // 라인 트레이스 (ECC_Visibility는 보통 모든 가시 오브젝트와 충돌)
    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleLocation, EndLocation, ECC_Visibility, Params);

    if (bHit)
    {
        if (AActor* HitActor = HitResult.GetActor())
        {
            // 데미지 전달
            UGameplayStatics::ApplyDamage(HitActor, CurrentStats.Damage, GetInstigatorController(), this, UDamageType::StaticClass());

            // 타격 이펙트 (추가예정)
            //UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect(변경), HitResult.ImpactPoint);
        }
        // 충돌 지점까지 선 표시 (테스트 용도) 나이아가라 트레일 이펙트로 변경예정
        DrawDebugLine(GetWorld(), MuzzleLocation, HitResult.ImpactPoint, FColor::Green, false, 0.05f, 0, 1.0f);
    }
    else
    {
        // 허공에 쐈을 때 최대 사거리까지 선 그리기
        DrawDebugLine(GetWorld(), MuzzleLocation, EndLocation, FColor::Red, false, 0.05f, 0, 0.5f);
    }

    // 총구 화염 사운드 (추가예정)
    // UGameplayStatics::PlaySoundAtLocation(this, FireSound(변경), MuzzleLocation);
}