// Source/ProjectTeam16/Cube/OptionWidget.cpp
#include "OptionWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectTeam16/Character/SP_Character.h"

void UOptionWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 1. [중요] 기존 등록된 이벤트를 지우고 다시 등록 (Ensure Failed 해결)
    if (RollButton)
    {
        RollButton->OnClicked.RemoveAll(this);
        RollButton->OnClicked.AddDynamic(this, &UOptionWidget::OnClickRoll);
    }
    if (CloseButton)
    {
        CloseButton->OnClicked.RemoveAll(this);
        CloseButton->OnClicked.AddDynamic(this, &UOptionWidget::OnClickClose);
    }

    if (!OptionComponent)
    {
        OptionComponent = NewObject<URandomOptionComponent>(this);
        OptionComponent->RollOptions();
    }

    RefreshUI();
}

void UOptionWidget::OnClickRoll()
{
    ASP_Character* PlayerChar = Cast<ASP_Character>(GetWorld()->GetFirstPlayerController()->GetPawn());

    // 버튼을 클릭했을 때만 실행됨
    if (PlayerChar && PlayerChar->CubeCount >= 5)
    {
        // 1. 비용 지불
        PlayerChar->CubeCount -= 5;

        // 2. 옵션 굴리기 (실제로 랜덤 값을 뽑는 곳)
        if (OptionComponent)
        {
            OptionComponent->RollOptions();
        }

        // 3. 결과 반영을 위해 UI 갱신 (개수가 줄었으니 다시 버튼이 꺼질 수 있음)
        RefreshUI();
    }
}

void UOptionWidget::RefreshUI()
{
    // 안전 장치: 필요한 컴포넌트나 버튼이 로드되지 않았으면 중단
    if (!OptionComponent || !GradeText || !Line1 || !Line2 || !Line3 || !RollButton) return;

    // 1. 현재 등급 텍스트 및 색상 설정
    FString GradeStr;
    FColor GradeColor;

    switch (OptionComponent->CurrentGrade)
    {
    case EOptionGrade::Rare:      GradeStr = TEXT("RARE");      GradeColor = FColor::Cyan;   break;
    case EOptionGrade::Epic:      GradeStr = TEXT("EPIC");      GradeColor = FColor::Purple; break;
    case EOptionGrade::Unique:    GradeStr = TEXT("UNIQUE");    GradeColor = FColor::Orange; break;
    case EOptionGrade::Legendary: GradeStr = TEXT("LEGENDARY"); GradeColor = FColor::Red;    break;
    }

    GradeText->SetText(FText::FromString(GradeStr));
    GradeText->SetColorAndOpacity(FSlateColor(GradeColor));

    // 2. 옵션 줄 내용 갱신
    Line1->SetText(FText::FromString(GetOptionString(OptionComponent->CurrentOptions[0])));
    Line2->SetText(FText::FromString(GetOptionString(OptionComponent->CurrentOptions[1])));
    Line3->SetText(FText::FromString(GetOptionString(OptionComponent->CurrentOptions[2])));

    // 3.버튼 활성화 로직 추가

    ASP_Character* PlayerChar = Cast<ASP_Character>(GetWorld()->GetFirstPlayerController()->GetPawn());
    if (PlayerChar)
    {
        FString CubeString = FString::Printf(TEXT("보유 큐브: %d 개"), PlayerChar->CubeCount);
        CubeCountText->SetText(FText::FromString(CubeString));

        // 큐브가 5개 이상일 때만 활성화
        bool bCanRoll = (PlayerChar->CubeCount >= 5);

        // 안전하게 버튼 상태 변경
        RollButton->SetIsEnabled(bCanRoll);

        // 로그를 통해 상태 확인 (Output Log 창 확인)
        UE_LOG(LogTemp, Warning, TEXT("UI Update - Cube: %d, RollEnabled: %s"),
            PlayerChar->CubeCount, bCanRoll ? TEXT("True") : TEXT("False"));
    }
}

FString UOptionWidget::GetOptionString(const FOptionLine& Data)
{
    FString Name;

    switch (Data.OptionType)
    {
    case EOptionType::ATKUP:     Name = TEXT("공격력");       break;
    case EOptionType::ATSUP:     Name = TEXT("연사속도");      break;
    case EOptionType::RangeUP:   Name = TEXT("사거리");        break;
    case EOptionType::PenUP:     Name = TEXT("관통력");        break;
    case EOptionType::CritRate:  Name = TEXT("크리티컬 확률");      break;
    case EOptionType::CritDMG:   Name = TEXT("크리티컬 데미지");    break;
    case EOptionType::EXPRate:   Name = TEXT("획득 경험치");   break;
    case EOptionType::BossDMG:   Name = TEXT("보스 데미지");   break;
    case EOptionType::SplashDMG: Name = TEXT("공격 시 폭발 확률");  break;
    case EOptionType::x2chance:  Name = TEXT("공격 시 2배 피해 확률");   break;
    case EOptionType::MoreCube:  Name = TEXT("큐브 획득량");   break;
    default:                     Name = TEXT("Option");             break;
    }

    return FString::Printf(TEXT("%s +%.0f%%"), *Name, Data.Value);
}

void UOptionWidget::OnClickClose()
{
    ASP_Character* PlayerChar = Cast<ASP_Character>(GetWorld()->GetFirstPlayerController()->GetPawn());

    // 1. 현재 화면에 보이는 최종 옵션을 캐릭터 스탯으로 확정 적용
    if (PlayerChar && OptionComponent)
    {
        PlayerChar->ApplyCubeOptions(OptionComponent->CurrentOptions);
    }

    // 2. 위젯 제거 및 게임 복귀
    RemoveFromParent();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = false;
        UGameplayStatics::SetGamePaused(GetWorld(), false);
    }
}