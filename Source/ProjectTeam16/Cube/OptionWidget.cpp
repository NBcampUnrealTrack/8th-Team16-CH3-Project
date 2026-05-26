// Source/ProjectTeam16/Cube/OptionWidget.cpp
#include "OptionWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/Image.h"
#include "NiagaraComponent.h"
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
    // 애니메이션 중이면 무시
    if (bIsAnimating) return;

    ASP_Character* PlayerChar = Cast<ASP_Character>(
        GetWorld()->GetFirstPlayerController()->GetPawn());

    if (PlayerChar && PlayerChar->CubeCount >= 5)
    {
        PlayerChar->CubeCount -= 5;

        if (OptionComponent)
            OptionComponent->RollOptions();

        // 버튼 잠금 + 애니메이션 시작
        bIsAnimating = true;
        RollButton->SetIsEnabled(false);
        CloseButton->SetIsEnabled(false);

        if (Anim_FlipOut)
        {
            FWidgetAnimationDynamicEvent FlipOutFinished;
            FlipOutFinished.BindDynamic(this, &UOptionWidget::OnFlipOutFinished);
            BindToAnimationFinished(Anim_FlipOut, FlipOutFinished);
            PlayAnimation(Anim_FlipOut);
        }
        else
        {
            // 애니메이션 없으면 즉시 갱신
            RefreshUI();
        }
    }
}

