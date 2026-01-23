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
#include "IsdkHandData.generated.h"

UENUM(BlueprintType)
enum class EIsdkHandType : uint8
{
  None,
  HandLeft,
  HandRight,
};

UENUM(BlueprintType)
enum class EIsdkHandBones : uint8
{
  HandStart = 0,
  HandPalm = HandStart + 0 UMETA(DisplayName = "Palm"),
  HandWristRoot = HandStart + 1 UMETA(DisplayName = "Wrist"),

  HandThumb1 = HandStart + 2 UMETA(DisplayName = "Thumb Metacarpal"),
  HandThumb2 = HandStart + 3 UMETA(DisplayName = "Thumb Proximal"),
  HandThumb3 = HandStart + 4 UMETA(DisplayName = "Thumb Distal"),
  HandThumbTip = HandStart + 5 UMETA(DisplayName = "Thumb Tip"),

  HandIndex0 = HandStart + 6 UMETA(DisplayName = "Index Metacarpal"),
  HandIndex1 = HandStart + 7 UMETA(DisplayName = "Index Proximal"),
  HandIndex2 = HandStart + 8 UMETA(DisplayName = "Index Intermediate"),
  HandIndex3 = HandStart + 9 UMETA(DisplayName = "Index Distal"),
  HandIndexTip = HandStart + 10 UMETA(DisplayName = "Index Tip"),

  HandMiddle0 = HandStart + 11 UMETA(DisplayName = "Middle Metacarpal"),
  HandMiddle1 = HandStart + 12 UMETA(DisplayName = "Middle Proximal"),
  HandMiddle2 = HandStart + 13 UMETA(DisplayName = "Middle Intermediate"),
  HandMiddle3 = HandStart + 14 UMETA(DisplayName = "Middle Distal"),
  HandMiddleTip = HandStart + 15 UMETA(DisplayName = "Middle Tip"),

  HandRing0 = HandStart + 16 UMETA(DisplayName = "Ring Metacarpal"),
  HandRing1 = HandStart + 17 UMETA(DisplayName = "Ring Proximal"),
  HandRing2 = HandStart + 18 UMETA(DisplayName = "Ring Intermediate"),
  HandRing3 = HandStart + 19 UMETA(DisplayName = "Ring Distal"),
  HandRingTip = HandStart + 20 UMETA(DisplayName = "Ring Tip"),

  HandPinky0 = HandStart + 21 UMETA(DisplayName = "Pinky Metacarpal"),
  HandPinky1 = HandStart + 22 UMETA(DisplayName = "Pinky Proximal"),
  HandPinky2 = HandStart + 23 UMETA(DisplayName = "Pinky Intermediate"),
  HandPinky3 = HandStart + 24 UMETA(DisplayName = "Pinky Distal"),
  HandPinkyTip = HandStart + 25 UMETA(DisplayName = "Pinky Tip"),

  HandEnd = HandStart + 26,
  EHandBones_MAX = HandEnd,
  Invalid,
};

struct isdk_HandData_;
typedef isdk_HandData_ isdk_HandData;

UCLASS(BlueprintType, DefaultToInstanced)
class OCULUSINTERACTION_API UIsdkHandData : public UObject
{
  GENERATED_BODY()
 public:
  UIsdkHandData();
  void ReadJoints(isdk_HandData& HandData);

  UFUNCTION(BlueprintPure, Category = InteractionSDK)
  static int32 GetNumJoints()
  {
    return static_cast<int32>(EIsdkHandBones::EHandBones_MAX);
  }

  UFUNCTION(BlueprintPure, Category = InteractionSDK)
  const TArray<FTransform>& GetJointPoses() const
  {
    return JointPoses;
  }
  TArray<FTransform>& GetJointPoses()
  {
    return JointPoses;
  }
  UFUNCTION(BlueprintPure, Category = InteractionSDK)
  const TArray<float>& GetJointRadii() const
  {
    return JointRadii;
  }
  TArray<float>& GetJointRadii()
  {
    return JointRadii;
  }

  UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category = InteractionSDK)
  void SetJoints(const TArray<FTransform>& InJointPoses)
  {
    JointPoses = InJointPoses;
  }
  void SetJoints(const isdk_HandData& HandData);
  void SetJoints(const UIsdkHandData& HandData);

  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void SetCachedJointPose(const FTransform& PoseIn, const int32 JointPoseIndex)
  {
    JointPosesCache[JointPoseIndex] = PoseIn;
  }

  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void SetCachedJointPoses(TArray<FTransform>& PosesIn)
  {
    JointPosesCache = PosesIn;
  }

  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void SetJointsToIdentity();

  /* Set Inbound Bone Mappings */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void SetInboundBoneMap(TMap<int32, int32>& InboundMap)
  {
    InboundBoneMapping = InboundMap;
  }

  /* Set Inbound Bone Mappings */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void SetOutboundBoneMap(TMap<int32, int32>& OutboundMap)
  {
    OutboundBoneMapping = OutboundMap;
  }

 private:
  UPROPERTY()
  TArray<FTransform> JointPoses{};
  UPROPERTY()
  TArray<float> JointRadii{};
  UPROPERTY()
  TArray<FTransform> JointPosesCache{};

  // Bone mapping to translate for sending to external skeleton
  UPROPERTY()
  TMap<int32, int32> OutboundBoneMapping = {};

  // Bone mapping to translate for receiving from external skeleton
  UPROPERTY()
  TMap<int32, int32> InboundBoneMapping = {};
};
