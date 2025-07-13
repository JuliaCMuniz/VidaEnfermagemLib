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

#include "Interaction/IsdkGrabberComponent.h"
#include "CoreMinimal.h"
#include <Components/SphereComponent.h>

#include "IsdkRuntimeSettings.h"
#include "OculusInteractionLog.h"
#include "Components/BoxComponent.h"
#include "Interaction/IsdkGrabbableComponent.h"
#include "Subsystem/IsdkWorldSubsystem.h"
#include "Utilities/IsdkDebugUtils.h"
#include "VisualLogger/VisualLogger.h"
#include "DrawDebugHelpers.h"

namespace isdk
{
extern TAutoConsoleVariable<bool> CVar_Meta_InteractionSDK_DebugInteractionVisuals;
}

UIsdkGrabberComponent::UIsdkGrabberComponent()
{
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
  InteractorTags.AddTag(IsdkGameplayTags::TAG_Isdk_Type_Interactor_Grab);

  CollidersByType.Emplace(EIsdkGrabColliderType::Custom, {});
  CollidersByType.Emplace(EIsdkGrabColliderType::Palm, {});
  CollidersByType.Emplace(EIsdkGrabColliderType::Pinch, {});
  CollidersByType.Emplace(EIsdkGrabColliderType::Unknown, {});
}

void UIsdkGrabberComponent::ComputeBestGrabbableColliderForGrabType(
    EIsdkGrabColliderType Type,
    UPrimitiveComponent*& MyCollider,
    UPrimitiveComponent*& GrabbableCollider,
    UIsdkGrabbableComponent*& Grabbable)
{
  UPrimitiveComponent* Best = nullptr;
  UIsdkGrabbableComponent* BestGrabbable = nullptr;
  UPrimitiveComponent* MyBestCollider = nullptr;
  float BestDist = FLT_MAX;
  TArray<UPrimitiveComponent*> MyColliders;
  GetCollidersByType(Type, MyColliders);
  for (const auto ACollider : MyColliders)
  {
    FIsdkColliderInfo Info;
    GetColliderInfo(ACollider, Type, Info);
    for (const TTuple<UPrimitiveComponent*, UIsdkGrabbableComponent*> HoverObject :
         Info.HoverObjects)
    {
      const auto ThisHoverCollider = HoverObject.Key;
      if (!Best)
      {
        Best = ThisHoverCollider;
        BestGrabbable = HoverObject.Value;
        MyBestCollider = ACollider;
      }
      else
      {
        const float dist =
            (ACollider->GetComponentLocation() - ThisHoverCollider->GetComponentLocation())
                .SizeSquared();
        if (dist < BestDist)
        {
          BestDist = dist;
          Best = ThisHoverCollider;
          BestGrabbable = HoverObject.Value;
          MyBestCollider = ACollider;
        }
      }
    }
  }

  MyCollider = MyBestCollider;
  GrabbableCollider = Best;
  Grabbable = BestGrabbable;
}

void UIsdkGrabberComponent::BeginPlay()
{
  Super::BeginPlay();

  if (!HasCollidersOfType(EIsdkGrabColliderType::Palm))
  {
    DefaultPalmCollider = NewObject<USphereComponent>(this, TEXT("Palm Collider"));
    DefaultPalmCollider->InitSphereRadius(PalmColliderRadius);
    DefaultPalmCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    DefaultPalmCollider->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    DefaultPalmCollider->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    DefaultPalmCollider->SetMobility(EComponentMobility::Movable);
    DefaultPalmCollider->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
    DefaultPalmCollider->RegisterComponent();
    AddCollider(DefaultPalmCollider, EIsdkGrabColliderType::Palm);
  }

  if (!HasCollidersOfType(EIsdkGrabColliderType::Pinch))
  {
    USphereComponent* Sphere = NewObject<USphereComponent>(this, TEXT("Pinch Collider"));
    Sphere->InitSphereRadius(PinchColliderRadius);
    Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Sphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    Sphere->SetMobility(EComponentMobility::Movable);
    Sphere->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
    Sphere->RegisterComponent();
    AddCollider(Sphere, EIsdkGrabColliderType::Pinch);
  }

  SetMobility(EComponentMobility::Movable);
}

