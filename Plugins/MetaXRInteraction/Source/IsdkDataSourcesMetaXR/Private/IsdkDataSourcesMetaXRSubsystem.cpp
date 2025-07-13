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

#include "IsdkDataSourcesMetaXRSubsystem.h"
#include "IsdkDataSourcesMetaXR.h"
#include "IsdkOculusXRHelper.h"
#include "DataSources/IsdkFromMetaXRHmdDataSource.h"
#include "Utilities/IsdkXRUtils.h"

UIsdkDataSourcesMetaXRSubsystem::UIsdkDataSourcesMetaXRSubsystem()
{
  CurrentControllerHandBehavior = EControllerHandBehavior::BothProcedural;
}

void UIsdkDataSourcesMetaXRSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
  Super::Initialize(Collection);

  // SetControllerHandBehavior calls into Meta XR plugin logic, so we need to
  // call it on initialize to make sure state lines up with the default value.
  SetControllerHandBehavior(CurrentControllerHandBehavior);
}

void UIsdkDataSourcesMetaXRSubsystem::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  // Update cached value indicating whether a controller is held
  const auto& MetaXRModule = FIsdkDataSourcesMetaXRModule::GetChecked();
  const auto LeftControllerType = MetaXRModule.GetControllerType(EIsdkHandedness::Left);
  const auto RightControllerType = MetaXRModule.GetControllerType(EIsdkHandedness::Right);

  // Hands may show up as Unknown or None
  const bool bIsControllerInLeftHand =
      !(LeftControllerType == EIsdkXRControllerType::Unknown ||
        LeftControllerType == EIsdkXRControllerType::None);
  const bool bIsControllerInRightHand =
      !(RightControllerType == EIsdkXRControllerType::Unknown ||
        RightControllerType == EIsdkXRControllerType::None);

  bIsHoldingAController = bIsControllerInLeftHand || bIsControllerInRightHand;
}

bool UIsdkDataSourcesMetaXRSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
  const auto World = Cast<UWorld>(Outer);
  if (!World)
  {
    return false;
  }

  // Disable this subsystem for cases like automated testing, cooking, etc
  if (World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::Inactive ||
      GIsAutomationTesting)
  {
    return false;
  }

  return true;
}

FIsdkTrackingDataSources
UIsdkDataSourcesMetaXRSubsystem::CreateHandDataSourceComponent_Implementation(
    UMotionControllerComponent* AttachToMotionController)
{
  check(AttachToMotionController);
  const auto OwnerActor = AttachToMotionController->GetOwner();
  HandDataSource = NewObject<UIsdkFromMetaXRHandDataSource>(
      OwnerActor, UIsdkFromMetaXRHandDataSource::StaticClass());
  HandDataSource->SetMotionController(AttachToMotionController);
  HandDataSource->bUpdateInTick = true;
  HandDataSource->CreationMethod = EComponentCreationMethod::Native;
  HandDataSource->RegisterComponent();

  // TODO T162574495: Read defaults from Unreal Config.
  HandDataSource->SetAllowInvalidTrackedData(false);

  return {HandDataSource, HandDataSource, HandDataSource, HandDataSource};
}

FIsdkTrackingDataSources
UIsdkDataSourcesMetaXRSubsystem::CreateControllerDataSourceComponent_Implementation(
    UMotionControllerComponent* AttachToMotionController)
{
  check(AttachToMotionController);
  const auto OwnerActor = AttachToMotionController->GetOwner();
  ControllerDataSource = NewObject<UIsdkFromMetaXRControllerDataSource>(
      OwnerActor, UIsdkFromMetaXRControllerDataSource::StaticClass());
  ControllerDataSource->SetMotionController(AttachToMotionController);
  ControllerDataSource->bUpdateInTick = true;
  ControllerDataSource->CreationMethod = EComponentCreationMethod::Native;
  ControllerDataSource->RegisterComponent();

  return {ControllerDataSource, ControllerDataSource, ControllerDataSource, ControllerDataSource};
}

TScriptInterface<IIsdkIHmdDataSource>
UIsdkDataSourcesMetaXRSubsystem::CreateHmdDataSourceComponent_Implementation(
    AActor* TrackingSpaceRoot)
{
  check(TrackingSpaceRoot);
  const auto Component = NewObject<UIsdkFromMetaXRHmdDataSource>(
      TrackingSpaceRoot, UIsdkFromMetaXRHmdDataSource::StaticClass());
  Component->RegisterComponent();
  return Component;
}

bool UIsdkDataSourcesMetaXRSubsystem::IsEnabled_Implementation()
{
  const bool bIsOculusXrLoaded = FIsdkOculusXRHelper::IsOculusXrLoaded();
  const bool bOculusXrEnabled = IsdkXRUtils::IsUsingOculusXR();
  return bIsOculusXrLoaded && bOculusXrEnabled;
}

EControllerHandBehavior UIsdkDataSourcesMetaXRSubsystem::GetControllerHandBehavior()
{
  return CurrentControllerHandBehavior;
}

void UIsdkDataSourcesMetaXRSubsystem::SetControllerHandBehavior(
    EControllerHandBehavior ControllerHandBehavior)
{
  const auto& MetaXRModule = FIsdkDataSourcesMetaXRModule::GetChecked();

  switch (ControllerHandBehavior)
  {
    case EControllerHandBehavior::BothProcedural:
      MetaXRModule.Input_SetControllerDrivenHandPoses(
          EIsdkXRControllerDrivenHandPoseType::Controller);
      break;
    case EControllerHandBehavior::BothAnimated:
      MetaXRModule.Input_SetControllerDrivenHandPoses(EIsdkXRControllerDrivenHandPoseType::None);
      break;
    case EControllerHandBehavior::ControllerOnly:
      MetaXRModule.Input_SetControllerDrivenHandPoses(EIsdkXRControllerDrivenHandPoseType::None);
      break;
    case EControllerHandBehavior::HandsOnlyProcedural:
      MetaXRModule.Input_SetControllerDrivenHandPoses(EIsdkXRControllerDrivenHandPoseType::Natural);
      break;
    case EControllerHandBehavior::HandsOnlyAnimated:
      MetaXRModule.Input_SetControllerDrivenHandPoses(EIsdkXRControllerDrivenHandPoseType::None);
      break;
  }

  const auto PreviousBehavior = CurrentControllerHandBehavior;
  CurrentControllerHandBehavior = ControllerHandBehavior;

  if (PreviousBehavior != CurrentControllerHandBehavior)
  {
    ControllerHandsBehaviorChangedDelegate.Broadcast(
        this, PreviousBehavior, CurrentControllerHandBehavior);
  }
}

FIsdkTrackedDataSubsystem_ControllerHandsBehaviorChangedDelegate*
UIsdkDataSourcesMetaXRSubsystem::GetControllerHandBehaviorChangedDelegate()
{
  return &ControllerHandsBehaviorChangedDelegate;
}

TStatId UIsdkDataSourcesMetaXRSubsystem::GetStatId() const
{
  RETURN_QUICK_DECLARE_CYCLE_STAT(UIsdkDataSourcesMetaXRSubsystem, STATGROUP_Tickables);
}

bool UIsdkDataSourcesMetaXRSubsystem::IsHoldingAController()
{
  return bIsHoldingAController;
}
