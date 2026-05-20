#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ZombieAnimInstance.generated.h"

// 💡 [핵심] 상호 참조 무한 인클루드 에러를 방지하기 위한 전방 선언
class AZombie;
class ASkeletalMeshActor;

UCLASS()
class PROJECTTEAM16_API UZombieAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    // 언리얼 애니메이션 시스템 공식 초기화 및 업데이트 상속 함수
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    // ⚔️ 외부 및 에디터 호출용 무작위 공격 재생 함수
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayRandomAttackMontage();

    // 💀 외부 및 에디터 호출용 무작위 사망 재생 함수
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayRandomDeathMontage();

protected:
    // 에디터 디테일 패널에서 등록하는 애니메이션 몽타주 배열들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", Meta = (AllowPrivateAccess = "true"))
    TArray<UAnimMontage*> HitMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", Meta = (AllowPrivateAccess = "true"))
    TArray<UAnimMontage*> AttackMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", Meta = (AllowPrivateAccess = "true"))
    TArray<UAnimMontage*> DeathMontages;

    // 블루프린트 애님 그래프 전환 노드 제어용 부울 변수들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool IsHit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool IsDead = false;

private:
    // 내부 피격 랜덤 재생 처리 함수
    void PlayRandomHitMontage();

    // 이전 모션 중복 실행 방지용 인덱스 기억 변수들
    int32 LastPlayedIndex = -1;
    int32 LastPlayedAttackIndex = -1;

    // 실시간 체력 감소 폭 계산을 위한 이전 프레임 체력 저장 변수
    float LastHealth = -1.0f;
};
