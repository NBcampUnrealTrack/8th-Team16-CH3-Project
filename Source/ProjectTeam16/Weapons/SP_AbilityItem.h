// SP_AbilityItem.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectTeam16/Weapons/SP_WeaponType.h"
#include "SP_AbilityItem.generated.h"

UCLASS()
class PROJECTTEAM16_API ASP_AbilityItem : public AActor
{
    GENERATED_BODY()

public:
    ASP_AbilityItem();

protected:
    virtual void BeginPlay() override;

    // 아이템의 외형을 담당할 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UStaticMeshComponent* ItemMesh;

    // 플레이어 감지용 트리거 콜리전
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USphereComponent* CollisionSphere;

    // 에디터에서 이 아이템이 줄 특수 능력을 지정합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Settings")
    EWeaponSpecialAbility AbilityToGrant;

    // 왼손 무기에 줄지, 오른손 무기에 줄지 지정 (True: 오른손, False: 왼손)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Settings")
    bool bTargetRightHand;

    // 충돌 이벤트 함수
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
};