// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "VirtualCameraMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Used to help get stabilization to be more finely tunable
#define STABILIZATION_NRM_EXP .25f

UVirtualCameraMovementComponent::UVirtualCameraMovementComponent(const FObjectInitializer& ObjectInitializer)
{
	// Initialize Axis Settings Map
	AxisSettings.Add(EVirtualCameraAxis::LocationX);
	AxisSettings.Add(EVirtualCameraAxis::LocationY);
	AxisSettings.Add(EVirtualCameraAxis::LocationZ);
	AxisSettings.Add(EVirtualCameraAxis::RotationX);
	AxisSettings.Add(EVirtualCameraAxis::RotationY);
	AxisSettings.Add(EVirtualCameraAxis::RotationZ);

	bLockRelativeToFirstLockAxis = true;
	bUseGlobalBoom = true;
	bZeroDutchOnLock = true;
	CachedLockingAxis = FQuat::Identity;
}

void UVirtualCameraMovementComponent::AddInputVector(FVector WorldVector, bool bForce /*=false*/)
{
	if (WorldVector.IsZero())
	{
		return;
	}

	ApplyLocationScaling(WorldVector);
	ApplyLocationLocks(WorldVector);

	TargetLocation += WorldVector;
}

void UVirtualCameraMovementComponent::ProcessMovementInput(const FVector& TrackerLocation, const FRotator& TrackerRotation)
{
	FVector DeltaMovement = TrackerLocation - PreviousTrackerLocation;
	FRotator RotationReorient = GetRotationOffset();
	RotationReorient.Pitch = 0.f;
	// make sure movement is correctly oriented to current direction camera is facing
	DeltaMovement = RotationReorient.UnrotateVector(DeltaMovement);
	AddInputVector(DeltaMovement);
	PreviousTrackerLocation = TrackerLocation;

	// Calculate the global change in rotation and apply it
	FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(TrackerRotation, PreviousTrackerRotation);
	ApplyRotationLocks(DeltaRotation);
	TargetRotation = TrackerRotation - GetRotationOffset() + GetOwner()->GetActorRotation();
	PreviousTrackerRotation = TrackerRotation;

	FHitResult OutResult = FHitResult();
	SafeMoveUpdatedComponent(GetStabilizedDeltaLocation(), GetStabilizedRotation(), false, OutResult);
}

bool UVirtualCameraMovementComponent::ToggleAxisLock(const EVirtualCameraAxis AxisToToggle)
{
	bool bIsLocked = AxisSettings[AxisToToggle].ToggleLock();

	if (AxisToToggle == EVirtualCameraAxis::RotationX && bZeroDutchOnLock && bIsLocked)
	{
		// This needs to be applied to the lock offset directly to avoid adding to freeze
		AxisSettings[EVirtualCameraAxis::RotationX].LockOffset = TargetRotation.Roll;
	}

	// Update cached locking axis if coming from completely unlocked state
	if (bLockRelativeToFirstLockAxis && !IsLocationLockingActive())
	{
		CachedLockingAxis = UpdatedComponent->GetComponentRotation().Quaternion();
	}

	return bIsLocked;
}

bool UVirtualCameraMovementComponent::ToggleAxisFreeze(const EVirtualCameraAxis AxisToToggle)
{
	// Leving this out of header file for the inevitable refactor that will cause this to be more that one line of code.
	return AxisSettings[AxisToToggle].bIsFrozen = !AxisSettings[AxisToToggle].bIsFrozen;
}

