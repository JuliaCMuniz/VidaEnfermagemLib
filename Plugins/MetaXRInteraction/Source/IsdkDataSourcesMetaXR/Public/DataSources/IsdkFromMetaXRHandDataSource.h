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
#include "MotionControllerComponent.h"
#include "Core/IsdkConditionalBool.h"
#include "DataSources/IsdkExternalHandDataSource.h"
#include "DataSources/IsdkIHandPointerPose.h"
#include "DataSources/IsdkIRootPose.h"
#include "Components/PoseableMeshComponent.h"

#include "IsdkFromMetaXRHandDataSource.generated.h"

// Forward declarations of internal types
namespace isdk::api
{
class ExternalHandSource;
}

USTRUCT()
struct FBoneOVRToOXRMap
{
  GENERATED_BODY()

 public:
  FName OVRBoneName;
  FVector OXRWristOffset;
  float OXRRotation;
  int OXRBoneIndex;

 public:
  static FBoneOVRToOXRMap MappedIndex(int Index, FName BoneName)
  {
    FBoneOVRToOXRMap mapped{};
    mapped.OVRBoneName = BoneName;
    mapped.OXRBoneIndex = Index;
    mapped.OXRWristOffset = FVector::Zero();
    mapped.OXRRotation = 0.0;
    return mapped;
  }

  static FBoneOVRToOXRMap MappedOffset(int Index, FVector Offset, float Rotation)
  {
    FBoneOVRToOXRMap mapped{};
    mapped.OVRBoneName = FName();
    mapped.OXRBoneIndex = Index;
    mapped.OXRWristOffset = Offset;
    mapped.OXRRotation = Rotation;
    return mapped;
  }
};

UCLASS(
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK From MetaXR Hand Data Source"))
class ISDKDATASOURCESMETAXR_API UIsdkFromMetaXRHandDataSource : public UIsdkExternalHandDataSource,
                                                                public IIsdkIHandPointerPose,
                                                                public IIsdkIRootPose
{
  GENERATED_BODY()

 public:
  UIsdkFromMetaXRHandDataSource();

  virtual void InitializeComponent() override;

  virtual void TickComponent(
      float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  virtual bool IsPointerPoseValid_Implementation() override;

  virtual bool IsRootPoseValid_Implementation() override;

  virtual FTransform GetRootPose_Implementation() override;

  virtual void GetPointerPose_Implementation(FTransform& PointerPose, bool& IsValid) override;

  virtual void GetRelativePointerPose_Implementation(FTransform& PointerRelativePose, bool& IsValid)
      override;

  // Property Getters
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UMotionControllerComponent* GetMotionController() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  bool GetAllowInvalidTrackedData() const;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  virtual UIsdkConditional* GetRootPoseConnectedConditional_Implementation() override;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  virtual UIsdkConditional* GetRootPoseHighConfidenceConditional_Implementation() override;

  // Property Setters
  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetMotionController(UMotionControllerComponent* InMotionController);

  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetAllowInvalidTrackedData(bool bInAllowInvalidTrackedData);

 private:
  void ReadHandData();
  void ReadHandedness();
  void DrawDebugWidgets();

  FTransform RelativePointerPose{};

  bool bIsLastGoodRootPoseValid = false;
  bool bIsLastGoodPointerPoseValid = false;

  UPROPERTY(
      BlueprintGetter = GetAllowInvalidTrackedData,
      BlueprintSetter = SetAllowInvalidTrackedData,
      Category = InteractionSDK)
  bool bAllowLowConfidenceData = false;
  bool bHasLastKnownGood = false;

  UPROPERTY(
      BlueprintGetter = GetMotionController,
      BlueprintSetter = SetMotionController,
      Category = InteractionSDK,
      meta = (ExposeOnSpawn = true))
  UMotionControllerComponent* MotionController;

  FTransform LastGoodRootPose{};

  UPROPERTY()
  UIsdkConditionalBool* IsRootPoseConnected{};

  UPROPERTY()
  UIsdkConditionalBool* IsRootPoseHighConfidence{};

  // Required to generate the "OpenXR" Data
  UPROPERTY()
  UPoseableMeshComponent* OculusXrHandComponent = {};
  void SetUintEnumProperty(UObject* Target, FName InName, uint8 EnumValue);

  void GenerateBoneMap();
  TArray<FBoneOVRToOXRMap> BoneMap;
  TArray<float> DefaultJointRadii;

#if !UE_BUILD_SHIPPING
  // Log the state of all relevant data source state and interfaces.
  void DebugLog();
#endif
};
