#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SP_DamageText.generated.h"

UCLASS()
class PROJECTTEAM16_API ASP_DamageText : public AActor
{
    GENERATED_BODY()

public:
    ASP_DamageText();
    virtual void Tick(float DeltaTime) override;

    // 대미지 수치를 텍스트로 변환해 세팅하는 함수
    void SetDamageValue(float DamageAmount);

    // 크리 데미지 텍스트
    void SetCriticalEffect();

    // 2배 피해 전용 효과
    void SetX2DamageEffect();

    // 초월(치명타+2배) 효과
    void SetTranscendenceEffect();
protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UTextRenderComponent* TextRender;

    float Lifetime = 1.0f; // 1초 뒤 소멸
    float UpSpeed = 100.0f; // 매 초 위로 100유닛 상승
};