float UVirtualCameraMovementComponent::SetAxisStabilizationScale(const EVirtualCameraAxis AxisToAdjust, float NewStabilizationAmount)
{
	// Stabilization is applied as an exponential curve defined by STABILIZATION_NRM_EXP
	// This exponent is less than 1 so value changes at numbers closer to 0 have a greater degree of change than those closer to 1.
	// This has the effect of stabilization being introduced much faster when it is first applied, and giving a finer degree of control as values increase.
	// This is necessary because stabilization is not noticable or useful until a certain amount is applied.
	NewStabilizationAmount = FMath::Clamp<float>(NewStabilizationAmount, 0.0f, .97f);
	AxisSettings[AxisToAdjust].StabilizationScale = FMath::Pow(NewStabilizationAmount, STABILIZATION_NRM_EXP);
	AxisSettings[AxisToAdjust].StabilizationScale *= .97f;
	return AxisSettings[AxisToAdjust].StabilizationScale;
}

float UVirtualCameraMovementComponent::GetAxisStabilizationScale(const EVirtualCameraAxis AxisToRetrieve) const
{
	float ValueToReturn = AxisSettings[AxisToRetrieve].StabilizationScale / .97f;
	ValueToReturn = FMath::Pow(ValueToReturn, (1 / STABILIZATION_NRM_EXP));
	return ValueToReturn;
}

void UVirtualCameraMovementComponent::ResetCameraOffsetsToTracker()
{
	// Reorient pawn to be in "stage space", with stage origin acting as pawn root component
	TargetLocation = PreviousTrackerLocation;
	TargetRotation = PreviousTrackerRotation;
	UpdatedComponent->SetRelativeLocation(TargetLocation);
	UpdatedComponent->SetRelativeRotation(TargetRotation);

	// Clear all locks
	// ToDo: Do we need to clear freeze states here as well?
	for (auto Iterator = AxisSettings.CreateIterator(); Iterator; ++Iterator)
	{
		Iterator.Value().SetIsLocked(false);
		Iterator.Value().bIsFrozen = false;
		Iterator.Value().FreezeOffset = 0.f;
	}

	OnOffsetReset.Broadcast();
}

bool UVirtualCameraMovementComponent::IsLocationLockingActive() const 
{
	return AxisSettings[EVirtualCameraAxis::LocationX].bIsLocked || AxisSettings[EVirtualCameraAxis::LocationY].bIsLocked || AxisSettings[EVirtualCameraAxis::LocationZ].bIsLocked;
}

bool UVirtualCameraMovementComponent::IsAxisLocked(const EVirtualCameraAxis AxisToCheck) const
{
	return AxisSettings[AxisToCheck].bIsLocked;
}

void UVirtualCameraMovementComponent::OnMoveForward(const float InValue)
{
	FVector InputVector = UpdatedComponent->GetForwardVector();
	InputVector = GetOwner()->GetActorRotation().UnrotateVector(InputVector);
	InputVector *= InValue;

	if (InputVector.IsZero())
	{
		return;
	}

	InputVector *= AxisSettings[EVirtualCameraAxis::LocationX].MovementScale;
	ApplyLocationLocks(InputVector);

	TargetLocation += InputVector;
}

void UVirtualCameraMovementComponent::OnMoveRight(const float InValue)
{
	FVector InputVector = UpdatedComponent->GetRightVector();
	InputVector = GetOwner()->GetActorRotation().UnrotateVector(InputVector);
	
	InputVector *= InValue;

	if (InputVector.IsZero())
	{
		return;
	}

	// To preseve direction when moving diagonally use same scaling as forward movement
	InputVector *= AxisSettings[EVirtualCameraAxis::LocationY].MovementScale;
	ApplyLocationLocks(InputVector);
	
	TargetLocation += InputVector;
}

void UVirtualCameraMovementComponent::OnMoveUp(const float InValue)
{
	if (bUseGlobalBoom)
	{
		FVector InputVector = FVector(0.f, 0.f, 1.f);
		AddInputVector(InputVector * InValue);
	}
	else
	{
		FVector InputVector = UpdatedComponent->GetUpVector();
		InputVector = GetOwner()->GetActorRotation().UnrotateVector(InputVector);
		InputVector *= InValue;

		if (InputVector.IsZero())
		{
			return;
		}

		InputVector *= AxisSettings[EVirtualCameraAxis::LocationZ].MovementScale;
		ApplyLocationLocks(InputVector);

		TargetLocation += InputVector;
	}
}

