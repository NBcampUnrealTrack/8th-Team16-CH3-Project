#include "ProjectTeam16/Enemy/Item.h"
#include "Components\SphereComponent.h"
#include "ProjectTeam16/Character/SP_Character.h"
#include "ProjectTeam16/Data/ProjectDataStructs.h"
#include "ProjectTeam16/UI/LevelUpWidget.h"
#include "Kismet/GameplayStatics.h"

AItem::AItem()
{
 	
	PrimaryActorTick.bCanEverTick = false;

    PickupRange = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRange"));
    SetRootComponent(PickupRange);

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(PickupRange);

    PickupRange->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlap);
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

    if (ItemDataTable && !ItemRowName.IsNone())
    {
       
        static const FString ContextString(TEXT("Item Data Context"));
        FBoxDropData* RowData = ItemDataTable->FindRow<FBoxDropData>(ItemRowName, ContextString);

        if (RowData)
        {
            Heal = RowData->HealingAmount;      
            MaxHP = RowData->AddMaxHPAmount;    
            ATK = RowData->AddAttackAmount;      
            Stamina = RowData->AddStaminaAmount;  
            Exp = RowData->ExpAmount;       
        }
    }
}

void AItem::OnOverlap(
    UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex,
    bool bFromSweep, 
    const FHitResult& SweepResult)
{
    ASP_Character* Player = Cast<ASP_Character>(OtherActor);

    if (Player)
    {
        const int32 MaxLevelLimit = 8;
        if (Heal > 0.0f)
        {
            Player->CurrentHealth = FMath::Min(Player->CurrentHealth + Heal, Player->MaxHealth);
            Player->SyncHUDValues();

            UE_LOG(LogTemp, Warning, TEXT("[ItemBox] Heal HP. Current: %f"), Player->CurrentHealth);
        }

        if (MaxHP > 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("[ItemBox] MaxHP Item Touched! Current Level: %d, Current MaxHealth: %f"), Player->MaxHealthLevel, Player->MaxHealth);
            if (Player->MaxHealthLevel < MaxLevelLimit)
            {
                Player->MaxHealthLevel++;
                Player->MaxHealth += MaxHP;
                Player->CurrentHealth += MaxHP;
                Player->SyncHUDValues();

                UE_LOG(LogTemp, Warning, TEXT("[ItemBox] MaxHP UP! Level: %d, MaxHP: %f"),
                    Player->MaxHealthLevel, Player->MaxHealth);
            }
        }

        if (ATK > 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("[ItemBox] ATK Item Touched! Current Level: %d, Current AttackPower: %f"), Player->AttackPowerLevel, Player->AttackPower);
            if (Player->AttackPowerLevel < MaxLevelLimit)
            {
                Player->AttackPowerLevel++;
                Player->AttackPower *= ATK;
                Player->SyncHUDValues();

                UE_LOG(LogTemp, Warning, TEXT("[ItemBox] Attack UP! Level: %d, ATK: %f"),
                    Player->AttackPowerLevel, Player->AttackPower);
            }
        }

        if (Stamina > 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("[ItemBox] Stamina Item Touched! Current Level: %d, Current MaxStamina: %f"), Player->MaxStaminaLevel, Player->MaxStamina);
            if (Player->MaxStaminaLevel < MaxLevelLimit)
            {
                Player->MaxStaminaLevel++;

                Player->MaxStamina += Stamina;
                Player->CurrentStamina += Stamina;

                Player->SyncHUDValues();

                UE_LOG(LogTemp, Warning, TEXT("[ItemBox] Stamina UP! Level: %d, MaxStamina: %f"),
                    Player->MaxStaminaLevel, Player->MaxStamina);
            }
        }

        if (Exp > 0)
        {
            Player->AddExperience(Exp);
            UE_LOG(LogTemp, Warning, TEXT("Get Exp : %d"), Exp);
        }
    }

    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
    }

    Destroy();
}
    




