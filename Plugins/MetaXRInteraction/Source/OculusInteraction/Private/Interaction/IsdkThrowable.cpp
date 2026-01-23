/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * Licensed under the Oculus SDK License Agreement (the "License");
 * you may not use the Oculus SDK except in compliance with the License,
 * which is provided at the time of installation or download, or which
 * otherwise accompanies this software in either electronic or hard copy form.
 *
 * You may obtain a copy of the License at
 *
 * https://developer.oculus.com/licenses/oculussdk/
 *
 * Unless required by applicable law or agreed to in writing, the Oculus SDK
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Interaction/IsdkThrowable.h"
#include "Engine/World.h"
#include "Utilities/IsdkMathUtils.h"

UIsdkThrowable::UIsdkThrowable()
{
  PrimaryComponentTick.bCanEverTick = true;
}

void UIsdkThrowable::SetSamplePositions(TArray<TPair<FVector, float>>& Positions)
{
  LastNPositions = Positions;
}

void UIsdkThrowable::SetSampleRotations(TArray<TPair<FQuat, float>>& Rotations)
{
  LastNRotations = Rotations;
}

void UIsdkThrowable::BeginPlay()
{
  Super::BeginPlay();
  IsFirstFrame = true;
  LastNPositions.Init(TPair<FVector, float>(FVector::ZeroVector, 0.0f), Settings.SampleSize);
  LastNRotations.Init(TPair<FQuat, float>(FQuat::Identity, 0.0f), Settings.SampleSize);
  CurrentPositionIndex = 0;
  CurrentRotationIndex = 0;
  PreviousPositionIndex = 0;

  ResetKalmanFilter();

  KalmanParams.Q = KalmanProcessNoise;
  KalmanParams.R = KalmanObservationNoise;

  Settings.LastTime = GetWorld()->GetTimeSeconds();
}

// Called every frame
void UIsdkThrowable::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
  if (!IsValid(TrackedComponent) || !IsValid(GetWorld()))
  {
    return;
  }

  if (!CheckPositionConstraints())
  {
    return;
  }

  if (Settings.VelocityEstimationMethod == EIsdkVelocityEstimationMethod::VE_KalmanFilter)
  {
    ProcessKalmanFilter();
  }

  // Store the current position and timestamp
  LastNPositions[CurrentPositionIndex] =
      TPair<FVector, float>(TrackedComponent->GetComponentLocation(), GetWorld()->GetTimeSeconds());
  PreviousPositionIndex = CurrentPositionIndex;
  CurrentPositionIndex = (CurrentPositionIndex + 1) % Settings.SampleSize;
  LastNRotations[CurrentRotationIndex] =
      TPair<FQuat, float>(TrackedComponent->GetComponentQuat(), GetWorld()->GetTimeSeconds());
  CurrentRotationIndex = (CurrentRotationIndex + 1) % Settings.SampleSize;
}

void UIsdkThrowable::GetLastNPositions(int32 N, TArray<TPair<FVector, float>>& OutPositions)
{
  OutPositions.Empty();

  for (int32 i = 0; i < N; i++)
  {
    const int32 Index = (CurrentPositionIndex - i - 1 + Settings.SampleSize) % Settings.SampleSize;
    TPair<FVector, float> PositionTimestampPair = LastNPositions[Index];

    // Add initialized positions only
    if (PositionTimestampPair.Key != FVector::ZeroVector)
    {
      OutPositions.Insert(PositionTimestampPair, 0);
    }
  }
}

FVector UIsdkThrowable::GetVelocity()
{
  TArray<TPair<FVector, float>> Positions;
  GetLastNPositions(LastNPositions.Num(), Positions);

  int32 numPositions = Positions.Num();

  // Check if there are enough tracked positions
  if (numPositions <= 1)
  {
    return FVector::ZeroVector;
  }

  TArray<FVector> PositionValues;
  for (const TPair<FVector, float>& PositionTimestampPair : Positions)
  {
    PositionValues.Add(PositionTimestampPair.Key);
  }

  const FVector Mean = FIsdkMathUtils::GetMean(PositionValues);
  const FVector StandardDeviation = FIsdkMathUtils::GetStandardDeviation(PositionValues, Mean);

  // Filter out positions that are more than a certain number of standard deviations away from the
  // mean
  TArray<TPair<FVector, float>> FilteredPositions;
  FilteredPositions.Reserve(numPositions);

  for (const TPair<FVector, float>& PositionTimestampPair : Positions)
  {
    FVector Position = PositionTimestampPair.Key;
    FVector ZScore = (Position - Mean) / StandardDeviation;
    if (ZScore.GetMax() <= Settings.Z_Score_Threshold)
    {
      FilteredPositions.Add(PositionTimestampPair);
    }
  }

  LastNPositions.Init(TPair<FVector, float>(FVector::ZeroVector, 0.0f), Settings.SampleSize);

  switch (Settings.VelocityEstimationMethod)
  {
    case EIsdkVelocityEstimationMethod::VE_LeastSquares:
      return FIsdkMathUtils::LeastSquares(FilteredPositions)
          .GetClampedToSize(MinVelocity, MaxVelocity);
    case EIsdkVelocityEstimationMethod::VE_RANSAC:
      return FIsdkMathUtils::Ransac(FilteredPositions, Ransac_Iterations, Ransac_Score_Threshold);
    case EIsdkVelocityEstimationMethod::VE_KalmanFilter:
      return KalmanParams.V;
    default:
      return FVector::ZeroVector;
  }
}

