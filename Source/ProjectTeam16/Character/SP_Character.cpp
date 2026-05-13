
//SP_Character.cpp

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

	// 1인칭 카메라와 무기 배치 기준점을 캐릭터에 붙입니다.
	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
	FPSCamera->SetupAttachment(GetRootComponent());
	FPSCamera->bUsePawnControlRotation = true;

	// 왼손 무기 배치 기준점 생성 및 카메라에 첨부
	LeftHandWeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHandWeaponRoot"));
	LeftHandWeaponRoot->SetupAttachment(FPSCamera);
	LeftHandWeaponRoot->SetRelativeLocation(FVector(45.0f, -25.0f, -25.0f)); // 카메라 기준 왼쪽 아래 전방

	// 오른손 무기 배치 기준점 생성 및 카메라에 첨부
	RightHandWeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RightHandWeaponRoot"));
	RightHandWeaponRoot->SetupAttachment(FPSCamera);
	RightHandWeaponRoot->SetRelativeLocation(FVector(45.0f, 25.0f, -25.0f)); // 카메라 기준 오른쪽 아래 전방

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// 데이터 테이블 에셋 자동 로드
	//static ConstructorHelpers::FObjectFinder<UDataTable> DataTableAsset(TEXT("/Game/Data/DT_WeaponStats.DT_WeaponStats"));
	//if (DataTableAsset.Succeeded())
	//{
	//	GunDataTable = DataTableAsset.Object;
	//}

	// 초기 포인터 안전하게 초기화
	LeftHandWeapon = nullptr;
	RightHandWeapon = nullptr;

}

void ASP_Character::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
	SyncHUDValues();

	// 플레이어 입력 매핑 컨텍스트를 등록합니다.
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// 게임 시작 시 기본 권총을 왼손 오른손에 샷건 각각 지급
	LeftWeaponData = FWeaponData(EWeaponType::Standard);
	RightWeaponData = FWeaponData(EWeaponType::BasicShotgun);

	SpawnOrUpdateHandWeapon(false); // 왼손 스폰
	SpawnOrUpdateHandWeapon(true);  // 오른손 스폰

	SyncHUDValues();
}

void ASP_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleStamina(DeltaTime);

	// 사격 반동으로 인해 뒤로 밀려난 액터 원래대로 돌리기 
	if (LeftHandWeapon && LeftHandWeapon->GetRootComponent())
	{
		FVector CurrentLoc = LeftHandWeapon->GetRootComponent()->GetRelativeLocation();
		LeftHandWeapon->SetActorRelativeLocation(FMath::VInterpTo(CurrentLoc, FVector::ZeroVector, DeltaTime, 15.0f));
	}

	if (RightHandWeapon && RightHandWeapon->GetRootComponent())
	{
		FVector CurrentLoc = RightHandWeapon->GetRootComponent()->GetRelativeLocation();
		RightHandWeapon->SetActorRelativeLocation(FMath::VInterpTo(CurrentLoc, FVector::ZeroVector, DeltaTime, 15.0f));
	}
}


float ASP_Character::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.0f)
	{
		return 0.0f;
	}

	// 피해를 받은 뒤 HUD를 갱신하고, 체력이 0이면 게임오버 UI를 띄웁니다.
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
/*
void ASP_Character::AddExperience(int32 ExpAmount)
{
	if (ExpAmount <= 0)
	{
		return;
	}

	CurrentExp += ExpAmount;
	UE_LOG(LogTemp, Log, TEXT("Player exp gained. CurrentExp=%f MaxExp=%f Level=%d"), CurrentExp, MaxExp, CurrentLevel);

	// 레벨업마다 필요 경험치를 1.2배로 늘리고, 레벨업 카드 UI를 한 번 띄웁니다.
	bool bLeveledUp = false;

	while (MaxExp > 0.0f && CurrentExp >= MaxExp)
	{
		CurrentExp -= MaxExp;
		CurrentLevel++;
		MaxExp = FMath::RoundToFloat(MaxExp * 1.2f);
		bLeveledUp = true;
	}

	SyncHUDValues();

	if (bLeveledUp)
	{
		if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(GetController()))
		{
			Team16PlayerController->ShowLevelUpUI();
		}
	}
}*/



