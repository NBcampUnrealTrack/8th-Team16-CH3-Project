
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
#include "ProjectTeam16/Cube/OptionWidget.h"

ASP_Character::ASP_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	//마우스 입력 축 권한 고정
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = true;

	// 1인칭 카메라와 무기 배치 기준점을 캐릭터에 붙입니다.
	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
	FPSCamera->SetupAttachment(GetRootComponent());
	FPSCamera->bUsePawnControlRotation = true;

	// [질문자님 최적화 반영] 불필요한 3인칭 기본 메쉬 숨김 및 콜리전 해제
	if (GetMesh())
	{
		GetMesh()->SetHiddenInGame(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

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


	CubeCritRate = 5.0f;  // 기본 크리티컬 확률 10%
	CubeCritDMG = 1.5f;   // 추가 크리티컬 데미지 50% 
}

void ASP_Character::BeginPlay()
{
	Super::BeginPlay();

	TArray<USkeletalMeshComponent*> SkeletalMeshes;
	GetComponents<USkeletalMeshComponent>(SkeletalMeshes);

	for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
	{
		if (MeshComp)
		{
			FString CompName = MeshComp->GetName();
			if (CompName.Contains(TEXT("CharacterMesh1")) || CompName.Contains(TEXT("Mesh1P")))
			{
				Mesh1P = MeshComp;
				UE_LOG(LogTemp, Warning, TEXT("1인칭 팔 메시이름: %s"), *CompName);
				break;
			}
		}
	}

	if (!Mesh1P && SkeletalMeshes.Num() > 1)
	{
		for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
		{
			if (MeshComp && MeshComp != GetMesh())
			{
				Mesh1P = MeshComp;
				UE_LOG(LogTemp, Warning, TEXT("1인칭 팔 강제 지정 이름: %s"), *MeshComp->GetName());
				break;
			}
		}
	}

	if (Mesh1P)
	{
		Mesh1P->SetHiddenInGame(false);
		Mesh1P->SetVisibility(true);
		Mesh1P->SetOnlyOwnerSee(true);
		Mesh1P->CastShadow = false;
	}
	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("블루프린트에서 1인칭 컴포넌트를 찾지 못했습니다"));
	}

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
	LeftWeaponData.WeaponType = EWeaponType::Pistol;
	LeftWeaponData.EnhanceLevel = 1;
RightWeaponData.WeaponType = EWeaponType::Shotgun;
    RightWeaponData.EnhanceLevel = 1;

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

void ASP_Character::AddExperience(int32 ExpAmount)
{
	if (ExpAmount <= 0)
	{
		return;
	}

	// [수정] 큐브 옵션 보너스 적용 (EXPRate가 20이면 1.2배)
	float BonusMultiplier = 1.0f + (CubeEXPRate / 100.0f);
	float FinalExpAmount = (float)ExpAmount * BonusMultiplier;

	// 최종 경험치를 더해줍니다.
	CurrentExp += FinalExpAmount;

	UE_LOG(LogTemp, Log, TEXT("Exp Gained: %.1f (Base: %d, Bonus: %.0f%%)"),
		FinalExpAmount, ExpAmount, CubeEXPRate);

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

	// 레벨업 UI 실행 (주석 해제 및 함수명 확인)
	if (bLeveledUp)
	{
		if (ATeam16PlayerController* PC = Cast<ATeam16PlayerController>(GetController()))
		{	
			// 헤더파일 확인 결과 OpenLevelUpUI가 정의되어 있습니다.
			PC->OpenLevelUpUI();
		}
	}
}

void ASP_Character::EnhanceHandWeapon(bool bIsRightHand)
{
	// [헤더 매칭용 통합 추가] 인게임 강화 트리거 연결 베이스 구현
	FWeaponData& TargetData = bIsRightHand ? RightWeaponData : LeftWeaponData;
	EnhanceWeapon(TargetData.WeaponType);
}

/*
void ASP_Character::UpgradeHandWeapon(EWeaponType NewType, bool bIsRightHand)
{

	FWeaponData& TargetData = bIsRightHand ? RightWeaponData : LeftWeaponData;

	//새로운 무기로 즉시 교체
	TargetData.WeaponType = NewType;
	TargetData.ActiveAbility = EWeaponSpecialAbility::None; // 임시 초기화 후 스폰 시 채움

	UE_LOG(LogTemp, Log, TEXT("%s Hand Weapon Upgraded to Type: %d"), bIsRightHand ? TEXT("Right") : TEXT("Left"), (int32)NewType);

	SpawnOrUpdateHandWeapon(bIsRightHand);
}
*/

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

			AttachWeaponToHand(NewWeapon, bIsRightHand);
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


