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
#include "UObject/Interface.h"
#include "StructTypes.h"
#include "IsdkIInteractableState.generated.h"

class IIsdkIInteractableState;
class IIsdkIInteractorState;

USTRUCT(Category = InteractionSDK, BlueprintType)
struct OCULUSINTERACTION_API FIsdkInteractableStateEvent
{
  GENERATED_BODY()

  /**
   * The interactable that received this event. May be null if the interactable was deleted prior
   * to this event being emitted (a common case is when the interactable is deleted in response
   * to a Select event - the following Hover/Normal/Disabled events will still be emitted.)
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  TScriptInterface<IIsdkIInteractableState> Interactable{};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  FIsdkInteractableStateChangeArgs Args;

  static FIsdkInteractableStateEvent Create(
      const TScriptInterface<IIsdkIInteractableState>& Interactable,
      EIsdkInteractableState PreviousState,
      EIsdkInteractableState NewState)
  {
    FIsdkInteractableStateChangeArgs Args;
    Args.PreviousState = PreviousState;
    Args.NewState = NewState;
    return FIsdkInteractableStateEvent{Interactable, Args};
  }
};

UDELEGATE(
    BlueprintType,
    Blueprintable,
    meta = (DisplayName = "Interactable State Changed Delegate"))
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FIsdkInteractableStateChanged,
    const FIsdkInteractableStateEvent&,
    InteractableState);

// This class does not need to be modified.
UINTERFACE()
class OCULUSINTERACTION_API UIsdkIInteractableState : public UInterface
{
  GENERATED_BODY()
};

class OCULUSINTERACTION_API IIsdkIInteractableState
{
  GENERATED_BODY()

 public:
  /**
   * The current state of this interactable.
   * If there is no interactor relationship, this will be either Disabled or Normal.
   * If there is a single interactor relationship, this will be either Hover or Select.
   * If there are multiple interactor relationship, this will be Select if at least one of those is
   *     a 'Select' relationship; otherwise Hover.
   */
  virtual EIsdkInteractableState GetInteractableState() const
      PURE_VIRTUAL(IIsdkIStatefulInteractable::GetInteractableState, return {};);

  /**
   * Returns a structure summarizing the number of Hover and Select interactor relationships exist
   * on this interactor.
   */
  virtual FIsdkInteractionRelationshipCounts GetInteractableStateRelationshipCounts() const
      PURE_VIRTUAL(IIsdkIStatefulInteractable::GetInteractableStateRelationshipCounts, return {};);

  virtual void GetInteractableStateRelationships(
      EIsdkInteractableState State,
      TArray<TScriptInterface<IIsdkIInteractorState>>& OutInteractors) const
      PURE_VIRTUAL(IIsdkIStatefulInteractable::GetInteractableStateRelationships);

  virtual FIsdkInteractableStateChanged* GetInteractableStateChangedDelegate()
      PURE_VIRTUAL(IIsdkIStatefulInteractable::GetInteractableStateChangedDelegate,
                   return nullptr;);
};
