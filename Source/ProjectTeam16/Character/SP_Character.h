//SP_Character.h 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ProjectTeam16/Weapons/SP_WeaponType.h"
#include "ProjectTeam16/Cube/OptionTypes.h"
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
	// 플레이어 체력입니다. 체력이 바뀌면 HUD의 HealthProgressBar와 HpText를 갱신합니다.
	// pragma region status -> public 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AttackPower = 1.0f;

	// bIsRightHand가 true면 오른손 무기, false면 왼손 무기를 NewType으로 업그레이드합니다.
	//UFUNCTION(BlueprintCallable, Category = "Weapon")
	//void UpgradeHandWeapon(EWeaponType NewType, bool bIsRightHand);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EnhanceHandWeapon(bool bIsRightHand); //추가 

	// 아이템이 호출할 무기 능력 실시간 갱신 함수
	void ApplyAbilityToHandWeapon(EWeaponSpecialAbility NewAbility, bool bIsRightHand);
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // 입력 바인딩

#pragma region Components
protected:

	// 컴포넌트 설정 1인칭 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FPSCamera;

	//양손 무기 고정 부착을 위한 개별 기준점
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USceneComponent* LeftHandWeaponRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USceneComponent* RightHandWeaponRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* Mesh1P;
#pragma endregion

#pragma region status


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

protected:
	// 스폰할 무기 블루프린트 클래스 원본
	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<class ASP_WeaponBase> WeaponClass;

	// 기존 TMap 대신 UDataTable 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	class UDataTable* GunDataTable;

	// 총기가 사격 방향으로 반동을 주는 연출용 수치
	UPROPERTY(EditAnywhere, Category = "Visual")
	float RecoilIntensity = 5.0f;

public:
	//양손 무기 액터 포인터
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	class ASP_WeaponBase* LeftHandWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	class ASP_WeaponBase* RightHandWeapon;

	//현재 왼손과 오른손에 들고 있는 무기의 상태 데이터
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	FWeaponData LeftWeaponData;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	FWeaponData RightWeaponData;

	// 캐릭터 블루프린트에서 사격 애니메이션 몽타주 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Weapon")
	class UAnimMontage* LeftHandFireMontage; // LeftArmSlot 지정 필수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation|Weapon")
	class UAnimMontage* RightHandFireMontage; // RightArmSlot 지정 필수

private:
	// 내부 무기 스폰 및 외형/스탯 데이터 테이블 동기화용 함수
	void SpawnOrUpdateHandWeapon(bool bIsRightHand);

	// 스폰된 무기 액터를 애니메이션(손 뼈 소켓)에 부착시키는 함수
	void AttachWeaponToHand(class ASP_WeaponBase* WeaponToAttach, bool bIsRightHand);

	const FName LeftHandSocketName = FName("Pistol_Socket");
	const FName RightHandSocketName = FName("Shotgun_Socket");
#pragma endregion	


#pragma region InputAndMovement
protected:
	// 맵핑
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	//마우스 좌/우 클릭 독립 입력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Weapon")
	class UInputAction* LeftFireAction; // 마우스 왼쪽 클릭용

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Weapon")
	class UInputAction* RightFireAction; // 마우스 오른쪽 클릭용

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void FireLeftHand();
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void FireRightHand();

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

	

	public:
		UPROPERTY(BlueprintReadOnly, Category = "CubeOption")
		float CubeAttackPowerBonus = 0.0f;    // ATKUP

		UPROPERTY(BlueprintReadOnly, Category = "CubeOption")
		float CubeFireRateBonus = 0.0f;       // ATSUP (%)

		UPROPERTY(BlueprintReadOnly, Category = "CubeOption")
		float CubeRangeBonus = 0.0f;          // RangeUP (%)

		UPROPERTY(BlueprintReadOnly, Category = "CubeOption")
		float CubeCritRate = 0.0f;            // CritRate (%)

		UPROPERTY(BlueprintReadOnly, Category = "CubeOption")
		float CubeCritDMG = 0.0f;             // CritDMG (%)

		UPROPERTY(BlueprintReadOnly, Category = "CubeOption")
		float CubeEXPRate = 0.0f;             // EXPRate (%)

		UPROPERTY(BlueprintReadOnly, Category = "CubeOption")
		float CubeBossDMG = 0.0f;             // BossDMG (%)

		UPROPERTY(BlueprintReadOnly, Category = "CubeOption")
		float CubeSplashDMG = 0.0f;           // SplashDMG (%)

		UPROPERTY(BlueprintReadOnly, Category = "CubeOption")
		float CubeX2Chance = 0.0f;            // x2chance (%)

		UPROPERTY(BlueprintReadOnly, Category = "CubeOption")
		float CubeMoreCube = 0.0f;            // MoreCube (%)

		UPROPERTY(BlueprintReadOnly, Category = "CubeOption")
		bool bCubePenetration = false;        // PenUP


		UPROPERTY(BlueprintReadOnly, Category = "Cube")
		int32 CubeCount = 999; // 현재 보유한 큐브 개수

		UFUNCTION(BlueprintCallable, Category = "Cube")
		void AddCube(int32 Amount); // 큐브 획득 함수

		// 큐브 옵션 적용 함수 (OptionWidget에서 호출)
		UFUNCTION(BlueprintCallable, Category = "CubeOption")
		void ApplyCubeOptions(const TArray<FOptionLine>& Options);

		// 큐브 옵션 초기화
		void ResetCubeOptions();

		UPROPERTY(BlueprintReadOnly, Category = "StatUpgrade")
		int32 MaxHealthLevel = 0;
		UPROPERTY(BlueprintReadOnly, Category = "StatUpgrade")
		int32 AttackPowerLevel = 0;
		UPROPERTY(BlueprintReadOnly, Category = "StatUpgrade")
		int32 MaxStaminaLevel = 0;

		const int32 MAX_UPGRADE_LEVEL = 8; // 최대 강화 수치 정의

		// 💡 특정 무기를 가지고 있고, 해당 무기의 강화 수치를 반환하는 헬퍼 함수
		int32 GetWeaponEnhanceLevel(EWeaponType WeaponType) const;
		bool HasWeapon(EWeaponType WeaponType) const;

		// 강화 및 조합 함수
		void EnhanceWeapon(EWeaponType WeaponType);
		void CombineWeapons(EWeaponType MainWeapon, EWeaponType SubWeapon);

		// 💡 조합 가능 여부 체크 함수 (레벨업 UI 풀 생성 시 사용)
		bool CanEvolvePistol() const;
		bool CanEvolveShotgun() const;

		// 스테미너 설정
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina")
		float MaxStamina = 100.0f;

		UPROPERTY(BlueprintReadOnly, Category = "Stamina")
		float CurrentStamina = 100.0f;
#pragma endregion
};