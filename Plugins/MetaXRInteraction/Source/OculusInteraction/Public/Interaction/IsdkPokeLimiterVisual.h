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
#include "Components/ActorComponent.h"
#include "DataSources/IsdkSyntheticHand.h"
#include "Interaction/IsdkPokeInteractor.h"
#include "DataSources/IsdkHandDataSource.h"
#include "Subsystem/IsdkWorldSubsystem.h"
#include "IsdkPokeLimiterVisual.generated.h"

// Forward declarations of internal types
namespace isdk::api
{
class HandPokeLimiterVisual;

namespace helper
{
class FPokeLimiterVisualImpl;
}
} // namespace isdk::api

/* An ActorComponent that, when present on a PokeInteractable, will limit the hand visual from
 * passing through the surface that has been poked, regardless of how far past the user moves their
 * hand. */
UCLASS(
    Blueprintable,
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Poke Limiter Visual"))
class OCULUSINTERACTION_API UIsdkPokeLimiterVisual : public UActorComponent
{
  GENERATED_BODY()

 public:
  UIsdkPokeLimiterVisual();
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void BeginDestroy() override;

  /* Returns the PokeInteractor currently engaged */
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkPokeInteractor* GetPokeInteractor() const
  {
    return PokeInteractor;
  }
  /* Returns reference to the SyntheticHand being used in the interaction */
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkSyntheticHand* GetSyntheticHand() const
  {
    return SyntheticHand;
  }
  /* Returns the Hand Data source connected to the hand being used in the interaction */
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UIsdkHandDataSource* GetDataSource() const
  {
    return DataSource;
  }

  FIsdkIUpdateEventDelegate& GetUpdatedEventDelegate()
  {
    return Updated;
  }

  /* Sets the PokeInteractor to be engaged with this limiter, will set dependencies after */
  UFUNCTION(BlueprintSetter, BlueprintInternalUseOnly, Category = InteractionSDK)
  void SetPokeInteractor(UIsdkPokeInteractor* InPokeInteractor);

  /* Sets the SyntheticHand to be used in the interaction, will set dependencies after */
  UFUNCTION(BlueprintSetter, BlueprintInternalUseOnly, Category = InteractionSDK)
  void SetSyntheticHand(UIsdkSyntheticHand* InSyntheticHand);

  /* Sets the Hand Data source connected to the hand being used in the interaction, will set
   * dependencies after */
  UFUNCTION(BlueprintSetter, BlueprintInternalUseOnly, Category = InteractionSDK)
  void SetDataSource(UIsdkHandDataSource* InHandDataSource);

 private:
  void SetupImplDependencies();

  // State that need cleaning up when this actor leaves play
  TPimplPtr<isdk::api::helper::FPokeLimiterVisualImpl> PokeLimiterVisualImpl;

  UPROPERTY(
      BlueprintGetter = GetPokeInteractor,
      BlueprintSetter = SetPokeInteractor,
      Category = InteractionSDK)
  UIsdkPokeInteractor* PokeInteractor;
  UPROPERTY(
      BlueprintGetter = GetSyntheticHand,
      BlueprintSetter = SetSyntheticHand,
      Category = InteractionSDK)
  UIsdkSyntheticHand* SyntheticHand;
  UPROPERTY(
      BlueprintGetter = GetDataSource,
      BlueprintSetter = SetDataSource,
      Category = InteractionSDK)
  UIsdkHandDataSource* DataSource;

  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FIsdkIUpdateEventDelegate Updated;
  int64 UpdateEventToken{};
};