void ASP_Character::AttachWeaponToHand(ASP_WeaponBase* WeaponToAttach, bool bIsRightHand)
{
	// [질문자님 자산 완벽 복구] 1인칭 팔 뼈 소켓에 결합 및 소유자 전용 시야 제어 로직
	if (!WeaponToAttach) return;

	if (!Mesh1P)
	{
		TArray<USkeletalMeshComponent*> SkeletalMeshes;
		GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
		for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
		{
			if (MeshComp && MeshComp != GetMesh())
			{
				Mesh1P = MeshComp;
				break;
			}
		}
	}

	if (!Mesh1P)
	{
		UE_LOG(LogTemp, Error, TEXT("1인칭 팔 메쉬 획득 실패"));
		return;
	}

	FName TargetSocket = bIsRightHand ? RightHandSocketName : LeftHandSocketName;
	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);

	WeaponToAttach->AttachToComponent(Mesh1P, AttachRules, TargetSocket);

	if (UPrimitiveComponent* WeaponRoot = Cast<UPrimitiveComponent>(WeaponToAttach->GetRootComponent()))
	{
		WeaponRoot->SetOnlyOwnerSee(true);
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
			EnhancedInputComponent->BindAction(LeftFireAction, ETriggerEvent::Started, this, &ASP_Character::FireLeftHand);

		if (RightFireAction)
			EnhancedInputComponent->BindAction(RightFireAction, ETriggerEvent::Started, this, &ASP_Character::FireRightHand);

	}
}

void ASP_Character::FireLeftHand()
{
	if (bIsDead || !LeftHandWeapon) return;

	// 사격 명령 전달
	LeftHandWeapon->Fire(FPSCamera->GetForwardVector());

	// 큐브 공속 스탯 연동 사격 애니메이션 몽타주 배속 구동 로직
	if (UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance())
	{
		if (LeftHandFireMontage && !AnimInstance->Montage_IsPlaying(LeftHandFireMontage))
		{
			float AnimPlayRate = 1.0f + (CubeFireRateBonus / 100.0f);
			AnimInstance->Montage_Play(LeftHandFireMontage, AnimPlayRate);
		}
	}

	//고정형 상대위치 반동 주입
	USceneComponent* WeaponRootComp = LeftHandWeapon->GetRootComponent();
	if (WeaponRootComp)
	{
		WeaponRootComp->SetRelativeLocation(FVector(-RecoilIntensity, 0.0f, 0.0f));
	}

}

void ASP_Character::FireRightHand()
{
	if (bIsDead || !RightHandWeapon) return;

	// 사격 명령 전달
	RightHandWeapon->Fire(FPSCamera->GetForwardVector());

	
	// 오른손 무기 사격 애니메이션 몽타주 배속 구동 로직
	if (UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance())
	{
		if (RightHandFireMontage && !AnimInstance->Montage_IsPlaying(RightHandFireMontage))
		{
			float AnimPlayRate = 1.0f + (CubeFireRateBonus / 100.0f);
			AnimInstance->Montage_Play(RightHandFireMontage, AnimPlayRate);
		}
	}

	//오른손 무기 고정형 로컬 상대위치 제어 반동 주입
	USceneComponent* WeaponRootComp = RightHandWeapon->GetRootComponent();
	if (WeaponRootComp)
	{
		WeaponRootComp->SetRelativeLocation(FVector(-RecoilIntensity, 0.0f, 0.0f));
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

		if (bIsSprinting && CurrentStamina <= 0.0f) SprintEnd();
	}

	if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(GetController()))
	{
		Team16PlayerController->UpdateHUDStamina(CurrentStamina, MaxStamina);
	}

	//값이 변경되었을 때만 HUD를 갱신하여 낭비되는 CPU 자원 절약
	if (!FMath::IsNearlyEqual(PreviousStamina, CurrentStamina))
	{
		if (ATeam16PlayerController* Team16PlayerController = Cast<ATeam16PlayerController>(GetController()))
		{
			Team16PlayerController->UpdateHUDStamina(CurrentStamina, MaxStamina);
		}
	}
}