void ASP_Character::UpgradeHandWeapon(EWeaponType NewType, bool bIsRightHand)
{

	FWeaponData& TargetData = bIsRightHand ? RightWeaponData : LeftWeaponData;

	//새로운 무기로 즉시 교체
	TargetData.WeaponType = NewType;
	TargetData.ActiveAbility = EWeaponSpecialAbility::None; // 임시 초기화 후 스폰 시 채움

	UE_LOG(LogTemp, Log, TEXT("%s Hand Weapon Upgraded to Type: %d"), bIsRightHand ? TEXT("Right") : TEXT("Left"), (int32)NewType);

	SpawnOrUpdateHandWeapon(bIsRightHand);
}

void ASP_Character::SpawnOrUpdateHandWeapon(bool bIsRightHand)
{
	USceneComponent* AttachRoot = bIsRightHand ? RightHandWeaponRoot : LeftHandWeaponRoot;
	ASP_WeaponBase** TargetWeaponPtr = bIsRightHand ? &RightHandWeapon : &LeftHandWeapon;
	FWeaponData& TargetData = bIsRightHand ? RightWeaponData : LeftWeaponData;

	if (TargetData.WeaponType == EWeaponType::None) return;

	if (*TargetWeaponPtr)
	{
		(*TargetWeaponPtr)->Destroy();
		*TargetWeaponPtr = nullptr;
	}

	if (WeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASP_WeaponBase* NewWeapon = GetWorld()->SpawnActor<ASP_WeaponBase>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (NewWeapon)
		{

			NewWeapon->MoveIgnoreActorAdd(this);
			this->MoveIgnoreActorAdd(NewWeapon);

			NewWeapon->AttachToComponent(AttachRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			*TargetWeaponPtr = NewWeapon;

			if (GunDataTable)
			{
				FString RowNameString = StaticEnum<EWeaponType>()->GetNameStringByValue((int64)TargetData.WeaponType);
				FGunStats* RowData = GunDataTable->FindRow<FGunStats>(FName(*RowNameString), TEXT("HandWeaponUpdate"));

				if (RowData)
				{
					FGunStats UpdatedStats = *RowData;

					// 무기 데이터에 테이블에 정의된 고유 특수 능력 할당
					TargetData.ActiveAbility = RowData->DefaultSpecialAbility;

					// [특수 능력 실시간 스탯 변조 연동]
					if (TargetData.ActiveAbility == EWeaponSpecialAbility::DoubleDamage)
					{
						UpdatedStats.Damage *= 2.0f; // 대미지 대폭 증폭
					}
					else if (TargetData.ActiveAbility == EWeaponSpecialAbility::RapidFire)
					{
						UpdatedStats.FireRate *= 0.5f; // 딜레이를 절반으로 줄여 연사 속도 증가
					}
					else if (TargetData.ActiveAbility == EWeaponSpecialAbility::LongRange)
					{
						UpdatedStats.Range *= 1.8f; // 사거리 대폭 증가
					}

					NewWeapon->SetWeaponVisuals(UpdatedStats.WeaponMesh);
					NewWeapon->SetWeaponStats(UpdatedStats);

					// 무기 자체 액터에도 능력 상태 공유
					NewWeapon->SetSpecialAbility(TargetData.ActiveAbility);
				}
			}
		}
	}
}

void ASP_Character::ApplyAbilityToHandWeapon(EWeaponSpecialAbility NewAbility, bool bIsRightHand)
{
	//손에 맞는 데이터 및 무기 액터 포인터 가져오기
	FWeaponData& TargetData = bIsRightHand ? RightWeaponData : LeftWeaponData;
	ASP_WeaponBase* TargetWeapon = bIsRightHand ? RightHandWeapon : LeftHandWeapon;

	if (!TargetWeapon) return;

	//캐릭터 데이터의 현재 능력 상태 갱신
	TargetData.ActiveAbility = NewAbility;

	//현재 들고 있는 무기 액터에도 능력 플래그 공유
	TargetWeapon->SetSpecialAbility(NewAbility);

	//특수 능력 획득에 따른 실시간 스탯 재계산 및 반영
	// 기존에 테이블에서 로드해둔 무기의 기본 스탯 복사
	if (GunDataTable)
	{
		FString RowNameString = StaticEnum<EWeaponType>()->GetNameStringByValue((int64)TargetData.WeaponType);
		FGunStats* RowData = GunDataTable->FindRow<FGunStats>(FName(*RowNameString), TEXT("HandWeaponAbilityUpdate"));

		if (RowData)
		{
			FGunStats UpdatedStats = *RowData;

			// 새로 획득한 아이템 능력에 따라 스탯 변조 연동
			if (NewAbility == EWeaponSpecialAbility::DoubleDamage)
			{
				UpdatedStats.Damage *= 2.0f;
			}
			else if (NewAbility == EWeaponSpecialAbility::RapidFire)
			{
				UpdatedStats.FireRate *= 0.5f;
			}
			else if (NewAbility == EWeaponSpecialAbility::LongRange)
			{
				UpdatedStats.Range *= 1.8f;
			}

			// 실시간 변조된 스탯을 현재 장착 중인 무기에 강제 주입
			TargetWeapon->SetWeaponStats(UpdatedStats);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%s 무기에 새로운 특수 능력(%d)이 주입되었습니다!"), bIsRightHand ? TEXT("오른손") : TEXT("왼손"), (int32)NewAbility);
}

void ASP_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASP_Character::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASP_Character::Look);

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ASP_Character::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASP_Character::SprintEnd);

		if (LeftFireAction)
			EnhancedInputComponent->BindAction(LeftFireAction, ETriggerEvent::Triggered, this, &ASP_Character::FireLeftHand);

		if (RightFireAction)
			EnhancedInputComponent->BindAction(RightFireAction, ETriggerEvent::Triggered, this, &ASP_Character::FireRightHand);

	}
}

