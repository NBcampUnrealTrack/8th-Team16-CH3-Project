#include "ZombieAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"

// 💡 프로젝트 실제 폴더 구조에 맞는 팀원의 진짜 AZombie.h 헤더 파일 경로
#include "ProjectTeam16/Enemy/Zombie.h" 

void UZombieAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    ACharacter* OwningChar = Cast<ACharacter>(TryGetPawnOwner());
    if (!OwningChar) return;

    // 틱 최적화 우회 강제 활성화 (몽타주 시간 계산용)
    OwningChar->SetActorTickEnabled(true);
    OwningChar->PrimaryActorTick.bCanEverTick = true;

    if (USkeletalMeshComponent* MeshComp = OwningChar->GetMesh())
    {
        MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    }
}

void UZombieAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    APawn* OwningPawn = TryGetPawnOwner();
    if (!OwningPawn) return;

    AZombie* Zombie = Cast<AZombie>(OwningPawn);
    if (!Zombie) return;

    UClass* ActualClass = Zombie->GetClass();
    if (!ActualClass) return;

    // -------------------------------------------------------------
    // [초정밀 변수 스캔 및 Getter 백업 직결 호출]
    // -------------------------------------------------------------
    float CurrentHealth = -1.0f;
    bool bZombieIsDead = false;
    float ZombieLastAttackTime = 0.0f;

    for (TFieldIterator<FProperty> PropIt(ActualClass); PropIt; ++PropIt)
    {
        FString PropName = PropIt->GetName();

        if (PropName.Equals(TEXT("Health"), ESearchCase::IgnoreCase) || PropName.Equals(TEXT("HP"), ESearchCase::IgnoreCase))
        {
            if (FNumericProperty* NumericProp = CastField<FNumericProperty>(*PropIt))
            {
                if (NumericProp->IsFloatingPoint()) CurrentHealth = NumericProp->GetFloatingPointPropertyValue(PropIt->ContainerPtrToValuePtr<void>(Zombie));
                else CurrentHealth = static_cast<float>(NumericProp->GetSignedIntPropertyValue(PropIt->ContainerPtrToValuePtr<void>(Zombie)));
            }
        }
        else if (PropName.Equals(TEXT("bIsDead"), ESearchCase::IgnoreCase) || PropName.Equals(TEXT("IsDead"), ESearchCase::IgnoreCase))
        {
            if (FBoolProperty* BoolProp = CastField<FBoolProperty>(*PropIt)) bZombieIsDead = BoolProp->GetPropertyValue(PropIt->ContainerPtrToValuePtr<void>(Zombie));
        }
        else if (PropName.Equals(TEXT("LastAttackTime"), ESearchCase::IgnoreCase))
        {
            if (FNumericProperty* NumericProp = CastField<FNumericProperty>(*PropIt)) ZombieLastAttackTime = NumericProp->GetFloatingPointPropertyValue(PropIt->ContainerPtrToValuePtr<void>(Zombie));
        }
    }

    if (CurrentHealth < 0.0f)
    {
        CurrentHealth = Zombie->GetCurrentHealth();
    }
    // -------------------------------------------------------------

    static float LocalLastAttackTime = -1.0f;

    if (LastHealth < 0.0f)
    {
        LastHealth = CurrentHealth;
        LocalLastAttackTime = ZombieLastAttackTime;
        return;
    }

    bool bIsAnyMontagePlaying = Montage_IsPlaying(nullptr);

    // -------------------------------------------------------------
    // [무적 몽타주 재생 제어 판단 트리]
    // -------------------------------------------------------------

    // 💀 1. 사망 판정 (팀원 코드의 즉각적인 Destroy를 무력화하는 최종 하이재킹)
    if (CurrentHealth <= 0.0f || bZombieIsDead)
    {
        if (!IsDead)
        {
            IsDead = true;
            IsHit = false;

            UWorld* World = GetWorld();
            USkeletalMeshComponent* OriginalMesh = Zombie->GetMesh();

            // 에디터 패널에서 이미 등록 완료한 DeathMontages 배열의 데이터를 직접 안전하게 꺼내 씁니다.
            UAnimMontage* SafeDeathMontage = nullptr;
            if (DeathMontages.Num() > 0)
            {
                int32 RandomDeathIndex = FMath::RandRange(0, DeathMontages.Num() - 1);
                SafeDeathMontage = DeathMontages[RandomDeathIndex];
            }

            if (World && OriginalMesh && OriginalMesh->GetSkeletalMeshAsset() && SafeDeathMontage)
            {
                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                // 좀비 본체가 사라져도 삭제되지 않는 전역 월드 스페이스 소속의 가짜 독립 시체 액터 스폰
                ASkeletalMeshActor* GhostActor = World->SpawnActor<ASkeletalMeshActor>(
                    ASkeletalMeshActor::StaticClass(),
                    OriginalMesh->GetComponentLocation(),
                    OriginalMesh->GetComponentRotation(),
                    SpawnParams
                );

                if (GhostActor && GhostActor->GetSkeletalMeshComponent())
                {
                    USkeletalMeshComponent* GhostMesh = GhostActor->GetSkeletalMeshComponent();

                    // 복제 시체에 원본 스킨 메쉬 및 애님 블루프린트 강제 바인딩
                    GhostMesh->SetSkeletalMeshAsset(OriginalMesh->GetSkeletalMeshAsset());
                    GhostMesh->SetAnimInstanceClass(OriginalMesh->GetAnimClass());
                    GhostMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌 방지
                    GhostMesh->SetHiddenInGame(false); // 무조건 보이게 설정
                    GhostMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

                    // 독립 잔상 애님 인스턴스에서 안전하게 보호한 사망 몽타주를 무조건 집행합니다!
                    if (UAnimInstance* GhostAnimInst = GhostMesh->GetAnimInstance())
                    {
                        GhostAnimInst->Montage_Play(SafeDeathMontage, 1.0f);
                    }

                    // 💡 [1초 유예 소멸] 이 가짜 시체 액터에게 딱 1.0초의 전역 수명을 부여합니다.
                    GhostActor->SetLifeSpan(1.0f);
                }
            }

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("★ C++ Ghost Zombie Dead Activated! ★"));
        }
        return;
    }

    // 🤕 2. 피격 판정 (살아있을 때 체력이 깎이면 최우선 순위로 즉각 가속도 0으로 멈추고 재생)
    if (!IsDead && CurrentHealth < LastHealth)
    {
        IsHit = true;

        if (Zombie->GetCharacterMovement())
        {
            Zombie->GetCharacterMovement()->StopMovementImmediately();
        }
        if (AAIController* AIC = Cast<AAIController>(Zombie->GetController()))
        {
            AIC->StopMovement();
        }

        PlayRandomHitMontage();
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("★ C++ Hit Detected! ★"));
    }
    // ⚔️ 3. 공격 판정 (팀원 코드의 AttackLoop 루프 실행 타이밍 정밀 물리 역추적)
    else if (!IsDead && !IsHit && !bIsAnyMontagePlaying)
    {
        // 💡 [에러 원천 차단 우회] 문자열 타이머 핸들 조회를 완벽히 제거하고,
        // 좀비의 실제 마지막 공격 시간(LastAttackTime) 변수의 미세 수치 실시간 변화만을 안전하게 역감지합니다.
        if (ZombieLastAttackTime > 0.0f && ZombieLastAttackTime != LocalLastAttackTime)
        {
            LocalLastAttackTime = ZombieLastAttackTime;

            // AI가 이동 명령을 내리지 못하게 공격 순간 컴포넌트 강제 정지 처리
            if (Zombie->GetCharacterMovement())
            {
                Zombie->GetCharacterMovement()->StopMovementImmediately();
            }
            if (AAIController* AIC = Cast<AAIController>(Zombie->GetController()))
            {
                AIC->StopMovement();
            }

            PlayRandomAttackMontage();

            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("★ C++ Zombie Attack! ★"));
        }
    }

    // 상태 가중치 잠금 (걷기 모션의 도중 새치기 차단)
    if (bIsAnyMontagePlaying)
    {
        if (IsHit) IsHit = true;
    }
    else
    {
        IsHit = false;
    }

    // 실시간 출력 로그
    if (GEngine)
    {
        FString DebugMsg = FString::Printf(TEXT("[Getter Connected] Real HP: %.1f / Max HP: %.1f | Playing: %s"),
            CurrentHealth, Zombie->GetMaxHealth(), bIsAnyMontagePlaying ? TEXT("YES") : TEXT("NO"));
        GEngine->AddOnScreenDebugMessage(99, 0.01f, FColor::Orange, DebugMsg);
    }

    LastHealth = CurrentHealth;
}

