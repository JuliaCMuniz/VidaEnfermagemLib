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
#include "IsdkInteractorComponent.h"
#include "StructTypes.h"
#include "DataSources/IsdkIHandPointerPose.h"
#include "IsdkRayInteractor.generated.h"

// Forward declarations of internal types
namespace isdk::api
{
class RayInteractor;

namespace helper
{
class FSelectorImpl;
class FRayInteractorImpl;
} // namespace helper
} // namespace isdk::api

/**
 * IsdkRayInteractor drives the ability for a pawn to interact with actors that have an
 * IsdkRayInteractable attached to them, initiated by pointing at them with a directed hand raycast
 * or cursor.
 */
UCLASS(
    Blueprintable,
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Ray Interactor"))
class OCULUSINTERACTION_API UIsdkRayInteractor : public UIsdkInteractorComponent
{
  GENERATED_BODY()

 public:
  UIsdkRayInteractor();
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void BeginDestroy() override;
  virtual void TickComponent(
      float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  virtual bool IsApiInstanceValid() const override;
  virtual isdk::api::IInteractor* GetApiIInteractor() const override;
  isdk::api::RayInteractor* GetApiRayInteractor() const;

  /* Attempts to trigger a raycast selection with the given interactor position */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void Select();

  /* Triggers the deselection process */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void Unselect();

  /* Returns the object implementing IIsdkIHandPointerPose that is the current pose used for
   * tracking the hand pointer pose */
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  TScriptInterface<IIsdkIHandPointerPose> GetHandPointerPose() const
  {
    return HandPointerPose;
  }

  /* Returns the last collision surface hit information produced by the interactor raycasting */
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  const FIsdkOptionalSurfaceHit& GetCollisionInfo() const
  {
    return CollisionInfo;
  }

  /* Returns the current ray interactor configuration struct */
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  const FIsdkRayInteractor_Config& GetConfig() const
  {
    return Config;
  }

  /* Sets the object implementing IIsdkIHandPointerPose to be used for tracking hand pointer pose */
  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetHandPointerPose(const TScriptInterface<IIsdkIHandPointerPose>& InHandPointerPose);

  /* Sets the current ray interactor configuration via the passed struct */
  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetConfig(const FIsdkRayInteractor_Config& InConfig);

 protected:
  virtual bool ShouldInteractorBeEnabled() const override;
  virtual const TCHAR* GetInteractorTypeHint() const override
  {
    return InteractorTypeHint;
  }
  static constexpr const TCHAR* InteractorTypeHint = TEXT("ray");

 private:
  // State that need cleaning up when this actor leaves play
  TPimplPtr<isdk::api::helper::FRayInteractorImpl> RayInteractorImpl;
  TPimplPtr<isdk::api::helper::FSelectorImpl> SelectorImpl;

  // Properties
  UPROPERTY(
      BlueprintSetter = SetHandPointerPose,
      BlueprintGetter = GetHandPointerPose,
      EditInstanceOnly,
      Category = InteractionSDK)
  TScriptInterface<IIsdkIHandPointerPose> HandPointerPose;

  UPROPERTY(
      BlueprintGetter = GetConfig,
      BlueprintSetter = SetConfig,
      EditDefaultsOnly,
      Category = InteractionSDK)
  FIsdkRayInteractor_Config Config;

  UPROPERTY(BlueprintGetter = GetCollisionInfo, Category = InteractionSDK)
  FIsdkOptionalSurfaceHit CollisionInfo;

  void DrawDebugVisuals(const FTransform& PointerPose) const;
};
