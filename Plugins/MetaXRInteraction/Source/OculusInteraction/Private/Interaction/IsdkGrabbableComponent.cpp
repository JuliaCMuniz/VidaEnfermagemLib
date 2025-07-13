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

#include "Interaction/IsdkGrabbableComponent.h"
#include "Interaction/Grabbable/IsdkGrabFreeTransformer.h"
#include "Subsystem/IsdkWorldSubsystem.h"
#include "CoreMinimal.h"
#include "IsdkRuntimeSettings.h"
#include "OculusInteractionLog.h"
#include "Components/ShapeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Interaction/IsdkGrabberComponent.h"
#include "Utilities/IsdkDebugUtils.h"
#include "VisualLogger/VisualLogger.h"
#include "DrawDebugHelpers.h"

namespace isdk
{
extern TAutoConsoleVariable<bool> CVar_Meta_InteractionSDK_DebugInteractionVisuals;
}

UIsdkGrabbableComponent::UIsdkGrabbableComponent()
{
  // Component tick is used only for debug purposes.  Disable for non-editor builds.
#if WITH_EDITOR
  PrimaryComponentTick.bCanEverTick = true;
#else
  PrimaryComponentTick.bCanEverTick = false;
#endif

  // Prioritize pre physics in order to reset velocities when releasing before physics solver to
  // avoid weird velocity issues
  PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
  InteractableTags.AddTag(IsdkGameplayTags::TAG_Isdk_Type_Interactable_Grab);
}

void UIsdkGrabbableComponent::BeginPlay()
{
  SelectedGrabbers.Empty();
  HoveredGrabbers.Empty();

  DefaultCollider = Cast<UPrimitiveComponent>(ColliderComponent);

  if (DefaultCollider == nullptr)
  {
    FindCollider();
  }

  // Set custom GrabCollider and PhysicsCollider if provided, otherwise use the default collider
  GrabCollider = Cast<UPrimitiveComponent>(GrabColliderReference.GetComponent(GetOwner()));
  PhysicsCollider = Cast<UPrimitiveComponent>(PhysicsColliderReference.GetComponent(GetOwner()));

  if (IsValid(DefaultCollider))
  {
    if (!IsValid(GrabCollider))
    {
      GrabCollider = DefaultCollider;
    }
    if (!IsValid(PhysicsCollider))
    {
      PhysicsCollider = DefaultCollider;
    }
  }

  if (GrabCollider)
  {
    GrabCollider->SetGenerateOverlapEvents(true);
  }

  // This is just for debugging, so let's turn it off in non-editor builds
#if WITH_EDITOR && !WITH_DEV_AUTOMATION_TESTS
  InteractionPointerEvent.AddDynamic(this, &UIsdkGrabbableComponent::HandlePointerEvent);
#endif

  Super::BeginPlay();

  if (!IsValid(Grabbable.GetObject()))
  {
    auto Components = GetOwner()->GetComponents();
    for (UActorComponent* Component : Components)
    {
      if (Component->GetFName() == GrabbableName)
      {
        SetGrabbable(Component);
        break;
      }
    }
  }
}

void UIsdkGrabbableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
}

void UIsdkGrabbableComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* TickFn)
{
  Super::TickComponent(DeltaTime, TickType, TickFn);

  if (!isdk::CVar_Meta_InteractionSDK_DebugInteractionVisuals.GetValueOnAnyThread())
    return;

  const FColor DebugColor = UIsdkDebugUtils::GetInteractableStateDebugColor(State);

  FBox BoundsBox = GetCollider()->Bounds.GetBox();
  DrawDebugBox(GetWorld(), BoundsBox.GetCenter(), BoundsBox.GetExtent(), DebugColor, false);
  UE_VLOG_WIREBOX(GetOwner(), LogOculusInteraction, Verbose, BoundsBox, DebugColor, TEXT_EMPTY);
}

void UIsdkGrabbableComponent::GetInteractableStateRelationships(
    EIsdkInteractableState InState,
    TArray<TScriptInterface<IIsdkIInteractorState>>& OutInteractors) const
{
  for (auto* Interactor : Interactors)
  {
    auto* Grabber = Cast<UIsdkGrabberComponent>(Interactor);
    const bool bIsSelected = IsValid(Grabber) && IsSelectedBy(Grabber);

    const bool bWantsHovered = InState == EIsdkInteractableState::Hover;
    const bool bWantsSelected = InState == EIsdkInteractableState::Select;
    if ((!bIsSelected && bWantsHovered) || (bIsSelected && bWantsSelected))
    {
      OutInteractors.Add(Interactor);
    }
  }
}

