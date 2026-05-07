//SP_Character.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ProjectTeam16/Weapons/SP_WeaponType.h"
#include "SP_Character.generated.h"

UCLASS()
class ASP_Character : public ACharacter
{
	GENERATED_BODY()

public:
	ASP_Character();

	virtual float TakeDamage(
		float Damage,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser
	) override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SyncHUDValues();

	UFUNCTION(BlueprintCallable, Category = "Level")
	void AddExperience(int32 ExpAmount);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // 입력 바인딩

#pragma region Components
protected:

	// 컴포넌트 설정 1인칭 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FPSCamera;

	// 총기들이 붙을 부모 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USceneComponent* WeaponRoot;

#pragma endregion

#pragma region status
	// 플레이어 체력입니다. 체력이 바뀌면 HUD의 HealthProgressBar와 HpText를 갱신합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.0f;

	bool bIsDead = false;

	// 플레이어 성장 값입니다. 경험치 바는 CurrentExp / MaxExp 비율로 표시됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	float MaxExp = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Level")
	float CurrentExp = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Level")
	int32 CurrentLevel = 1;

#pragma endregion	

#pragma region WeaponSystem

	// 스폰할 무기 블루프린트 클래스
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<class ASP_WeaponBase> WeaponClass;

	// 팀원 시스템: 소지 무기 데이터 목록
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	TArray<FWeaponData> OwnedWeapons;

	// 스폰되어 관리되는 무기 액터들
	UPROPERTY()
	TArray<class ASP_WeaponBase*> EquippedWeapons;

	// 무기 등급별 데이터 테이블
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TMap<EWeaponType, FGunStats> GunDataTable;

	// 무기 추가 (팀원 로직 통합)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AddWeapon(EWeaponType WeaponType);

	// 무기 조합 (팀원 로직 통합)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void CombineWeapons(EWeaponType TypeA, EWeaponType TypeB);

	// 무기 비주얼/스탯 업데이트 (강화 시 호출)
	void UpdateWeaponVisuals(int32 Index);

	// 무기 위치 재정렬
	void RearrangeWeapons();

	// 자동 사격 로직
	void AutoFire();

	// 총기가 사격 방향으로 반동을 주는 연출용
	UPROPERTY(EditAnywhere, Category = "Visual")
	float RecoilIntensity = 5.0f;

	FTimerHandle FireTimerHandle;



#pragma endregion	


#pragma region InputAndMovement
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
	float CurrentStamina = 100.0f;

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


#pragma endregion
};
