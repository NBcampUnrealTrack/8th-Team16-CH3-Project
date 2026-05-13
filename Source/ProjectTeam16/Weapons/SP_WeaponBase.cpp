//SP_WeaponBase.cpp
#define ENABLE_WEAPON_DEBUG 1  // 1이면 테스트 모드 켜짐, 0이면 꺼짐

#include "SP_WeaponBase.h"
#include "ProjectTeam16/Weapons/SP_WeaponType.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Pawn.h"
#include "SP_DamageText.h"
#include "DrawDebugHelpers.h"
#include "ProjectTeam16/Character/SP_Character.h"

ASP_WeaponBase::ASP_WeaponBase()
{
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

    CurrentStats.Damage = 10.0f;
    CurrentStats.Range = 5000.0f;

    WeaponMesh->SetMobility(EComponentMobility::Movable);

    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
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

    // 데이터 테이블의 GunName 문자열 검사 대신, 캐릭터 소유 무기 타입 데이터를 기반으로 판별
    bool bIsShotgun = false;

   
    // 데이터 테이블에서 가져온 고유 특수 능력 또는 스탯 상에서 샷건 범주에 속하는지 체크.
    if (const ASP_Character* OwnerCharacter = Cast<ASP_Character>(GetOwner()))
    {
        // 왼손 무기인지 오른손 무기인지 확인하여 현재 무기의 장착 타입을 알아냅니다.
        EWeaponType CheckType = (OwnerCharacter->LeftHandWeapon == this) ? OwnerCharacter->LeftWeaponData.WeaponType : OwnerCharacter->RightWeaponData.WeaponType;

        // 샷건 열거형에 해당하는 무기 타입들을 직접 명시합니다.
        if (CheckType == EWeaponType::BasicShotgun ||
            CheckType == EWeaponType::DoubleShotgun ||
            CheckType == EWeaponType::CombatShotgun)
        {
            bIsShotgun = true;
        }
    }

    int32 PelletCount = bIsShotgun ? 12 : 1;  // 샷건 총알개수

    float SpreadIntensity = bIsShotgun ? 0.22f : 0.0f;

    for (int32 i = 0; i < PelletCount; i++)
    {
        FVector FireDirection = ForwardVector;
        if (bIsShotgun)
        {
            // 각 탄환(Pellet)마다 완전히 무작위의 고유한 궤적 방향 벡터를 계산합니다.
            FireDirection.X += FMath::FRandRange(-SpreadIntensity, SpreadIntensity);
            FireDirection.Y += FMath::FRandRange(-SpreadIntensity, SpreadIntensity);
            FireDirection.Z += FMath::FRandRange(-SpreadIntensity, SpreadIntensity);
            FireDirection.Normalize();
        }

        // 사거리 증가가 적용되어 확장된 Range 스탯 반영
        float FireRange = (CurrentStats.Range > 0.0f) ? CurrentStats.Range : 5000.0f;
        FVector EndLocation = MuzzleLocation + (FireDirection * FireRange);

        // 관통(Penetration) 능력이 켜져 있다면 단일 레이가 아닌 다중 레이트레이스를 돌려야 합니다.
        if (MyAbility == EWeaponSpecialAbility::Penetration)
        {
            TArray<FHitResult> HitResults;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(this);
            Params.AddIgnoredActor(GetOwner());

            bool bMultiHit = GetWorld()->LineTraceMultiByChannel(HitResults, MuzzleLocation, EndLocation, ECC_Visibility, Params);
            FVector FinalTraceEnd = (bMultiHit && HitResults.Num() > 0) ? HitResults.Last().ImpactPoint : EndLocation;

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

            if (bMultiHit && HitResults.Num() > 0)
            {
                AController* DamageInstigator = GetInstigatorController() ? GetInstigatorController() : (GetOwner() ? GetOwner()->GetInstigatorController() : nullptr);
                float FinalDamage = CurrentStats.Damage;
                if (const ASP_Character* OwnerCharacter = Cast<ASP_Character>(GetOwner()))
                {
                    FinalDamage *= OwnerCharacter->AttackPower;
                }

                TArray<AActor*> HitActorsThisTrace;

                for (const FHitResult& Hit : HitResults)
                {
                    AActor* HitActor = Hit.GetActor();
                    if (HitActor)
                    {
                        if (!HitActorsThisTrace.Contains(HitActor))
                        {
                            HitActorsThisTrace.Add(HitActor);
                            UGameplayStatics::ApplyDamage(HitActor, FinalDamage, DamageInstigator, this, UDamageType::StaticClass());

#if ENABLE_WEAPON_DEBUG
                            DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 15.0f, 8, FColor::Purple, false, 2.0f, 0, 1.5f);

                            FActorSpawnParameters TextSpawnParams;
                            TextSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                            FVector TextSpawnLocation = Hit.ImpactPoint + (Hit.ImpactNormal * 10.0f) + FVector(FMath::FRandRange(-10.f, 10.f), FMath::FRandRange(-10.f, 10.f), FMath::FRandRange(0.f, 15.f));

                            ASP_DamageText* DamagePopup = GetWorld()->SpawnActor<ASP_DamageText>(ASP_DamageText::StaticClass(), TextSpawnLocation, FRotator::ZeroRotator, TextSpawnParams);
                            if (DamagePopup)
                            {
                                DamagePopup->SetDamageValue(FinalDamage);
                            }
#endif
                        }
                    }

                    if (CurrentStats.ImpactEffect)
                        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurrentStats.ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
                    if (CurrentStats.ImpactSound)
                        UGameplayStatics::PlaySoundAtLocation(this, CurrentStats.ImpactSound, Hit.ImpactPoint);
                }
            }
        }
        // 관통 능력이 없을 때는 단일 피격(Single Trace) 구조
        else
        {
            FHitResult HitResult;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(this);
            Params.AddIgnoredActor(GetOwner());

            bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleLocation, EndLocation, ECC_Visibility, Params);
            FVector FinalTraceEnd = bHit ? HitResult.ImpactPoint : EndLocation;

            if (CurrentStats.TracerEffect)
            {
                UNiagaraComponent* TracerComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurrentStats.TracerEffect, MuzzleLocation, FRotator::ZeroRotator);
                if (TracerComp)
                {
                    TracerComp->SetVariableVec3(TEXT("User.Start"), MuzzleLocation);
                    TracerComp->SetVariableVec3(TEXT("User.End"), FinalTraceEnd);
                }
            }

            if (bHit)
            {
                AActor* HitActor = HitResult.GetActor();
                if (HitActor)
                {
                    AController* DamageInstigator = GetInstigatorController() ? GetInstigatorController() : (GetOwner() ? GetOwner()->GetInstigatorController() : nullptr);
                    float FinalDamage = CurrentStats.Damage;
                    if (const ASP_Character* OwnerCharacter = Cast<ASP_Character>(GetOwner()))
                    {
                        FinalDamage *= OwnerCharacter->AttackPower;
                    }

                    UGameplayStatics::ApplyDamage(HitActor, FinalDamage, DamageInstigator, this, UDamageType::StaticClass());

#if ENABLE_WEAPON_DEBUG
                    DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 15.0f, 8, FColor::Purple, false, 2.0f, 0, 1.5f);

                    FActorSpawnParameters TextSpawnParams;
                    TextSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                    FVector TextSpawnLocation = HitResult.ImpactPoint + (HitResult.ImpactNormal * 10.0f) + FVector(FMath::FRandRange(-10.f, 10.f), FMath::FRandRange(-10.f, 10.f), FMath::FRandRange(0.f, 15.f));

                    ASP_DamageText* DamagePopup = GetWorld()->SpawnActor<ASP_DamageText>(ASP_DamageText::StaticClass(), TextSpawnLocation, FRotator::ZeroRotator, TextSpawnParams);
                    if (DamagePopup)
                    {
                        DamagePopup->SetDamageValue(FinalDamage);
                    }
#endif
                }

                if (CurrentStats.ImpactEffect)
                    UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurrentStats.ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
                if (CurrentStats.ImpactSound)
                    UGameplayStatics::PlaySoundAtLocation(this, CurrentStats.ImpactSound, HitResult.ImpactPoint);
            }
        }
    }
}