#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SP_Character.generated.h"

UCLASS()
class ASP_Character : public ACharacter
{
	GENERATED_BODY()

public:
	ASP_Character();

protected:
	virtual void BeginPlay() override;

	// 컴포넌트 설정 1인칭 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FPSCamera;

	// 총기들이 붙을 부모 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USceneComponent* WeaponRoot;

	// 총기 메쉬들을 담을 배열
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TArray<UStaticMeshComponent*> GunMeshes;

	// 총 개수 (블루프린트에서 수정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 GunCount = 10;

	// 사격 설정 (블루프린트에서 수정 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float FireRate = 0.15f;



	FTimerHandle FireTimerHandle;

	// 핵심 기능 함수
	void AutoFire();


public:
	virtual void Tick(float DeltaTime) override;

protected:
	// 총기가 사격 방향으로 반동을 주는 연출용
	UPROPERTY(EditAnywhere, Category = "Visual")
	float RecoilIntensity = 5.0f;

protected:
	// 맵핑
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	// 이동
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* MoveAction;

	// 이동시야
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* LookAction;

	// 입력 처리 함수
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

protected:
	// 입력 바인딩
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};