void ASP_Character::ResetCubeOptions()
{
	CubeAttackPowerBonus = 0.0f;
	CubeFireRateBonus = 0.0f;
	CubeRangeBonus = 0.0f;
	CubeCritRate = 0.0f;
	CubeCritDMG = 0.0f;
	CubeEXPRate = 0.0f;
	CubeBossDMG = 0.0f;
	CubeSplashDMG = 0.0f;
	CubeX2Chance = 0.0f;
	CubeMoreCube = 0.0f;
	bCubePenetration = false;
}

void ASP_Character::ApplyCubeOptions(const TArray<FOptionLine>& Options)
{
	// 1. 이전 옵션 초기화 (이 코드가 있어서 누적되지 않습니다)
	ResetCubeOptions();

	// 2. 새 옵션 적용
	for (const FOptionLine& Option : Options)
	{
		switch (Option.OptionType)
		{
		case EOptionType::ATKUP:     CubeAttackPowerBonus += Option.Value; break;
		case EOptionType::ATSUP:     CubeFireRateBonus += Option.Value; break;
		case EOptionType::RangeUP:   CubeRangeBonus += Option.Value; break;
		case EOptionType::PenUP:     bCubePenetration = true; break;
		case EOptionType::CritRate:  CubeCritRate += Option.Value; break;
		case EOptionType::CritDMG:   CubeCritDMG += Option.Value; break;
		case EOptionType::EXPRate:   CubeEXPRate += Option.Value; break;
		case EOptionType::BossDMG:   CubeBossDMG += Option.Value; break;
		case EOptionType::SplashDMG: CubeSplashDMG += Option.Value; break;
		case EOptionType::x2chance:  CubeX2Chance += Option.Value; break;
		case EOptionType::MoreCube:  CubeMoreCube += Option.Value; break;
		}
	}

	// 3. 캐릭터 기본 공격력 실시간 반영
	AttackPower = 1.0f + (CubeAttackPowerBonus / 100.0f);

	// 4. 무기 스탯 재적용 (FireRate, Range 반영)
	auto ApplyWeaponBonus = [&](ASP_WeaponBase* Weapon, FWeaponData& WeaponData)
		{
			if (!Weapon || !GunDataTable) return;

			FString RowName = StaticEnum<EWeaponType>()->GetNameStringByValue((int64)WeaponData.WeaponType);
			FGunStats* RowData = GunDataTable->FindRow<FGunStats>(FName(*RowName), TEXT("CubeOption"));

			if (!RowData) return;

			FGunStats UpdatedStats = *RowData;

			// 기존 무기 고유 특수능력 효과 유지
			if (WeaponData.ActiveAbility == EWeaponSpecialAbility::DoubleDamage)
				UpdatedStats.Damage *= 2.0f;
			else if (WeaponData.ActiveAbility == EWeaponSpecialAbility::RapidFire)
				UpdatedStats.FireRate *= 0.5f;
			else if (WeaponData.ActiveAbility == EWeaponSpecialAbility::LongRange)
				UpdatedStats.Range *= 1.8f;

			// [중요] 현재 큐브 옵션 보너스만 딱 적용
			UpdatedStats.FireRate *= (1.0f - CubeFireRateBonus / 100.0f);
			UpdatedStats.Range *= (1.0f + CubeRangeBonus / 100.0f);

			// 관통 능력 적용
			if (bCubePenetration)
				Weapon->SetSpecialAbility(EWeaponSpecialAbility::Penetration);

			Weapon->SetWeaponStats(UpdatedStats);
		};

	ApplyWeaponBonus(LeftHandWeapon, LeftWeaponData);
	ApplyWeaponBonus(RightHandWeapon, RightWeaponData);

	UE_LOG(LogTemp, Log, TEXT("Cube Options Updated: ATK Bonus = %.0f%%"), CubeAttackPowerBonus);
}

