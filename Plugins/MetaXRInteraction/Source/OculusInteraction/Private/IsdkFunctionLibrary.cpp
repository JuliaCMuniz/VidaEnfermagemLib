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

#include "IsdkFunctionLibrary.h"

#include "OculusInteractionLog.h"
#include "IsdkRuntimeSettings.h"
#include "Core/IsdkIGameplayTagContainer.h"
#include "Input/IsdkIPose.h"

TArray<FIsdkBoundsClipper> UIsdkFunctionLibrary::MakeBoundsClippersFromPose(
    const TScriptInterface<IIsdkIPose>& InPose,
    const FVector2D Size)
{
  TArray<FIsdkBoundsClipper> BoundsClippers;
  FIsdkBoundsClipper BoundsClipper;
  BoundsClipper.PoseProvider = InPose;
  BoundsClipper.Size = FVector3f(0.01f, Size.X, Size.Y);
  BoundsClippers.Add(BoundsClipper);
  return BoundsClippers;
}

TScriptInterface<IIsdkITrackingDataSubsystem> UIsdkFunctionLibrary::FindTrackingDataSubsystem(
    UWorld* World)
{
  auto& Subsystems = World->GetSubsystemArray<UWorldSubsystem>();
  const auto& DisabledSubsystems = UIsdkRuntimeSettings::Get().DisabledTrackingDataSubsystems;
  for (UWorldSubsystem* Subsystem : Subsystems)
  {
    if (DisabledSubsystems.Contains(Subsystem->GetClass()))
    {
      UE_LOG(
          LogOculusInteraction,
          Log,
          TEXT(
              "Ignoring subsystem '%s' as it is in UIsdkRuntimeSettings.DisabledTrackingDataSubsystems"),
          *Subsystem->GetClass()->GetName());
      continue;
    }

    if (Subsystem->Implements<UIsdkITrackingDataSubsystem>())
    {
      const auto TrackingDataSubsystem = TScriptInterface<IIsdkITrackingDataSubsystem>(Subsystem);
      if (!IIsdkITrackingDataSubsystem::Execute_IsEnabled(Subsystem))
      {
        continue;
      }
      return TrackingDataSubsystem;
    }
  }
  return nullptr;
}

TArray<FIsdkExternalHandPositionFrame_ThumbJointMapping>
UIsdkFunctionLibrary::GetDefaultOpenXRThumbMapping()
{
  const TArray ThumbJointMappings{
      MapXrThumbJoint(EIsdkThumbJoint::Metacarpal, 2), // Thumb_1
      MapXrThumbJoint(EIsdkThumbJoint::Proximal, 3), // Thumb_2
      MapXrThumbJoint(EIsdkThumbJoint::Distal, 4), // Thumb_3
      MapXrThumbJoint(EIsdkThumbJoint::Tip, 5)}; // Thumb_Tip
  return ThumbJointMappings;
}

TArray<FIsdkExternalHandPositionFrame_FingerJointMapping>
UIsdkFunctionLibrary::GetDefaultOpenXRFingerMapping()
{
  const TArray FingerJointMappings{
      // Index Finger
      MapXrFingerJoint(EIsdkFingerType::Index, EIsdkFingerJoint::Metacarpal, 6), // Index_0
      MapXrFingerJoint(EIsdkFingerType::Index, EIsdkFingerJoint::Proximal, 7), // Index_1
      MapXrFingerJoint(EIsdkFingerType::Index, EIsdkFingerJoint::Intermediate, 8), // Index_2
      MapXrFingerJoint(EIsdkFingerType::Index, EIsdkFingerJoint::Distal, 9), // Index_3
      MapXrFingerJoint(EIsdkFingerType::Index, EIsdkFingerJoint::Tip, 10), // Index_Tip

      // Middle Finger
      MapXrFingerJoint(EIsdkFingerType::Middle, EIsdkFingerJoint::Metacarpal, 11), // Middle_0
      MapXrFingerJoint(EIsdkFingerType::Middle, EIsdkFingerJoint::Proximal, 12), // Middle_1
      MapXrFingerJoint(EIsdkFingerType::Middle, EIsdkFingerJoint::Intermediate, 13), // Middle_2
      MapXrFingerJoint(EIsdkFingerType::Middle, EIsdkFingerJoint::Distal, 14), // Middle_3
      MapXrFingerJoint(EIsdkFingerType::Middle, EIsdkFingerJoint::Tip, 15), // Middle_Tip

      // Ring Finger
      MapXrFingerJoint(EIsdkFingerType::Ring, EIsdkFingerJoint::Metacarpal, 16), // Ring_0
      MapXrFingerJoint(EIsdkFingerType::Ring, EIsdkFingerJoint::Proximal, 17), // Ring_1
      MapXrFingerJoint(EIsdkFingerType::Ring, EIsdkFingerJoint::Intermediate, 18), // Ring_2
      MapXrFingerJoint(EIsdkFingerType::Ring, EIsdkFingerJoint::Distal, 19), // Ring_3
      MapXrFingerJoint(EIsdkFingerType::Ring, EIsdkFingerJoint::Tip, 20), // Ring_Tip

      // Pinky Finger
      MapXrFingerJoint(EIsdkFingerType::Pinky, EIsdkFingerJoint::Metacarpal, 21), // Pinky_0
      MapXrFingerJoint(EIsdkFingerType::Pinky, EIsdkFingerJoint::Proximal, 22), // Pinky_1
      MapXrFingerJoint(EIsdkFingerType::Pinky, EIsdkFingerJoint::Intermediate, 23), // Pinky_2
      MapXrFingerJoint(EIsdkFingerType::Pinky, EIsdkFingerJoint::Distal, 24), // Pinky_3
      MapXrFingerJoint(EIsdkFingerType::Pinky, EIsdkFingerJoint::Tip, 25)}; // Pinky_Tip
  return FingerJointMappings;
}

