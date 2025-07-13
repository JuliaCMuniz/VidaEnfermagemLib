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
#include "Subsystem/IsdkITrackingDataSubsystem.h"
#include "IsdkControllerRigComponent.generated.h"

enum class EControllerHandBehavior : uint8;

UCLASS(Abstract, ClassGroup = ("InteractionSDK|Rig"), meta = (BlueprintSpawnableComponent))
class OCULUSINTERACTIONPREBUILTS_API UIsdkControllerRigComponent : public UIsdkRigComponent
{
  GENERATED_BODY()

 public:
  UIsdkControllerRigComponent() {}

  UIsdkControllerRigComponent(
      EIsdkHandType HandType,
      const FString& HandBPPath,
      const FObjectInitializer& ObjectInitializer);

  virtual void BeginPlay() override;

 protected:
  virtual void OnControllerVisualsAttached() override;
  virtual FVector GetPalmColliderOffset() const override;
  virtual USkinnedMeshComponent* GetPinchAttachMesh() const override;

 private:
  virtual TSubclassOf<AActor> FindBPFromPath(const FString& Path);

  UFUNCTION()
  void HandleControllerHandBehaviorChanged(
      TScriptInterface<IIsdkITrackingDataSubsystem> IsdkITrackingDataSubsystem,
      EControllerHandBehavior ControllerHandBehavior,
      EControllerHandBehavior ControllerHandBehavior1);
};

UCLASS(
    Blueprintable,
    ClassGroup = ("InteractionSDK|Rig"),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Controller Rig (L)"))
class OCULUSINTERACTIONPREBUILTS_API UIsdkControllerRigComponentLeft
    : public UIsdkControllerRigComponent
{
  GENERATED_BODY()

 public:
  UIsdkControllerRigComponentLeft(
      const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

UCLASS(
    Blueprintable,
    ClassGroup = ("InteractionSDK|Rig"),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Controller Rig (R)"))
class OCULUSINTERACTIONPREBUILTS_API UIsdkControllerRigComponentRight
    : public UIsdkControllerRigComponent
{
  GENERATED_BODY()

 public:
  UIsdkControllerRigComponentRight(
      const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
