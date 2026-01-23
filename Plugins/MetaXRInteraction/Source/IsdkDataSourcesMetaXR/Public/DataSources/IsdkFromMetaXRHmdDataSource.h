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
#include "UObject/Object.h"
#include "Components/ActorComponent.h"
#include "IsdkFromMetaXRHmdDataSource.generated.h"

/**
 *
 */
UCLASS(
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK From MetaXR HMD Data Source"))
class ISDKDATASOURCESMETAXR_API UIsdkFromMetaXRHmdDataSource : public UActorComponent,
                                                               public IIsdkIHmdDataSource
{
  GENERATED_BODY()
  virtual bool IsHmdTracked_Implementation() override;
  virtual void GetHmdPose_Implementation(FTransform& HmdPose, bool& IsTracked) override;
  virtual void GetRelativeHmdPose_Implementation(FTransform& HmdRelativePose, bool& IsTracked)
      override;
};