int UIsdkFunctionLibrary::GetFingerMappingIndex(
    const UIsdkHandJointMappings* HandJointMappings,
    EIsdkFingerType Finger,
    EIsdkFingerJoint Joint)
{
  const auto FoundMapping = HandJointMappings->FingerJointMappings.FindByPredicate(
      [&](auto& FingerJointMapping)
      { return FingerJointMapping.Finger == Finger && FingerJointMapping.Joint == Joint; });
  return FoundMapping ? FoundMapping->Index : INDEX_NONE;
}

int UIsdkFunctionLibrary::GetThumbMappingIndex(
    const UIsdkHandJointMappings* HandJointMappings,
    EIsdkThumbJoint Joint)
{
  const auto FoundMapping = HandJointMappings->ThumbJointMappings.FindByPredicate(
      [&](auto& ThumbJointMapping) { return ThumbJointMapping.Joint == Joint; });
  return FoundMapping ? FoundMapping->Index : INDEX_NONE;
}

TArray<float> UIsdkFunctionLibrary::GetDefaultJointRadii()
{
  constexpr float OculusJointRadii[26]{
      1.638251762f, /// XR_HAND_JOINT_PALM_EXT
      2.146084826f, /// XR_HAND_JOINT_WRIST_EXT
      1.938251762f, /// XR_HAND_JOINT_THUMB_METACARPAL_EXT
      1.228295141f, /// XR_HAND_JOINT_THUMB_PROXIMAL_EXT
      0.9768804535f, /// XR_HAND_JOINT_THUMB_DISTAL_EXT
      0.8768804535f, /// XR_HAND_JOINT_THUMB_TIP_EXT
      2.122828129f, /// XR_HAND_JOINT_INDEX_METACARPAL_EXT
      1.029525977f, /// XR_HAND_JOINT_INDEX_PROXIMAL_EXT
      0.8538101718f, /// XR_HAND_JOINT_INDEX_INTERMEDIATE_EXT
      0.7636196446f, /// XR_HAND_JOINT_INDEX_DISTAL_EXT
      0.6636196446f, /// XR_HAND_JOINT_INDEX_TIP_EXT
      2.123196456f, /// XR_HAND_JOINT_MIDDLE_METACARPAL_EXT
      1.117393933f, /// XR_HAND_JOINT_MIDDLE_PROXIMAL_EXT
      0.8030958474f, /// XR_HAND_JOINT_MIDDLE_INTERMEDIATE_EXT
      0.7629410829f, /// XR_HAND_JOINT_MIDDLE_DISTAL_EXT
      0.6629410829f, /// XR_HAND_JOINT_MIDDLE_TIP_EXT
      1.908827524f, /// XR_HAND_JOINT_RING_METACARPAL_EXT
      0.9922137484f, /// XR_HAND_JOINT_RING_PROXIMAL_EXT
      0.7611672394f, /// XR_HAND_JOINT_RING_INTERMEDIATE_EXT
      0.7231088821f, /// XR_HAND_JOINT_RING_DISTAL_EXT
      0.6231088821f, /// XR_HAND_JOINT_RING_TIP_EXT
      1.808827524f, /// XR_HAND_JOINT_LITTLE_METACARPAL_EXT
      0.8483353071f, /// XR_HAND_JOINT_LITTLE_PROXIMAL_EXT
      0.6764194462f, /// XR_HAND_JOINT_LITTLE_INTERMEDIATE_EXT
      0.6425984669f, /// XR_HAND_JOINT_LITTLE_DISTAL_EXT
      0.5425984669f, /// XR_HAND_JOINT_LITTLE_TIP_EXT
  };
  TArray<float> JointRadii = TArray<float>();
  JointRadii.Append(OculusJointRadii);
  return JointRadii;
}

