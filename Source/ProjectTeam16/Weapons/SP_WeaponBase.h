//SP_WeaponBase.h 
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectTeam16/Weapons/SP_WeaponType.h"
#include "SP_WeaponBase.generated.h"

UCLASS()
class ASP_WeaponBase : public AActor
{
    GENERATED_BODY()

public:
    ASP_WeaponBase();

    // 캐릭터가 호출하는 사격 함수 
    virtual void Fire(FVector ForwardVector);

    // 무기 데이터 설정 함수
    void SetWeaponStats(FGunStats NewStats) { CurrentStats = NewStats; }

    // 추가 캐릭터가 무기의 현재 실시간 스탯을 읽어갈 수 있도록 겟터 함수 추가
    FGunStats GetWeaponStats() const { return CurrentStats; }

    // 메쉬를 동적으로 바꾸기 위한 함수
    void SetWeaponVisuals(UStaticMesh* NewMesh);

    void SetSpecialAbility(EWeaponSpecialAbility Ability) { MyAbility = Ability; }

    // Adds an actor to the list of actors to ignore during movement (collision).
    void MoveIgnoreActorAdd(AActor* ActorToIgnore)
    {
        if (UPrimitiveComponent* PrimitiveRoot = Cast<UPrimitiveComponent>(GetRootComponent()))
        {
            PrimitiveRoot->MoveIgnoreActors.AddUnique(ActorToIgnore);
        }
    }

protected:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* WeaponMesh;

    UPROPERTY(BlueprintReadOnly)
    FGunStats CurrentStats;

    EWeaponSpecialAbility MyAbility;

    // 블루프린트에서 무기별로 지정해 줄 카메라 셰이크 에셋 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSubclassOf<class UCameraShakeBase> FireCameraShakeClass;
};