void UIsdkGrabberComponent::UnregisterAllColliders()
{
  // We can't modify the map as we iterate it - so take a copy of all colliders we want to
  // unregister, to do it in a second pass.
  TArray<TPair<UPrimitiveComponent*, EIsdkGrabColliderType>> ToRemove;
  for (const auto& TypeIT : CollidersByType)
  {
    const auto Type = TypeIT.Key;
    const auto& CollidersToInfosMap = TypeIT.Value;
    for (const auto& It : CollidersToInfosMap)
    {
      UPrimitiveComponent* Collider = It.Key;
      ToRemove.Add({Collider, Type});
    }
  }

  for (const auto& ToRemovePair : ToRemove)
  {
    UnregisterCollider(ToRemovePair.Key, ToRemovePair.Value);
  }
}

void UIsdkGrabberComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  UnregisterAllColliders();
}

void UIsdkGrabberComponent::BeginDestroy()
{
  Super::BeginDestroy();
}

void UIsdkGrabberComponent::Deactivate()
{
  UnselectPalm();
  UnselectPinch();
  Super::Deactivate();
}

bool UIsdkGrabberComponent::HasCandidate() const
{
  return IsValid(CandidateGrabbable);
}

bool UIsdkGrabberComponent::HasInteractable() const
{
  return State == EIsdkInteractorState::Hover;
}

bool UIsdkGrabberComponent::HasSelectedInteractable() const
{
  return State == EIsdkInteractorState::Select;
}

UIsdkGrabbableComponent* FindGrabbableInActor(AActor* Actor)
{
  if (auto* Grabbable = Actor->GetComponentByClass(UIsdkGrabbableComponent::StaticClass()))
  {
    return Cast<UIsdkGrabbableComponent>(Grabbable);
  }

  return nullptr;
}

void UIsdkGrabberComponent::BeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
  FIsdkColliderInfo Info;

  auto Type = FindColliderType(OverlappedComponent);
  // first make sure it's one of the collider components we care about and we've got a legit other
  // component
  if (IsValid(OtherComp) && GetColliderInfo(OverlappedComponent, Type, Info))
  {
    if (UIsdkGrabbableComponent* Grabbable = FindGrabbableInActor(OtherComp->GetOwner()))
    {
      if (OtherComp == Grabbable->GetCollider(true)) // Use grab collider
      {
        if (!Grabbable->AllowsGrabType(Type))
        {
          return;
        }

        if (Type == EIsdkGrabColliderType::Palm && !bAllowPalmGrab)
        {
          return;
        }

        if (Type == EIsdkGrabColliderType::Pinch && !bAllowPinchGrab)
        {
          return;
        }

        if (!Info.HoverObjects.Contains(OtherComp))
        {
          Info.HoverObjects.Add(OtherComp, Grabbable);
        }
        Info.RankIndex = NextRankIndex++;
        if (Info.State == EIsdkInteractorState::Normal)
        {
          Info.State = EIsdkInteractorState::Hover;
        }
        SetColliderInfo(OverlappedComponent, Type, Info);
        PostEvent(EIsdkPointerEventType::Hover, Grabbable);

        UPrimitiveComponent* MyCollider;
        UPrimitiveComponent* BestCollider;
        UIsdkGrabbableComponent* BestGrabbable;
        ComputeBestGrabbableColliderForGrabType(Type, MyCollider, BestCollider, BestGrabbable);
        CandidateGrabbable = BestGrabbable;

        UpdateState();
      }
    }
  }

  UpdateOverlappedSet();
}

