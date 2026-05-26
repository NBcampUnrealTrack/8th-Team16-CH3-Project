#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Zombie.generated.h"

class USphereComponent;
class USoundBase;
class UParticleSystem;
class UDataTable;

UCLASS()
class PROJECTTEAM16_API AZombie : public ACharacter
{
    GENERATED_BODY()

public:
    AZombie();

    float GetCurrentHealth() const { return Health; }
    float GetMaxHealth() const { return MaxHealth; }
    bool IsZombieDead() const { return bIsDead; }
    float GetLastAttackTime() const { return LastAttackTime; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DT")
    FName StatRowName;

    float Health;
    float MaxHealth;
    float DamageAmount;
    float LastAttackTime;
    float LastHitSoundTime = 0.0f;
    int32 ExpAmount;

    FTimerHandle AttackTimerHandle;
    FTimerHandle SeeTimerHandle;
    FTimerHandle ResetChaseTimerHandle;

    bool bIsDead = false;

    UPROPERTY(BlueprintReadOnly, Category = "AI")
    APawn* TargetPlayer = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    TObjectPtr<USphereComponent> AttackRangeSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TObjectPtr<USoundBase> HitSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TObjectPtr<UParticleSystem> HitParticle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DT")
    TObjectPtr<UDataTable> ZombieStatTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundAttenuation* HitSoundAttenuation;

    virtual void BeginPlay() override;
    virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION()
    void OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void AttackLoop();
    void CheckVisibility();
    void ChasePlayer();
    void MoveToNearbyTarget();
    void RetryChaseWithDelay();

    // 사망 후 월드 정리를 위한 지연 함수
    void HandleDeathCleanup();

    // 3초 뒤에 이동을 다시 허용할 함수
    void ResumeMovement();

    // 정지 해제용 타이머 핸들
    FTimerHandle StopMovementTimerHandle;

    // 피격 경직(0.2초)을 해제할 함수
    void ResumeMovementFromHit();

    // 피격 경직용 타이머 핸들
    FTimerHandle HitStunTimerHandle;

public:
    void SetEnrageMode(bool bIsEnraged, float SpeedMultiplier);
};
