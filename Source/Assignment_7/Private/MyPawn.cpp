#include "MyPawn.h"
#include "MyPlayerController.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

AMyPawn::AMyPawn()
{
    PrimaryActorTick.bCanEverTick = false;

    CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
    CapsuleComp->InitCapsuleSize(40.f, 90.f);
    SetRootComponent(CapsuleComp);

    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
    MeshComp->SetupAttachment(CapsuleComp);
    MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComp->SetupAttachment(CapsuleComp);
    SpringArmComp->TargetArmLength = 300.0f;
    SpringArmComp->bUsePawnControlRotation = true;

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComp->SetupAttachment(SpringArmComp);

    MovementComp = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComp"));

    bUseControllerRotationYaw = false;

    NormalSpeed = 600.0f;
    SprintSpeed = 1020.0f;
    MovementComp->MaxSpeed = NormalSpeed;
}

void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EI = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetController()))
        {
            if (PC->MoveAction)
                EI->BindAction(PC->MoveAction, ETriggerEvent::Triggered, this, &AMyPawn::Move);
            if (PC->LookAction)
                EI->BindAction(PC->LookAction, ETriggerEvent::Triggered, this, &AMyPawn::Look);
            if (PC->SprintAction)
            {
                EI->BindAction(PC->SprintAction, ETriggerEvent::Triggered, this, &AMyPawn::StartSprint);
                EI->BindAction(PC->SprintAction, ETriggerEvent::Completed, this, &AMyPawn::StopSprint);
            }
        }
    }
}

void AMyPawn::Move(const FInputActionValue& Value)
{
    FVector2D Input = Value.Get<FVector2D>();
    if (Input.IsNearlyZero()) return;

    FRotator Rotation = GetControlRotation();
    FRotator Yaw(0, Rotation.Yaw, 0);
    FVector Forward = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X);
    FVector Right = FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y);

    FVector MoveDir = (Forward * Input.X + Right * Input.Y).GetSafeNormal();
    if (!MoveDir.IsNearlyZero())
    {
        SetActorRotation(FRotator(0.f, MoveDir.Rotation().Yaw, 0.f));
    }

    AddMovementInput(Forward, Input.X);
    AddMovementInput(Right, Input.Y);
}

void AMyPawn::Look(const FInputActionValue& Value)
{
    FVector2D LookInput = Value.Get<FVector2D>();
    AddControllerYawInput(LookInput.X);
    AddControllerPitchInput(LookInput.Y);
}

void AMyPawn::StartSprint(const FInputActionValue& Value)
{
    MovementComp->MaxSpeed = SprintSpeed;
}

void AMyPawn::StopSprint(const FInputActionValue& Value)
{
    MovementComp->MaxSpeed = NormalSpeed;
}