void ASP_Character::AddCube(int32 Amount)
{
	// 큐브 계산
	float BonusMultiplier = 1.0f + (CubeMoreCube / 100.0f);
	int32 FinalAmount = FMath::Max(1, FMath::RoundToInt(Amount * BonusMultiplier));

	// 큐브 추가
	CubeCount += FinalAmount;

	// [확인용 로그] 이 로그가 출력 로그 창에 뜨는지 꼭 확인하세요!
	UE_LOG(LogTemp, Error, TEXT("!!! CUBE ADDED !!! Amount: %d, Total: %d"), FinalAmount, CubeCount);

	// UI 갱신
	ATeam16PlayerController* PC = Cast<ATeam16PlayerController>(GetController());
	if (PC && PC->OptionWidgetInstance)
	{
		PC->OptionWidgetInstance->RefreshUI();
	}
}

int32 ASP_Character::GetWeaponEnhanceLevel(EWeaponType WeaponType) const
{
	if (LeftWeaponData.WeaponType == WeaponType) return LeftWeaponData.EnhanceLevel;
	if (RightWeaponData.WeaponType == WeaponType) return RightWeaponData.EnhanceLevel;
	return 0;
}

bool ASP_Character::HasWeapon(EWeaponType WeaponType) const
{
	return (LeftWeaponData.WeaponType == WeaponType || RightWeaponData.WeaponType == WeaponType);
}

void ASP_Character::EnhanceWeapon(EWeaponType WeaponType)
{
	auto EnhanceSlot = [this](FWeaponData& SlotData, EWeaponType TargetType) {
		if (SlotData.WeaponType == TargetType)
		{
			if (SlotData.EnhanceLevel < MAX_UPGRADE_LEVEL)
			{
				SlotData.EnhanceLevel++;
				UE_LOG(LogTemp, Log, TEXT("%d Weapon Enhanced to Level %d"), (int32)TargetType, SlotData.EnhanceLevel);
			}
		}
		};

	EnhanceSlot(LeftWeaponData, WeaponType);
	EnhanceSlot(RightWeaponData, WeaponType);

	// 무기 비주얼이나 스탯 갱신
	SpawnOrUpdateHandWeapon(LeftWeaponData.WeaponType == WeaponType ? false : true);
}

// 💡 [신규] 권총 진화 조건 체크: 기본 권총(Pistol) 8강 이고 최대 체력 레벨이 8인 경우
bool ASP_Character::CanEvolvePistol() const
{
	return HasWeapon(EWeaponType::Pistol)
		&& (GetWeaponEnhanceLevel(EWeaponType::Pistol) >= MAX_UPGRADE_LEVEL)
		&& (MaxHealthLevel >= MAX_UPGRADE_LEVEL);
}

// 💡 [신규] 샷건 진화 조건 체크: 기본 샷건(Shotgun) 8강 이고 공격력 증가가 8인 경우
bool ASP_Character::CanEvolveShotgun() const
{
	return HasWeapon(EWeaponType::Shotgun)
		&& (GetWeaponEnhanceLevel(EWeaponType::Shotgun) >= MAX_UPGRADE_LEVEL)
		&& (AttackPowerLevel >= MAX_UPGRADE_LEVEL);
}

void ASP_Character::CombineWeapons(EWeaponType MainWeapon, EWeaponType SubWeapon)
{
	// 권총 조합 (진화)
	if (MainWeapon == EWeaponType::Pistol && CanEvolvePistol())
	{
		FWeaponData& TargetSlot = (LeftWeaponData.WeaponType == EWeaponType::Pistol) ? LeftWeaponData : RightWeaponData;
		TargetSlot.WeaponType = EWeaponType::Requiem; // 진화형 권총으로 변경
		TargetSlot.EnhanceLevel = 0; // 진화 후 강화 단계 초기화
		SpawnOrUpdateHandWeapon(LeftWeaponData.WeaponType == EWeaponType::Requiem ? false : true);
		UE_LOG(LogTemp, Log, TEXT("Pistol Evolved into Requiem!"));
	}
	// 샷건 조합 (진화)
	else if (MainWeapon == EWeaponType::Shotgun && CanEvolveShotgun())
	{
		FWeaponData& TargetSlot = (LeftWeaponData.WeaponType == EWeaponType::Shotgun) ? LeftWeaponData : RightWeaponData;
		TargetSlot.WeaponType = EWeaponType::Blast; // 진화형 샷건으로 변경
		TargetSlot.EnhanceLevel = 0;
		SpawnOrUpdateHandWeapon(LeftWeaponData.WeaponType == EWeaponType::Blast ? false : true);
		UE_LOG(LogTemp, Log, TEXT("Shotgun Evolved into Blast!"));
	}
}