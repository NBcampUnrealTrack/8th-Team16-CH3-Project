#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBox.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class AItem;
class UDataTable;

UCLASS()
class PROJECTTEAM16_API AItemBox : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AItemBox();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComp;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> BoxComp;

	// 파티클 
	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraSystem* HitEffect;

	// 사운드
	UPROPERTY(EditAnywhere, Category = "Effects")
	class USoundBase* HitSound;

	FTimerHandle HitReactionTimer;

	// 원래 크기를 기억해둘 변수
	FVector OriginalScale;
	FRotator OriginalRotation;

	UPROPERTY(EditAnywhere, Category = "Effects")
	float HitScaleMultiplier = 1.1f; // 얼마나 커질지

	UPROPERTY(EditAnywhere, Category = "Effects")
	float ReactionDuration = 0.1f;  // 얼마나 빨리 돌아올지

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HP")
	float MaxHealth;
	UPROPERTY(VisibleAnywhere, Category = "HP")
	float Health;

	UPROPERTY(EditAnywhere, Category = "DropItem")
	TObjectPtr<UDataTable> DropTable;

public:
	virtual void BeginPlay() override;
	virtual float TakeDamage(
		float DamageAmount, 
		struct FDamageEvent const& DamageEvent, 
		class AController* EventInstigator,
		AActor* DamageCauser) override;

	void DestroyBox();

	// 타격 시 상자가 움찔거리는 효과
	void PlayHitReaction();
	void ResetScale();
	
};
