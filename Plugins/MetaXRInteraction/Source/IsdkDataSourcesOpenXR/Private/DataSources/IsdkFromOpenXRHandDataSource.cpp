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

#include "DataSources/IsdkFromOpenXRHandDataSource.h"
#include "DrawDebugHelpers.h"
#include "IHandTracker.h"
#include "IsdkFunctionLibrary.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "DataSources/IsdkOpenXRHelper.h"
#include "Features/IModularFeatures.h"
#include "Utilities/IsdkXRUtils.h"

// Sets default values for this component's properties
UIsdkFromOpenXRHandDataSource::UIsdkFromOpenXRHandDataSource()
{
  PrimaryComponentTick.bCanEverTick = true;
  PrimaryComponentTick.bStartWithTickEnabled = true;
  PrimaryComponentTick.TickGroup = TG_PrePhysics;
  PrimaryComponentTick.SetTickFunctionEnable(true);

  IsRootPoseConnected = NewObject<UIsdkConditionalBool>(this, TEXT("IsRootPoseConnected"));
  IsRootPoseHighConfidence =
      NewObject<UIsdkConditionalBool>(this, TEXT("IsRootPoseHighConfidence"));

  bIsLastGoodRootPoseValid = false;
  bIsLastGoodPointerPoseValid = false;
  bAllowLowConfidenceData = false;
  bHasLastKnownGood = false;
  bWantsInitializeComponent = true;

  DefaultJointRadii = UIsdkFunctionLibrary::GetDefaultJointRadii();
}

void UIsdkFromOpenXRHandDataSource::InitializeComponent()
{
  Super::InitializeComponent();

  const auto ThumbJointMappings = UIsdkFunctionLibrary::GetDefaultOpenXRThumbMapping();
  const auto FingerJointMappings = UIsdkFunctionLibrary::GetDefaultOpenXRFingerMapping();

  SetHandJointMappings(ThumbJointMappings, FingerJointMappings);
}

// Called every frame
void UIsdkFromOpenXRHandDataSource::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
  if (!IsValid(MotionController))
  {
    return;
  }

  auto& ModularFeatures = IModularFeatures::Get();
  if (!ModularFeatures.IsModularFeatureAvailable(IHandTracker::GetModularFeatureName()))
  {
    return;
  }
  auto& HandTracker =
      ModularFeatures.GetModularFeature<IHandTracker>(IHandTracker::GetModularFeatureName());
  bool bIsHandTrackingEnabled = HandTracker.IsHandTrackingStateValid();
  ReadHandedness();
  EControllerHand Hand =
      (Handedness == EIsdkHandedness::Left) ? EControllerHand::Left : EControllerHand::Right;
  OutPositions.Empty();
  OutRotations.Empty();
  OutRadii.Empty();
  bool bIsHandDataValid =
      HandTracker.GetAllKeypointStates(Hand, OutPositions, OutRotations, OutRadii);
  FName DesiredSourceName;
  if (bIsHandTrackingEnabled && bIsHandDataValid)
  {
    DesiredSourceName = Handedness == EIsdkHandedness::Left ? IsdkXRUtils::LeftSourceName
                                                            : IsdkXRUtils::RightSourceName;
    // Update the motion controller's tracking source if necessary
    if (MotionController->GetTrackingMotionSource() != DesiredSourceName)
    {
      MotionController->SetTrackingMotionSource(DesiredSourceName);
    }
    ReadHandData();
    // Enable this to see debug visuals for the wrist position, aim/pointer ray origin and direction
    if constexpr (false)
    {
      DrawDebugWidgets();
    }
  }
  else
  {
    DesiredSourceName = Handedness == EIsdkHandedness::Left ? IsdkXRUtils::LeftAimSourceName
                                                            : IsdkXRUtils::RightAimSourceName;
    if (MotionController->GetTrackingMotionSource() != DesiredSourceName)
    {
      MotionController->SetTrackingMotionSource(DesiredSourceName);
    }
    IsRootPoseHighConfidence->SetValue(false);
    bIsLastGoodPointerPoseValid = false;
    bIsLastGoodRootPoseValid = false;
    bIsHandDataValid = false;
    bIsHandTrackingEnabled = false;
  }
  IsRootPoseConnected->SetValue(MotionController && bIsHandDataValid && bIsHandTrackingEnabled);
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UIsdkFromOpenXRHandDataSource::IsPointerPoseValid_Implementation()
{
  return bIsLastGoodPointerPoseValid;
}

