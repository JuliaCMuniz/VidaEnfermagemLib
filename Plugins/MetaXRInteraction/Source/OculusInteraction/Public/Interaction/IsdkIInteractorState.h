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
#include "IsdkIInteractorState.generated.h"

class IIsdkIInteractorState;
class IIsdkIInteractableState;

USTRUCT(Category = InteractionSDK, BlueprintType)
struct OCULUSINTERACTION_API FIsdkInteractorStateEvent
{
  GENERATED_BODY()

  /**
   * The interactor that invoked this event. May be null if the interactor was deleted prior
   * to this event being emitted (a common case is when the interactor deletes itself in response
   * to a Select event - the following Hover/Normal/Disabled events will still be emitted.)
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  TScriptInterface<IIsdkIInteractorState> Interactor{};

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = InteractionSDK)
  FIsdkInteractorStateChangeArgs Args;

  static FIsdkInteractorStateEvent
  Create(UObject* Interactor, EIsdkInteractorState PreviousState, EIsdkInteractorState NewState)
  {
    FIsdkInteractorStateChangeArgs Args;
    Args.PreviousState = PreviousState;
    Args.NewState = NewState;
    return FIsdkInteractorStateEvent{Interactor, Args};
  }
};

UDELEGATE(BlueprintType, Blueprintable, meta = (DisplayName = "Interactor State Changed Delegate"))
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FIsdkInteractorStateChanged,
    const FIsdkInteractorStateEvent&,
    InteractorState);

// This class does not need to be modified.
UINTERFACE()
class OCULUSINTERACTION_API UIsdkIInteractorState : public UInterface
{
  GENERATED_BODY()
};

class OCULUSINTERACTION_API IIsdkIInteractorState
{
  GENERATED_BODY()

 public:
  // Add interface functions to this class. This is the class that will be inherited to implement
  // this interface.
  virtual bool HasSelectedInteractable() const
      PURE_VIRTUAL(IIsdkIStatefulInteractor::HasSelectedInteractable, return false;);

  virtual EIsdkInteractorState GetInteractorState() const
      PURE_VIRTUAL(IIsdkIStatefulInteractor::GetInteractorState, return {};);

  /**
   * Returns a structure summarizing the number of Hover and Select interactor relationships exist
   * on this interactor.
   */
  virtual FIsdkInteractionRelationshipCounts GetInteractorStateRelationshipCounts() const
      PURE_VIRTUAL(IIsdkIStatefulInteractor::GetInteractorStateRelationshipCounts, return {};);

  virtual void GetInteractorStateRelationships(
      EIsdkInteractableState State,
      TArray<TScriptInterface<IIsdkIInteractableState>>& OutInteractables) const
      PURE_VIRTUAL(IIsdkIStatefulInteractor::GetInteractorStateRelationships);

  virtual FIsdkInteractorStateChanged* GetInteractorStateChangedDelegate()
      PURE_VIRTUAL(IIsdkIStatefulInteractor::GetInteractorStateChangedDelegate, return nullptr;);
};
