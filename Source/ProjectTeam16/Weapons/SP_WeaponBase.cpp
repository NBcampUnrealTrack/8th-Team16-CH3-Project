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
    
    //USceneComponent* DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));
    //RootComponent = DummyRoot;

    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(RootComponent); 

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

    bool bIsShotgun = false;
    ASP_Character* OwnerCharacter = Cast<ASP_Character>(GetOwner());

    // 버그 수정 무기 자체 능력이 관통이거나, 캐릭터 큐브 옵션으로 관통을 획득한 경우 둘 다 인정
    bool bHasPenetration = (MyAbility == EWeaponSpecialAbility::Penetration) || (OwnerCharacter && OwnerCharacter->bCubePenetration);

    if (OwnerCharacter)
    {
        EWeaponType CheckType = (OwnerCharacter->LeftHandWeapon == this) ? OwnerCharacter->LeftWeaponData.WeaponType : OwnerCharacter->RightWeaponData.WeaponType;
        if (CheckType == EWeaponType::Shotgun || CheckType == EWeaponType::Blast)
        {
            bIsShotgun = true;
        }
    }

    int32 PelletCount = bIsShotgun ? 12 : 1;
    float SpreadIntensity = bIsShotgun ? CurrentStats.WeaponSpread : 0.0f;

    float TotalDamageAccumulated = 0.0f;
    FVector FirstHitLocation = FVector::ZeroVector;
    FVector FirstHitNormal = FVector::ForwardVector;
    bool bAnyZombieHit = false;
    bool bFinalIsCritical = false;
    bool bFinalIsX2Damage = false;

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

        float FireRange = (CurrentStats.Range > 0.0f) ? CurrentStats.Range : 5000.0f;
        FVector EndLocation = MuzzleLocation + (FireDirection * FireRange);

        // 관통(Penetration) 무기 로직
        if (MyAbility == EWeaponSpecialAbility::Penetration)
        {
            TArray<FHitResult> HitResults;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(this);
            Params.AddIgnoredActor(GetOwner());

            bool bMultiHit = GetWorld()->LineTraceMultiByChannel(HitResults, MuzzleLocation, EndLocation, ECC_Visibility, Params);
            FVector FinalTraceEnd = (bMultiHit && HitResults.Num() > 0) ? HitResults.Last().ImpactPoint : EndLocation;

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
                TArray<AActor*> HitActorsThisTrace;

                for (const FHitResult& Hit : HitResults)
                {
                    AActor* HitActor = Hit.GetActor();
                    if (HitActor)
                    {
                        if (!HitActorsThisTrace.Contains(HitActor))
                        {
                            HitActorsThisTrace.Add(HitActor);

                            float FinalDamage = CurrentStats.Damage;
                            bool bIsCritical = false;
                            bool bIsX2Damage = false;

                            // 대입 연산을 if문 밖으로 완전히 분리하여 에러를 원천 차단합니다.
                            OwnerCharacter = Cast<ASP_Character>(GetOwner());
                            if (OwnerCharacter)
                            {
                                if (FMath::FRandRange(0.0f, 100.0f) <= OwnerCharacter->CubeCritRate)
                                {
                                    bIsCritical = true;
                                    float CritMultiplier = 1.5f + (OwnerCharacter->CubeCritDMG / 100.0f);
                                    FinalDamage *= CritMultiplier;
                                }

                                if (FMath::FRandRange(0.0f, 100.0f) <= OwnerCharacter->CubeX2Chance)
                                {
                                    bIsX2Damage = true;
                                    FinalDamage *= 2.0f;
                                }

                                FinalDamage *= OwnerCharacter->AttackPower;
                            }

                            // 오브젝트/사물 포함 일단 무조건 대미지 먼저 적용
                            UGameplayStatics::ApplyDamage(HitActor, FinalDamage, DamageInstigator, this, UDamageType::StaticClass());

                            // 모든 오브젝트 공통 타격 이펙트 및 사운드 출력
                            if (CurrentStats.ImpactEffect)
                                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurrentStats.ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
                            if (CurrentStats.ImpactSound)
                                UGameplayStatics::PlaySoundAtLocation(this, CurrentStats.ImpactSound, Hit.ImpactPoint);

                            // 맞은 대상이 좀비일 때만 대미지 팝업 연산 누적
                            if (HitActor->ActorHasTag(TEXT("Zombie")))
                            {
                                bAnyZombieHit = true;
                                TotalDamageAccumulated += FinalDamage;

                                if (bIsCritical) bFinalIsCritical = true;
                                if (bIsX2Damage) bFinalIsX2Damage = true;

                                if (FirstHitLocation.IsZero())
                                {
                                    FirstHitLocation = Hit.ImpactPoint;
                                    FirstHitNormal = Hit.ImpactNormal;
                                }

                                // 권총(단발 무기) 데미지 텍스트 실시간 팝업 출력
                                if (!bIsShotgun)
                                {
#if ENABLE_WEAPON_DEBUG
                                    DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 15.0f, 8, FColor::Purple, false, 2.0f, 0, 1.5f);

                                    FActorSpawnParameters TextSpawnParams;
                                    TextSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                                    FVector TextSpawnLocation = Hit.ImpactPoint + (Hit.ImpactNormal * 10.0f) + FVector(FMath::FRandRange(-10.f, 10.f), FMath::FRandRange(-10.f, 10.f), FMath::FRandRange(0.f, 15.f));

                                    // ZeroRotator 주입 (텍스트의 Tick에서 빌보드 정렬)
                                    ASP_DamageText* DamagePopup = GetWorld()->SpawnActor<ASP_DamageText>(ASP_DamageText::StaticClass(), TextSpawnLocation, FRotator::ZeroRotator, TextSpawnParams);
                                    if (DamagePopup)
                                    {
                                        DamagePopup->SetDamageValue(FinalDamage);

                                        if (bIsCritical && bIsX2Damage) DamagePopup->SetTranscendenceEffect();
                                        else if (bIsX2Damage) DamagePopup->SetX2DamageEffect();
                                        else if (bIsCritical) DamagePopup->SetCriticalEffect();
                                    }
#endif
                                }
                            }
                        }
                    }
                }
            }
        }
        // 관통 능력이 없을 때 (Single Trace) 구조
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

            if (bHit && HitResult.GetActor())
            {
                AActor* HitActor = HitResult.GetActor();
                AController* DamageInstigator = GetInstigatorController() ? GetInstigatorController() : (GetOwner() ? GetOwner()->GetInstigatorController() : nullptr);
                float FinalDamage = CurrentStats.Damage;
                bool bIsCritical = false;
                bool bIsX2Damage = false;

                // 대입 연산을 if문 밖으로 완전히 분리하여 에러를 원천 차단합니다.
                OwnerCharacter = Cast<ASP_Character>(GetOwner());
                if (OwnerCharacter)
                {
                    if (FMath::FRandRange(0.0f, 100.0f) <= OwnerCharacter->CubeCritRate)
                    {
                        bIsCritical = true;
                        float CritMultiplier = 1.5f + (OwnerCharacter->CubeCritDMG / 100.0f);
                        FinalDamage *= CritMultiplier;
                    }

                    if (FMath::FRandRange(0.0f, 100.0f) <= OwnerCharacter->CubeX2Chance)
                    {
                        bIsX2Damage = true;
                        FinalDamage *= 2.0f;
                    }

                    FinalDamage *= OwnerCharacter->AttackPower;
                }

                // 오브젝트/사물 포함 일단 무조건 대미지 먼저 적용
                UGameplayStatics::ApplyDamage(HitActor, FinalDamage, DamageInstigator, this, UDamageType::StaticClass());

                // 모든 오브젝트 공통 타격 이펙트 및 사운드 출력
                if (CurrentStats.ImpactEffect)
                    UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CurrentStats.ImpactEffect, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
                if (CurrentStats.ImpactSound)
                    UGameplayStatics::PlaySoundAtLocation(this, CurrentStats.ImpactSound, HitResult.ImpactPoint);

                // 맞은 대상이 좀비일 때만 대미지 팝업 연산 누적
                if (HitActor->ActorHasTag(TEXT("Zombie")))
                {                 
                    bAnyZombieHit = true;
                    TotalDamageAccumulated += FinalDamage;
                    if (bIsCritical) bFinalIsCritical = true;
                    if (bIsX2Damage) bFinalIsX2Damage = true;

                    if (FirstHitLocation.IsZero())
                    {
                        FirstHitLocation = HitResult.ImpactPoint;
                        FirstHitNormal = HitResult.ImpactNormal;
                    }

                    // 권총(단발 무기) 팝업 처리
                    if (!bIsShotgun)
                    {
#if ENABLE_WEAPON_DEBUG
                        DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 12.0f, 8, FColor::Red, false, 2.0f, 0, 1.0f);

                        FActorSpawnParameters TextSpawnParams;
                        TextSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                        FVector TextSpawnLocation = HitResult.ImpactPoint + (HitResult.ImpactNormal * 10.0f) + FVector(FMath::FRandRange(-10.f, 10.f), FMath::FRandRange(-10.f, 10.f), FMath::FRandRange(0.f, 15.f));

                        ASP_DamageText* DamagePopup = GetWorld()->SpawnActor<ASP_DamageText>(ASP_DamageText::StaticClass(), TextSpawnLocation, FRotator::ZeroRotator, TextSpawnParams);
                        if (DamagePopup)
                        {
                            DamagePopup->SetDamageValue(FinalDamage);

                            if (bIsCritical && bIsX2Damage) DamagePopup->SetTranscendenceEffect();
                            else if (bIsX2Damage) DamagePopup->SetX2DamageEffect();
                            else if (bIsCritical) DamagePopup->SetCriticalEffect();
                        }
#endif
                    }
                }
            }
        }
    }

    // 샷건 최종 합산 팝업 출격
    if (bIsShotgun && bAnyZombieHit)
    {
#if ENABLE_WEAPON_DEBUG
        FActorSpawnParameters TextSpawnParams;
        TextSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        FVector TextSpawnLocation = FirstHitLocation + (FirstHitNormal * 15.0f);

        ASP_DamageText* DamagePopup = GetWorld()->SpawnActor<ASP_DamageText>(ASP_DamageText::StaticClass(), TextSpawnLocation, FRotator::ZeroRotator, TextSpawnParams);
        if (DamagePopup)
        {
            DamagePopup->SetDamageValue(TotalDamageAccumulated);

            if (bFinalIsCritical && bFinalIsX2Damage) DamagePopup->SetTranscendenceEffect();
            else if (bFinalIsX2Damage) DamagePopup->SetX2DamageEffect();
            else if (bFinalIsCritical) DamagePopup->SetCriticalEffect();
        }
#endif
    }
}

void ASP_WeaponBase::EnhanceDamage(float Multiplier)
{
    CurrentStats.Damage *= Multiplier;
    UE_LOG(LogTemp, Log, TEXT("Weapon Damage Enhanced: %f"), CurrentStats.Damage);
}