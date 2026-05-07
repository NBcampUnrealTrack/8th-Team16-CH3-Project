//SP_Character.cpp   //최종..

#include "ProjectTeam16/Character/SP_Character.h"
#include "ProjectTeam16/Weapons/SP_WeaponBase.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h" 
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Team16PlayerController.h"
#include "ProjectTeam16/Weapons/SP_WeaponType.h"

ASP_Character::ASP_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	// 카메라 설정
	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
	FPSCamera->SetupAttachment(GetRootComponent());
	FPSCamera->bUsePawnControlRotation = true;

	// 무기 위치 설정
	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
	WeaponRoot->SetupAttachment(FPSCamera);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; //기본 속도조절

	// 데이터 테이블 에셋 경로 자동 로드 예시
	static ConstructorHelpers::FObjectFinder<UDataTable> DataTableAsset(TEXT("/Game/Data/DT_WeaponStats.DT_WeaponStats"));
	if (DataTableAsset.Succeeded())
	{
		GunDataTable = DataTableAsset.Object;
	}

}

void ASP_Character::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
	SyncHUDValues();

	// 입력 매핑 컨텍스트 등록
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	//초기무기 지급
	AddWeapon(EWeaponType::Standard);

	//지급된 무기의 데이터 테이블 값을 찾아 타이머 시작
	if (GunDataTable && OwnedWeapons.Num() > 0)
	{
		EWeaponType InitType = OwnedWeapons[0].WeaponType;

		// Enum을 RowName으로 변환
		FString RowNameString = StaticEnum<EWeaponType>()->GetNameStringByValue((int64)InitType);
		FGunStats* RowData = GunDataTable->FindRow<FGunStats>(FName(*RowNameString), TEXT("InitTimer"));

		if (RowData)
		{
			GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ASP_Character::AutoFire, RowData->FireRate, true);
		}
	}
}


float ASP_Character::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.0f)
	{
		return 0.0f;
	}

	// 좀비에게 맞으면 체력을 줄이고 HUD의 체력 바와 HpText를 바로 갱신합니다.
	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);
	SyncHUDValues();

	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		bIsDead = true;

		if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(GetController()))
		{
			Team16PlayerController->ShowGameOver();
		}
	}

	return ActualDamage;
}

void ASP_Character::SyncHUDValues()
{
	if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(GetController()))
	{
		Team16PlayerController->UpdateHUDHealth(CurrentHealth, MaxHealth);
		Team16PlayerController->UpdateHUDStamina(CurrentStamina, MaxStamina);
		Team16PlayerController->UpdateHUDExperience(CurrentExp, MaxExp);
		Team16PlayerController->UpdateHUDLevel(CurrentLevel);
	}
}

void ASP_Character::AddExperience(int32 ExpAmount)
{
	if (ExpAmount <= 0)
	{
		return;
	}

	CurrentExp += ExpAmount;

	// 경험치가 가득 차면 레벨을 올리고 남은 경험치는 다음 레벨로 넘깁니다.
	while (MaxExp > 0.0f && CurrentExp >= MaxExp)
	{
		CurrentExp -= MaxExp;
		CurrentLevel++;
	}

	SyncHUDValues();
}