void UVirtualCameraMovementComponent::Teleport(const FTransform& TargetTransform)
{
	ResetCameraOffsetsToTracker();

	// The change between where we were and where we are teleporting to.
	FVector DeltaOffset = TargetTransform.GetLocation() - UpdatedComponent->GetComponentLocation();
	DeltaOffset = GetOwner()->GetActorRotation().UnrotateVector(DeltaOffset);

	UpdatedComponent->AddLocalOffset(DeltaOffset);
	TargetLocation += DeltaOffset;
}

void UVirtualCameraMovementComponent::ApplyLocationLocks(const FVector& InVector)
{
	FVector ForwardVector;
	FVector RightVector;
	FVector UpVector;

	GetDirectionVectorsForCamera(ForwardVector, RightVector, UpVector);

	FVector AdjustedVector = InVector;

	if (AxisSettings[EVirtualCameraAxis::LocationX].IsAxisImmobilized())
	{
		AdjustedVector = FVector::VectorPlaneProject(AdjustedVector, ForwardVector);
	}

	if (AxisSettings[EVirtualCameraAxis::LocationY].IsAxisImmobilized())
	{
		AdjustedVector = FVector::VectorPlaneProject(AdjustedVector, RightVector);
	}

	if (AxisSettings[EVirtualCameraAxis::LocationZ].IsAxisImmobilized())
	{
		AdjustedVector = FVector::VectorPlaneProject(AdjustedVector, UpVector);
	}

	AdjustedVector = InVector - AdjustedVector;

	AxisSettings[EVirtualCameraAxis::LocationX].AddOffset(AdjustedVector.X);
	AxisSettings[EVirtualCameraAxis::LocationY].AddOffset(AdjustedVector.Y);
	AxisSettings[EVirtualCameraAxis::LocationZ].AddOffset(AdjustedVector.Z);
}

FVector UVirtualCameraMovementComponent::GetStabilizedDeltaLocation() const 
{
	FVector ReturnVector = FVector::ZeroVector;
	FVector ClosestPoint;

	FVector TargetLocationWithOffsets = TargetLocation - GetLocationOffset();

	// Calculate each component by getting the needed vector component and doing a lerp with the stabilization scale for that axis
	FMath::PointDistToLine(TargetLocationWithOffsets, UpdatedComponent->GetForwardVector(), UpdatedComponent->RelativeLocation, ClosestPoint);
	ReturnVector += (ClosestPoint - UpdatedComponent->RelativeLocation) * (1 - AxisSettings[EVirtualCameraAxis::LocationX].StabilizationScale);

	FMath::PointDistToLine(TargetLocationWithOffsets, UpdatedComponent->GetRightVector(), UpdatedComponent->RelativeLocation, ClosestPoint);
	ReturnVector += (ClosestPoint - UpdatedComponent->RelativeLocation) * (1 - AxisSettings[EVirtualCameraAxis::LocationY].StabilizationScale);

	FMath::PointDistToLine(TargetLocationWithOffsets, UpdatedComponent->GetUpVector(), UpdatedComponent->RelativeLocation, ClosestPoint);
	ReturnVector += (ClosestPoint - UpdatedComponent->RelativeLocation) * (1 - AxisSettings[EVirtualCameraAxis::LocationZ].StabilizationScale);

	ReturnVector = GetOwner()->GetActorRotation().RotateVector(ReturnVector);
	return ReturnVector;
}

