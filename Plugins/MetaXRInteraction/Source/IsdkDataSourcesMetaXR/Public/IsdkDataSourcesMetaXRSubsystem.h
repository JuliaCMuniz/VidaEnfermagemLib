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
#include "DataSources/IsdkIHmdDataSource.h"
#include "DataSources/IsdkFromMetaXRControllerDataSource.h"
#include "DataSources/IsdkFromMetaXRHandDataSource.h"
#include "Subsystem/IsdkITrackingDataSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "IsdkDataSourcesMetaXRSubsystem.generated.h"

/**
 *
 */
UCLASS()
class ISDKDATASOURCESMETAXR_API UIsdkDataSourcesMetaXRSubsystem : public UTickableWorldSubsystem,
                                                                  public IIsdkITrackingDataSubsystem
{
  GENERATED_BODY()

  UIsdkDataSourcesMetaXRSubsystem();
  virtual void Initialize(FSubsystemCollectionBase& Collection) override;
  virtual void Tick(float DeltaTime) override;
  virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

 public:
  virtual FIsdkTrackingDataSources CreateHandDataSourceComponent_Implementation(
      UMotionControllerComponent* AttachToMotionController) override;
  virtual FIsdkTrackingDataSources CreateControllerDataSourceComponent_Implementation(
      UMotionControllerComponent* AttachToMotionController) override;
  virtual TScriptInterface<IIsdkIHmdDataSource> CreateHmdDataSourceComponent_Implementation(
      AActor* TrackingSpaceRoot) override;
  virtual bool IsEnabled_Implementation() override;

  virtual EControllerHandBehavior GetControllerHandBehavior() override;
  virtual void SetControllerHandBehavior(EControllerHandBehavior ControllerHandBehavior) override;
  virtual FIsdkTrackedDataSubsystem_ControllerHandsBehaviorChangedDelegate*
  GetControllerHandBehaviorChangedDelegate() override;

  virtual TStatId GetStatId() const override;

  virtual bool IsHoldingAController();

 private:
  UPROPERTY()
  UIsdkFromMetaXRHandDataSource* HandDataSource;

  UPROPERTY()
  UIsdkFromMetaXRControllerDataSource* ControllerDataSource;

  EControllerHandBehavior CurrentControllerHandBehavior;
  FIsdkTrackedDataSubsystem_ControllerHandsBehaviorChangedDelegate
      ControllerHandsBehaviorChangedDelegate;

  /*
   * A cached value indicating whether a controller is held in at least one hand.  We cache this
   * to avoid expensive calls into the meta xr module instance.
   */
  bool bIsHoldingAController = true;
};
