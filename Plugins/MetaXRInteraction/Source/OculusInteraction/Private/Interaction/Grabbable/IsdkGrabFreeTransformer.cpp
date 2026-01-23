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

#include "Interaction/Grabbable/IsdkGrabFreeTransformer.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

namespace isdk
{
extern TAutoConsoleVariable<bool> CVar_Meta_InteractionSDK_DebugInteractionVisuals;
}

UIsdkGrabFreeTransformer::UIsdkGrabFreeTransformer()
    : Config(true, FIsdkConstraintAxes{}, FIsdkConstraintAxes{}, true, FIsdkAxisConstraints{})
{
}

void UIsdkGrabFreeTransformer::UpdateGrabPointDeltas(FVector Centroid)
{
  for (int i = 0; i < SelectedPoses.Num(); i++)
  {
    FVector CentroidOffset = Centroid - SelectedPoses[i].Position();
    Deltas[i].UpdateData(CentroidOffset, SelectedPoses[i].Orientation());
  }
}

void UIsdkGrabFreeTransformer::BeginTransform(
    const TArray<FIsdkGrabPose>& GrabPoses,
    const FIsdkTargetTransform& Target)
{
  SelectedPoses.Empty();
  Deltas.Empty();

  for (const auto& Pose : GrabPoses)
  {
    SelectedPoses.Push(Pose);
  }

  FVector Centroid = GetCentroid(SelectedPoses);

  for (const auto& GrabPose : SelectedPoses)
  {
    FVector CentroidOffset = Centroid - GrabPose.Position();
    Deltas.Push(FIsdkGrabPointDelta(CentroidOffset, GrabPose.Orientation()));
  }

  auto CentroidToTargetVector = Centroid - Target.WorldTransform.GetLocation();
  RelativeCentroidToTargetVector =
      Target.WorldTransform.InverseTransformVector(CentroidToTargetVector);
  InitialTargetWorldRotation = Target.WorldTransform.GetRotation();
  LastRotation = FQuat::Identity;
  LastScale = Target.RelativeTransform.GetScale3D();
}

FTransform UIsdkGrabFreeTransformer::UpdateTransform(
    const TArray<FIsdkGrabPose>& GrabPoses,
    const FIsdkTargetTransform& Target)
{
  if (SelectedPoses.IsEmpty())
  {
    return Target.RelativeTransform;
  }

  // Update state of the transformer
  UpdateSelectedPoses(GrabPoses);
  FVector Centroid = GetCentroid(SelectedPoses);
  UpdateGrabPointDeltas(Centroid);

  // Compute the result scale
  FVector ResultScale = Target.RelativeTransform.GetScale3D();
  LastScale = SelectedPoses.Num() <= 1 ? ResultScale : GetDeltaScale() * LastScale;
  ResultScale =
      FIsdkTransformerUtils::GetConstrainedTransformScale(LastScale, TargetScaleConstraint);

  // Compute the result orientation
  LastRotation = GetDeltaRotation() * LastRotation;
  FQuat WorldRotation = LastRotation * InitialTargetWorldRotation;
  WorldRotation = FIsdkTransformerUtils::GetConstrainedTransformRotation(
      WorldRotation, Config.RotateConstraintAxes, Target.ParentWorldTransform);
  FQuat ResultRotation = Target.ParentWorldTransform.InverseTransformRotation(WorldRotation);

  // Compute the result position
  FVector WorldPosition =
      Centroid - Target.WorldTransform.TransformVector(RelativeCentroidToTargetVector);
  WorldPosition = FIsdkTransformerUtils::GetConstrainedTransformPosition(
      WorldPosition, TargetTranslateConstraintAxes, Target.ParentWorldTransform);
  FVector ResultPosition = Target.ParentWorldTransform.InverseTransformPosition(WorldPosition);

  // Draw debug state
  if (isdk::CVar_Meta_InteractionSDK_DebugInteractionVisuals.GetValueOnAnyThread())
  {
    const auto Depth = ESceneDepthPriorityGroup::SDPG_Foreground;
    DrawDebugCoordinateSystem(
        GetWorld(), Centroid, FRotator::ZeroRotator, 1.0, false, 0.0, Depth, 0.2);
    for (const auto& Pose : SelectedPoses)
    {
      DrawDebugCoordinateSystem(
          GetWorld(), Pose.Position(), FRotator(Pose.Orientation()), 1.0, false, 0.0, Depth, 0.2);
    }
  }

  // Return the local transform
  return FTransform(ResultRotation, ResultPosition, ResultScale);
}

