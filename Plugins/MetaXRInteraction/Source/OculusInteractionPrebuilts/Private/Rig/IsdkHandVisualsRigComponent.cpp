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

#include "Rig/IsdkHandVisualsRigComponent.h"

#include "IsdkHandMeshComponent.h"
#include "IsdkContentAssetPaths.h"
#include "DataSources/IsdkOneEuroFilterDataModifier.h"
#include "DataSources/IsdkSyntheticHand.h"

#include "Components/SceneComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"

UIsdkHandVisualsRigComponent::UIsdkHandVisualsRigComponent()
{
  PrimaryComponentTick.bCanEverTick = false;
  bWantsInitializeComponent = true;

  SyntheticHand = CreateDefaultSubobject<UIsdkSyntheticHand>(TEXT("SyntheticHand"));
  TrackedHandVisual = CreateDefaultSubobject<UIsdkHandMeshComponent>(TEXT("TrackedHandVisual"));
  SyntheticHandVisual = CreateDefaultSubobject<UIsdkHandMeshComponent>(TEXT("SyntheticHandVisual"));
  OneEuroFilterDataModifier =
      CreateDefaultSubobject<UIsdkOneEuroFilterDataModifier>(TEXT("OneEuroFilter"));
}

void UIsdkHandVisualsRigComponent::InitializeComponent()
{
  Super::InitializeComponent();

  // Set data sources to default state, which will de-activate the components.
  OnDataSourcesCreated();
}

void UIsdkHandVisualsRigComponent::SetSubobjectPropertyDefaults(EIsdkHandType InHandType)
{
  // Sets default values for our components (such as mesh and material assets, bone mappings).
  // These are only used for property defaults - they can be overridden by a user in the details
  // panel of the actor instance in the editor.
  HandType = InHandType;

  struct FStaticObjectFinders
  {
    ConstructorHelpers::FObjectFinderOptional<USkeletalMesh> LeftHandMeshFinder;
    ConstructorHelpers::FObjectFinderOptional<USkeletalMesh> RightHandMeshFinder;
    ConstructorHelpers::FObjectFinderOptional<UMaterial> HandMaterialFinder;

    FStaticObjectFinders()
        : LeftHandMeshFinder(IsdkContentAssetPaths::Models::Hand::OpenXRLeftHand),
          RightHandMeshFinder(IsdkContentAssetPaths::Models::Hand::OpenXRRightHand),
          HandMaterialFinder(IsdkContentAssetPaths::Models::Hand::OculusHandMaterial)
    {
    }
  };

  static FStaticObjectFinders StaticObjectFinders;
  UMaterial* DefaultHandMaterial = StaticObjectFinders.HandMaterialFinder.Get();
  USkeletalMesh* DefaultHandMesh = nullptr;
  if (HandType == EIsdkHandType::HandLeft)
  {
    DefaultHandMesh = StaticObjectFinders.LeftHandMeshFinder.Get();
  }
  else if (HandType == EIsdkHandType::HandRight)
  {
    DefaultHandMesh = StaticObjectFinders.RightHandMeshFinder.Get();
  }

  SetHandVisualPropertyDefaults(TrackedHandVisual, DefaultHandMesh, DefaultHandMaterial);
  SetHandVisualPropertyDefaults(SyntheticHandVisual, DefaultHandMesh, DefaultHandMaterial);

  // Initialize Bones to default values. If a custom hand mesh is used, these can be overridden by
  // a user in the details panel of the actor instance in the editor.
  TrackedHandVisual->SetMappedBoneNamesAsDefault(HandType);
  SyntheticHandVisual->SetMappedBoneNamesAsDefault(HandType);

  // By default, hide the tracked hand visual so that only synthetic hand is displayed
  TrackedHandVisual->SetVisibility(false);
}

USceneComponent* UIsdkHandVisualsRigComponent::GetTrackedVisual() const
{
  return TrackedHandVisual;
}

USceneComponent* UIsdkHandVisualsRigComponent::GetSyntheticVisual() const
{
  return SyntheticHandVisual;
}

void UIsdkHandVisualsRigComponent::GetInteractorSocket(
    USceneComponent*& OutSocketComponent,
    FName& OutSocketName,
    EIsdkHandBones HandBone) const
{
  OutSocketComponent = GetTrackedVisual();
  OutSocketName = NAME_None;
  if (HandBone != EIsdkHandBones::Invalid && TrackedHandVisual)
  {
    OutSocketName = TrackedHandVisual->MappedBoneNames[static_cast<size_t>(HandBone)];
  }
}

FTransform UIsdkHandVisualsRigComponent::GetCurrentSyntheticTransform()
{
  return IIsdkIRootPose::Execute_GetRootPose(SyntheticHand);
}

void UIsdkHandVisualsRigComponent::OnDataSourcesCreated()
{
  // Now set data sources on the components
  if (IsValid(OneEuroFilterDataModifier))
  {
    OneEuroFilterDataModifier->SetInputDataSource(LocalDataSources.DataSourceComponent);
    SyntheticHand->SetInputDataSource(OneEuroFilterDataModifier);
    TrackedHandVisual->SetJointsDataSource(OneEuroFilterDataModifier);
    TrackedHandVisual->SetRootPoseDataSource(OneEuroFilterDataModifier);
    if (IsValid(LocalDataSources.DataSourceComponent))
    {
      LocalDataSources.DataSourceComponent->SetInboundHandData(
          IIsdkIHandJoints::Execute_GetHandData(OneEuroFilterDataModifier));
    }
  }
  else
  {
    // Wire up data sources to poke limiting
    SyntheticHand->SetInputDataSource(LocalDataSources.DataSourceComponent);
    // Wire up data sources to hand visuals
    TrackedHandVisual->SetJointsDataSource(LocalDataSources.DataSourceComponent);
    TrackedHandVisual->SetRootPoseDataSource(LocalDataSources.DataSourceComponent);
  }

  SyntheticHandVisual->SetJointsDataSource(SyntheticHand);
  SyntheticHandVisual->SetRootPoseDataSource(SyntheticHand);

  const bool bHasValidDataSource = IsValid(LocalDataSources.DataSourceComponent);
  OneEuroFilterDataModifier->bUpdateInTick = bHasValidDataSource;
  SyntheticHand->bUpdateInTick = bHasValidDataSource;
}

void UIsdkHandVisualsRigComponent::SetHandVisualPropertyDefaults(
    UIsdkHandMeshComponent* HandMesh,
    USkeletalMesh* DefaultHandMesh,
    UMaterial* DefaultHandMaterial)
{
  // Set mesh and material properties to default values.
  // These can be overridden by a user in the details panel of the actor instance in the editor.
  if (ensureMsgf(
          IsValid(DefaultHandMesh),
          TEXT("%s: Failed to find hand mesh"),
          *HandMesh->GetOwner()->GetFullName()))
  {
    HandMesh->SetSkinnedAsset(DefaultHandMesh);
  }

  if (ensureMsgf(
          IsValid(DefaultHandMaterial),
          TEXT("%s: Failed to find hand material"),
          *HandMesh->GetOwner()->GetFullName()))
  {
    HandMesh->SetMaterial(0, DefaultHandMaterial);
  }
}
