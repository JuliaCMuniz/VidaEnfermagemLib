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
#include "Engine/DataAsset.h"
#include "Animation/PoseAsset.h"
#include "Animation/Skeleton.h"
#include "OculusInteractionLog.h"
#include "IsdkHandData.h"
#include "Engine/DataTable.h"
#include "Engine/SkinnedAsset.h"
#include "DataSources/IsdkIHandJoints.h"
#include "IsdkHandPoseData.generated.h"

class UIsdkHandMeshComponent;

UENUM(BlueprintType)
enum class EIsdkJointFreedom : uint8
{
  Free,
  Constrained,
  Locked
};

UCLASS(Blueprintable, ClassGroup = (InteractionSDK), meta = (DisplayName = "ISDK Hand Pose Data"))
class OCULUSINTERACTION_API UIsdkHandPoseData : public UPrimaryDataAsset, public IIsdkIHandJoints
{
  GENERATED_BODY()
 public:
  UIsdkHandPoseData();

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK")
  EIsdkHandType Handedness;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK|Joint Freedom")
  EIsdkJointFreedom Thumb;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK|Joint Freedom")
  EIsdkJointFreedom Index;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK|Joint Freedom")
  EIsdkJointFreedom Middle;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK|Joint Freedom")
  EIsdkJointFreedom Ring;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractionSDK|Joint Freedom")
  EIsdkJointFreedom Pinky;

  /* Hand Data object, storing joint & bone information */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  UIsdkHandData* HandData{};

  /* Thumb and Finger Joint Mappings */
  UPROPERTY(Transient)
  UIsdkHandJointMappings* HandJointMapping{};

  virtual UIsdkHandData* GetHandData_Implementation() override
  {
    return HandData;
  }
  virtual bool IsHandJointDataValid_Implementation() override
  {
    return HandData != nullptr;
  }
  virtual EIsdkHandedness GetHandedness_Implementation() override
  {
    return Handedness == EIsdkHandType::HandLeft ? EIsdkHandedness::Left : EIsdkHandedness::Right;
  }
  virtual const UIsdkHandJointMappings* GetHandJointMappings_Implementation() override
  {
    return HandJointMapping;
  }

  /* Array of Joint Names */
  UPROPERTY(EditAnywhere, Category = InteractionSDK)
  FName JointNames[static_cast<size_t>(EIsdkHandBones::EHandBones_MAX)];

  static const size_t GetJointCount()
  {
    return static_cast<size_t>(EIsdkHandBones::EHandBones_MAX);
  }

  /* Get the quaternion at a given joint index*/
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  FQuat GetJoint(EIsdkHandBones JointIndex)
  {
    return HandData->GetJointPoses()[uint8(JointIndex)].GetRotation();
  }

  /* Set the quaternion of a given joint index*/
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void SetJoint(EIsdkHandBones JointIndex, FQuat Rotation)
  {
    HandData->GetJointPoses()[uint8(JointIndex)].SetRotation(Rotation);
  }

  /* Mark this asset as dirty and required to be saved */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void SetDirty()
  {
    Modify();
  }
  /* Sets the Hand Pose (all joints) from a given skeleton */
  UFUNCTION(CallInEditor, BlueprintCallable, Category = InteractionSDK)
  void SetRotationFromSkeleton(USkinnedAsset* SkinnedAsset);

  /* Sets the Hand Pose (all joints) from a given Hand Visual Component */
  UFUNCTION(CallInEditor, BlueprintCallable, Category = InteractionSDK)
  void SetRotationFromVisual(UIsdkHandMeshComponent* HandMesh);

  /* Sets the Hand Pose (all joints) from a given named Pose Asset */
  UFUNCTION(CallInEditor, BlueprintCallable, Category = InteractionSDK)
  void SetRotationFromPoseWithName(UPoseAsset* Pose, FName Name);

  /* Returns the pose lerp time */
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  float GetPoseLerpTime() const
  {
    return PoseLerpTime;
  }

 protected:
  /* If positive, will lerp into/out of hand pose with the given timing */
  UPROPERTY(
      BlueprintGetter = GetPoseLerpTime,
      EditAnywhere,
      BlueprintReadWrite,
      Category = "InteractionSDK")
  float PoseLerpTime = 0.f;
};
