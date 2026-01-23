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

#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IsdkInteractionEvents.h"

#include "StructTypes.h"
#include "Interaction/IsdkIInteractorState.h"
#include "Core/IsdkIGameplayTagContainer.h"

#include "IsdkInteractorComponent.generated.h"

// Forward declarations of internal types
namespace isdk::api
{
class IInteractor;
class InteractorPayload;
class IActiveState;

namespace helper
{
class FInteractorPayloadImpl;
class FInteractorActiveStateImpl;
} // namespace helper
} // namespace isdk::api

UCLASS(Abstract)
class OCULUSINTERACTION_API UIsdkInteractorComponent : public USceneComponent,
                                                       public IIsdkIGameplayTagContainer,
                                                       public IIsdkIInteractorState
{
  GENERATED_BODY()

 public:
  UIsdkInteractorComponent();
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void BeginDestroy() override;
  virtual void TickComponent(
      float DeltaTime,
      ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;
  virtual void Activate(bool bReset = false) override;
  virtual void Deactivate() override;

  virtual isdk::api::InteractorPayload* GetApiInteractorPayload();

  // IsdkIStatefulInteractor implementation
  virtual EIsdkInteractorState GetInteractorState() const override
  {
    return GetCurrentState();
  }
  virtual FIsdkInteractionRelationshipCounts GetInteractorStateRelationshipCounts() const override;
  virtual void GetInteractorStateRelationships(
      EIsdkInteractableState State,
      TArray<TScriptInterface<IIsdkIInteractableState>>& OutInteractables) const override;
  virtual FIsdkInteractorStateChanged* GetInteractorStateChangedDelegate() override
  {
    return &InteractorStateChanged;
  }
  virtual bool HasSelectedInteractable() const override;

  // Getters
  UFUNCTION(BlueprintPure, BlueprintInternalUseOnly, Category = InteractionSDK)
  EIsdkInteractorState GetCurrentState() const;
  FIsdkIUpdateEventDelegate& GetUpdatedEventDelegate()
  {
    return Updated;
  }

  bool HasCandidate() const;
  bool HasInteractable() const;

  // GameplayTags used for labeling and driving interaction behavior. Returned by
  // IIsdkIGameplayTagContainer::GetGameplayTagContainer().
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = InteractionSDK)
  FGameplayTagContainer InteractorTags;

  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  bool GetGameplayTagContainer_Implementation(FGameplayTagContainer& ContainerOut) override
  {
    ContainerOut = InteractorTags;
    return !ContainerOut.IsEmpty();
  };

 protected:
  virtual isdk::api::IActiveState* GetApiIActiveState();
  void UpdateInteractorEnabled();
  void SetInteractorEnabled(bool IsEnabled);
  virtual bool ShouldInteractorBeEnabled() const;
  virtual const TCHAR* GetInteractorTypeHint() const
      PURE_VIRTUAL(UIsdkInteractorComponent::GetInteractorTypeHint, return nullptr;);

  virtual bool IsApiInstanceValid() const
  {
    return false;
  }

  virtual isdk::api::IInteractor* GetApiIInteractor() const
      PURE_VIRTUAL(UIsdkInteractorComponent::GetApiIInteractor, return nullptr;);

  UPROPERTY(
      BlueprintReadOnly,
      BlueprintGetter = GetCurrentState,
      Category = InteractionSDK,
      meta = (ToolTip = "The current state of the interactor."))
  EIsdkInteractorState CurrentState;

 private:
  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FIsdkInteractorStateChanged InteractorStateChanged;

  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FIsdkIUpdateEventDelegate Updated;

  TPimplPtr<isdk::api::helper::FInteractorPayloadImpl> InteractorPayloadImpl;
  TPimplPtr<isdk::api::helper::FInteractorActiveStateImpl> InteractorActiveStateImpl;

  int64 InteractorStateEventToken;
  int64 UpdateEventToken;
};
