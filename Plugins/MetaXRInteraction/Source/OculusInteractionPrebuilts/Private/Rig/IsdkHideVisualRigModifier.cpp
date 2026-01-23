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

#include "Rig/IsdkHideVisualRigModifier.h"
#include "Rig/IsdkRigComponent.h"
#include "Core/IsdkConditionalGroup.h"
#include "Rig/IsdkInteractionGroupRigComponent.h"
#include "Interaction/IsdkGrabberComponent.h"
#include "Interaction/IsdkGrabbableComponent.h"
#include "Interaction/IsdkPokeInteractor.h"
#include "Interaction/IsdkRayInteractor.h"
#include "OculusInteractionPrebuiltsLog.h"
#include "Rig/IsdkTrackedDataSourceRigComponent.h"

void UIsdkHideVisualRigModifier::InitializeRigModifier_Implementation(
    UIsdkRigComponent* CallingRigComponent)
{
  Super::InitializeRigModifier_Implementation(CallingRigComponent);
  if (!IsValid(CallingRigComponent))
  {
    UE_LOG(
        LogOculusInteractionPrebuilts,
        Error,
        TEXT("UIsdkHideVisualRigModifier::InitializeRigModifier - CallingRigComponent is null!"));
    return;
  }

  // Get the Visuals of the Rig Component
  UIsdkTrackedDataSourceRigComponent* VisualComponent = CallingRigComponent->GetVisuals();
  if (!IsValid(VisualComponent))
  {
    UE_LOG(
        LogOculusInteractionPrebuilts,
        Error,
        TEXT(
            "UIsdkHideVisualRigModifier::InitializeRigModifier - UIsdkTrackedDataSourceRigComponent is null!"));
    return;
  }

  // Establish our bool Conditional and add it to the VisualComponent ConditionalGroup (All)
  // This is the conditional we will be potentially updating as Interactor state changes and our
  // filtering is applied
  VisualHideConditional = NewObject<UIsdkConditionalBool>();
  VisualComponent->GetForceOffVisibility()->AddConditional(VisualHideConditional);

  UIsdkInteractionGroupRigComponent* InteractionGroupRig =
      CallingRigComponent->GetInteractionGroup();

  bool bBoundToDelegate = false;

  TMap<UObject*, FIsdkInteractorGroupMember> MemberMap = InteractionGroupRig->GetMemberInfoMap();
  for (const auto& Elem : MemberMap)
  {
    if (IsValid(Elem.Key) && Elem.Key->Implements<UIsdkIInteractorState>())
    {
      auto Interactor = TScriptInterface<IIsdkIInteractorState>(Elem.Key);
      FIsdkInteractorStateChanged& StateChangeDelegate =
          *Interactor->GetInteractorStateChangedDelegate();
      StateChangeDelegate.AddUniqueDynamic(
          this, &UIsdkHideVisualRigModifier::HandleInteractorStateEvent);
      bBoundToDelegate = true;
    }
  }

  if (!bBoundToDelegate)
  {
    UE_LOG(
        LogOculusInteractionPrebuilts,
        Warning,
        TEXT(
            "UIsdkHideVisualRigModifier::InitializeRigModifier - No delegates were bound! This modifier will not trigger."));
  }
}

void UIsdkHideVisualRigModifier::ShutdownRigModifier_Implementation(
    UIsdkRigComponent* CallingRigComponent)
{
  UIsdkInteractionGroupRigComponent* InteractionGroupRig =
      CallingRigComponent->GetInteractionGroup();

  TMap<UObject*, FIsdkInteractorGroupMember> MemberMap = InteractionGroupRig->GetMemberInfoMap();
  for (auto& Elem : MemberMap)
  {
    if (IsValid(Elem.Key) && Elem.Key->Implements<UIsdkIInteractorState>())
    {
      auto Interactor = TScriptInterface<IIsdkIInteractorState>(Elem.Key);
      FIsdkInteractorStateChanged& StateChangeDelegate =
          *Interactor->GetInteractorStateChangedDelegate();
      StateChangeDelegate.RemoveDynamic(
          this, &UIsdkHideVisualRigModifier::HandleInteractorStateEvent);
    }
  }
}

void UIsdkHideVisualRigModifier::HandleInteractorStateEvent(
    const FIsdkInteractorStateEvent& StateEvent)
{
  bool bFinalHideVisualState = false;
  bool bFoundInteractor = false;

  // Ignore any events that involve disabled/disabling interactors, those are noise for us
  if (StateEvent.Args.NewState == EIsdkInteractorState::Disabled)
  {
    return;
  }

  /*
    TODO:
    For Ray & Poke, either per class or via parent IsdkInteractorComponent, get
    InteractableComponent and pass to ShouldModifierTrigger. For now we pass nullptr, which will be
    validated in the next step.
  */
  const auto* Interactor = StateEvent.Interactor.GetObject();

  // Poke
  const UIsdkPokeInteractor* EventPokeComponent = Cast<UIsdkPokeInteractor>(Interactor);
  if (IsValid(EventPokeComponent))
  {
    bFinalHideVisualState = ShouldModifierTrigger(StateEvent, nullptr);
    bFoundInteractor = true;
  }

  // Ray
  if (!bFoundInteractor)
  {
    const UIsdkRayInteractor* EventRayComponent = Cast<UIsdkRayInteractor>(Interactor);
    if (IsValid(EventRayComponent))
    {
      bFinalHideVisualState = ShouldModifierTrigger(StateEvent, nullptr);
      bFoundInteractor = true;
    }
  }

  // Grabbable
  if (!bFoundInteractor)
  {
    const UIsdkGrabberComponent* EventGrabberComponent = Cast<UIsdkGrabberComponent>(Interactor);
    if (IsValid(EventGrabberComponent))
    {
      UIsdkGrabbableComponent* GrabbableComponent = nullptr;
      EventGrabberComponent->GetGrabbingStateWithComponent(GrabbableComponent);
      bFinalHideVisualState = ShouldModifierTrigger(StateEvent, GrabbableComponent);
    }
  }

  // Push the final value
  VisualHideConditional->SetValue(bFinalHideVisualState);
}
