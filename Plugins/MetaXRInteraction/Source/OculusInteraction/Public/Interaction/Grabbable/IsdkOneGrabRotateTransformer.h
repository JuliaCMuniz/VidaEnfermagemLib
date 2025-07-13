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

#pragma once

#include "CoreMinimal.h"
#include "IsdkTransformerUtils.h"
#include "MathUtil.h"
#include "IsdkITransformer.h"
#include "IsdkOneGrabRotateTransformer.generated.h"

USTRUCT(Blueprintable)
struct FOneGrabRotationConstraint
{
  GENERATED_BODY()

 public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  bool bUseAngleFromAxisConstraint;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  TEnumAsByte<EAxis::Type> RotationAxis;
  UPROPERTY(
      EditAnywhere,
      BlueprintReadWrite,
      Category = InteractionSDK,
      meta =
          (EditCondition = "(!bUseAngleFromAxisConstraint) && (RotationAxis == 0)",
           EditConditionHides))
  FIsdkConstraintAxes RotationConstraint;
  UPROPERTY(
      EditAnywhere,
      BlueprintReadWrite,
      Category = InteractionSDK,
      meta =
          (EditCondition = "bUseAngleFromAxisConstraint",
           EditConditionHides,
           ClampMin = "0.0",
           ClampMax = "180.0",
           UIMin = "0.0",
           UIMax = "180.0"))
  float MaxAngleFromAxis = 0.0;
  UPROPERTY(
      EditAnywhere,
      BlueprintReadWrite,
      Category = InteractionSDK,
      meta =
          (EditCondition = "(!bUseAngleFromAxisConstraint) && (RotationAxis != 0)",
           EditConditionHides))
  FIsdkAxisConstraints AxisAngleRange = FIsdkAxisConstraints();

  FOneGrabRotationConstraint()
  {
    bUseAngleFromAxisConstraint = false;
    RotationAxis = EAxis::Type::X;
    RotationConstraint = FIsdkConstraintAxes();
    MaxAngleFromAxis = 0.0;
    AxisAngleRange = FIsdkAxisConstraints();
  }
};

UCLASS(
    Blueprintable,
    ClassGroup = (InteractionSDK),
    meta = (DisplayName = "ISDK One Grab Rotate Transformer"))
class OCULUSINTERACTION_API UIsdkOneGrabRotateTransformer : public UObject, public IIsdkITransformer
{
  GENERATED_BODY()
 public:
  UIsdkOneGrabRotateTransformer();
  virtual bool IsTransformerActive() const override
  {
    return bIsActive;
  }
  virtual int MaxGrabPoints() const override
  {
    return 1;
  }
  virtual void BeginTransform(
      const TArray<FIsdkGrabPose>& GrabPoses,
      const FIsdkTargetTransform& Target) override;
  virtual FTransform UpdateTransform(
      const TArray<FIsdkGrabPose>& GrabPoses,
      const FIsdkTargetTransform& Target) override;
  virtual FTransform EndTransform(const FIsdkTargetTransform& Target) override;
  virtual void UpdateConstraints(const FIsdkTargetTransform& Target) override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  FOneGrabRotationConstraint Constraint = FOneGrabRotationConstraint();

 private:
  bool bIsActive = false;
  FQuat CachedTargetRelativeRotation = FQuat::Identity;
  FVector CachedPoseVector = FVector::ZeroVector;
  FIsdkGrabPose CachedSelectedPose = FIsdkGrabPose();
  FIsdkGrabPose GetSelectedPose(const TArray<FIsdkGrabPose>& GrabPoses)
  {
    for (const auto& GrabPose : GrabPoses)
    {
      if (GrabPose.Identifier == CachedSelectedPose.Identifier)
      {
        return GrabPose;
      }
    }
    return CachedSelectedPose;
  }

  FVector TransformGrabVector(const FVector& Vector);
  void DrawDebug(
      const FTransform& PivotTransform,
      const FVector& Axis,
      const FVector& StartAxis,
      const FVector& CurrentAxis);
};
