#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBox.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth;
	UPROPERTY(VisibleAnywhere, Category = "Stats")
	float Health;

	UPROPERTY(EditAnywhere, Category = "DropItem")
	TSubclassOf<AActor> DropItem;

public:
	virtual void BeginPlay() override;
	virtual float TakeDamage(
		float DamageAmount, 
		struct FDamageEvent const& DamageEvent, 
		class AController* EventInstigator,
		AActor* DamageCauser) override;


	void DestroyBox();
	
	

};