void UIsdkGrabberComponent::EndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
  FIsdkColliderInfo Info;

  auto Type = FindColliderType(OverlappedComponent);
  // first make sure it's one of the collider components we care about
  if (!GetColliderInfo(OverlappedComponent, Type, Info))
  {
    return;
  }

  // have we already started tracking this particular component of the grabbable's?
  if (UIsdkGrabbableComponent** Grabbable = Info.HoverObjects.Find(OtherComp))
  {
    if (IsValid(*Grabbable))
    {
      PostEvent(EIsdkPointerEventType::Unhover, *Grabbable);
    }
    Info.HoverObjects.Remove(OtherComp);
    if (Info.HoverObjects.IsEmpty() && Info.State != EIsdkInteractorState::Select)
    {
      Info.State = EIsdkInteractorState::Normal;
    }
    SetColliderInfo(OverlappedComponent, Type, Info);
  }

  if (IsValid(OtherComp))
  {
    UIsdkGrabbableComponent* Grabbable = FindGrabbableInActor(OtherComp->GetOwner());
    if (Grabbable)
    {
      // if hand moves too fast it can move outside the currently grabbed object, so check here to
      // see if we're currently grabbing something
      if (State != EIsdkInteractorState::Select)
      {
        if (Grabbable == Info.SelectObject)
        {
          Info.SelectObject = nullptr;
          SetColliderInfo(OverlappedComponent, Type, Info);
        }
      }

      if (CandidateGrabbable == Grabbable)
      {
        UPrimitiveComponent* BestCollider;
        UIsdkGrabbableComponent* BestGrabbable;
        UPrimitiveComponent* MyCollider;
        ComputeBestGrabbableColliderForGrabType(Type, MyCollider, BestCollider, BestGrabbable);
        CandidateGrabbable = BestGrabbable;
      }

      UpdateState();
    }
  }

  UpdateOverlappedSet();
}

void UIsdkGrabberComponent::UpdateOverlappedSet()
{
  OverlappedGrabbables.Empty();

  for (const TTuple<EIsdkGrabColliderType, PrimitiveComponentToColliderInfoMap>&
           ColliderTypeInfoMapPair : CollidersByType)
  {
    for (const TTuple<UPrimitiveComponent*, FIsdkColliderInfo>& ColliderEntry :
         ColliderTypeInfoMapPair.Value)
    {
      for (const TTuple<UPrimitiveComponent*, UIsdkGrabbableComponent*>& HoverObject :
           ColliderEntry.Value.HoverObjects)
      {
        UIsdkGrabbableComponent* HoveredGrabbable = HoverObject.Value;
        if (IsValid(HoveredGrabbable))
        {
          OverlappedGrabbables.FindOrAdd(HoveredGrabbable);
        }
      }
      if (IsValid(ColliderEntry.Value.SelectObject))
      {
        OverlappedGrabbables.FindOrAdd(ColliderEntry.Value.SelectObject);
      }
    }
  }
}

void UIsdkGrabberComponent::UpdateState()
{
  EIsdkInteractorState NewState = EIsdkInteractorState::Normal;
  for (const auto& TypeIT : CollidersByType)
  {
    auto Type = TypeIT.Key;
    for (auto It : CollidersByType[Type])
    {
      FIsdkColliderInfo& Info = It.Value;
      if (NewState == EIsdkInteractorState::Normal && Info.State == EIsdkInteractorState::Hover)
      {
        NewState = EIsdkInteractorState::Hover;
      }
      else if (Info.State == EIsdkInteractorState::Select)
      {
        NewState = EIsdkInteractorState::Select;
        break;
      }
    }
  }

  if (NewState != GetInteractorState())
  {
    SetState(NewState);
  }
}

void UIsdkGrabberComponent::UpdatePalmOffset(const FVector& InPalmOffset)
{
  if (DefaultPalmCollider)
  {
    DefaultPalmCollider->SetRelativeLocation(InPalmOffset);
  }
}

