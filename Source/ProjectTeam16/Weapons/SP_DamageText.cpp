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
    TextRender->SetTextRenderColor(FColor::Red);
    TextRender->SetWorldScale3D(FVector(2.0f));
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

void ASP_DamageText::SetCriticalEffect()
{
    if (TextRender)
    {
        TextRender->SetTextRenderColor(FColor::Yellow); // 크리티컬 색상
        TextRender->SetWorldScale3D(FVector(4.0f));     // 크기 확장
        UpSpeed = 250.0f;                               // 튀어오르는 속도
    }
}

void ASP_DamageText::SetX2DamageEffect()
{
    if (TextRender)
    {
        TextRender->SetTextRenderColor(FColor::Cyan);
        TextRender->SetWorldScale3D(FVector(4.0f));
        UpSpeed = 250.0f; 
        // 여기에 추가로 노란색 파티클을 스폰하는 로직을 넣으면 더 좋습니다.
    }
}

// 치명타 + 2배 피해
void ASP_DamageText::SetTranscendenceEffect()
{
    if (TextRender)
    {
        TextRender->SetTextRenderColor(FColor::Purple);
        TextRender->SetWorldScale3D(FVector(6.0f)); 
        UpSpeed = 300.0f; 

        // 여기에 화면 흔들림(Camera Shake)을 넣으면 타격감이 극대화됩니다.
        /*
        UGameplayStatics::PlayWorldCameraShake(GetWorld(), TranscendenceShakeClass, GetActorLocation(), 0.0f, 500.0f);
        */
    }
}