bool UIsdkFromOpenXRHandDataSource::IsRootPoseValid_Implementation()
{
  return bIsLastGoodRootPoseValid;
}

FTransform UIsdkFromOpenXRHandDataSource::GetRootPose_Implementation()
{
  FTransform MotionTransform = MotionController->GetComponentTransform();
  MotionTransform.SetRotation(
      MotionTransform.GetRotation() * IsdkXRUtils::OXR::HandRootFixupRotation);
  FTransform WristOffset = Handedness == EIsdkHandedness::Left
      ? FTransform(IsdkXRUtils::OXR::HandRootWristOffsetLeft)
      : FTransform(IsdkXRUtils::OXR::HandRootWristOffsetRight);
  return WristOffset * MotionTransform;
}

void UIsdkFromOpenXRHandDataSource::GetPointerPose_Implementation(
    FTransform& PointerPose,
    bool& IsValid)
{
  if (MotionController)
  {
    PointerPose = RelativePointerPose * MotionController->GetComponentTransform();
    IsValid = bIsLastGoodPointerPoseValid;
  }
  else
  {
    IsValid = false;
  }
}

void UIsdkFromOpenXRHandDataSource::GetRelativePointerPose_Implementation(
    FTransform& PointerRelativePose,
    bool& IsValid)
{
  PointerRelativePose = RelativePointerPose;
  IsValid = bIsLastGoodPointerPoseValid;
}

UMotionControllerComponent* UIsdkFromOpenXRHandDataSource::GetMotionController() const
{
  return MotionController;
}

bool UIsdkFromOpenXRHandDataSource::GetAllowInvalidTrackedData() const
{
  return bAllowLowConfidenceData;
}

UIsdkConditional* UIsdkFromOpenXRHandDataSource::GetRootPoseConnectedConditional_Implementation()
{
  return IsRootPoseConnected;
}

UIsdkConditional*
UIsdkFromOpenXRHandDataSource::GetRootPoseHighConfidenceConditional_Implementation()
{
  return IsRootPoseHighConfidence;
}

void UIsdkFromOpenXRHandDataSource::SetMotionController(
    UMotionControllerComponent* InMotionController)
{
  if (IsValid(MotionController))
  {
    RemoveTickPrerequisiteComponent(MotionController);
  }
  MotionController = InMotionController;

  // Ensure that this component ticks after the input component
  if (IsValid(MotionController))
  {
    AddTickPrerequisiteComponent(MotionController);
    ReadHandedness();
  }
}

void UIsdkFromOpenXRHandDataSource::DrawDebugWidgets()
{
  constexpr float DebugPointSizeSmall = 10.0f;
  constexpr float DebugPointSizeMedium = 15.0f;
  constexpr float DebugPointSizeLarge = 20.0f;
  constexpr float DebugRayLength = 50.0f;

  DrawDebugPoint(
      GetWorld(),
      RelativePointerPose.GetLocation(),
      DebugPointSizeLarge,
      FColor::Blue); // pointer pose, local space

  DrawDebugPoint(
      GetWorld(),
      MotionController->GetComponentTransform().GetLocation(),
      DebugPointSizeSmall,
      FColor::Black); // wrist

  FTransform PointerPose;
  bool PointerPoseValid;
  // pointer pose, world space
  Execute_GetPointerPose(this, PointerPose, PointerPoseValid);
  DrawDebugPoint(
      GetWorld(),
      PointerPose.GetLocation(),
      DebugPointSizeMedium,
      PointerPoseValid ? FColor::Green : FColor::Red);

  // line from pointer pose, in the direction of the pointer ray
  const FVector RayEndPosition = PointerPose.TransformPosition({DebugRayLength, 0, 0});
  DrawDebugLine(GetWorld(), PointerPose.GetLocation(), RayEndPosition, FColor::Black);
}

