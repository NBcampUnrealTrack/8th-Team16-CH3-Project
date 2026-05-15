#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Zombie.generated.h"

class UPawnSensingComponent;
class USphereComponent;
class USoundBase;
class UParticleSystem;
class UDataTable;

UCLASS()
class PROJECTTEAM16_API AZombie : public ACharacter
{
	GENERATED_BODY()

public:

	AZombie();

	float GetCurrentHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DT")
	FName StatRowName;

	float Health;
	float MaxHealth;
	float DamageAmount;
	float LastAttackTime;
	int32 ExpAmount;

	bool bIsAggroed = false;

	FTimerHandle AttackTimerHandle;
	FTimerHandle SeeTimerHandle;

	bool bIsDead = false;

	//탐지된 플레이어 정보저장
	APawn* TargetPlayer = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UPawnSensingComponent> PawnSensing;
	
	// 공격 범위를 감지할 충돌 구체
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<USphereComponent> AttackRangeSphere;

	// 타격 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<USoundBase> HitSound;

	// 타격 파티클 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UParticleSystem> HitParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DT")
	TObjectPtr<UDataTable> ZombieStatTable;

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
	//플레이어 감지 함수
	void CheckVisibility();

public:
	void SetEnrageMode(bool bIsEnraged, float SpeedMultiplier);
};