FIsdkInteractionRelationshipCounts UIsdkGrabberComponent::GetInteractorStateRelationshipCounts()
    const
{
  const int SelectCount = IsValid(GrabbedComponent) ? 1 : 0;
  return {OverlappedGrabbables.Num() - SelectCount, SelectCount};
}

void UIsdkGrabberComponent::GetInteractorStateRelationships(
    EIsdkInteractableState InState,
    TArray<TScriptInterface<IIsdkIInteractableState>>& OutInteractables) const
{
  if (InState == EIsdkInteractableState::Select)
  {
    OutInteractables.Add(GrabbedComponent);
  }
  else if (InState == EIsdkInteractableState::Hover)
  {
    for (UIsdkGrabbableComponent* OverlappedGrabbable : OverlappedGrabbables)
    {
      if (OverlappedGrabbable != GrabbedComponent)
      {
        OutInteractables.Add(OverlappedGrabbable);
      }
    }
  }
}

void UIsdkGrabberComponent::SelectByGrabType(EIsdkGrabColliderType Type)
{
  if (GetInteractorState() == EIsdkInteractorState::Select)
  {
    return;
  }

  UPrimitiveComponent* BestCollider;
  UPrimitiveComponent* MyCollider;
  UIsdkGrabbableComponent* BestGrabbable;
  ComputeBestGrabbableColliderForGrabType(Type, MyCollider, BestCollider, BestGrabbable);

  if (BestCollider)
  {
    SelectingCollider = MyCollider;
    SelectingColliderType = Type;
    FIsdkColliderInfo Info;
    if (GetColliderInfo(SelectingCollider, Type, Info))
    {
      Info.State = EIsdkInteractorState::Select;
      Info.SelectObject = BestGrabbable;
      SetColliderInfo(SelectingCollider, Type, Info);
      GrabbedComponent = BestGrabbable;
      bIsGrabbing = true;
      UpdateState();

      // The grabber should start listening to the grabbable before sending the select pointer event
      // out, because the grabbable might cancel the select event depending on its configuration. So
      // the grabber would miss the cancel and continue to be in an incorrect state.
      TScriptInterface<IIsdkIGrabbable> TargetGrabbable = GrabbedComponent->GetGrabbable();
      if (IsValid(TargetGrabbable.GetObject()))
      {
        TargetGrabbable->GetCancelGrabEventDelegate()->AddUniqueDynamic(
            this, &UIsdkGrabberComponent::HandleGrabbableCancelEvent);
      }

      PostEvent(EIsdkPointerEventType::Select, Info.SelectObject);
    }
  }
}

void UIsdkGrabberComponent::UnselectByGrabType(EIsdkGrabColliderType Type)
{
  if (SelectingCollider && SelectingColliderType == Type)
  {
    FIsdkColliderInfo Info;
    if (GetColliderInfo(SelectingCollider, Type, Info))
    {
      if (Info.SelectObject)
      {
        PostEvent(EIsdkPointerEventType::Unselect, Info.SelectObject);
      }
      if (!Info.HoverObjects.IsEmpty())
      {
        Info.State = EIsdkInteractorState::Hover;
      }
      else
      {
        Info.State = EIsdkInteractorState::Normal;
      }
      SetColliderInfo(SelectingCollider, Type, Info);
    }
    SelectingCollider = nullptr;

    if (bIsGrabbing)
    {
      TScriptInterface<IIsdkIGrabbable> TargetGrabbable = GrabbedComponent->GetGrabbable();
      if (IsValid(TargetGrabbable.GetObject()))
      {
        TargetGrabbable->GetCancelGrabEventDelegate()->RemoveDynamic(
            this, &UIsdkGrabberComponent::HandleGrabbableCancelEvent);
      }

      GrabbedComponent = nullptr;
      bIsGrabbing = false;
    }
  }

  UpdateState();
}

