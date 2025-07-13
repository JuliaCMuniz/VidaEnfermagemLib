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

#include "Interaction/Grabbable/IsdkOneGrabTranslateTransformer.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"

namespace isdk
{
extern TAutoConsoleVariable<bool> CVar_Meta_InteractionSDK_DebugInteractionVisuals;
}

UIsdkOneGrabTranslateTransformer::UIsdkOneGrabTranslateTransformer() {}

void UIsdkOneGrabTranslateTransformer::BeginTransform(
    const TArray<FIsdkGrabPose>& GrabPoses,
    const FIsdkTargetTransform& Target)
{
  if (GrabPoses.IsEmpty())
  {
    return;
  }
  bIsActive = true;
  CachedSelectedPose = GrabPoses[0];
  FVector ToTargetOffset = Target.WorldTransform.GetLocation() - CachedSelectedPose.Position();
  InitialTargetOffset = Target.ParentWorldTransform.InverseTransformVector(ToTargetOffset);
}

FTransform UIsdkOneGrabTranslateTransformer::UpdateTransform(
    const TArray<FIsdkGrabPose>& GrabPoses,
    const FIsdkTargetTransform& Target)
{
  if (GrabPoses.IsEmpty())
  {
    return Target.RelativeTransform;
  }
  CachedSelectedPose = GetSelectedPose(GrabPoses);
  auto LocalPosition =
      Target.ParentWorldTransform.InverseTransformPosition(CachedSelectedPose.Position());
  auto TargetRelativePosition = LocalPosition + InitialTargetOffset;
  auto ConstrainedRelativePosition = FIsdkTransformerUtils::GetConstrainedTransformPosition(
      TargetRelativePosition, TargetPositionConstraint);
  DrawDebug(Target);

  // Return the local transform
  return FTransform(
      Target.RelativeTransform.GetRotation(),
      ConstrainedRelativePosition,
      Target.RelativeTransform.GetScale3D());
}

FTransform UIsdkOneGrabTranslateTransformer::EndTransform(const FIsdkTargetTransform& Target)
{
  bIsActive = false;
  return Target.RelativeTransform;
}

void UIsdkOneGrabTranslateTransformer::UpdateConstraints(const FIsdkTargetTransform& Target)
{
  TargetPositionConstraint = PositionConstraint;
  if (bIsRelativeConstraint)
  {
    TargetPositionConstraint = FIsdkTransformerUtils::GenerateParentPositionConstraints(
        PositionConstraint, Target.RelativeTransform.GetLocation());
  }
}

void UIsdkOneGrabTranslateTransformer::DrawDebug(const FIsdkTargetTransform& Target)
{
  if (isdk::CVar_Meta_InteractionSDK_DebugInteractionVisuals.GetValueOnAnyThread())
  {
    auto GrabLocation = CachedSelectedPose.Position();
    auto GrabRotation = CachedSelectedPose.Orientation().Rotator();
    auto TargetLocation = Target.WorldTransform.GetLocation();
    const auto Depth = ESceneDepthPriorityGroup::SDPG_Foreground;
    DrawDebugCoordinateSystem(GetWorld(), GrabLocation, GrabRotation, 1.0, false, 0.0, Depth, 0.2);
    DrawDebugLine(GetWorld(), GrabLocation, TargetLocation, FColor::Red, false, 0.0, Depth, 0.2);
  }
}