// 무기 추가 
// 무기 추가 로직 (팀원 로직 + 액터 스폰)
void ASP_Character::AddWeapon(EWeaponType WeaponType)
{
	// 1. 이미 소지 중인지 체크 (팀원 로직: 중복 시 강화레벨 상승)
	for (int32 i = 0; i < OwnedWeapons.Num(); i++)
	{
		if (OwnedWeapons[i].WeaponType == WeaponType)
		{
			if (OwnedWeapons[i].EnhanceLevel < 3)
			{
				OwnedWeapons[i].EnhanceLevel++;
				// 실제 액터 스탯 업데이트
				UpdateWeaponVisuals(i);
			}
			return;
		}
	}

	// 2. 새 무기 데이터 추가
	OwnedWeapons.Add(FWeaponData(WeaponType, 0));

	// 3. 실제 무기 액터 스폰 및 부착
	if (WeaponClass)
	{
		ASP_WeaponBase* NewWeapon = GetWorld()->SpawnActor<ASP_WeaponBase>(WeaponClass);
		if (NewWeapon)
		{
			NewWeapon->AttachToComponent(WeaponRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			EquippedWeapons.Add(NewWeapon);

			UpdateWeaponVisuals(EquippedWeapons.Num() - 1); // 스탯 설정
			RearrangeWeapons(); // 위치 재배치
		}
	}
}

// 무기 조합 로직 완성본
void ASP_Character::CombineWeapons(EWeaponType TypeA, EWeaponType TypeB)
{
	// 1. 실제 액터 및 데이터 제거
	// 뒤에서부터 지워야 인덱스 꼬임이 없습니다.
	for (int32 i = OwnedWeapons.Num() - 1; i >= 0; i--)
	{
		if (OwnedWeapons[i].WeaponType == TypeA || OwnedWeapons[i].WeaponType == TypeB)
		{
			if (EquippedWeapons.IsValidIndex(i))
			{
				EquippedWeapons[i]->Destroy();
				EquippedWeapons.RemoveAt(i);
			}
			OwnedWeapons.RemoveAt(i);
		}
	}

	// 2. 팀원의 레시피 적용
	EWeaponType ResultType = EWeaponType::None;

	if ((TypeA == EWeaponType::Standard && TypeB == EWeaponType::Old) ||
		(TypeA == EWeaponType::Old && TypeB == EWeaponType::Standard))
	{
		ResultType = EWeaponType::Improved;
	}
	else if ((TypeA == EWeaponType::Supply && TypeB == EWeaponType::Spare) ||
		(TypeA == EWeaponType::Spare && TypeB == EWeaponType::Supply))
	{
		ResultType = EWeaponType::Enhanced;
	}
	else if ((TypeA == EWeaponType::Enhanced && TypeB == EWeaponType::Improved) ||
		(TypeA == EWeaponType::Improved && TypeB == EWeaponType::Enhanced))
	{
		ResultType = EWeaponType::Special;
	}


	//ResultType이 None이 아님을 확실히 체크
	if (ResultType != EWeaponType::None)
	{
		AddWeapon(ResultType);

		if (GunDataTable)// 결과 생성 및 타이머 갱신
		{
			// Enum을 RowName으로 변환
			FString RowNameString = StaticEnum<EWeaponType>()->GetNameStringByValue((int64)ResultType);
			FName RowName = FName(*RowNameString);

			// 데이터 테이블에서 정보 찾기
			static const FString ContextString(TEXT("CombineWeaponTimer"));
			FGunStats* RowData = GunDataTable->FindRow<FGunStats>(RowName, ContextString);

			if (RowData)
			{
				float NewInterval = RowData->FireRate;

				// 기존 타이머를 초기화하고 새로운 연사 속도로 재설정
				GetWorldTimerManager().ClearTimer(FireTimerHandle);
				GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ASP_Character::AutoFire, NewInterval, true);
			}
			else
			{
				// ResultType은 None이 아닌데 테이블에 없는 경우 (데이터 누락)
				UE_LOG(LogTemp, Warning, TEXT("ResultType [%s] exists but row missing in DT!"), *RowNameString);
			}
		}
	}
	else
	{
		// 조합법이 없는 경우에 대한 로그 (선택 사항)
		UE_LOG(LogTemp, Log, TEXT("No valid combination for these weapons."));
	}
}

// 무기 스탯 및 비주얼 업데이트 보조 함수
void ASP_Character::UpdateWeaponVisuals(int32 Index)
{
	//인덱스 유효성 검사
	if (!EquippedWeapons.IsValidIndex(Index) || !OwnedWeapons.IsValidIndex(Index)) return;
	if (!EquippedWeapons[Index]) return;


	EWeaponType Type = OwnedWeapons[Index].WeaponType;

	//None 타입 처리: 데이터 테이블에 None 
	if (Type == EWeaponType::None) return;

	int32 Level = OwnedWeapons[Index].EnhanceLevel;

	//Enum을 데이터 테이블의 Row Name(행 이름)으로 변환
	FString RowNameString = StaticEnum<EWeaponType>()->GetNameStringByValue((int64)Type);
	FName RowName = FName(*RowNameString);

	//데이터 테이블에서 정보 찾기
	static const FString ContextString(TEXT("WeaponUpdateContext"));
	FGunStats* RowData = GunDataTable->FindRow<FGunStats>(RowName, ContextString);

	if (RowData)
	{
		//원본 데이터를 복사해서 가져옴 (원본 보존)
		FGunStats UpdatedStats = *RowData;

		//강화 수치 적용 (기본 데미지의 레벨당 20% 추가)
		UpdatedStats.Damage += (UpdatedStats.Damage * (Level * 0.2f));

		//무기 액터에 외형(Mesh) 변경 적용
		EquippedWeapons[Index]->SetWeaponVisuals(UpdatedStats.WeaponMesh);

		//최종 계산된 스탯(데미지, 사운드, 이펙트 포함) 전달
		EquippedWeapons[Index]->SetWeaponStats(UpdatedStats);

		// 강화 수치 로그 출력 테스트용
		UE_LOG(LogTemp, Log, TEXT("Weapon %s Updated: Level %d, Final Damage: %f"), *RowNameString, Level, UpdatedStats.Damage);
	}
	else
	{
		// 이 로그가 뜬다면 데이터 테이블의 Row Name과 Enum 이름이 맞는지 확인해야 합니다.
		UE_LOG(LogTemp, Warning, TEXT("Row [%s] not found in GunDataTable!"), *RowNameString);
	}


}


