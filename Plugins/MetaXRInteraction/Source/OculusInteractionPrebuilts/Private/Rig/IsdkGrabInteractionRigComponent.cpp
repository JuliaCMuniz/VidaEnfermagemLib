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

#include "Rig/IsdkGrabInteractionRigComponent.h"
#include "EnhancedInputComponent.h"
#include "Core/IsdkConditionalComponentIsActive.h"
#include "Core/IsdkConditionalGroupAll.h"
#include "Interaction/IsdkGrabberComponent.h"
#include "HandPoseDetection/IsdkHandFingerPinchGrabRecognizer.h"
#include "HandPoseDetection/IsdkHandPalmGrabRecognizer.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UIsdkGrabInteractionRigComponent::UIsdkGrabInteractionRigComponent()
{
  PrimaryComponentTick.bCanEverTick = true;
  bWantsInitializeComponent = true;
  bAutoActivate = true;

  Grabber = CreateDefaultSubobject<UIsdkGrabberComponent>(
      TEXT("IsdkGrabInteraction IsdkGrabberComponent"));

  PinchGrabRecognizer = CreateDefaultSubobject<UIsdkHandFingerPinchGrabRecognizer>(
      TEXT("IsdkHandFingerPinchGrabRecognizer"));
  PalmGrabRecognizer =
      CreateDefaultSubobject<UIsdkHandPalmGrabRecognizer>(TEXT("IsdkHandPalmGrabRecognizer"));

  EnabledConditionalGroup =
      CreateDefaultSubobject<UIsdkConditionalGroupAll>(TEXT("GrabEnabledConditionalAll"));
  IsActiveConditional =
      CreateDefaultSubobject<UIsdkConditionalComponentIsActive>(TEXT("GrabIsActiveConditional"));
}

void UIsdkGrabInteractionRigComponent::InitializeComponent()
{
  Super::InitializeComponent();

  IsActiveConditional->SetComponent(this);
  EnabledConditionalGroup->AddConditional(IsActiveConditional);
  EnabledConditionalGroup->ResolvedValueChanged.AddWeakLambda(
      this, [this](bool bIsEnabled) { HandleIsEnabledConditionalChanged(bIsEnabled); });
}

void UIsdkGrabInteractionRigComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  // Tick the pinch grab recognizer
  if (IsValid(PinchGrabRecognizer))
  {
    // Update the pinch grab recognizer's wrist forward vector
    if (IsValid(HandRootPose.GetObject()))
    {
      PinchGrabRecognizer->CurrentWristForward = UKismetMathLibrary::GetForwardVector(
          IIsdkIRootPose::Execute_GetRootPose(HandRootPose.GetObject()).GetRotation().Rotator());
    }

    // Update the pinch grab recognizer's hmd forward vector
    if (IsValid(HmdDataSource.GetObject()))
    {
      FTransform HmdPose;
      bool bIsTracked;
      IIsdkIHmdDataSource::Execute_GetHmdPose(HmdDataSource.GetObject(), HmdPose, bIsTracked);
      PinchGrabRecognizer->CurrentHMDForward =
          UKismetMathLibrary::GetForwardVector(HmdPose.GetRotation().Rotator());
    }

    PinchGrabRecognizer->UpdateState(DeltaTime);
  }

  // Tick the palm grab recognizer
  if (IsValid(PalmGrabRecognizer))
  {
    PalmGrabRecognizer->UpdateState(DeltaTime);
  }

  // Handle changes to grab state
  if (Grabber->bIsGrabbing != bLastIsGrabbing)
  {
    UpdatePinchCollidersAttachment();
  }

  // Store last grabbing state for next frame
  bLastIsGrabbing = Grabber->bIsGrabbing;
}

void UIsdkGrabInteractionRigComponent::HandleIsEnabledConditionalChanged(bool bIsEnabled)
{
  if (!bIsEnabled)
  {
    Grabber->Deactivate();
  }
  else
  {
    Grabber->Activate();
  }
}

void UIsdkGrabInteractionRigComponent::BindDataSources(
    const FIsdkTrackingDataSources& DataSources,
    const TScriptInterface<IIsdkIHmdDataSource>& HmdDataSourceIn,
    USceneComponent* AttachToComponent,
    FName AttachToComponentSocket)
{
  Grabber->AttachToComponent(
      AttachToComponent, FAttachmentTransformRules::KeepRelativeTransform, AttachToComponentSocket);

  // Watch for the data sources becoming enabled/disabled.
  const auto IsConnectedConditional =
      IIsdkIRootPose::Execute_GetRootPoseConnectedConditional(DataSources.HandRootPose.GetObject());
  if (ensure(IsValid(IsConnectedConditional)))
  {
    EnabledConditionalGroup->AddConditional(IsConnectedConditional);
  }

  HandRootPose = DataSources.HandRootPose;
  if (IsValid(HmdDataSourceIn.GetObject()))
  {
    HmdDataSource = HmdDataSourceIn;
  }
}