void UIsdkGrabberComponent::SelectPinch()
{
  if (!IsActive())
  {
    return;
  }

  SelectByGrabType(EIsdkGrabColliderType::Pinch);
}

void UIsdkGrabberComponent::SelectPalm()
{
  if (!IsActive())
  {
    return;
  }

  SelectByGrabType(EIsdkGrabColliderType::Palm);
}

void UIsdkGrabberComponent::UnselectPinch()
{
  if (!IsActive())
  {
    return;
  }

  UnselectByGrabType(EIsdkGrabColliderType::Pinch);
}

void UIsdkGrabberComponent::UnselectPalm()
{
  if (!IsActive())
  {
    return;
  }

  UnselectByGrabType(EIsdkGrabColliderType::Palm);
}

void UIsdkGrabberComponent::PostEvent(EIsdkPointerEventType Type, UIsdkGrabbableComponent* Dest)
{
  if (IsValid(Dest))
  {
    FIsdkInteractionPointerEvent Evt;
    Evt.Type = Type;
    Evt.Pose.Orientation = GetComponentQuat();
    Evt.Pose.Position = static_cast<FVector3f>(GetSelectingColliderPosition());
    Evt.Interactor = this;
    Evt.Interactable = Dest;
    Evt.Identifier = ID;
    Dest->PostEvent(Evt);
  }
}

void UIsdkGrabberComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  if (isdk::CVar_Meta_InteractionSDK_DebugInteractionVisuals.GetValueOnAnyThread())
  {
    DrawDebugVisuals();
  }

  if (State == EIsdkInteractorState::Select)
  {
    if (SelectingCollider)
    {
      FIsdkColliderInfo Info;
      if (GetColliderInfo(SelectingCollider, SelectingColliderType, Info))
      {
        PostEvent(EIsdkPointerEventType::Move, Info.SelectObject);
      }
    }
  }
}

void UIsdkGrabberComponent::AddCollider(UPrimitiveComponent* Collider, EIsdkGrabColliderType Type)
{
  if (IsValid(Collider))
  {
    auto& CollidersOfType = CollidersByType[Type];
    if (!CollidersOfType.Contains(Collider))
    {
      FIsdkColliderInfo Info;
      CollidersOfType.Emplace(Collider, Info);
      Collider->OnComponentBeginOverlap.AddDynamic(this, &UIsdkGrabberComponent::BeginOverlap);
      Collider->OnComponentEndOverlap.AddDynamic(this, &UIsdkGrabberComponent::EndOverlap);
    }
  }
}

void UIsdkGrabberComponent::UnregisterCollider(
    UPrimitiveComponent* Collider,
    EIsdkGrabColliderType Type)
{
  auto& CollidersOfType = CollidersByType[Type];
  if (CollidersOfType.Contains(Collider))
  {
    if (IsValid(Collider))
    {
      Collider->OnComponentBeginOverlap.RemoveDynamic(this, &UIsdkGrabberComponent::BeginOverlap);
      Collider->OnComponentEndOverlap.RemoveDynamic(this, &UIsdkGrabberComponent::EndOverlap);
    }
    CollidersOfType.Remove(Collider);
  }
}

EIsdkGrabColliderType UIsdkGrabberComponent::FindColliderType(UPrimitiveComponent* Collider) const
{
  for (const auto& TypeIT : CollidersByType)
  {
    auto Type = TypeIT.Key;
    if (TypeIT.Value.Contains(Collider))
    {
      return Type;
    }
  }

  return EIsdkGrabColliderType::Unknown;
}

FVector UIsdkGrabberComponent::GetSelectingColliderPosition()
{
  if (!IsValid(SelectingCollider))
  {
    return GetComponentLocation();
  }
  return SelectingCollider->GetComponentLocation();
}