bool UIsdkFunctionLibrary::GetISDKGameplayTagsFromActor(
    const AActor* ActorIn,
    FGameplayTagContainer& ContainerOut)
{
  if (!IsValid(ActorIn))
  {
    UE_LOG(
        LogOculusInteraction,
        Error,
        TEXT("UIsdkFunctionLibrary::GetGameplayTagsFromActor - Actor is invalid!"));
    return false;
  }

  ContainerOut.Reset();
  TArray<UActorComponent*> ImplementedComponents =
      ActorIn->GetComponentsByInterface(UIsdkIGameplayTagContainer::StaticClass());

  if (ImplementedComponents.Num() == 0)
  {
    return false;
  }

  for (UActorComponent* ThisComponent : ImplementedComponents)
  {
    FGameplayTagContainer ThisComponentGPTContainer;
    if (IIsdkIGameplayTagContainer::Execute_GetGameplayTagContainer(
            ThisComponent, ThisComponentGPTContainer))
    {
      ContainerOut.AppendTags(ThisComponentGPTContainer);
    }
  }

  return !ContainerOut.IsEmpty();
}

EControllerHandBehavior UIsdkFunctionLibrary::GetControllerHandBehavior(UObject* WorldContextObject)
{
  const auto World =
      GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
  if (!World)
  {
    UE_LOG(
        LogOculusInteraction,
        Warning,
        TEXT(
            "UIsdkFunctionLibrary::GetControllerHandBehavior: Invalid World Object passed in, defaulting to 'BothAnimated' Hand Behavior"));
    return EControllerHandBehavior::BothAnimated;
  }

  const auto TrackingDataSubsystem = FindTrackingDataSubsystem(World);
  if (!TrackingDataSubsystem.GetInterface())
  {
    UE_LOG(
        LogOculusInteraction,
        Warning,
        TEXT(
            "UIsdkFunctionLibrary::GetControllerHandBehavior: No Tracking Data Subsystem returned from FindTrackingDataSubsystem, defaulting to 'BothAnimated' Hand Behavior"));
    return EControllerHandBehavior::BothAnimated;
  }

  return TrackingDataSubsystem.GetInterface()->GetControllerHandBehavior();
}

void UIsdkFunctionLibrary::SetControllerHandBehavior(
    UObject* WorldContextObject,
    EControllerHandBehavior ControllerHandBehavior)
{
  const auto World =
      GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
  if (!World)
  {
    UE_LOG(
        LogOculusInteraction,
        Error,
        TEXT("UIsdkFunctionLibrary::SetControllerHandBehavior: Invalid World Object passed in!"));
    return;
  }

  const auto TrackingDataSubsystem = FindTrackingDataSubsystem(World);
  if (!TrackingDataSubsystem.GetInterface())
  {
    UE_LOG(
        LogOculusInteraction,
        Error,
        TEXT(
            "UIsdkFunctionLibrary::SetControllerHandBehavior: No Tracking Data Subsystem Returned from FindTrackingDataSubsystem!"));
    return;
  }

  // Procedural version is hidden, and not intended for use at this time.
  if (ControllerHandBehavior == EControllerHandBehavior::BothProcedural)
  {
    ControllerHandBehavior = EControllerHandBehavior::BothAnimated;
    UE_LOG(
        LogOculusInteraction,
        Warning,
        TEXT(
            "Attempted to set ControllerHandBehavior to \"Both (Procedural)\", which is not yet available"));
  }

  TrackingDataSubsystem.GetInterface()->SetControllerHandBehavior(ControllerHandBehavior);
}

FIsdkTrackedDataSubsystem_ControllerHandsBehaviorChangedDelegate*
UIsdkFunctionLibrary::GetControllerHandBehaviorDelegate(UObject* WorldContextObject)
{
  const auto World =
      GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
  if (!World)
  {
    return nullptr;
  }

  const auto TrackingDataSubsystem = FindTrackingDataSubsystem(World);
  if (!TrackingDataSubsystem.GetInterface())
  {
    return nullptr;
  }

  return TrackingDataSubsystem.GetInterface()->GetControllerHandBehaviorChangedDelegate();
}
