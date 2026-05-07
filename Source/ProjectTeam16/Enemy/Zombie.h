#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Zombie.generated.h"

class UPawnSensingComponent;

UCLASS()
class PROJECTTEAM16_API AZombie : public ACharacter
{
	GENERATED_BODY()

public:

	AZombie();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float Health;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HP")
	float MaxHealth;

	UPROPERTY(EditAnywhere, Category = "Damage")
	float DamageAmount = 5.0f;

	float LastAttackTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	int32 ExpReward = 1;

	FTimerHandle AttackTimerHandle;

	bool bIsDead = false;

	//탐지된 플레이어 정보저장
	APawn* TargetPlayer = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UPawnSensingComponent> PawnSensing;
	
	// 공격 범위를 감지할 충돌 구체
	UPROPERTY(VisibleAnywhere, Category = "AI")
	class USphereComponent* AttackRangeSphere;

	virtual void BeginPlay() override;

	virtual float TakeDamage(
		float Damage,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

	UFUNCTION()
	void OnSeePlayer(APawn* SeenPawn);

	// 충돌 시작/종료 시 호출될 함수들
	UFUNCTION()
	void OnAttackOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnAttackOverlapEnd(
		UPrimitiveComponent* OverlappedComp, 
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex	
	);

	// 실제 공격 루프
	void AttackLoop();

	


};
