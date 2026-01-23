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
#include "IsdkGrabFreeTransformer.generated.h"

USTRUCT()
struct FIsdkGrabPointDelta
{
  GENERATED_USTRUCT_BODY()

 public:
  static constexpr float _epsilon = 0.000001f;

  FVector PrevCentroidOffset;
  FVector CentroidOffset;
  FQuat PrevRotation;
  FQuat Rotation;

  FIsdkGrabPointDelta() : FIsdkGrabPointDelta(FVector::Zero(), FQuat::Identity) {}
  FIsdkGrabPointDelta(FVector CentroidPoseOffset, FQuat PoseRotation)
  {
    PrevCentroidOffset = CentroidOffset = CentroidPoseOffset;
    PrevRotation = Rotation = PoseRotation;
  }

  void UpdateData(FVector CentroidPoseOffset, FQuat PoseRotation)
  {
    PrevCentroidOffset = CentroidOffset;
    CentroidOffset = CentroidPoseOffset;
    PrevRotation = Rotation;

    auto NewRotation = PoseRotation;

    // Quaternions have two ways of expressing the same rotation.
    // This code ensures that the result is the same rotation but expressed in the desired sign.
    if ((NewRotation | Rotation) < 0)
    {
      NewRotation.X = -NewRotation.X;
      NewRotation.Y = -NewRotation.Y;
      NewRotation.Z = -NewRotation.Z;
      NewRotation.W = -NewRotation.W;
    }

    Rotation = NewRotation;
  }

  bool IsValidAxis() const
  {
    return CentroidOffset.SquaredLength() > _epsilon;
  }
};

USTRUCT(Blueprintable) struct FIsdkGrabFreeTransformerConfig
{
  GENERATED_USTRUCT_BODY()

 public:
  FIsdkGrabFreeTransformerConfig(
      bool bUseRelativeTranslation = true,
      const FIsdkConstraintAxes& TranslateConstraints = FIsdkConstraintAxes(),
      const FIsdkConstraintAxes& RotateConstraints = FIsdkConstraintAxes(),
      bool bUseRelativeScale = true,
      const FIsdkAxisConstraints& ScaleConstraints = FIsdkAxisConstraints())
      : bUseRelativeTranslation(bUseRelativeTranslation),
        TranslateConstraintAxes(TranslateConstraints),
        RotateConstraintAxes(RotateConstraints),
        bUseRelativeScale(bUseRelativeScale),
        ScaleConstraint(ScaleConstraints)
  {
  }

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK")
  bool bUseRelativeTranslation = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK")
  FIsdkConstraintAxes TranslateConstraintAxes;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK")
  FIsdkConstraintAxes RotateConstraintAxes;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK")
  bool bUseRelativeScale = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK")
  FIsdkAxisConstraints ScaleConstraint;
};

/* Scene component utilized for transforming a grabbable object, including scale and axis
 *  constraints and physics considerations */
UCLASS(
    Blueprintable,
    ClassGroup = (InteractionSDK),
    meta = (DisplayName = "ISDK Grab Free Transformer"))
class OCULUSINTERACTION_API UIsdkGrabFreeTransformer : public UObject, public IIsdkITransformer
{
  GENERATED_BODY()
 public:
  UIsdkGrabFreeTransformer();
  virtual bool IsTransformerActive() const override
  {
    return Deltas.Num() > 0;
  }
  virtual int MaxGrabPoints() const override
  {
    return -1;
  }
  virtual void BeginTransform(
      const TArray<FIsdkGrabPose>& GrabPoses,
      const FIsdkTargetTransform& Target) override;
  virtual FTransform UpdateTransform(
      const TArray<FIsdkGrabPose>& GrabPoses,
      const FIsdkTargetTransform& Target) override;
  virtual FTransform EndTransform(const FIsdkTargetTransform& Target) override;
  virtual void UpdateConstraints(const FIsdkTargetTransform& Target) override;

  /* Returns the number of grab points on this transformer */
  UFUNCTION(BlueprintCallable, Category = "InteractionSDK")
  int GetGrabCount()
  {
    return Deltas.Num();
  }

  /* Returns the current interactable state, driven by events */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK")
  FIsdkGrabFreeTransformerConfig Config;

 private:
  TArray<FIsdkGrabPose> SelectedPoses;
  FVector RelativeCentroidToTargetVector = FVector::ZeroVector;
  FQuat InitialTargetWorldRotation = FQuat::Identity;
  FQuat LastRotation = FQuat::Identity;
  FVector LastScale = FVector::ZeroVector;
  TArray<FIsdkGrabPointDelta> Deltas;

  // These might be the constraints from the config or relative constraints depending on the
  // configuration
  FIsdkConstraintAxes TargetTranslateConstraintAxes;
  FIsdkAxisConstraints TargetScaleConstraint;

  // helper functions
  void UpdateSelectedPoses(const TArray<FIsdkGrabPose>& GrabPoses)
  {
    for (auto& SelectedPose : SelectedPoses)
    {
      for (const auto& GrabPose : GrabPoses)
      {
        if (SelectedPose.Identifier == GrabPose.Identifier)
        {
          SelectedPose = GrabPose;
          break;
        }
      }
    }
  }
  FVector GetCentroid(const TArray<FIsdkGrabPose>& Poses) const;
  float GetDeltaScale();
  FQuat GetDeltaRotation();
  void UpdateGrabPointDeltas(FVector Centroid);
};