bool UIsdkGrabberComponent::GetGrabbingStateWithComponent(
    UIsdkGrabbableComponent*& GrabbableOut) const
{
  GrabbableOut = GrabbedComponent;
  return bIsGrabbing && IsValid(GrabbableOut);
}

void UIsdkGrabberComponent::DrawDebugVisuals() const
{
  // We may have a multiple grabbers attached for hands/controllers... we'll only debug the
  // active ones to reduce noise
  if (!IsActive())
  {
    return;
  }

  const auto InteractorState = GetInteractorState();
  FColor DebugColor = UIsdkDebugUtils::GetInteractorStateDebugColor(InteractorState);

  for (const auto& TypeToColliderMapPair : CollidersByType)
  {
    const auto& Type = TypeToColliderMapPair.Key;
    const auto& ColliderMap = TypeToColliderMapPair.Value;

    // Instead of having a bunch of different colors, slightly dim the different grab collider color
    // depending on types.  This way don't bloat the debug color options and have some easy visual
    // differentiation.
    float ColorCoefficient;
    switch (Type)
    {
      case EIsdkGrabColliderType::Palm:
        ColorCoefficient = 0.9f;
        break;
      case EIsdkGrabColliderType::Custom:
      case EIsdkGrabColliderType::Unknown:
        ColorCoefficient = 0.8f;
        break;
      case EIsdkGrabColliderType::Pinch:
      default:
        ColorCoefficient = 1.f;
        break;
    }
    const auto FinalDebugColor = (ColorCoefficient * DebugColor).ToFColor(true);

    for (const auto& PrimitiveToInfoPair : ColliderMap)
    {
      const auto GrabCollider = PrimitiveToInfoPair.Key;

      if (const auto ColliderAsSphere = Cast<USphereComponent>(GrabCollider))
      {
        // Draw a sphere if the collider is a sphere
        DrawDebugSphere(
            GetWorld(),
            ColliderAsSphere->GetComponentLocation(),
            ColliderAsSphere->GetScaledSphereRadius(),
            12,
            FinalDebugColor);
        UE_VLOG_WIRESPHERE(
            GetOwner(),
            LogOculusInteraction,
            Verbose,
            ColliderAsSphere->GetComponentLocation(),
            ColliderAsSphere->GetScaledSphereRadius(),
            FinalDebugColor,
            TEXT_EMPTY);
      }
      else if (const auto ColliderAsBox = Cast<UBoxComponent>(GrabCollider))
      {
        // Draw a box if the collider is a box
        const auto BoxLocation = ColliderAsBox->GetComponentLocation();
        const auto BoxExtent = ColliderAsBox->GetScaledBoxExtent();
        const auto Box = FBox(BoxLocation - BoxExtent, BoxLocation + BoxExtent);
        DrawDebugBox(
            GetWorld(),
            ColliderAsBox->GetComponentLocation(),
            ColliderAsBox->GetScaledBoxExtent(),
            ColliderAsBox->GetComponentQuat(),
            FinalDebugColor);
        UE_VLOG_WIREOBOX(
            GetOwner(),
            LogOculusInteraction,
            Verbose,
            Box,
            ColliderAsBox->GetComponentQuat().ToMatrix(),
            FinalDebugColor,
            TEXT_EMPTY);
      }
      else
      {
        // Draw a bounding box otherwise
        const auto BoxLocation = GrabCollider->GetComponentLocation();
        const auto BoxExtent = GrabCollider->GetLocalBounds().BoxExtent;
        const auto Box = FBox(BoxLocation - BoxExtent, BoxLocation + BoxExtent);
        DrawDebugBox(
            GetWorld(),
            ColliderAsBox->GetComponentLocation(),
            ColliderAsBox->GetScaledBoxExtent(),
            FinalDebugColor);
        UE_VLOG_WIREBOX(
            GetOwner(), LogOculusInteraction, Verbose, Box, FinalDebugColor, TEXT_EMPTY);
      }
    }
  }
}
