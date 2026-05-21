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

    // 기존 기획하신 위로 둥둥 뜨는 로직 유지
    FVector NewLocation = GetActorLocation() + (FVector::UpVector * UpSpeed * DeltaTime);
    SetActorLocation(NewLocation);

    //  빌보드(Billboard) 연출 코드 추가
    if (UWorld* World = GetWorld())
    {
        // 월드에서 현재 게임을 플레이 중인 1번 플레이어의 컨트롤러를 찾습니다.
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            // 플레이어를 담당하는 카메라 매니저가 있다면
            if (PC->PlayerCameraManager)
            {
                // 실시간 카메라 회전값을 안전하게 가로챕니다.
                FRotator CameraRotation = PC->PlayerCameraManager->GetCameraRotation();

                // 가독성을 위해 글씨가 위아래(Pitch, Roll)로 눕지 않고 좌우(Yaw)로만 정면을 보게 세팅합니다.
                FRotator BillboardRotation = FRotator(CameraRotation.Pitch, CameraRotation.Yaw + 180.0f, CameraRotation.Roll);

                // 내 텍스트 액터의 각도를 카메라 정면으로 강제 일치시킵니다.
                SetActorRotation(BillboardRotation);
            }
        }
    }
    
     // 수명 관리 로직 유지
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