void UZombieAnimInstance::PlayRandomHitMontage()
{
    if (HitMontages.Num() == 0) return;
    int32 NewIndex = (HitMontages.Num() > 1) ? FMath::RandRange(0, HitMontages.Num() - 1) : 0;
    if (HitMontages.Num() > 1 && NewIndex == LastPlayedIndex) NewIndex = (NewIndex + 1) % HitMontages.Num();
    LastPlayedIndex = NewIndex;
    if (UAnimMontage* SelectedMontage = HitMontages[NewIndex]) Montage_Play(SelectedMontage, 1.0f);
}

void UZombieAnimInstance::PlayRandomAttackMontage()
{
    if (AttackMontages.Num() == 0) return;
    int32 NewIndex = (AttackMontages.Num() > 1) ? FMath::RandRange(0, AttackMontages.Num() - 1) : 0;
    LastPlayedAttackIndex = NewIndex;
    if (UAnimMontage* SelectedMontage = AttackMontages[NewIndex]) Montage_Play(SelectedMontage, 1.0f);
}

void UZombieAnimInstance::PlayRandomDeathMontage()
{
    if (DeathMontages.Num() == 0) return;
    int32 NewIndex = FMath::RandRange(0, DeathMontages.Num() - 1);
    if (UAnimMontage* SelectedMontage = DeathMontages[NewIndex]) Montage_Play(SelectedMontage, 1.0f);
}