void UIsdkGrabInteractionRigComponent::BindControllerGrabInput(
    UEnhancedInputComponent* EnhancedInputComponent,
    const UInputAction* PinchGrabAction,
    const UInputAction* PalmGrabAction)
{
  // Configure pinch grab
  if (IsValid(PinchGrabAction))
  {
    EnhancedInputComponent->BindAction(
        PinchGrabAction,
        ETriggerEvent::Started,
        Grabber.Get(),
        &UIsdkGrabberComponent::SelectPinch);
    EnhancedInputComponent->BindAction(
        PinchGrabAction,
        ETriggerEvent::Completed,
        Grabber.Get(),
        &UIsdkGrabberComponent::UnselectPinch);
    EnhancedInputComponent->BindAction(
        PinchGrabAction,
        ETriggerEvent::Canceled,
        Grabber.Get(),
        &UIsdkGrabberComponent::UnselectPinch);
  }

  // Configure palm grab
  if (IsValid(PalmGrabAction))
  {
    EnhancedInputComponent->BindAction(
        PalmGrabAction, ETriggerEvent::Started, Grabber.Get(), &UIsdkGrabberComponent::SelectPalm);
    EnhancedInputComponent->BindAction(
        PalmGrabAction,
        ETriggerEvent::Completed,
        Grabber.Get(),
        &UIsdkGrabberComponent::UnselectPalm);
    EnhancedInputComponent->BindAction(
        PalmGrabAction,
        ETriggerEvent::Canceled,
        Grabber.Get(),
        &UIsdkGrabberComponent::UnselectPalm);
  }
}

void UIsdkGrabInteractionRigComponent::BindHandGrabInput(
    UIsdkHandMeshComponent* SyntheticHandVisual,
    EIsdkFingerType FingerType,
    const FName& InThumbTipSocketName)
{
  // Bind to grab action
  if (ensure(PinchGrabRecognizer))
  {
    if (IsValid(SyntheticHandVisual))
    {
      AddTickPrerequisiteComponent(SyntheticHandVisual);
    }
    PinchGrabRecognizer->HandVisual = SyntheticHandVisual;
    PinchGrabRecognizer->FingerType = FingerType;
    PinchGrabRecognizer->PinchGrabStarted.AddUniqueDynamic(
        Grabber, &UIsdkGrabberComponent::SelectPinch);
    PinchGrabRecognizer->PinchGrabFinished.AddUniqueDynamic(
        Grabber, &UIsdkGrabberComponent::UnselectPinch);
  }

  if (ensure(PalmGrabRecognizer))
  {
    PalmGrabRecognizer->HandVisual = SyntheticHandVisual;
    PalmGrabRecognizer->PalmGrabStarted.AddUniqueDynamic(
        Grabber, &UIsdkGrabberComponent::SelectPalm);
    PalmGrabRecognizer->PalmGrabFinished.AddUniqueDynamic(
        Grabber, &UIsdkGrabberComponent::UnselectPalm);
  }

  UpdatePinchCollidersAttachment();
}

void UIsdkGrabInteractionRigComponent::UpdateMeshDependencies(
    const FVector& PalmColliderOffset,
    USkinnedMeshComponent* InPinchAttachMesh,
    const FName& InThumbTipSocketName)
{
  Grabber->UpdatePalmOffset(PalmColliderOffset);
  PinchAttachMesh = InPinchAttachMesh;
  ThumbTipSocketName = InThumbTipSocketName;

  // Update pinch attachment now that we've updated the pinch attach mesh
  UpdatePinchCollidersAttachment();
}

void UIsdkGrabInteractionRigComponent::UpdatePinchCollidersAttachment()
{
  if (!PinchAttachMesh || !Grabber)
  {
    return;
  }

  // Collect all pinch colliders for this grabber
  TArray<UPrimitiveComponent*> PinchColliders;
  Grabber->GetCollidersByType(EIsdkGrabColliderType::Pinch, PinchColliders);

  for (const auto PinchCollider : PinchColliders)
  {
    if (Grabber->bIsGrabbing)
    {
      // If the Grabber is grabbing, attach the PinchCollider to the Grabber and set its transform
      // to the thumb tip.  This keeps the transform of what is being grabbed constant relative to
      // the root of the controller / hand - small movement in the thumb will be ignored.
      PinchCollider->AttachToComponent(Grabber, FAttachmentTransformRules::KeepRelativeTransform);

      const FTransform ThumbTipPosition = PinchAttachMesh->GetBoneTransform(ThumbTipSocketName);
      PinchCollider->SetWorldTransform(ThumbTipPosition);
    }
    else
    {
      // If the Grabber is not grabbing, attach the PinchCollider to the thumb tip socket on the
      // HandVisual
      PinchCollider->AttachToComponent(
          PinchAttachMesh,
          FAttachmentTransformRules::SnapToTargetIncludingScale,
          ThumbTipSocketName);
    }
  }
}