FTransform UIsdkGrabFreeTransformer::EndTransform(const FIsdkTargetTransform& Target)
{
  SelectedPoses.Empty();
  Deltas.Empty();
  return Target.RelativeTransform;
}

void UIsdkGrabFreeTransformer::UpdateConstraints(const FIsdkTargetTransform& Target)
{
  TargetTranslateConstraintAxes = Config.TranslateConstraintAxes;
  if (Config.bUseRelativeTranslation)
  {
    TargetTranslateConstraintAxes = FIsdkTransformerUtils::GenerateParentPositionConstraints(
        Config.TranslateConstraintAxes, Target.RelativeTransform.GetLocation());
  }

  TargetScaleConstraint = Config.ScaleConstraint;
  if (Config.bUseRelativeScale)
  {
    TargetScaleConstraint = FIsdkTransformerUtils::GenerateParentScaleConstraints(
        Config.ScaleConstraint, Target.RelativeTransform.GetScale3D());
  }
}

// Calculate the delta rotation from previous frame
FQuat UIsdkGrabFreeTransformer::GetDeltaRotation()
{
  FQuat CombinedRotation = FQuat::Identity;

  if (Deltas.IsEmpty())
  {
    return CombinedRotation;
  }

  // each point can only affect a fraction of the rotation
  const float Fraction = 1.f / static_cast<float>(Deltas.Num());
  for (const auto& Delta : Deltas)
  {
    // overall delta rotation since last update
    FQuat RotDelta = Delta.Rotation * Delta.PrevRotation.Inverse();

    if (Delta.IsValidAxis())
    {
      FVector AimingAxis =
          Delta.CentroidOffset.GetUnsafeNormal(); // IsValidAxis checks the centroid offset
      // Rotation along the aiming axis
      FQuat DirDelta =
          FQuat::FindBetweenVectors(Delta.PrevCentroidOffset.GetSafeNormal(), AimingAxis);
      // Partial application of delta rotation
      CombinedRotation = FQuat::Slerp(FQuat::Identity, DirDelta, Fraction) * CombinedRotation;

      // Twist along the aiming axis
      RotDelta = FIsdkTransformerUtils::GetRotationTwistAroundAxis(RotDelta, AimingAxis, true);
    }

    CombinedRotation = FQuat::Slerp(FQuat::Identity, RotDelta, Fraction) * CombinedRotation;
  }

  return CombinedRotation;
}

FVector UIsdkGrabFreeTransformer::GetCentroid(const TArray<FIsdkGrabPose>& GrabPoses) const
{
  FVector SumPosition = FVector::ZeroVector;
  for (const auto& GrabPose : GrabPoses)
  {
    SumPosition += GrabPose.Position();
  }
  return SumPosition / static_cast<float>(GrabPoses.Num());
}

float UIsdkGrabFreeTransformer::GetDeltaScale()
{
  float ScaleDelta = 0;
  const float Fraction = 1.f / static_cast<float>(Deltas.Num());
  for (const auto& Delta : Deltas)
  {
    if (Delta.IsValidAxis())
    {
      const float Factor = FMath::Sqrt(
          Delta.CentroidOffset.SquaredLength() / Delta.PrevCentroidOffset.SquaredLength());
      ScaleDelta += Factor * Fraction;
    }
    else
    {
      ScaleDelta += Fraction;
    }
  }
  return ScaleDelta;
}
