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
#include "IsdkOneGrabTranslateTransformer.generated.h"

UCLASS(
    Blueprintable,
    ClassGroup = (InteractionSDK),
    meta = (DisplayName = "ISDK One Grab Translate Transformer"))
class OCULUSINTERACTION_API UIsdkOneGrabTranslateTransformer : public UObject,
                                                               public IIsdkITransformer
{
  GENERATED_BODY()
 public:
  UIsdkOneGrabTranslateTransformer();
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

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK|Constraint")
  bool bIsRelativeConstraint = true;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK|Constraint")
  FIsdkConstraintAxes PositionConstraint = FIsdkConstraintAxes();

 private:
  bool bIsActive = false;
  FVector InitialTargetOffset = FVector::ZeroVector;
  FIsdkConstraintAxes TargetPositionConstraint = FIsdkConstraintAxes();
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

  void DrawDebug(const FIsdkTargetTransform& Target);
};