FQuat UIsdkThrowable::GetAngularVelocity()
{
  FVector SumAngularVelocity = FVector::ZeroVector;
  for (int i = 0; i < LastNRotations.Num() - 1; i++)
  {
    const FQuat CurrentRotation = LastNRotations[i].Key;
    const FQuat NextRotation = LastNRotations[i + 1].Key;
    const FQuat RotationDifference = FQuat::Slerp(
        CurrentRotation,
        NextRotation,
        (LastNRotations[i + 1].Value - LastNRotations[i].Value) / Settings.SampleSize);
    FVector Axis;
    float Angle;
    RotationDifference.ToAxisAndAngle(Axis, Angle);
    Angle = FMath::Fmod(Angle, 2 * PI);
    const FVector AngularVelocity =
        Axis * Angle / (LastNRotations[i + 1].Value - LastNRotations[i].Value);
    SumAngularVelocity += AngularVelocity;
  }

  LastNRotations.Init(TPair<FQuat, float>(FQuat::Identity, 0.0f), Settings.SampleSize);

  const FVector AverageAngularVelocity = SumAngularVelocity / (LastNRotations.Num() - 1);
  FMath::Clamp(AverageAngularVelocity.X, -MaxAngularSpeed, MaxAngularSpeed);
  FMath::Clamp(AverageAngularVelocity.Y, -MaxAngularSpeed, MaxAngularSpeed);
  FMath::Clamp(AverageAngularVelocity.Z, -MaxAngularSpeed, MaxAngularSpeed);
  const FQuat AverageAngularVelocityQuat =
      FQuat::MakeFromEuler(AverageAngularVelocity * Settings.AngularVelocityScale);
  return AverageAngularVelocityQuat;
}

void UIsdkThrowable::ResetKalmanFilter()
{
  if (IsValid(TrackedComponent))
  {
    KalmanParams.X = TrackedComponent->GetComponentLocation();
  }
  else
  {
    KalmanParams.X = this->GetComponentLocation();
  }
  KalmanParams.V = FVector::ZeroVector;
  KalmanParams.P = KalmanErrorCovariance;
  KalmanParams.K = FVector::ZeroVector;
}

// WIP TODO: Implement Exponential Smoothing Filter for Position Data T=1082914832757572
void UIsdkThrowable::ProcessKalmanFilter()
{
  FVector Z = TrackedComponent->GetComponentLocation();
  double T = GetWorld()->GetTimeSeconds();
  double DT = T - Settings.LastTime;

  if (DT >= MAX_TIME_DELTA)
  {
    Settings.LastTime = T;
    return;
  }

  FIsdkMathUtils::UpdateKalmanFilter(KalmanParams, Z, DT);

  KalmanParams.V.X = FMath::Clamp(KalmanParams.V.X, MinVelocity, MaxVelocity);
  KalmanParams.V.Y = FMath::Clamp(KalmanParams.V.Y, MinVelocity, MaxVelocity);
  KalmanParams.V.Z = FMath::Clamp(KalmanParams.V.Z, MinVelocity, MaxVelocity);

  Settings.LastTime = GetWorld()->GetTimeSeconds();
}

bool UIsdkThrowable::CheckPositionConstraints()
{
  if (!IsFirstFrame)
  {
    if (LastNPositions[PreviousPositionIndex].Key != FVector::ZeroVector)
    {
      // Check for erratic movement and lack of movement
      if (auto Distance = FVector::Dist(
              LastNPositions[PreviousPositionIndex].Key, TrackedComponent->GetComponentLocation());
          Distance < MinPositionThreshold || Distance > MaxPositionThreshold)
      {
        ResetKalmanFilter();
        LastNPositions.Init(TPair<FVector, float>(FVector::ZeroVector, 0.0f), Settings.SampleSize);

        return false;
      }
    }
  }
  else
  {
    IsFirstFrame = false;
  }

  return true;
}
