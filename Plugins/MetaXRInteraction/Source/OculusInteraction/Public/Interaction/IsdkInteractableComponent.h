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
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "IsdkIInteractableState.h"

#include "StructTypes.h"
#include "IsdkIInteractableState.h"
#include "Core/IsdkIGameplayTagContainer.h"

#include "IsdkInteractableComponent.generated.h"

// Forward declarations of internal types
namespace isdk::api
{
class IInteractable;
class PointerEventQueue;
} // namespace isdk::api

UCLASS(Abstract)
class OCULUSINTERACTION_API UIsdkInteractableComponent : public USceneComponent,
                                                         public IIsdkIGameplayTagContainer,
                                                         public IIsdkIInteractableState
{
  GENERATED_BODY()
 public:
  UIsdkInteractableComponent();
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void BeginDestroy() override;

  /* IsdkIStatefulInteractable implementation */
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InteractionSDK|Scene Interactable")
  virtual EIsdkInteractableState GetInteractableState() const override
  {
    return GetCurrentState();
  }

  virtual FIsdkInteractableStateChanged* GetInteractableStateChangedDelegate() override
  {
    return &InteractableStateChanged;
  }

  /* Returns the current state of this interactable */
  UFUNCTION(BlueprintPure, BlueprintInternalUseOnly, Category = InteractionSDK)
  EIsdkInteractableState GetCurrentState() const;

  virtual FIsdkInteractionRelationshipCounts GetInteractableStateRelationshipCounts()
      const override;

  virtual void GetInteractableStateRelationships(
      EIsdkInteractableState State,
      TArray<TScriptInterface<IIsdkIInteractorState>>& OutInteractors) const override;

  virtual void TickComponent(
      float DeltaTime,
      enum ELevelTick TickType,
      FActorComponentTickFunction* ThisTickFunction) override;
  virtual void Activate(bool bReset) override;
  virtual void Deactivate() override;

  /* GameplayTags used for labeling and driving interaction behavior. Returned by
   IIsdkIGameplayTagContainer::GetGameplayTagContainer(). */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = InteractionSDK)
  FGameplayTagContainer InteractableTags;

  /* IIsdkIGameplayTagContainer Implementation */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  bool GetGameplayTagContainer_Implementation(FGameplayTagContainer& ContainerOut) override
  {
    ContainerOut = InteractableTags;
    return !ContainerOut.IsEmpty();
  };

 protected:
  virtual void OnVisibilityChanged() override;

  void UpdateInteractableEnabled();
  virtual bool ShouldApiInstanceBeEnabled() const;

  virtual bool IsApiInstanceValid() const
  {
    return false;
  }

  virtual isdk::api::IInteractable* GetApiIInteractable() const
      PURE_VIRTUAL(UIsdkInteractableComponent::GetApiIInteractable, return nullptr;);

  virtual void HandleApiInstanceCreated(isdk::api::IInteractable* ApiInstance);

  /* Current Interactable State */
  UPROPERTY(BlueprintReadOnly, BlueprintGetter = GetCurrentState, Category = InteractionSDK)
  EIsdkInteractableState CurrentState = EIsdkInteractableState::Disabled;
  int64 InteractableStateEventToken{};

  /* Delegate broadcast when the interactable state changes */
  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FIsdkInteractableStateChanged InteractableStateChanged;
};
