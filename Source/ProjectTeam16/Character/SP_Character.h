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

#pragma region Gun
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

	// 총기가 사격 방향으로 반동을 주는 연출용
	UPROPERTY(EditAnywhere, Category = "Visual")
	float RecoilIntensity = 5.0f;

	FTimerHandle FireTimerHandle;

	// 핵심 기능 함수
	void AutoFire();
#pragma endregion	

public:
	virtual void Tick(float DeltaTime) override;

#pragma region Input
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

	// 점프
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* JumpAction;

	// 달리기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* SprintAction;

	// 걷는속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 600.0f;

	// 달리는 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 1000.0f;

	// 스테미너 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Stamina")   //UI에 표기 가능
	float CurrentStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaDrainRate = 20.0f; // 초당 소모량

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
	float StaminaRegenRate = 10.0f; // 초당 회복량

	bool bIsSprinting = false;


	// 함수 선언
	void Move(const FInputActionValue& Value); //이동
	void Look(const FInputActionValue& Value); //시야
	void SprintStart();                        //달리기 시작
	void SprintEnd();                          //달리기 끝
	void HandleStamina(float DeltaTime);       // 스테미너 처리 로직

protected:
	// 입력 바인딩
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
#pragma endregion	
