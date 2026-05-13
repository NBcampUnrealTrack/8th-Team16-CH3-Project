//SP_WeaponBase.cpp

#include "SP_WeaponBase.h"
#include "ProjectTeam16/Weapons/SP_WeaponType.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Pawn.h"
#include "ProjectTeam16/Character/SP_Character.h"

ASP_WeaponBase::ASP_WeaponBase()
{
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

    CurrentStats.Damage = 10.0f;
    CurrentStats.Range = 5000.0f;

    WeaponMesh->SetMobility(EComponentMobility::Movable);
}

void ASP_WeaponBase::SetWeaponVisuals(UStaticMesh* NewMesh)
{
    if (WeaponMesh && NewMesh)
    {
        WeaponMesh->SetStaticMesh(NewMesh);

        WeaponMesh->SetHiddenInGame(false);
        WeaponMesh->SetVisibility(true);
    }
}

void ASP_WeaponBase::Fire(FVector ForwardVector)
{
    if (!WeaponMesh) return;

    FName MuzzleSocketName = TEXT("Muzzle");
    FVector MuzzleLocation = WeaponMesh->DoesSocketExist(MuzzleSocketName) ? WeaponMesh->GetSocketLocation(MuzzleSocketName) : GetActorLocation();

    if (CurrentStats.FireSound) UGameplayStatics::PlaySoundAtLocation(this, CurrentStats.FireSound, MuzzleLocation);
    if (CurrentStats.MuzzleFlash) UNiagaraFunctionLibrary::SpawnSystemAttached(CurrentStats.MuzzleFlash, WeaponMesh, MuzzleSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);

    bool bIsShotgun = CurrentStats.GunName.Contains(TEXT("Shotgun"));
    int32 PelletCount = bIsShotgun ? 8 : 1;
    float SpreadIntensity = 0.08f;

    for (int32 i = 0; i < PelletCount; i++)
    {
        FVector FireDirection = ForwardVector;
        if (bIsShotgun)
        {
            FireDirection.X += FMath::FRandRange(-SpreadIntensity, SpreadIntensity);
            FireDirection.Y += FMath::FRandRange(-SpreadIntensity, SpreadIntensity);
            FireDirection.Z += FMath::FRandRange(-SpreadIntensity, SpreadIntensity);
            FireDirection.Normalize();
        }

        // [특수 능력 연동]: 사거리 증가가 적용되어 확장된 Range 스탯 반영
        float FireRange = (CurrentStats.Range > 0.0f) ? CurrentStats.Range : 5000.0f;
        FVector EndLocation = MuzzleLocation + (FireDirection * FireRange);

        // 관통 레이캐스팅을 위한 다중 충돌 배열 선언
        TArray<FHitResult> HitResults;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(GetOwner());

        // 다중 히트 트레이스 실행
        bool bHit = GetWorld()->LineTraceMultiByChannel(HitResults, MuzzleLocation, EndLocation, ECC_Visibility, Params);
        FVector FinalTraceEnd = (bHit && HitResults.Num() > 0) ? HitResults.Last().ImpactPoint : EndLocation;

        // 탄도 궤적 생성
        if (CurrentStats.TracerEffect)
        {
            UNiagaraComponent* TracerComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurrentStats.TracerEffect, MuzzleLocation, FRotator::ZeroRotator);
            if (TracerComp)
            {
                TracerComp->SetVariableVec3(TEXT("User.Start"), MuzzleLocation);
                TracerComp->SetVariableVec3(TEXT("User.End"), FinalTraceEnd);
            }
        }

        if (bHit && HitResults.Num() > 0)
        {
            AController* DamageInstigator = GetInstigatorController();
            if (!DamageInstigator && GetOwner())
            {
                DamageInstigator = GetOwner()->GetInstigatorController();
            }

            float FinalDamage = CurrentStats.Damage;
            if (const ASP_Character* OwnerCharacter = Cast<ASP_Character>(GetOwner()))
            {
                FinalDamage *= OwnerCharacter->AttackPower;
            }

            // 펠릿/단발 사격에서 이미 대미지를 입은 액터들을 기억하는 배열
            TArray<AActor*> HitActorsThisTrace;

            // 충돌한 물체들을 순회하며 대미지 처리
            for (const FHitResult& Hit : HitResults)
            {
                AActor* HitActor = Hit.GetActor();
                if (HitActor)
                {
                    //처음 맞춘 액터인 경우에만 대미지 전달 (동일 타겟 중복 타격 방지)
                    if (!HitActorsThisTrace.Contains(HitActor))
                    {
                        HitActorsThisTrace.Add(HitActor);
                        UGameplayStatics::ApplyDamage(HitActor, FinalDamage, DamageInstigator, this, UDamageType::StaticClass());
                    }
                }
                if (CurrentStats.ImpactEffect) 
                    UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurrentStats.ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
                if (CurrentStats.ImpactSound) 
                    UGameplayStatics::PlaySoundAtLocation(this, CurrentStats.ImpactSound, Hit.ImpactPoint);

                // [특수 능력 연동]: 관통 능력이 없는 무기라면 첫 번째 오브젝트(벽/적) 충돌 즉시 루프 탈출 (관통 방지)
                if (MyAbility != EWeaponSpecialAbility::Penetration)
                {
                    break;
                }
            }
        }
    }
}