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
#include "DataSources/IsdkIRootPose.h"

#include "IsdkPokeInteractor.generated.h"

// Forward declarations of internal types
namespace isdk::api
{
class PokeInteractor;

namespace helper
{
class FPokeInteractorImpl;
}
} // namespace isdk::api

/**
 * IsdkPokeInteractor drives the ability for a pawn to interact with actors that have an
 * IsdkPokeInteractable attached to them, initiated by a single finger non-physics interaction.
 */
UCLASS(
    Blueprintable,
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Poke Interactor"))
class OCULUSINTERACTION_API UIsdkPokeInteractor : public UIsdkInteractorComponent
{
  GENERATED_BODY()

 public:
  UIsdkPokeInteractor();
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void BeginDestroy() override;
  virtual void TickComponent(
      float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;

  virtual bool IsApiInstanceValid() const override;
  virtual isdk::api::IInteractor* GetApiIInteractor() const override;
  isdk::api::PokeInteractor* GetApiPokeInteractor() const;

  /* Returns the root pose object (implementing IIsdkIRootPose) being utilized by this
   * PokeInteractor, not guaranteed to be valid if one has not been set. */
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  TScriptInterface<IIsdkIRootPose> GetRootPose() const
  {
    return RootPose;
  }
  /* Returns a struct with the current threshold and radius configurations for this PokeInteractor
   */
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  const FIsdkPokeInteractor_Config& GetConfig() const
  {
    return Config;
  }

  /* Sets the root pose object (implementing IIsdkIRootPose) to be utilized by this PokeInteractor
   * (does not currently validate). */
  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetRootPose(const TScriptInterface<IIsdkIRootPose>& InRootPose);

  /* Overwrites the current threshold/radius configuration with the given struct */
  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetConfig(const FIsdkPokeInteractor_Config& InConfig);

 protected:
  virtual bool ShouldInteractorBeEnabled() const override;
  virtual const TCHAR* GetInteractorTypeHint() const override
  {
    return InteractorTypeHint;
  }
  static constexpr const TCHAR* InteractorTypeHint = TEXT("poke");

 private:
  bool IsRootPoseValid() const;
  void SetPointTransform(const FTransform& Transform);

  TPimplPtr<isdk::api::helper::FPokeInteractorImpl> PokeInteractorImpl;

  // Properties
  UPROPERTY(
      BlueprintGetter = GetConfig,
      BlueprintSetter = SetConfig,
      EditDefaultsOnly,
      Category = InteractionSDK)
  FIsdkPokeInteractor_Config Config;

  UPROPERTY(
      BlueprintSetter = SetRootPose,
      BlueprintGetter = GetRootPose,
      Category = InteractionSDK,
      EditInstanceOnly,
      meta = (ExposeOnSpawn = true))
  TScriptInterface<IIsdkIRootPose> RootPose;

  void DrawDebugVisuals() const;
};
