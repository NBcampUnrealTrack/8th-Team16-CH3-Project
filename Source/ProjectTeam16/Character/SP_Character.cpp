#include "ProjectTeam16/Character/SP_Character.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h" 
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"


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

	// 총기 일자 배치 로직
	float SideOffset = 35.0f; // 총기 사이의 좌우 간격 변경가능(cm)

	// 총기 10개 배치 
	for (int32 i = 0; i < GunCount; i++)
	{
		FName MeshName = *FString::Printf(TEXT("GunMesh_%d"), i);
		UStaticMeshComponent* NewGun = CreateDefaultSubobject<UStaticMeshComponent>(MeshName);
		NewGun->SetupAttachment(WeaponRoot);

		// 일자 배치를 위한 Y축(좌우) 좌표 계산
		// (i - 중앙값) * 간격 을 하면 캐릭터 정면을 중심으로 정렬됩니다.
		float YPos = (i - (GunCount / 2.0f - 0.5f)) * SideOffset;

		// 모든 총이 정면을 바라보도록 Rotation은 0으로 고정
		NewGun->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));

		// 정면(X: 100), 좌우(Y: YPos), 높이(Z: -30)
		NewGun->SetRelativeLocation(FVector(50.f, YPos, 0.f));

		// 총기 매쉬 
		GunMeshes.Add(NewGun);

		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; //기본 속도조절
	}
}

void ASP_Character::BeginPlay()
{
	Super::BeginPlay();

	// 입력 매핑 컨텍스트 등록
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// 자동 사격 타이머 시작 (연사 속도마다 AutoFire 호출)
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ASP_Character::AutoFire, FireRate, true);
}


void ASP_Character::AutoFire()
{
	// 카메라가 바라보는 정면 방향과 위치
	//FVector CameraLocation = FPSCamera->GetComponentLocation();
	FVector ForwardVector = FPSCamera->GetForwardVector();

	// 모든 총구에서 발사
	for (UStaticMeshComponent* Gun : GunMeshes)
	{
		// 발사 시 총을 뒤로 살짝 밀기 (반동 시작)
		FVector CurrentLoc = Gun->GetRelativeLocation();
		Gun->SetRelativeLocation(CurrentLoc - FVector(RecoilIntensity, 0.f, 0.f));

		FVector MuzzleLocation = Gun->GetSocketLocation("Muzzle"); // 총구 소켓 위치
		// 사거리 설정 (5000유닛)
		FVector EndLocation = MuzzleLocation + (ForwardVector * 5000.0f);

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this); // 자기 자신은 무시

		// 정면 레이캐스트 (실제 총알 판정)
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, MuzzleLocation, EndLocation, ECC_Pawn, Params);

		if (bHit)
		{
			// 적에게 데미지 전달 로직
			AActor* HitActor = HitResult.GetActor();
			if (HitActor)
			{
				// 언리얼 데미지 시스템 호출
				UGameplayStatics::ApplyDamage(HitActor, 10.0f, GetController(), this, UDamageType::StaticClass());
			}

		}

		// 디버그 라인으로 총알 궤적 보여주기 (나중에 이펙트로 변경)
		DrawDebugLine(GetWorld(), MuzzleLocation, EndLocation, FColor::Yellow, false, 0.05f, 0, 1.0f);
	}
}




void ASP_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleStamina(DeltaTime); // 매 프레임 스테미너 계산

	// 마우스 회전에 따라 총기 묶음이 부드럽게 따라오도록 보정
	for (int32 i = 0; i < GunMeshes.Num(); i++)
	{
		if (UStaticMeshComponent* Gun = GunMeshes[i])
		{
			// 원래 있어야 할 위치 계산 (생성자에서 설정한 위치와 동일해야 함)
			float AngleStep = 12.0f;
			float Angle = (i - (GunCount / 2.0f)) * AngleStep;
			FVector TargetOrigin = FVector(100.f, Angle * 0.7f, -30.f);

			// 현재 위치에서 원래 위치로 부드럽게 보간 (VInterpTo)
			FVector CurrentLoc = Gun->GetRelativeLocation();
			FVector NewLoc = FMath::VInterpTo(CurrentLoc, TargetOrigin, DeltaTime, 10.0f);
			Gun->SetRelativeLocation(NewLoc);
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
	if (bIsSprinting)
	{
		// 달리는 중: 소모
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - (StaminaDrainRate * DeltaTime));

		// 스테미너가 다 떨어지면 강제로 달리기 중지
		if (CurrentStamina <= 0.0f)
		{
			SprintEnd();
		}
	}
	else
	{
		// 쉬는 중: 회복 (최대치까지)
		CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + (StaminaRegenRate * DeltaTime));
	}
}