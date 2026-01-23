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

#include "DataSources/IsdkFromOpenXRControllerDataSource.h"
#include "IHandTracker.h"
#include "IXRTrackingSystem.h"
#include "DataSources/IsdkOpenXRHelper.h"
#include "Features/IModularFeatures.h"
#include "Utilities/IsdkXRUtils.h"

// Sets default values for this component's properties
UIsdkFromOpenXRControllerDataSource::UIsdkFromOpenXRControllerDataSource()
    : MotionController(nullptr), bIsLastGoodRootPoseValid(false), bIsLastGoodPointerPoseValid(false)
{
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;
  PrimaryComponentTick.TickGroup = TG_PrePhysics;
  PrimaryComponentTick.SetTickFunctionEnable(true);

  IsRootPoseConnected = NewObject<UIsdkConditionalBool>(this, TEXT("IsRootPoseConnected"));
  IsRootPoseHighConfidence =
      NewObject<UIsdkConditionalBool>(this, TEXT("IsRootPoseHighConfidence"));
}

void UIsdkFromOpenXRControllerDataSource::GetPointerPose_Implementation(
    FTransform& PointerPose,
    bool& IsValid)
{
  IsValid = bIsLastGoodPointerPoseValid;
  IsValid &= IsRootPoseConnected->GetResolvedValue();
  if (IsValid)
  {
    PointerPose = RelativePointerPose * MotionController->GetComponentTransform();
  }
}

void UIsdkFromOpenXRControllerDataSource::GetRelativePointerPose_Implementation(
    FTransform& PointerRelativePose,
    bool& IsValid)
{
  IsValid = bIsLastGoodPointerPoseValid;
  IsValid &= IsRootPoseConnected->GetResolvedValue();
  if (IsValid)
  {
    PointerRelativePose = RelativePointerPose;
  }
}

FTransform UIsdkFromOpenXRControllerDataSource::GetRootPose_Implementation()
{
  return MotionController->GetComponentTransform();
}

bool UIsdkFromOpenXRControllerDataSource::IsRootPoseValid_Implementation()
{
  return bIsLastGoodRootPoseValid;
}

UIsdkConditional*
UIsdkFromOpenXRControllerDataSource::GetRootPoseConnectedConditional_Implementation()
{
  return IsRootPoseConnected;
}

void UIsdkFromOpenXRControllerDataSource::ReadControllerData()
{
  FTransform GripPose;
  if (GetGripPose(GripPose))
  {
    IsRootPoseHighConfidence->SetValue(true);
  }
  else
  {
    IsRootPoseHighConfidence->SetValue(false);
  }
  RelativePointerPose = FTransform{IsdkXRUtils::OXR::ControllerRelativePointerOffset};
  bIsLastGoodPointerPoseValid = true;
}

void UIsdkFromOpenXRControllerDataSource::ReadHandedness()
{
  Handedness = FIsdkOpenXRHelper::ReadHandedness(MotionController);
}

bool UIsdkFromOpenXRControllerDataSource::IsHandTrackingEnabled()
{
  if (auto& ModularFeatures = IModularFeatures::Get();
      ModularFeatures.IsModularFeatureAvailable(IHandTracker::GetModularFeatureName()))
  {
    const auto& HandTracker =
        ModularFeatures.GetModularFeature<IHandTracker>(IHandTracker::GetModularFeatureName());
    return HandTracker.IsHandTrackingStateValid();
  }
  return false;
}

bool UIsdkFromOpenXRControllerDataSource::GetGripPose(FTransform& OutGripPose)
{
  auto XRTrackingSystem = GEngine->XRSystem.Get();
  if (!XRTrackingSystem)
  {
    return false;
  }
  FQuat Orientation;
  FVector Position;
  if (XRTrackingSystem->GetCurrentPose(
          XRTrackingSystem->GetTrackingOrigin(), Orientation, Position))
  {
    OutGripPose = FTransform(Orientation, Position);
    return true;
  }
  return false;
}

// Called every frame
void UIsdkFromOpenXRControllerDataSource::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
  auto& ModularFeatures = IModularFeatures::Get();
  if (!ModularFeatures.IsModularFeatureAvailable(IHandTracker::GetModularFeatureName()))
  {
    return;
  }
  auto& HandTracker =
      ModularFeatures.GetModularFeature<IHandTracker>(IHandTracker::GetModularFeatureName());
  EControllerHand Hand =
      (Handedness == EIsdkHandedness::Left) ? EControllerHand::Left : EControllerHand::Right;
  OutPositions.Empty();
  OutRotations.Empty();
  OutRadii.Empty();
  bool bIsHandDataValid =
      HandTracker.GetAllKeypointStates(Hand, OutPositions, OutRotations, OutRadii);

  IsRootPoseConnected->SetValue(
      MotionController && !bIsHandDataValid && MotionController->IsTracked());
  FName DesiredSourceName;
  if (IsRootPoseConnected->GetResolvedValue())
  {
    LastGoodRootPose = GetRootPose_Implementation();
    bIsLastGoodRootPoseValid = true;
    bIsLastGoodPointerPoseValid = true;
    // We need to set the tracking source to aim pose for openxr controllers
    if (!bIsHandDataValid)
    {
      DesiredSourceName = Handedness == EIsdkHandedness::Left ? IsdkXRUtils::LeftAimSourceName
                                                              : IsdkXRUtils::RightAimSourceName;
      if (MotionController->GetTrackingMotionSource() != DesiredSourceName)
      {
        MotionController->SetTrackingMotionSource(DesiredSourceName);
      }
    }
    ReadControllerData();
  }
  else
  {
    DesiredSourceName = Handedness == EIsdkHandedness::Left ? IsdkXRUtils::LeftSourceName
                                                            : IsdkXRUtils::RightSourceName;
    if (MotionController->GetTrackingMotionSource() != DesiredSourceName)
    {
      MotionController->SetTrackingMotionSource(DesiredSourceName);
    }
    IsRootPoseHighConfidence->SetValue(false);
  }
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

UMotionControllerComponent* UIsdkFromOpenXRControllerDataSource::GetMotionController() const
{
  return MotionController;
}

void UIsdkFromOpenXRControllerDataSource::SetMotionController(
    UMotionControllerComponent* InMotionController)
{
  if (IsValid(MotionController))
  {
    RemoveTickPrerequisiteComponent(MotionController);
  }
  MotionController = InMotionController;
  if (IsValid(MotionController))
  {
    AddTickPrerequisiteComponent(MotionController);
  }
  ReadHandedness();
}

bool UIsdkFromOpenXRControllerDataSource::IsPointerPoseValid_Implementation()
{
  return bIsLastGoodPointerPoseValid;
}