void UOptionWidget::RefreshUI()
{
    // 안전 장치: 필요한 컴포넌트나 버튼이 로드되지 않았으면 중단
    if (!OptionComponent || !GradeText || !Line1 || !Line2 || !Line3 || !RollButton) return;

    // 1. 현재 등급 텍스트 및 색상 설정
    FString GradeStr = TEXT("UNKNOWN");
    FColor GradeColor = FColor::White;

    switch (OptionComponent->CurrentGrade)
    {
    case EOptionGrade::Rare:      GradeStr = TEXT("RARE");      GradeColor = FColor::Cyan;   break;
    case EOptionGrade::Epic:      GradeStr = TEXT("EPIC");      GradeColor = FColor::Purple; break;
    case EOptionGrade::Unique:    GradeStr = TEXT("UNIQUE");    GradeColor = FColor::Orange; break;
    case EOptionGrade::Legendary: GradeStr = TEXT("LEGENDARY"); GradeColor = FColor::Red;    break;
    default:                      GradeStr = TEXT("UNKNOWN");   GradeColor = FColor::White;  break;
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
        FString CubeString = FString::Printf(TEXT("보유 모듈 에너지: %d"), PlayerChar->CubeCount);
        CubeCountText->SetText(FText::FromString(CubeString));

        // 큐브가 5개 이상일 때만 활성화
        bool bCanRoll = (PlayerChar->CubeCount >= 5);

        // 안전하게 버튼 상태 변경
        RollButton->SetIsEnabled(bCanRoll);

        // 로그를 통해 상태 확인 (Output Log 창 확인)
        UE_LOG(LogTemp, Warning, TEXT("UI Update - Cube: %d, RollEnabled: %s"),
            PlayerChar->CubeCount, bCanRoll ? TEXT("True") : TEXT("False"));
    }
    // 등급 업그레이드 감지
    bool bJustUpgraded = (OptionComponent->CurrentGrade != PreviousGrade);
    PreviousGrade = OptionComponent->CurrentGrade;

    // 시각 효과 적용
    ApplyGradeVisuals(OptionComponent->CurrentGrade, bJustUpgraded);
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

    if (ActiveUpgradeVFXComponent)
    {
        ActiveUpgradeVFXComponent->DestroyComponent();
        ActiveUpgradeVFXComponent = nullptr;
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

void UOptionWidget::OnFlipInFinished()
{
    UnbindAllFromAnimationFinished(Anim_FlipIn);

    bIsAnimating = false;

    ASP_Character* PlayerChar = Cast<ASP_Character>(
        GetWorld()->GetFirstPlayerController()->GetPawn());
    if (PlayerChar)
    {
        RollButton->SetIsEnabled(PlayerChar->CubeCount >= 5);
    }
    CloseButton->SetIsEnabled(true);
}

void UOptionWidget::OnFlipOutFinished()
{
    UnbindAllFromAnimationFinished(Anim_FlipOut);
    RefreshUI();

    // 등급 업그레이드 시에만 사운드 재생
    if (OptionComponent->CurrentGrade != PreviousGrade)  // ← 업그레이드 됐을 때만
    {
        USoundBase* SoundToPlay = nullptr;
        switch (OptionComponent->CurrentGrade)
        {
        case EOptionGrade::Rare:      SoundToPlay = RareSound;      break;
        case EOptionGrade::Epic:      SoundToPlay = EpicSound;      break;
        case EOptionGrade::Unique:    SoundToPlay = UniqueSound;    break;
        case EOptionGrade::Legendary: SoundToPlay = LegendarySound; break;
        }
        if (SoundToPlay)
            UGameplayStatics::PlaySound2D(GetWorld(), SoundToPlay);
    }

    if (Anim_FlipIn)
    {
        FWidgetAnimationDynamicEvent FlipInFinished;
        FlipInFinished.BindDynamic(this, &UOptionWidget::OnFlipInFinished);
        BindToAnimationFinished(Anim_FlipIn, FlipInFinished);
        PlayAnimation(Anim_FlipIn);
    }
    else
    {
        bIsAnimating = false;
        RollButton->SetIsEnabled(true);
        CloseButton->SetIsEnabled(true);
    }
}

void UOptionWidget::ApplyGradeVisuals(EOptionGrade Grade, bool bJustUpgraded)
{
    // 1. 등급별 테두리 텍스처 및 텍스트 색상 선택
    UTexture2D* BorderTexture = nullptr;
    FLinearColor TextColor;

    switch (Grade)
    {
    case EOptionGrade::Rare:
        BorderTexture = RareBorderTexture;
        TextColor = FColor::Cyan;    // 하늘색
        break;
    case EOptionGrade::Epic:
        BorderTexture = EpicBorderTexture;
        TextColor = FColor::Purple;   // 보라색
        break;
    case EOptionGrade::Unique:
        BorderTexture = UniqueBorderTexture;
        TextColor = FColor::Orange;   // 주황색
        break;
    case EOptionGrade::Legendary:
        BorderTexture = LegendaryBorderTexture;
        TextColor = FColor::Red;      // 빨간색
        break;
    }

    // 테두리 이미지 교체
    if (GradeBorderImage && BorderTexture)
    {
        FSlateBrush Brush = GradeBorderImage->GetBrush(); // 기존 브러시 크기 유지
        Brush.SetResourceObject(BorderTexture);
        GradeBorderImage->SetBrush(Brush);
    }

    // 2. 등급별 배경 텍스처 선택
    UTexture2D* BackgroundTexture = nullptr;

    switch (Grade)
    {
    case EOptionGrade::Rare:      BackgroundTexture = RareBackgroundTexture;      break;
    case EOptionGrade::Epic:      BackgroundTexture = EpicBackgroundTexture;      break;
    case EOptionGrade::Unique:    BackgroundTexture = UniqueBackgroundTexture;    break;
    case EOptionGrade::Legendary: BackgroundTexture = LegendaryBackgroundTexture; break;
    }

    // 배경 이미지 교체
    if (GradeBackgroundImage && BackgroundTexture)
    {
        FSlateBrush Brush = GradeBackgroundImage->GetBrush(); // 기존 브러시 크기 유지
        Brush.SetResourceObject(BackgroundTexture);
        GradeBackgroundImage->SetBrush(Brush);
    }

    // 3. 등급 업그레이드 시 파티클 (VFX) 효과 적용
    if (bJustUpgraded)
    {
        // [수정] 컴파일 에러를 해결하기 위해 VFX 변수를 먼저 선언합니다.
        UNiagaraSystem* VFX = nullptr;

        switch (Grade)
        {
        case EOptionGrade::Epic:      VFX = EpicUpgradeVFX;      break;
        case EOptionGrade::Unique:    VFX = UniqueUpgradeVFX;    break;
        case EOptionGrade::Legendary: VFX = LegendaryUpgradeVFX; break;
        default: break;
        }

        if (VFX)
        {
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC)
            {
                // 카메라 위치와 방향 가져오기
                FVector CamLoc;
                FRotator CamRot;
                PC->GetPlayerViewPoint(CamLoc, CamRot);

                // 카메라 바로 앞에 스폰
                FVector SpawnLoc = CamLoc + CamRot.Vector() * 100.0f;

                // 기존에 이미 켜져 있던 이펙트가 있다면 안전하게 먼저 파괴
                if (ActiveUpgradeVFXComponent)
                {
                    ActiveUpgradeVFXComponent->DestroyComponent();
                    ActiveUpgradeVFXComponent = nullptr;
                }

                // 스폰하면서 반환되는 컴포넌트를 변수에 저장
                ActiveUpgradeVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                    GetWorld(), VFX, SpawnLoc, CamRot);

                // 컴포넌트가 존재할 경우 자동 파괴 설정 활성화
                if (ActiveUpgradeVFXComponent)
                {
                    ActiveUpgradeVFXComponent->SetAutoDestroy(true);
                }
            }
        }

        // 업그레이드 UI 애니메이션 재생
        if (Anim_GradeUpgrade)
        {
            PlayAnimation(Anim_GradeUpgrade);
        }
    }
}