// 3. 무기 재배치 (간격 조정)
void ASP_Character::RearrangeWeapons()
{
	float SideOffset = 35.0f;
	int32 TotalGuns = EquippedWeapons.Num();

	for (int32 i = 0; i < TotalGuns; i++)
	{
		if (EquippedWeapons[i])
		{
			float YPos = (i - (TotalGuns / 2.0f - 0.5f)) * SideOffset;
			EquippedWeapons[i]->SetActorRelativeLocation(FVector(100.f, YPos, -30.f));
		}
	}
}

void ASP_Character::AutoFire()
{
	// 카메라가 바라보는 정면 방향과 위치
	//FVector CameraLocation = FPSCamera->GetComponentLocation();
	FVector ForwardVector = FPSCamera->GetForwardVector();

	for (ASP_WeaponBase* Weapon : EquippedWeapons)
	{
		if (Weapon)
		{
			// 반동 연출 (X축으로 살짝 밀기)
			FVector CurrentLoc = Weapon->GetRootComponent()->GetRelativeLocation();
			Weapon->SetActorRelativeLocation(CurrentLoc - FVector(RecoilIntensity, 0.f, 0.f));

			// 실제 사격 로직 실행 (WeaponBase 내부의 LineTrace 실행)
			Weapon->Fire(ForwardVector);
		}
	}


}

void ASP_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleStamina(DeltaTime); // 매 프레임 스테미너 계산

	// 반동 부드러운 복구 (VInterpTo)
	float SideOffset = 35.0f;
	int32 TotalGuns = EquippedWeapons.Num(); // 현재 장착된 총 개수

	for (int32 i = 0; i < TotalGuns; i++)
	{
		if (ASP_WeaponBase* Weapon = EquippedWeapons[i])
		{
			// 현재 소지한 총 개수에 맞춰 중앙 정렬 위치 계산
			float YPos = (i - (TotalGuns / 2.0f - 0.5f)) * SideOffset;
			FVector TargetOrigin = FVector(100.f, YPos, -30.f);

			// 현재 위치에서 원래 위치로 부드럽게 보간
			FVector CurrentLoc = Weapon->GetRootComponent()->GetRelativeLocation();
			Weapon->SetActorRelativeLocation(FMath::VInterpTo(CurrentLoc, TargetOrigin, DeltaTime, 15.0f));
		}
	}
}


void ASP_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 움직임과 시선 처리
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASP_Character::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASP_Character::Look);

		//점프
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// 달리기 
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ASP_Character::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASP_Character::SprintEnd);

		//총기 테스트용 추가 삭제예정
		EnhancedInputComponent->BindAction(TestAddWeaponAction, ETriggerEvent::Started, this, &ASP_Character::DebugAddRandomWeapon);
		EnhancedInputComponent->BindAction(TestCombineAction, ETriggerEvent::Started, this, &ASP_Character::DebugTryCombine);

	}
}

void ASP_Character::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ASP_Character::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASP_Character::SprintStart()
{
	// 스테미너가 있을 때만 달리기 시작
	if (CurrentStamina > 0.0f)
	{
		bIsSprinting = true;
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void ASP_Character::SprintEnd()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ASP_Character::HandleStamina(float DeltaTime)
{
	//달리기 상태일 때만 소모
	bool bIsMoving = GetVelocity().SizeSquared() > 100.f;
	if (bIsSprinting && bIsMoving)
	{
		// 달리는 중: 소모
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - (StaminaDrainRate * DeltaTime));

		// 스테미너가 다 떨어지면 강제로 달리기 중지
		if (CurrentStamina <= 0.0f) SprintEnd();
	}
	else
	{
		// 멈춰있거나 걷는 중이면 회복
		CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (StaminaRegenRate * DeltaTime));

		// 스테미너가 0일 때 속도가 안 올라가게
		if (bIsSprinting && CurrentStamina <= 0.0f) SprintEnd();
	}

	if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(GetController()))
	{
		Team16PlayerController->UpdateHUDStamina(CurrentStamina, MaxStamina);
	}
}

void ASP_Character::DebugAddRandomWeapon()
{
	// 하위 4개 무기 중 하나를 무작위로 추가 (테스트용)
	EWeaponType RandomType = static_cast<EWeaponType>(FMath::RandRange(1, 4));

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
		FString::Printf(TEXT("Test: Adding Weapon Type %d"), (int32)RandomType));

	AddWeapon(RandomType);
}

void ASP_Character::DebugTryCombine()
{
	// 현재 가지고 있는 무기들 중 조합 가능한 것이 있는지 체크 후 실행
	// 예: 첫 번째와 두 번째 무기를 강제로 조합 시도
	if (OwnedWeapons.Num() >= 2)
	{
		EWeaponType TypeA = OwnedWeapons[0].WeaponType;
		EWeaponType TypeB = OwnedWeapons[1].WeaponType;

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Test: Attempting Combine..."));
		CombineWeapons(TypeA, TypeB);
	}
}