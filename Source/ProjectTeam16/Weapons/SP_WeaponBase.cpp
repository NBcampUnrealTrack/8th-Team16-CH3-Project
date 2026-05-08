
#include "SP_WeaponBase.h"
#include "ProjectTeam16/Weapons/SP_WeaponType.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
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

    // 총구 소켓이 있으면 총구 위치에서, 없으면 무기 위치에서 발사합니다.
    FName MuzzleSocketName = TEXT("Muzzle");
    FVector MuzzleLocation = WeaponMesh->DoesSocketExist(MuzzleSocketName)
        ? WeaponMesh->GetSocketLocation(MuzzleSocketName)
        : GetActorLocation();

    if (CurrentStats.FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CurrentStats.FireSound, MuzzleLocation);
    }

    if (CurrentStats.MuzzleFlash)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            CurrentStats.MuzzleFlash,
            WeaponMesh,
            MuzzleSocketName,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );
    }

    float FireRange = (CurrentStats.Range > 0.0f) ? CurrentStats.Range : 5000.0f;
    FVector EndLocation = MuzzleLocation + (ForwardVector * FireRange);


    // 플레이어가 바라보는 방향으로 라인트레이스를 쏴서 맞은 액터에게 피해를 줍니다.
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleLocation, EndLocation, ECC_Visibility, Params);

    if (bHit)
    {
        if (AActor* HitActor = HitResult.GetActor())
        {
            AController* DamageInstigator = GetInstigatorController();
            if (!DamageInstigator)
            {
                if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
                {
                    DamageInstigator = OwnerPawn->GetController();
                }
            }

            float FinalDamage = CurrentStats.Damage;
            if (const ASP_Character* OwnerCharacter = Cast<ASP_Character>(GetOwner()))
            {
                FinalDamage *= OwnerCharacter->AttackPower;
            }

            UGameplayStatics::ApplyDamage(HitActor, FinalDamage, DamageInstigator, this, UDamageType::StaticClass());
        }

        // 피격 위치에 이펙트와 사운드를 출력합니다.
        if (CurrentStats.ImpactEffect)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurrentStats.ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
        }

        if (CurrentStats.ImpactSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, CurrentStats.ImpactSound, HitResult.ImpactPoint);
        }


        DrawDebugLine(GetWorld(), MuzzleLocation, HitResult.ImpactPoint, FColor::Green, false, 0.05f, 0, 1.0f);
    }
    else
    {
        DrawDebugLine(GetWorld(), MuzzleLocation, EndLocation, FColor::Red, false, 0.05f, 0, 0.5f);
    }

}
