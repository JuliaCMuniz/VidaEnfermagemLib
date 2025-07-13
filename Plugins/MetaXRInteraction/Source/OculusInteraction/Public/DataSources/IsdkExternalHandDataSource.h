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
#include "UObject/Object.h"
#include "DataSources/IsdkHandDataSource.h"
#include "IsdkExternalHandDataSource.generated.h"

// Forward declarations of internal types
namespace isdk::api
{
class ExternalHandSource;

namespace helper
{
class FExternalHandDataSourceImpl;
}
} // namespace isdk::api

/* Hand Data Source, acts as a container for hand data, joint mappings, and other related data for
 * an external source */
UCLASS(
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK External Hand Data Source"))
class OCULUSINTERACTION_API UIsdkExternalHandDataSource : public UIsdkHandDataSource
{
  GENERATED_BODY()

 public:
  UIsdkExternalHandDataSource();
  virtual void BeginPlay() override;
  virtual void BeginDestroy() override;

  /* Sets this Data Source's Hand Data object, passing in its current transform in worldspace */
  UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category = InteractionSDK)
  void SetHandData(const UIsdkHandData* InHandData, const FTransform& WorldTransform);

  /* Sets the finger and thumb joint mappings used for this data source */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void SetHandJointMappings(
      const TArray<FIsdkExternalHandPositionFrame_ThumbJointMapping>& ThumbJointMappings,
      const TArray<FIsdkExternalHandPositionFrame_FingerJointMapping>& FingerJointMappings);

  virtual bool IsApiInstanceValid() const override;
  virtual isdk::api::IHandDataSource* GetApiIHandDataSource() override;
  virtual isdk::api::ExternalHandSource* GetApiExternalHandSource();

  // From IHandJoints
  virtual EIsdkHandedness GetHandedness_Implementation() override
  {
    return Handedness;
  }

  virtual const UIsdkHandJointMappings* GetHandJointMappings_Implementation() override
  {
    return HandJointMappings;
  }

  virtual bool IsHandJointDataValid_Implementation() override
  {
    return bIsHandJointDataValid;
  }

  /* The handedness of this data source (left or right) */
  UPROPERTY(
      BlueprintReadWrite,
      EditAnywhere,
      Category = InteractionSDK,
      meta = (ExposeOnSpawn = true))
  EIsdkHandedness Handedness = EIsdkHandedness::Left;

  /* Generates the mapping between OVR and OpenXR bones */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void PopulateOpenXRBoneMaps();

 protected:
  void SetImplHandData(const FTransform& Transform);

  TPimplPtr<isdk::api::helper::FExternalHandDataSourceImpl> ExternalHandDataSourceImpl;

  UPROPERTY()
  UIsdkHandJointMappings* HandJointMappings;

  UPROPERTY()
  bool bIsHandJointDataValid{false};
};
