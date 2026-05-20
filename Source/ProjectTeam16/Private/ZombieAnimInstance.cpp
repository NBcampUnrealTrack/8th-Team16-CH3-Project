#include "ZombieAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "ProjectTeam16/Enemy/Zombie.h" 

void UZombieAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    TargetZombie = Cast<AZombie>(TryGetPawnOwner());
}

void UZombieAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    // 좀비 링크가 없거나 유효하지 않으면 패스 (크래시 방지)
    if (!IsValid(TargetZombie)) return;

    float CurrentHealth = TargetZombie->GetCurrentHealth();
    bool bZombieIsDead = TargetZombie->IsZombieDead();
    float ZombieLastAttackTime = TargetZombie->GetLastAttackTime();

    if (LastHealth < 0.0f)
    {
        LastHealth = CurrentHealth;
        LocalLastAttackTime = ZombieLastAttackTime;
        return;
    }

    bool bIsAnyMontagePlaying = Montage_IsPlaying(nullptr);

    // 💀 1. 사망 판정
    if (bZombieIsDead || CurrentHealth <= 0.0f)
    {
        if (!IsDead)
        {
            IsDead = true;
            IsHit = false;
            PlayRandomDeathMontage();
        }
        return;
    }

    // 🤕 2. 피격 판정
    if (CurrentHealth < LastHealth)
    {
        IsHit = true;

        if (TargetZombie->GetCharacterMovement()) TargetZombie->GetCharacterMovement()->StopMovementImmediately();
        if (AAIController* AIC = Cast<AAIController>(TargetZombie->GetController())) AIC->StopMovement();

        PlayRandomHitMontage();
    }
    // ⚔️ 3. 공격 판정
    else if (!IsHit && !bIsAnyMontagePlaying)
    {
        if (ZombieLastAttackTime > 0.0f && ZombieLastAttackTime != LocalLastAttackTime)
        {
            LocalLastAttackTime = ZombieLastAttackTime;

            if (TargetZombie->GetCharacterMovement()) TargetZombie->GetCharacterMovement()->StopMovementImmediately();
            if (AAIController* AIC = Cast<AAIController>(TargetZombie->GetController())) AIC->StopMovement();

            PlayRandomAttackMontage();
        }
    }

    // 상태 플래그 동기화
    if (!bIsAnyMontagePlaying)
    {
        IsHit = false;
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
    int32 NewIndex = FMath::RandRange(0, AttackMontages.Num() - 1);
    if (UAnimMontage* SelectedMontage = AttackMontages[NewIndex]) Montage_Play(SelectedMontage, 1.0f);
}

void UZombieAnimInstance::PlayRandomDeathMontage()
{
    if (DeathMontages.Num() == 0) return;
    int32 NewIndex = FMath::RandRange(0, DeathMontages.Num() - 1);
    if (UAnimMontage* SelectedMontage = DeathMontages[NewIndex]) Montage_Play(SelectedMontage, 1.0f);
}
