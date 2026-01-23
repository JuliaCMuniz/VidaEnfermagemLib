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
#include "UObject/Interface.h"
#include "StructTypes.h"
#include "Components/SceneComponent.h"
#include "IsdkITransformer.generated.h"

/**
 * @brief Stores the pose and identifier meant to be used in the Transformer computation.
 * @param - Identifier: Should represent the source of the pose, be identifiable by it, and be
 * @param unique in the Transformer.
 * @param - Pose: represents the modification point of the source, e.g. the pinch point, palm
 * @param center, snap point, etc..
 */
USTRUCT(
    BlueprintType,
    meta = (HasNativeMake = "OculusInteraction.IsdkFunctionLibrary.MakeGrabPoseStruct"))
struct FIsdkGrabPose
{
  GENERATED_BODY()

 public:
  // Should represent the source of the pose, be identifiable by it, and be different to any other
  // sent to the same transformer
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  int Identifier = 0;
  // represents the modification point of the source, e.g. the pinch point, palm grab center, snap
  // point, etc..
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  FIsdkPosef Pose = FIsdkPosef{};
  FIsdkGrabPose() : FIsdkGrabPose(0, FIsdkPosef{}) {}
  FIsdkGrabPose(int Identifier, FIsdkPosef Pose) : Identifier(Identifier), Pose(Pose) {}
  FVector Position() const
  {
    return FVector(Pose.Position);
  }
  FQuat Orientation() const
  {
    return Pose.Orientation;
  }
};

/**
 * @brief Stores the transformation matrices that represents the current state of the target
 * @brief USceneComponent the user is trying to update through a Transformer.
 */
USTRUCT(
    BlueprintType,
    meta = (HasNativeMake = "OculusInteraction.IsdkFunctionLibrary.MakeTargetTransformStruct"))
struct FIsdkTargetTransform
{
  GENERATED_BODY()

 public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  FTransform WorldTransform{};
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  FTransform RelativeTransform{};
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  FTransform ParentWorldTransform{};

  FIsdkTargetTransform()
  {
    WorldTransform = FTransform::Identity;
    RelativeTransform = FTransform::Identity;
    ParentWorldTransform = FTransform::Identity;
  }
  FIsdkTargetTransform(const USceneComponent* Target)
  {
    WorldTransform = Target->GetComponentTransform();
    RelativeTransform = Target->GetRelativeTransform();
    ParentWorldTransform = FTransform::Identity;
    if (Target->GetAttachParent() != nullptr)
    {
      ParentWorldTransform = Target->GetAttachParent()->GetComponentTransform();
    }
  }
};

UINTERFACE(BlueprintType, Category = "InteractionSDK")
class OCULUSINTERACTION_API UIsdkITransformer : public UInterface
{
  GENERATED_BODY()
};

class OCULUSINTERACTION_API IIsdkITransformer
{
  GENERATED_BODY()

 public:
  // Initialize the transformation
  virtual void BeginTransform(
      const TArray<FIsdkGrabPose>& SelectPoses,
      const FIsdkTargetTransform& Target) PURE_VIRTUAL(IIsdkITransformer::BeginTransform, ;);
  // Flush any local state and generate a final update to the target transform, for example
  // finishing some interpolation
  [[nodiscard]] virtual FTransform EndTransform(const FIsdkTargetTransform& Target)
      PURE_VIRTUAL(IIsdkITransformer::EndTransform, return FTransform::Identity;);
  // Generate the transform update, the result should be a FTransform relative to the parent
  [[nodiscard]] virtual FTransform UpdateTransform(
      const TArray<FIsdkGrabPose>& SelectPoses,
      const FIsdkTargetTransform& Target)
      PURE_VIRTUAL(IIsdkITransformer::UpdateTransform, return FTransform::Identity;);
  // Meant to be used to generate constraints based on the state of the target at some point in time
  virtual void UpdateConstraints(const FIsdkTargetTransform& Target)
      PURE_VIRTUAL(IIsdkITransformer::UpdateConstraints, ;);
  virtual bool IsTransformerActive() const PURE_VIRTUAL(IIsdkITransformer::IsActive, return false;);
  // Max grab points or -1 if infinite
  virtual int MaxGrabPoints() const PURE_VIRTUAL(IIsdkITransformer::MaxGrabPoints, return 0;);
};