void ASP_Character::FireLeftHand()
{
	if (bIsDead || !LeftHandWeapon) return;

	//왼손 무기 로컬 상대 위치
	USceneComponent* WeaponRootComp = LeftHandWeapon->GetRootComponent();
	if (WeaponRootComp)
	{
		FVector CurrentLoc = WeaponRootComp->GetRelativeLocation();
		// 로컬 X축(앞뒤) 방향으로만 반동을 주므로 안전합니다.
		WeaponRootComp->SetRelativeLocation(CurrentLoc - FVector(RecoilIntensity, 0.0f, 0.0f));
	}

	// 사격 명령 전달
	LeftHandWeapon->Fire(FPSCamera->GetForwardVector());
}

void ASP_Character::FireRightHand()
{
	if (bIsDead || !RightHandWeapon) return;

	//오른손 무기 동일하게 컴포넌트 로컬 상대 위치 제어.
	USceneComponent* WeaponRootComp = RightHandWeapon->GetRootComponent();
	if (WeaponRootComp)
	{
		FVector CurrentLoc = WeaponRootComp->GetRelativeLocation();
		WeaponRootComp->SetRelativeLocation(CurrentLoc - FVector(RecoilIntensity, 0.0f, 0.0f));
	}

	// 사격 명령 전달
	RightHandWeapon->Fire(FPSCamera->GetForwardVector());
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
	// 달리는 중에는 스태미나를 소모하고, 아니면 회복한 뒤 HUD에 반영합니다.
	bool bIsMoving = GetVelocity().SizeSquared() > 100.f;
	float PreviousStamina = CurrentStamina;

	if (bIsSprinting && bIsMoving)
	{
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - (StaminaDrainRate * DeltaTime));

		if (CurrentStamina <= 0.0f) SprintEnd();
	}
	else
	{
		CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (StaminaRegenRate * DeltaTime));

		//if (bIsSprinting && CurrentStamina <= 0.0f) SprintEnd();
	}

	//if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(GetController()))
	//{
	//	Team16PlayerController->UpdateHUDStamina(CurrentStamina, MaxStamina);
	//}

	//값이 변경되었을 때만 HUD를 갱신하여 낭비되는 CPU 자원 절약
	if (!FMath::IsNearlyEqual(PreviousStamina, CurrentStamina))
	{
		if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(GetController()))
		{
			Team16PlayerController->UpdateHUDStamina(CurrentStamina, MaxStamina);
		}
	}
}

