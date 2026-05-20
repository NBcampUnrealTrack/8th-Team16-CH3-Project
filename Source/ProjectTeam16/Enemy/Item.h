#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class USphereComponent;
class USoundBase;

UCLASS()
class PROJECTTEAM16_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AItem();

	float Heal;
	float MaxHP;
	float ATK;
	float Stamina;
	int32 Exp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DT")
	UDataTable* ItemDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DT")
	FName ItemRowName;
protected:
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Settings")
	TObjectPtr<USphereComponent> PickupRange; 
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	TObjectPtr<UStaticMeshComponent> StaticMesh;
	UPROPERTY(EditAnywhere, Category = "Settings")
	TObjectPtr<USoundBase> PickupSound;

	bool bIsFollowingPlayer = false;
	class AActor* TargetPlayer = nullptr;

	UPROPERTY(EditAnywhere, Category="Settings")
	float MoveSpeed = 5.0f;

	UFUNCTION()
	void OnOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex,
		bool bFromSweep, 
		const FHitResult& SweepResult);
};