void UIsdkGrabbableComponent::PostEvent(const FIsdkInteractionPointerEvent& Event)
{
  const auto& Grabber = Cast<UIsdkGrabberComponent>(Event.Interactor);
  if (!IsValid(Grabber))
  {
    return;
  }

  switch (Event.Type)
  {
    //----- HOVER
    case EIsdkPointerEventType::Hover:
      if (!IsHoveredBy(Grabber))
      {
        HoveredGrabbers.Add(Grabber);
      }
      break;
    case EIsdkPointerEventType::Unhover:
      if (IsHoveredBy(Grabber))
      {
        HoveredGrabbers.Remove(Grabber);
      }
      break;
    //----- SELECT
    case EIsdkPointerEventType::Select:
      if (!IsSelectedBy(Grabber))
      {
        SelectedGrabbers.Add(Grabber);
      }
      break;
    case EIsdkPointerEventType::Unselect:
      if (IsSelectedBy(Grabber))
      {
        SelectedGrabbers.Remove(Grabber);
      }
      break;
    //----- CANCEL
    case EIsdkPointerEventType::Cancel:
      if (IsHoveredBy(Grabber))
      {
        HoveredGrabbers.Remove(Grabber);
      }
      if (IsSelectedBy(Grabber))
      {
        SelectedGrabbers.Remove(Grabber);
      }
      break;
    default:
      break;
  }

  if (SelectedGrabbers.Num() > 0)
  {
    SetState(EIsdkInteractableState::Select);
  }
  else if (HoveredGrabbers.Num() > 0)
  {
    SetState(EIsdkInteractableState::Hover);
  }
  else
  {
    SetState(EIsdkInteractableState::Normal);
  }

  InteractionPointerEvent.Broadcast(Event);
  if (IsValid(Grabbable.GetObject()))
  {
    Grabbable->ProcessPointerEvent(Event);
  }
}

UPrimitiveComponent* UIsdkGrabbableComponent::GetCollider(const bool bForGrabbing)
{
  if (!GrabCollider)
  {
    FindCollider();
  }

  return bForGrabbing ? GrabCollider : PhysicsCollider;
}

void UIsdkGrabbableComponent::SetGrabCollider(UPrimitiveComponent* NewGrabCollider)
{
  if (IsValid(NewGrabCollider))
  {
    GrabCollider = NewGrabCollider;
  }
}

void UIsdkGrabbableComponent::SetPhysicsCollider(UPrimitiveComponent* NewPhysicsCollider)
{
  if (IsValid(NewPhysicsCollider))
  {
    PhysicsCollider = NewPhysicsCollider;
  }
}

void UIsdkGrabbableComponent::SetGrabbable(TScriptInterface<IIsdkIGrabbable> NewGrabbable)
{
  if (IsValid(Grabbable.GetObject()))
  {
    for (const auto& Interactor : SelectedGrabbers)
    {
      auto Event =
          FIsdkInteractionPointerEvent::CreateCancelEvent(Interactor->ID, Interactor, this);
      Grabbable->ProcessPointerEvent(Event);
    }
    for (const auto& Interactor : HoveredGrabbers)
    {
      auto Event =
          FIsdkInteractionPointerEvent::CreateCancelEvent(Interactor->ID, Interactor, this);
      Grabbable->ProcessPointerEvent(Event);
    }
  }
  Grabbable = NewGrabbable;
}

void UIsdkGrabbableComponent::SetState(EIsdkInteractableState NewState)
{
  const auto oldState = GetInteractableState();
  if (oldState == NewState)
  {
    return;
  }
  Super::SetState(NewState);
}

UPrimitiveComponent* UIsdkGrabbableComponent::FindCollider()
{
  bool Found{false};
  UStaticMeshComponent* FoundMesh{nullptr};

  if (const auto Root = GetOwner()->GetRootComponent())
  {
    TArray<USceneComponent*> Candidates;
    Root->GetChildrenComponents(true, Candidates);
    Candidates.Add(Root);
    for (const auto Candidate : Candidates)
    {
      if (UShapeComponent* Shape = Cast<UShapeComponent>(Candidate))
      {
        SetCollider(Shape);
        Found = true;
        break;
      }
      else if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Candidate))
      {
        FoundMesh = Mesh;
      }
    }
  }

  // create and register collision if not provided
  if (!Found && FoundMesh)
  {
    // no collider in our actor?  Fine then, we'll use the whole static mesh
    FoundMesh->UpdateCollisionFromStaticMesh();
    SetCollider(FoundMesh);
  }

  if (DefaultCollider)
  {
    DefaultCollider->SetMobility(EComponentMobility::Movable);
  }

  return DefaultCollider;
}

void UIsdkGrabbableComponent::HandlePointerEvent(const FIsdkInteractionPointerEvent& PointerEvent)
{
  // Only debug draw if the appropriate cvar is on
  if (!isdk::CVar_Meta_InteractionSDK_DebugInteractionVisuals.GetValueOnAnyThread())
  {
    return;
  }

  // Don't draw move pointer events, they're too noisy
  if (PointerEvent.Type == EIsdkPointerEventType::Move)
  {
    return;
  }

  const FColor DebugColor = UIsdkDebugUtils::GetPointerEventDebugColor(PointerEvent.Type);

  const auto DebugLocation = FVector(PointerEvent.Pose.Position);
  const auto DebugRadius = GetDefault<UIsdkRuntimeSettings>()->PointerEventDebugRadius;
  const auto DebugDuration = GetDefault<UIsdkRuntimeSettings>()->PointerEventDebugDuration;
  DrawDebugSphere(GetWorld(), DebugLocation, DebugRadius, 12, DebugColor, false, DebugDuration);
  UE_VLOG_SPHERE(
      GetOwner(), LogOculusInteraction, Log, DebugLocation, DebugRadius, DebugColor, TEXT_EMPTY);
  UE_VLOG(
      GetOwner(),
      LogOculusInteraction,
      Log,
      TEXT("PointerEvent\nType: %s\nInteractor: %s\nInteractable: %s\nLocation: %s"),
      *UEnum::GetValueAsString(PointerEvent.Type),
      *GetFullNameSafe(PointerEvent.Interactor),
      *GetFullNameSafe(PointerEvent.Interactable.GetObject()),
      *PointerEvent.Pose.Position.ToString());
}
