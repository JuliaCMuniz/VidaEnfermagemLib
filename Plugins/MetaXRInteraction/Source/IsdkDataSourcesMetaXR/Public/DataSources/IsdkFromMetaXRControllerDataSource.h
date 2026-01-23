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
#include "Components/ActorComponent.h"
#include "Core/IsdkConditionalBool.h"
#include "DataSources/IsdkIHandPointerPose.h"
#include "DataSources/IsdkIRootPose.h"
#include "DataSources/IsdkExternalHandDataSource.h"
#include "IsdkFromMetaXRControllerDataSource.generated.h"

// Forward declarations of internal types
namespace isdk::api
{
class ExternalHandSource;
}

UCLASS(
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK From MetaXR Controller Data Source"))
class ISDKDATASOURCESMETAXR_API UIsdkFromMetaXRControllerDataSource
    : public UIsdkExternalHandDataSource,
      public IIsdkIHandPointerPose,
      public IIsdkIRootPose
{
  GENERATED_BODY()

 public:
  // Sets default values for this component's properties
  UIsdkFromMetaXRControllerDataSource();
  virtual void TickComponent(
      float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  // Property Getters
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UMotionControllerComponent* GetMotionController() const;

  // Property Setters
  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetMotionController(UMotionControllerComponent* InMotionController);

  // IIsdkIHandPointerPose
  virtual bool IsPointerPoseValid_Implementation() override;
  virtual void GetPointerPose_Implementation(FTransform& PointerPose, bool& IsValid) override;
  virtual void GetRelativePointerPose_Implementation(FTransform& PointerRelativePose, bool& IsValid)
      override;
  // ~IIsdkIHandPointerPose

  // IIsdkIRootPose
  virtual FTransform GetRootPose_Implementation() override;
  virtual bool IsRootPoseValid_Implementation() override;

  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  virtual UIsdkConditional* GetRootPoseConnectedConditional_Implementation() override;
  // ~IIsdkIRootPose

 private:
  void ReadHandedness();
  void ReadControllerData();

  FTransform RelativePointerPose{};
  FTransform LastGoodRootPose{};

  bool bIsLastGoodRootPoseValid = false;
  bool bIsLastGoodPointerPoseValid = false;

  UPROPERTY(
      BlueprintGetter = GetMotionController,
      BlueprintSetter = SetMotionController,
      Category = InteractionSDK,
      meta = (ExposeOnSpawn = true))
  UMotionControllerComponent* MotionController;

  UPROPERTY()
  TObjectPtr<UIsdkConditionalBool> IsRootPoseConnected;

  UPROPERTY()
  TObjectPtr<UIsdkConditionalBool> IsRootPoseHighConfidence;

#if !UE_BUILD_SHIPPING
  void DebugLog();
#endif
};
