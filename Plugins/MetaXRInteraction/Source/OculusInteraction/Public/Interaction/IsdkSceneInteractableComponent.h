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
#include "Core/IsdkIGameplayTagContainer.h"

#include "IsdkSceneInteractableComponent.generated.h"

class UIsdkSceneInteractorComponent;

UDELEGATE(BlueprintType, Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FSceneInteractableInteractorCountChanged,
    UIsdkSceneInteractableComponent*,
    Sender,
    int,
    NewCount);

UDELEGATE(BlueprintType, Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FSceneInteractableEnabledStateChanged,
    UIsdkSceneInteractableComponent*,
    Sender,
    bool,
    NewState);

UCLASS(
    Blueprintable,
    ClassGroup = (InteractionSDK),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Scene Interactable"))
class OCULUSINTERACTION_API UIsdkSceneInteractableComponent : public USceneComponent,
                                                              public IIsdkIGameplayTagContainer,
                                                              public IIsdkIInteractableState
{
  GENERATED_BODY()
 public:
  UIsdkSceneInteractableComponent() = default;

  // IsdkIStatefulInteractable implementation
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InteractionSDK|Scene Interactable")
  virtual EIsdkInteractableState GetInteractableState() const override
  {
    return State;
  }

  virtual FIsdkInteractableStateChanged* GetInteractableStateChangedDelegate() override
  {
    return &StateChangedDelegate;
  }

  virtual FIsdkInteractionRelationshipCounts GetInteractableStateRelationshipCounts() const override
  {
    return RelationshipCounts;
  }

  /* Enables this interactable component, broadcasts the EnabledStateChangedDelegate delegate */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  virtual void Enable()
  {
    if (!Enabled)
    {
      Enabled = true;
      EnabledStateChangedDelegate.Broadcast(this, Enabled);
    }
  }

  /* Disables this interactable component, broadcasts the EnabledStateChangedDelegate delegate */
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  virtual void Disable()
  {
    if (Enabled)
    {
      Enabled = false;
      EnabledStateChangedDelegate.Broadcast(this, Enabled);
    }
  }

  /* Returns whether or not this interactable component is currently enabled */
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = InteractionSDK)
  virtual bool IsEnabled()
  {
    return Enabled;
  }

  virtual void AddInteractor(UIsdkSceneInteractorComponent* interactor)
  {
    Interactors.Add(interactor);
    InteractorCountChangedDelegate.Broadcast(this, Interactors.Num());
  }

  virtual void RemoveInteractor(UIsdkSceneInteractorComponent* interactor)
  {
    Interactors.Remove(interactor);
    InteractorCountChangedDelegate.Broadcast(this, Interactors.Num());
  }

  /* Delegate broadcast when the state of this interactable changes */
  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FIsdkInteractableStateChanged StateChangedDelegate;

  /* Delegate broadcast when the number of interactors engaged with this interactable changes */
  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FSceneInteractableInteractorCountChanged InteractorCountChangedDelegate;

  /* Delegate broadcast when this interactable becomes enabled or disabled */
  UPROPERTY(BlueprintAssignable, Category = InteractionSDK)
  FSceneInteractableEnabledStateChanged EnabledStateChangedDelegate;

  // GameplayTags used for labeling and driving interaction behavior. Returned by
  // IIsdkIGameplayTagContainer::GetGameplayTagContainer().
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = InteractionSDK)
  FGameplayTagContainer InteractableTags;

  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  bool GetGameplayTagContainer_Implementation(FGameplayTagContainer& ContainerOut) override
  {
    ContainerOut = InteractableTags;
    return !ContainerOut.IsEmpty();
  };

 protected:
  /* The current interaction state of this interactable, driven by interactor events */
  UPROPERTY(BlueprintReadOnly, BlueprintGetter = GetInteractableState, Category = InteractionSDK)
  EIsdkInteractableState State{};

  UPROPERTY(BlueprintReadOnly, BlueprintGetter = IsEnabled, Category = InteractionSDK)
  bool Enabled{};

  virtual bool CanChangeStateTo(EIsdkInteractableState NewState)
  {
    return true;
  }

  virtual bool SetStateStep(EIsdkInteractableState NewState)
  {
    if (CanChangeStateTo(NewState))
    {
      if (NewState != State)
      {
        switch (State)
        {
          case EIsdkInteractableState::Normal:
            break;
          case EIsdkInteractableState::Hover:
            if (NewState == EIsdkInteractableState::Normal)
            {
              RelationshipCounts.NumHover--;
              if (RelationshipCounts.NumHover > 0)
              {
                return false;
              }
            }
            break;
          case EIsdkInteractableState::Select:
            RelationshipCounts.NumSelect--;
            if (RelationshipCounts.NumSelect > 0)
            {
              return false;
            }
            break;
          case EIsdkInteractableState::Disabled:
            break;
        }

        switch (NewState)
        {
          case EIsdkInteractableState::Normal:
            break;
          case EIsdkInteractableState::Hover:
            if (State == EIsdkInteractableState::Normal)
            {
              RelationshipCounts.NumHover++;
            }
            break;
          case EIsdkInteractableState::Select:
            if (State == EIsdkInteractableState::Hover)
            {
              RelationshipCounts.NumSelect++;
            }
            break;
          case EIsdkInteractableState::Disabled:
            break;
        }

        const auto Args = FIsdkInteractableStateEvent::Create(this, State, NewState);
        State = NewState;
        StateChangedDelegate.Broadcast(Args);
      }
    }

    return true;
  }

  virtual void SetState(EIsdkInteractableState NewState)
  {
    if (NewState == State)
    {
      return;
    }

    if (NewState > State)
    {
      for (int iState = static_cast<int>(State) + 1; iState <= static_cast<int>(NewState); iState++)
      {
        if (!SetStateStep(static_cast<EIsdkInteractableState>(iState)))
          break;
      }
    }
    else
    {
      for (int iState = static_cast<int>(State) - 1; iState >= static_cast<int>(NewState); iState--)
      {
        if (!SetStateStep(static_cast<EIsdkInteractableState>(iState)))
          break;
      }
    }
  }

  TArray<UIsdkSceneInteractorComponent*> Interactors{};
  FIsdkInteractionRelationshipCounts RelationshipCounts{};
};
