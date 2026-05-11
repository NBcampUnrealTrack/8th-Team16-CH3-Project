#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ZombiePawn.generated.h"

UCLASS()
class PROJECTTEAM16_API AZombiePawn : public APawn
{
    GENERATED_BODY()

public:
    AZombiePawn();

protected:
    virtual void BeginPlay() override;

    // --- 컴포넌트 ---
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UCapsuleComponent* CapsuleComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    class USkeletalMeshComponent* MeshComp;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    class UFloatingPawnMovement* MovementComp;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    class UPawnSensingComponent* PawnSensing;

    UPROPERTY(VisibleAnywhere, Category = "AI")
    class USphereComponent* AttackRangeSphere;

    // --- 데이터 및 상태 ---
    UPROPERTY(EditAnywhere, Category = "HP")
    float Health;
    UPROPERTY(EditAnywhere, Category = "HP")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Damage")
    float DamageAmount = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Reward")
    int32 ExpReward = 10;

    float LastAttackTime;
    bool bIsDead = false;
    APawn* TargetPlayer = nullptr;

    FTimerHandle AttackTimerHandle;
    FTimerHandle SeeTimerHandle;

    // --- 로직 함수 ---
    virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    void OnSeePlayer(APawn* SeenPawn);

    UFUNCTION()
    void OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void AttackLoop();
    void CheckVisibility();
};