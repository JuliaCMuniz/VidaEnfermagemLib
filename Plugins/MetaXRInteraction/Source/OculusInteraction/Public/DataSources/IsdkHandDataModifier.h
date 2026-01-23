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
#include "IsdkHandDataSource.h"
#include "IsdkIRootPose.h"

#include "IsdkHandDataModifier.generated.h"

// Forward declarations of internal types
namespace isdk::api
{
class IHandDataModifier;
}
typedef struct isdk_IHandDataSource_ isdk_IHandDataSource;

/* Hand Data Source intended to take and modify an existing HandData Source, implements
 * IsdkIRootPose */
UCLASS(Abstract)
class OCULUSINTERACTION_API UIsdkHandDataModifier : public UIsdkHandDataSource,
                                                    public IIsdkIRootPose
{
  GENERATED_BODY()

 public:
  UIsdkHandDataModifier();
  virtual void BeginPlay() override;
  virtual void TickComponent(
      float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  /* Get the Hand Data Source this Modifier should be taking in as input */
  UFUNCTION(BlueprintPure, BlueprintInternalUseOnly, Category = InteractionSDK)
  UIsdkHandDataSource* GetInputDataSource() const
  {
    return InputDataSource;
  }

  /* Sets the hand data source this modifier should use as input */
  UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category = InteractionSDK)
  void SetInputDataSource(UIsdkHandDataSource* InInputDataSource);

  /* Return whether or not this hand data modifier will recursively update its data sources */
  UFUNCTION(BlueprintPure, BlueprintInternalUseOnly, Category = InteractionSDK)
  bool GetRecursiveUpdate() const
  {
    return bRecursiveUpdate;
  }

  /* Set whether or not this hand data modifier will recursively update its data sources */
  UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category = InteractionSDK)
  void SetRecursiveUpdate(bool bInRecursiveUpdate);

  virtual isdk::api::IHandDataModifier* GetApiIHandDataModifier()
      PURE_VIRTUAL(UIsdkHandDataSource::GetApiIHandDataModifier, return nullptr;);

  // From IIsdkRootPose
  virtual FTransform GetRootPose_Implementation() override
  {
    return ModifiedRootPose;
  }
  virtual bool IsRootPoseValid_Implementation() override;

  virtual UIsdkConditional* GetRootPoseConnectedConditional_Implementation() override;
  virtual UIsdkConditional* GetRootPoseHighConfidenceConditional_Implementation() override;

  // From IIsdkIHandJoints
  virtual EIsdkHandedness GetHandedness_Implementation() override;
  virtual bool IsHandJointDataValid_Implementation() override;

  virtual const UIsdkHandJointMappings* GetHandJointMappings_Implementation() override;

 protected:
  virtual bool ShouldUpdateDataSource() override;
  bool TryGetApiFromDataSource(isdk_IHandDataSource*& OutFromDataSource) const;

  /* The Hand Data Source used as input for this modifier */
  UPROPERTY(
      BlueprintGetter = GetInputDataSource,
      BlueprintSetter = SetInputDataSource,
      EditAnywhere,
      Category = InteractionSDK)
  UIsdkHandDataSource* InputDataSource = nullptr;

  /* Whether or not this hand data modifier will recursively update its data sources */
  UPROPERTY(
      BlueprintGetter = GetRecursiveUpdate,
      BlueprintSetter = SetRecursiveUpdate,
      EditAnywhere,
      Category = InteractionSDK)
  bool bRecursiveUpdate = false;

  FTransform ModifiedRootPose;
};
