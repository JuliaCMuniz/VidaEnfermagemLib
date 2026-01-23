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

#include "Interaction/Grabbable/IsdkGrabbable.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"

UIsdkGrabbable::UIsdkGrabbable()
{
  PrimaryComponentTick.bCanEverTick = false;
}

void UIsdkGrabbable::BeginPlay()
{
  Super::BeginPlay();
  GrabPoses.ClearPoses();

  // Add Transform Target from name if there is no Transform Target
  if (!IsValid(TransformTarget))
  {
    TArray<USceneComponent*> Children;
    GetOwner()->GetRootComponent()->GetChildrenComponents(true, Children);
    for (USceneComponent* Child : Children)
    {
      if (Child->GetFName() == TransformTargetName)
      {
        SetTransformTarget(Child);
        break;
      }
    }
  }
  else
  {
    SetTransformTarget(TransformTarget);
  }

  if (bIsThrowable && !IsValid(ThrowableComponent))
  {
    ThrowableComponent = Cast<UIsdkThrowable>(GetOwner()->AddComponentByClass(
        UIsdkThrowable::StaticClass(), false, FTransform::Identity, true));
    ThrowableComponent->TrackedComponent = nullptr;
    ThrowableComponent->Settings = ThrowSettings;
    GetOwner()->FinishAddComponent(ThrowableComponent, false, FTransform::Identity);
  }
}

void UIsdkGrabbable::ProcessPointerEvent(const FIsdkInteractionPointerEvent& Event)
{
  if (GrabPoses.GetSelectPoses().Num() >= 1 && Event.Type == EIsdkPointerEventType::Select)
  {
    if (GrabType == EIsdkGrabType::SingleGrabFirstRetained)
    {
      // Cancel new pointer event and stop the new select event
      CancelGrabEvent.Broadcast(Event.Identifier, this);
      return;
    }
    else if (GrabType == EIsdkGrabType::SingleGrabTransferToSecond)
    {
      // Cancel all current pointer events so only the new event is used
      ForceCancel();
    }
  }

  GrabPoses.UpdatePoseArrays(Event);

  switch (Event.Type)
  {
    case EIsdkPointerEventType::Select:
    case EIsdkPointerEventType::Unselect:
    case EIsdkPointerEventType::Cancel:
      EndTransform();
      // Calling "BeginTransform" here because "Select", "Unselect", and "Cancel" change the number
      // of grab poses changes, so the grabbable could go from a None to Single or Single to Multi,
      // and vice versa. It is possible that after a "Unselect" or "Cancel" the state goes to
      // None in which case the "BeginTransform" function would not trigger any "Transformer" as it
      // checks the number of grab points before activation.
      BeginTransform();
      break;
    case EIsdkPointerEventType::Move:
      UpdateTransform();
      break;
    default:
      break;
  }
}

void UIsdkGrabbable::ForceCancel()
{
  const TArray<FIsdkGrabPose> PoseIdentifiers = GrabPoses.GetSelectPoses();
  GrabPoses.ClearPoses();
  // End transform here should be called after the grab poses are removed to make sure the reset
  // physics state code is executed
  EndTransform();
  for (const FIsdkGrabPose& Pose : PoseIdentifiers)
  {
    CancelGrabEvent.Broadcast(Pose.Identifier, this);
  }
}

void UIsdkGrabbable::BeginTransform()
{
  int SelectCount = GrabPoses.GetSelectPoses().Num();

  switch (SelectCount)
  {
    case 0:
      ActiveGrabTransformer = {};
      break;
    case 1:
      ActiveGrabTransformer = SingleGrabTransformer;
      break;
    default:
      ActiveGrabTransformer = MultiGrabTransformer;
      break;
  }

  if (!IsValid(ActiveGrabTransformer.GetObject()))
  {
    return;
  }

  // If a transformer was activated then stop the physics simulation
  // and cache its current state
  if (!bWasPhysicsCached)
  {
    bWasPhysicsCached = true;
    auto bIsSimulatingPhysics = TransformTarget->IsSimulatingPhysics();
    if (bIsSimulatingPhysics)
    {
      auto* TargetPrimitiveComponent = Cast<UPrimitiveComponent>(TransformTarget);
      if (IsValid(TargetPrimitiveComponent))
      {
        const auto Velocity = TargetPrimitiveComponent->GetPhysicsLinearVelocity();
        TargetPrimitiveComponent->AddForce(-Velocity);
        TargetPrimitiveComponent->SetSimulatePhysics(false);
        TargetPrimitiveComponent->SetEnableGravity(false);
      }
    }
    bWasSimulatingPhysics = bIsSimulatingPhysics;
  }

  if (bIsThrowable && IsValid(ThrowableComponent))
  {
    ThrowableComponent->TrackedComponent = TransformTarget;
  }

  ActiveGrabTransformer->BeginTransform(
      GrabPoses.GetSelectPoses(), FIsdkTargetTransform(TransformTarget));
  GrabbableEvent.Broadcast(TransformEvent::BeginTransform, this);
}

void UIsdkGrabbable::UpdateTransform()
{
  if (!IsValid(ActiveGrabTransformer.GetObject()))
  {
    return;
  }
  auto TargetRelativeTransform = ActiveGrabTransformer->UpdateTransform(
      GrabPoses.GetSelectPoses(), FIsdkTargetTransform(TransformTarget));
  TransformTarget.Get()->SetRelativeTransform(TargetRelativeTransform);
  GrabbableEvent.Broadcast(TransformEvent::UpdateTransform, this);
}

void UIsdkGrabbable::EndTransform()
{
  if (!IsValid(ActiveGrabTransformer.GetObject()))
  {
    return;
  }
  auto TargetRelativeTransform =
      ActiveGrabTransformer->EndTransform(FIsdkTargetTransform(TransformTarget));
  TransformTarget.Get()->SetRelativeTransform(TargetRelativeTransform);
  GrabbableEvent.Broadcast(TransformEvent::EndTransform, this);

  ActiveGrabTransformer = {};

  // Target was released
  if (GrabPoses.GetSelectPoses().IsEmpty())
  {
    bWasPhysicsCached = false;
    auto* TargetPrimitiveComponent = Cast<UPrimitiveComponent>(TransformTarget);
    if (IsValid(TargetPrimitiveComponent))
    {
      TargetPrimitiveComponent->SetSimulatePhysics(bWasSimulatingPhysics);

      // Throw
      if (bIsThrowable && IsValid(ThrowableComponent))
      {
        const FVector ReleaseVelocity = ThrowableComponent->GetVelocity();
        const FVector AngularVelocity = ThrowableComponent->GetAngularVelocity().Euler();

        TargetPrimitiveComponent->SetPhysicsLinearVelocity(ReleaseVelocity);
        TargetPrimitiveComponent->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
        TargetPrimitiveComponent->SetEnableGravity(EnableGravityWhenThrown);

        ThrowableComponent->TrackedComponent = nullptr;
      }
    }
  }
}
