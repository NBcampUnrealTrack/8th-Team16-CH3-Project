#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ZombieAnimInstance.generated.h"

UCLASS()
class PROJECTTEAM16_API UZombieAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZombieAnim")
    bool IsHit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZombieAnim")
    bool IsDead = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZombieAnim")
    int32 RandomHitIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ZombieAnim")
    int32 RandomBurstIndex = 0;
    // 💡 블루프린트 매 프레임 업데이트(Tick)에 대응하는 C++ 내부 함수입니다.
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
    float LastHealth = -1.0f; // 이전 프레임 체력 기억용 변수
};
