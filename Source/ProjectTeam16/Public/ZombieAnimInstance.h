#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ZombieAnimInstance.generated.h"

class AZombie;

UCLASS()
class PROJECTTEAM16_API UZombieAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayRandomAttackMontage();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayRandomDeathMontage();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    TArray<UAnimMontage*> HitMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    TArray<UAnimMontage*> AttackMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    TArray<UAnimMontage*> DeathMontages;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool IsHit = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool IsDead = false;

private:
    void PlayRandomHitMontage();

    UPROPERTY()
    AZombie* TargetZombie = nullptr;

    int32 LastPlayedIndex = -1;
    float LastHealth = -1.0f;
    float LocalLastAttackTime = 0.0f;
};