void UVirtualCameraMovementComponent::ApplyLocationScaling(FVector& VectorToAdjust)
{
	// Get the axes to scale along
	FVector ForwardVector;
	FVector RightVector;
	FVector UpVector;

	GetDirectionVectorsForCamera(ForwardVector, RightVector, UpVector);

	// Orient to global Z up, but maintain yaw
	ForwardVector = FVector::VectorPlaneProject(ForwardVector, FVector::UpVector);
	RightVector = FVector::VectorPlaneProject(RightVector, FVector::UpVector);
	UpVector = FVector::UpVector;

	ForwardVector.Normalize();
	RightVector.Normalize();

	FVector XComponent = VectorToAdjust.ProjectOnTo(ForwardVector);
	FVector YComponent = VectorToAdjust.ProjectOnTo(RightVector);
	FVector ZComponent = VectorToAdjust.ProjectOnTo(UpVector);

	VectorToAdjust = XComponent * AxisSettings[EVirtualCameraAxis::LocationX].MovementScale;
	VectorToAdjust += YComponent * AxisSettings[EVirtualCameraAxis::LocationY].MovementScale;
	VectorToAdjust += ZComponent * AxisSettings[EVirtualCameraAxis::LocationZ].MovementScale;
}

void UVirtualCameraMovementComponent::ApplyRotationLocks(const FRotator& InRotation)
{
	// The AddOffset function will ignore incoming offsets if the axis is not immobilized
	AxisSettings[EVirtualCameraAxis::RotationX].AddOffset(InRotation.Roll);
	AxisSettings[EVirtualCameraAxis::RotationY].AddOffset(InRotation.Pitch);
	AxisSettings[EVirtualCameraAxis::RotationZ].AddOffset(InRotation.Yaw);
}

FRotator UVirtualCameraMovementComponent::GetStabilizedRotation() const
{
	// Needs to be done as Quaternions to avoid gimbal locking
	FRotator TargetAdjustment = UKismetMathLibrary::ComposeRotators(TargetRotation, UpdatedComponent->GetComponentRotation().GetInverse());

	TargetAdjustment.Roll *= 1 - AxisSettings[EVirtualCameraAxis::RotationX].StabilizationScale;
	TargetAdjustment.Pitch *= 1 - AxisSettings[EVirtualCameraAxis::RotationY].StabilizationScale;
	TargetAdjustment.Yaw *= 1 - AxisSettings[EVirtualCameraAxis::RotationZ].StabilizationScale;

	return UKismetMathLibrary::ComposeRotators(TargetAdjustment, UpdatedComponent->GetComponentRotation());
}

void UVirtualCameraMovementComponent::GetDirectionVectorsForCamera(FVector& OutForwardVector, FVector& OutRightVector, FVector& OutUpVector) const 
{
	// Break the vector into local components so we can apply individual scaling
	if (bLockRelativeToFirstLockAxis && IsLocationLockingActive())
	{
		// If needed, use the cached axis rather than always using local
		OutForwardVector = CachedLockingAxis.GetForwardVector();
		OutRightVector = CachedLockingAxis.GetRightVector();
		OutUpVector = CachedLockingAxis.GetUpVector();
	}
	else
	{
		OutForwardVector = UpdatedComponent->GetForwardVector();
		OutRightVector = UpdatedComponent->GetRightVector();
		OutUpVector = UpdatedComponent->GetUpVector();
	}

	OutForwardVector = GetOwner()->GetActorRotation().UnrotateVector(OutForwardVector);
	OutRightVector = GetOwner()->GetActorRotation().UnrotateVector(OutRightVector);
	OutUpVector = GetOwner()->GetActorRotation().UnrotateVector(OutUpVector);
}

FVector UVirtualCameraMovementComponent::GetLocationOffset() const 
{
	return FVector(
		AxisSettings[EVirtualCameraAxis::LocationX].GetOffset(),
		AxisSettings[EVirtualCameraAxis::LocationY].GetOffset(),
		AxisSettings[EVirtualCameraAxis::LocationZ].GetOffset()
	);
}

FRotator UVirtualCameraMovementComponent::GetRotationOffset() const 
{
	return FRotator(
		AxisSettings[EVirtualCameraAxis::RotationY].GetOffset(),
		AxisSettings[EVirtualCameraAxis::RotationZ].GetOffset(),
		AxisSettings[EVirtualCameraAxis::RotationX].GetOffset()
	);
}
