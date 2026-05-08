
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

	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
	WeaponRoot->SetupAttachment(FPSCamera);

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; 

	// 무기 스탯은 데이터 테이블에서 읽어와서 무기 생성/강화 때 적용합니다.
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

	// 플레이어 입력 매핑 컨텍스트를 등록합니다.
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// 기본 무기를 지급하고 첫 자동 발사 타이머를 시작합니다.
	AddWeapon(EWeaponType::Standard);

	if (GunDataTable && OwnedWeapons.Num() > 0)
	{
		EWeaponType InitType = OwnedWeapons[0].WeaponType;

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
}


void ASP_Character::AddWeapon(EWeaponType WeaponType)
{
	// 이미 가진 무기라면 새로 만들지 않고 강화 단계만 올립니다.
	for (int32 i = 0; i < OwnedWeapons.Num(); i++)
	{
		if (OwnedWeapons[i].WeaponType == WeaponType)
		{
			if (OwnedWeapons[i].EnhanceLevel < 3)
			{
				OwnedWeapons[i].EnhanceLevel++;
				UpdateWeaponVisuals(i);
			}
			return;
		}
	}

	OwnedWeapons.Add(FWeaponData(WeaponType, 0));

	// 새 무기는 캐릭터를 Owner/Instigator로 지정해서 처치 보상과 공격력 보정이 연결되게 합니다.
	if (WeaponClass)
	{
		ASP_WeaponBase* NewWeapon = GetWorld()->SpawnActor<ASP_WeaponBase>(WeaponClass);
		if (NewWeapon)
		{
			NewWeapon->SetOwner(this);
			NewWeapon->SetInstigator(this);

			NewWeapon->AttachToComponent(WeaponRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			EquippedWeapons.Add(NewWeapon);

			UpdateWeaponVisuals(EquippedWeapons.Num() - 1);
			RearrangeWeapons();
		}
	}
}

void ASP_Character::CombineWeapons(EWeaponType TypeA, EWeaponType TypeB)
{
	// 조합에 사용한 무기는 장착 목록과 소지 목록에서 제거합니다.
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

	// 조합 결과 무기를 지급하고, 새 무기 기준으로 자동 발사 주기를 갱신합니다.
	if (ResultType != EWeaponType::None)
	{
		AddWeapon(ResultType);

		if (GunDataTable)
		{
			FString RowNameString = StaticEnum<EWeaponType>()->GetNameStringByValue((int64)ResultType);
			FName RowName = FName(*RowNameString);

			static const FString ContextString(TEXT("CombineWeaponTimer"));
			FGunStats* RowData = GunDataTable->FindRow<FGunStats>(RowName, ContextString);

			if (RowData)
			{
				float NewInterval = RowData->FireRate;

				GetWorldTimerManager().ClearTimer(FireTimerHandle);
				GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ASP_Character::AutoFire, NewInterval, true);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ResultType [%s] exists but row missing in DT!"), *RowNameString);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("No valid combination for these weapons."));
	}
}

void ASP_Character::UpdateWeaponVisuals(int32 Index)
{
	if (!EquippedWeapons.IsValidIndex(Index) || !OwnedWeapons.IsValidIndex(Index)) return;
	if (!EquippedWeapons[Index]) return;

	// 데이터 테이블 스탯에 강화 단계 보너스를 더해 실제 장착 무기에 적용합니다.
	EWeaponType Type = OwnedWeapons[Index].WeaponType;

	if (Type == EWeaponType::None) return;

	int32 Level = OwnedWeapons[Index].EnhanceLevel;

	FString RowNameString = StaticEnum<EWeaponType>()->GetNameStringByValue((int64)Type);
	FName RowName = FName(*RowNameString);

	static const FString ContextString(TEXT("WeaponUpdateContext"));
	FGunStats* RowData = GunDataTable->FindRow<FGunStats>(RowName, ContextString);

	if (RowData)
	{
		FGunStats UpdatedStats = *RowData;

		UpdatedStats.Damage += (UpdatedStats.Damage * (Level * 0.2f));

		EquippedWeapons[Index]->SetWeaponVisuals(UpdatedStats.WeaponMesh);

		EquippedWeapons[Index]->SetWeaponStats(UpdatedStats);

		UE_LOG(LogTemp, Log, TEXT("Weapon %s Updated: Level %d, Final Damage: %f"), *RowNameString, Level, UpdatedStats.Damage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Row [%s] not found in GunDataTable!"), *RowNameString);
	}


}


void ASP_Character::RearrangeWeapons()
{
	// 장착한 무기들을 카메라 앞쪽에 가운데 정렬로 배치합니다.
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
	// 현재 장착 중인 모든 무기를 카메라 전방으로 자동 발사합니다.
	FVector ForwardVector = FPSCamera->GetForwardVector();

	for (ASP_WeaponBase* Weapon : EquippedWeapons)
	{
		if (Weapon)
		{
			FVector CurrentLoc = Weapon->GetRootComponent()->GetRelativeLocation();
			Weapon->SetActorRelativeLocation(CurrentLoc - FVector(RecoilIntensity, 0.f, 0.f));

			Weapon->Fire(ForwardVector);
		}
	}


}

void ASP_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleStamina(DeltaTime);

	// 반동으로 밀린 무기를 원래 위치로 부드럽게 되돌립니다.
	float SideOffset = 35.0f;
	int32 TotalGuns = EquippedWeapons.Num();

	for (int32 i = 0; i < TotalGuns; i++)
	{
		if (ASP_WeaponBase* Weapon = EquippedWeapons[i])
		{
			float YPos = (i - (TotalGuns / 2.0f - 0.5f)) * SideOffset;
			FVector TargetOrigin = FVector(100.f, YPos, -30.f);

			FVector CurrentLoc = Weapon->GetRootComponent()->GetRelativeLocation();
			Weapon->SetActorRelativeLocation(FMath::VInterpTo(CurrentLoc, TargetOrigin, DeltaTime, 15.0f));
		}
	}
}


void ASP_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASP_Character::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASP_Character::Look);

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ASP_Character::SprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASP_Character::SprintEnd);

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
}

void ASP_Character::DebugAddRandomWeapon()
{
	EWeaponType RandomType = static_cast<EWeaponType>(FMath::RandRange(1, 4));

	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
		FString::Printf(TEXT("Test: Adding Weapon Type %d"), (int32)RandomType));

	AddWeapon(RandomType);
}

void ASP_Character::DebugTryCombine()
{
	if (OwnedWeapons.Num() >= 2)
	{
		EWeaponType TypeA = OwnedWeapons[0].WeaponType;
		EWeaponType TypeB = OwnedWeapons[1].WeaponType;

		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Test: Attempting Combine..."));
		CombineWeapons(TypeA, TypeB);
	}
}

void ASP_Character::EnhanceWeapon(EWeaponType WeaponType)
{
	for (int32 i = 0; i < OwnedWeapons.Num(); i++)
	{
		if (OwnedWeapons[i].WeaponType == WeaponType)
		{
			if (OwnedWeapons[i].EnhanceLevel < 3)
			{
				OwnedWeapons[i].EnhanceLevel++;
				UpdateWeaponVisuals(i);

				UE_LOG(LogTemp, Warning, TEXT("%d weapon enhanced! Level: %d"),
					(int32)WeaponType, OwnedWeapons[i].EnhanceLevel);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("%d weapon already max enhanced."),
					(int32)WeaponType);
			}
			return;
		}
	}
}

