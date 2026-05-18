#include "ZombieAnimInstance.h"
#include "GameFramework/Pawn.h"
#include "ProjectTeam16/Enemy/ZombiePawn.h" 

void UZombieAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    APawn* OwningPawn = TryGetPawnOwner();
    if (!OwningPawn) return;

    AZombiePawn* ZombiePawn = Cast<AZombiePawn>(OwningPawn);
    if (!ZombiePawn) return;

    // -------------------------------------------------------------
    // [핵심] 오프셋 숫자 없이, 언리얼 리플렉션 시스템으로 Health 변수를 강제로 찾아옵니다.
    // -------------------------------------------------------------
    float CurrentHealth = 100.0f;

    // AZombiePawn 클래스 내부에 선언된 "Health"라는 이름의 속성(Property)을 찾습니다.
    if (FProperty* HealthProperty = AZombiePawn::StaticClass()->FindPropertyByName(TEXT("Health")))
    {
        // 찾은 속성의 메모리 주소에서 진짜 float 값을 오차 없이 안전하게 긁어옵니다.
        if (float* HealthPtr = HealthProperty->ContainerPtrToValuePtr<float>(ZombiePawn))
        {
            CurrentHealth = *HealthPtr;
        }
    }
    // -------------------------------------------------------------

    // 처음 시작할 때 LastHealth 초기화
    if (LastHealth < 0.0f)
    {
        LastHealth = CurrentHealth;
        return;
    }

    // [피격 감지] 체력이 줄어들었다면 IsHit를 켭니다.
    if (CurrentHealth < LastHealth)
    {
        IsHit = true;

        // 💡 [테스트용 코드 추가] C++이 실제로 체력 감소를 성공적으로 감지했는지 화면에 출력합니다.
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("★ C++ Hit Detected! ★"));
        }
    }

    // 💡 [추가 점검 코드] 매 프레임 C++이 팀원 객체에서 읽어오고 있는 체력 값이 몇인지 실시간으로 띄워봅니다.
    if (GEngine)
    {
        FString HealthMsg = FString::Printf(TEXT("Current Zombie Health: %f"), CurrentHealth);
        GEngine->AddOnScreenDebugMessage(1, 0.01f, FColor::Yellow, HealthMsg);
    }
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("★ C++ Hit Detected! ★"));
    }

    LastHealth = CurrentHealth;
}