void UIsdkFromOpenXRHandDataSource::SetAllowInvalidTrackedData(bool bInAllowInvalidTrackedData)
{
  bAllowLowConfidenceData = bInAllowInvalidTrackedData;
}

void UIsdkFromOpenXRHandDataSource::ReadHandData()
{
  FXRMotionControllerData MotionControllerData;
  EControllerHand Hand =
      (Handedness == EIsdkHandedness::Left) ? EControllerHand::Left : EControllerHand::Right;

  UHeadMountedDisplayFunctionLibrary::GetMotionControllerData(this, Hand, MotionControllerData);

  if (MotionControllerData.bValid && !MotionControllerData.HandKeyPositions.IsEmpty())
  {
    bHasLastKnownGood = true;
    LastGoodRootPose = GetRootPose_Implementation();
    IsRootPoseHighConfidence->SetValue(true);
    bIsLastGoodRootPoseValid = true;
    bIsLastGoodPointerPoseValid = true;
    // Offset for pointer pose
    FVector Offset = (Handedness == EIsdkHandedness::Left)
        ? IsdkXRUtils::OXR::HandRootWristOffsetLeft
        : IsdkXRUtils::OXR::HandRootWristOffsetRight;
    FTransform RootTransformInverse = FTransform(Offset).Inverse();
    RootTransformInverse.AddToTranslation(IsdkXRUtils::OXR::HandRelativePointerOffset);
    RelativePointerPose = FTransform(
        IsdkXRUtils::OXR::HandRelativePointerRotation, RootTransformInverse.GetTranslation());

    //  Update Joint Poses
    TArray<FTransform>& JointPoses = HandData->GetJointPoses();

    auto RootPose = GetRootPose_Implementation();
    auto RootPoseInverse = RootPose.Inverse();

    JointPoses[1] = FTransform::Identity;
    for (int32 Index = 0; Index < MotionControllerData.HandKeyPositions.Num(); ++Index)
    {
      if (Index == 1) // Wrist pose added above
      {
        continue;
      }

      auto AdjustedTransform = FTransform(
          MotionControllerData.HandKeyRotations[Index],
          MotionControllerData.HandKeyPositions[Index],
          FVector::One());
      AdjustedTransform = AdjustedTransform * RootPoseInverse;
      AdjustedTransform.SetRotation(
          AdjustedTransform.GetRotation() * IsdkXRUtils::OXR::HandJointFixupRotation);
      JointPoses[Index] = AdjustedTransform;
    }
    if (IsValid(HandDataInbound))
    {
      HandDataInbound->SetCachedJointPoses(JointPoses);
    }
    // set the bone radii
    TArray<float>& JointRadii = HandData->GetJointRadii();
    float HandScale = LastGoodRootPose.GetScale3D().X; // Scale should be the same for all axis
    for (int JointIndex = 0; JointIndex < JointRadii.Num(); JointIndex++)
    {
      JointRadii[JointIndex] = DefaultJointRadii[JointIndex] * HandScale;
    }
    SetImplHandData(LastGoodRootPose);
    bIsHandJointDataValid = true;
  }
  else if (bHasLastKnownGood)
  {
    // No high confidence data, keep state from last known good.
  }
  else
  {
    // No high confidence data, no available last known good.
    IsRootPoseHighConfidence->SetValue(false);
    bIsLastGoodRootPoseValid = false;
    bIsLastGoodPointerPoseValid = false;
    bIsHandJointDataValid = false;
    RelativePointerPose = {};
  }
}

void UIsdkFromOpenXRHandDataSource::ReadHandedness()
{
  Handedness = FIsdkOpenXRHelper::ReadHandedness(MotionController);
}
