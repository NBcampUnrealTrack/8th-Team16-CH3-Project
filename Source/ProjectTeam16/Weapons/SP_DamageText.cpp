#include "SP_DamageText.h"
#include "Components/TextRenderComponent.h"

ASP_DamageText::ASP_DamageText()
{
    PrimaryActorTick.bCanEverTick = true;

    TextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextRender"));
    RootComponent = TextRender;

    // 텍스트 정렬 및 스타일 세팅
    TextRender->SetHorizontalAlignment(EHTA_Center);
    TextRender->SetVerticalAlignment(EVRTA_TextCenter);
    TextRender->SetTextRenderColor(FColor::Red); // 대미지는 빨간색
    TextRender->SetWorldScale3D(FVector(2.0f));  // 글씨 크기 조정
}

void ASP_DamageText::BeginPlay()
{
    Super::BeginPlay();
}

void ASP_DamageText::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 💡 매 프레임 글씨를 위로 둥둥 띄웁니다.
    FVector NewLocation = GetActorLocation() + (FVector::UpVector * UpSpeed * DeltaTime);
    SetActorLocation(NewLocation);

    // 수명 관리 후 자동 소멸
    Lifetime -= DeltaTime;
    if (Lifetime <= 0.0f)
    {
        Destroy();
    }
}

void ASP_DamageText::SetDamageValue(float DamageAmount)
{
    if (TextRender)
    {
        // 소수점 잘라내고 정수로 표기 (예: "15")
        FString DamageString = FString::FromInt(FMath::RoundToInt(DamageAmount));
        TextRender->SetText(FText::FromString(DamageString));
    }
}

