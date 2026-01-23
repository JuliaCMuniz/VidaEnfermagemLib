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

#include "Rig/IsdkControllerRigComponent.h"

#include "IsdkContentAssetPaths.h"
#include "IsdkControllerMeshComponent.h"
#include "IsdkFunctionLibrary.h"
#include "IsdkHandMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Rig/IsdkControllerVisualsRigComponent.h"
#include "UObject/ConstructorHelpers.h"

void UIsdkControllerRigComponent::OnControllerVisualsAttached()
{
  Super::OnControllerVisualsAttached();
  ControllerVisuals->CreateDataSourcesAsTrackedController();
  UpdateComponentDataSources();
}

FVector UIsdkControllerRigComponent::GetPalmColliderOffset() const
{
  const auto ControllerHandBehavior = UIsdkFunctionLibrary::GetControllerHandBehavior(GetWorld());
  const bool bUseHandOffset =
      ControllerHandBehavior == EControllerHandBehavior::HandsOnlyAnimated ||
      ControllerHandBehavior == EControllerHandBehavior::HandsOnlyProcedural;

  if (bUseHandOffset)
  {
    return HandPalmColliderOffset;
  }

  return ControllerPalmColliderOffset;
}

USkinnedMeshComponent* UIsdkControllerRigComponent::GetPinchAttachMesh() const
{
  const auto ControllerHandBehavior = UIsdkFunctionLibrary::GetControllerHandBehavior(GetWorld());
  const bool bUseHandMesh = ControllerHandBehavior == EControllerHandBehavior::HandsOnlyAnimated ||
      ControllerHandBehavior == EControllerHandBehavior::HandsOnlyProcedural;

  if (bUseHandMesh)
  {
    if (ControllerHandBehavior == EControllerHandBehavior::HandsOnlyAnimated)
    {
      return ControllerVisuals->GetAnimatedHandMeshComponent();
    }

    return ControllerVisuals->GetPoseableHandMeshComponent();
  }

  return nullptr;
}

TSubclassOf<AActor> UIsdkControllerRigComponent::FindBPFromPath(const FString& Path)
{
  ConstructorHelpers::FClassFinder<AActor> ClassFinder(*Path);
  if (ClassFinder.Succeeded())
  {
    return ClassFinder.Class;
  }

  return nullptr;
}

void UIsdkControllerRigComponent::HandleControllerHandBehaviorChanged(
    TScriptInterface<IIsdkITrackingDataSubsystem> IsdkITrackingDataSubsystem,
    EControllerHandBehavior ControllerHandBehavior,
    EControllerHandBehavior ControllerHandBehavior1)
{
  // Interactors may need to have their attach components / sockets updated when controller hand
  // behavior changes (for instance, hand-only interactor colliders should be placed differently
  // from behaviors which include a controller mesh)
  UpdateComponentDataSources();
}

UIsdkControllerRigComponentLeft::UIsdkControllerRigComponentLeft(
    const FObjectInitializer& ObjectInitializer)
    : UIsdkControllerRigComponent(
          EIsdkHandType::HandLeft,
          IsdkContentAssetPaths::ControllerDrivenHands::ControllerDrivenHandAnimBlueprintL,
          ObjectInitializer)
{
}

UIsdkControllerRigComponentRight::UIsdkControllerRigComponentRight(
    const FObjectInitializer& ObjectInitializer)
    : UIsdkControllerRigComponent(
          EIsdkHandType::HandRight,
          IsdkContentAssetPaths::ControllerDrivenHands::ControllerDrivenHandAnimBlueprintR,
          ObjectInitializer)
{
}

UIsdkControllerRigComponent::UIsdkControllerRigComponent(
    EIsdkHandType HandType,
    const FString& HandBPPath,
    const FObjectInitializer& ObjectInitializer)
    : UIsdkRigComponent(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("HandVisuals")))
{
  SetRigComponentDefaults(HandType);
}

void UIsdkControllerRigComponent::BeginPlay()
{
  Super::BeginPlay();

  const auto Delegate = UIsdkFunctionLibrary::GetControllerHandBehaviorDelegate(GetWorld());

  if (!Delegate)
  {
    return;
  }

  Delegate->AddUObject(this, &UIsdkControllerRigComponent::HandleControllerHandBehaviorChanged);
}
