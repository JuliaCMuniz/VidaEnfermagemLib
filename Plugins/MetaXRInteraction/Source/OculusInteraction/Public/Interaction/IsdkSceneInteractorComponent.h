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

#include "StructTypes.h"
#include "Interaction/IsdkIInteractorState.h"
#include "Core/IsdkIGameplayTagContainer.h"

#include "IsdkSceneInteractorComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogISDK, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FSceneInteractorEnabledStateChanged,
    UIsdkSceneInteractorComponent*,
    Sender,
    bool,
    NewState);

UCLASS(
    Blueprintable,
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Scene Interactor"))
class OCULUSINTERACTION_API UIsdkSceneInteractorComponent : public USceneComponent,
                                                            public IIsdkIGameplayTagContainer,
                                                            public IIsdkIInteractorState
{
  GENERATED_BODY()
 public:
  UIsdkSceneInteractorComponent()
  {
    ID = NextID++;
  }

  /* Returns the current state of this interactor, driven by events */
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InteractionSDK|Scene Interactor")
  virtual EIsdkInteractorState GetInteractorState() const override
  {
    return State;
  }

  virtual FIsdkInteractorStateChanged* GetInteractorStateChangedDelegate() override
  {
    return &StateChangedDelegate;
  }

  /* Enables this interactor component, broadcasts the EnabledStateChangedDelegate delegate */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  virtual void Enable()
  {
    if (!Enabled)
    {
      Enabled = true;
      EnabledStateChangedDelegate.Broadcast(this, Enabled);
    }
  }

  /* Disables this interactor component, broadcasts the EnabledStateChangedDelegate delegate */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  virtual void Disable()
  {
    if (Enabled)
    {
      Enabled = false;
      EnabledStateChangedDelegate.Broadcast(this, Enabled);
    }
  }

  /* Returns whether or not this interactor component is currently enabled */
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = InteractionSDK)
  virtual bool IsEnabled()
  {
    return Enabled;
  }

  /* Delegate broadcast when the state of this interactor changes */
  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FIsdkInteractorStateChanged StateChangedDelegate;

  /* Delegate broadcast when this interactor is enabled or disabled */
  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FSceneInteractorEnabledStateChanged EnabledStateChangedDelegate;

  UPROPERTY(BlueprintReadOnly, Category = InteractionSDK)
  int ID;

  virtual bool HasCandidate() const
      PURE_VIRTUAL(UIsdkSceneInteractorComponent::HasCandidate, return false;);
  virtual bool HasInteractable() const
      PURE_VIRTUAL(UIsdkSceneInteractorComponent::HasInteractable, return false;);

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
  /* The current interaction state of this interactable, driven by user events */
  UPROPERTY(BlueprintReadOnly, BlueprintGetter = GetInteractorState, Category = InteractionSDK)
  EIsdkInteractorState State{};

  UPROPERTY(BlueprintReadOnly, BlueprintGetter = IsEnabled, Category = InteractionSDK)
  bool Enabled{false};

  static int NextID;

  virtual void SetState(EIsdkInteractorState newState)
  {
    if (newState != State)
    {
      FIsdkInteractorStateEvent Event{this, {}};
      Event.Args.PreviousState = State;
      Event.Args.NewState = newState;

      State = newState;
      StateChangedDelegate.Broadcast(Event);
    }
  }
};
