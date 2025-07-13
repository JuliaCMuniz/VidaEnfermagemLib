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
#include "IsdkRigComponent.h"
#include "Components/ActorComponent.h"
#include "IsdkHandRigComponent.generated.h"

UCLASS(ClassGroup = ("InteractionSDK|Rig"))
class OCULUSINTERACTIONPREBUILTS_API UIsdkHandRigComponent : public UIsdkRigComponent
{
  GENERATED_BODY()

 public:
  UIsdkHandRigComponent(const FObjectInitializer& ObjectInitializer) {}
  virtual FVector GetPalmColliderOffset() const override;
  virtual USkinnedMeshComponent* GetPinchAttachMesh() const override;
};

UCLASS(
    Blueprintable,
    ClassGroup = ("InteractionSDK|Rig"),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Hand Rig (L)"))
class OCULUSINTERACTIONPREBUILTS_API UIsdkHandRigComponentLeft : public UIsdkHandRigComponent
{
  GENERATED_BODY()

 public:
  UIsdkHandRigComponentLeft(const FObjectInitializer& ObjectInitializer);

 protected:
  virtual void OnHandVisualsAttached() override;
};

UCLASS(
    Blueprintable,
    ClassGroup = ("InteractionSDK|Rig"),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Hand Rig (R)"))
class OCULUSINTERACTIONPREBUILTS_API UIsdkHandRigComponentRight : public UIsdkHandRigComponent
{
  GENERATED_BODY()

 public:
  UIsdkHandRigComponentRight(const FObjectInitializer& ObjectInitializer);

 protected:
  virtual void OnHandVisualsAttached() override;
};
