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

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IsdkITransformer.h"
#include "../Pointable/IsdkInteractionPointerEvent.h"
#include "IsdkIGrabbable.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FIsdkCancelGrabEventDelegate,
    int,
    Identifier,
    TScriptInterface<IIsdkIGrabbable>,
    Grabbable);

UENUM(BlueprintType)
enum class EIsdkGrabType : uint8
{
  SingleGrabFirstRetained,
  SingleGrabTransferToSecond,
  MultiGrab
};

/**
 * @brief Helper struct meant to be used to gather the pointer or grab pose events sent to a class
 * @brief meant to be used as a IGrabbable
 */
USTRUCT(BlueprintType)
struct FIsdkGrabPoseCollection
{
  GENERATED_BODY()

 public:
  void UpdatePoseArrays(const FIsdkInteractionPointerEvent& Event)
  {
    const auto EventPose = FIsdkGrabPose(Event.Identifier, Event.Pose);
    UpdatePoseArrays(Event.Type, EventPose);
  }
  void UpdatePoseArrays(const EIsdkPointerEventType Type, const FIsdkGrabPose Pose)
  {
    switch (Type)
    {
      case EIsdkPointerEventType::Hover:
        Hover(Pose);
        break;
      case EIsdkPointerEventType::Unhover:
        Unhover(Pose.Identifier);
        break;
      case EIsdkPointerEventType::Select:
        Select(Pose);
        break;
      case EIsdkPointerEventType::Unselect:
        Unselect(Pose.Identifier);
        break;
      case EIsdkPointerEventType::Move:
        Move(Pose);
        break;
      case EIsdkPointerEventType::Cancel:
        RemovePose(Pose.Identifier);
        break;
    }
  }

  void RemovePose(int Identifier);

  static int FindPose(const TArray<FIsdkGrabPose>& InPoses, int Identifier);
  const TArray<FIsdkGrabPose>& GetHoverPoses() const
  {
    return HoverPoses;
  }
  const TArray<FIsdkGrabPose>& GetSelectPoses() const
  {
    return SelectPoses;
  }

  void ClearPoses()
  {
    HoverPoses.Empty();
    SelectPoses.Empty();
  }

 private:
  TArray<FIsdkGrabPose> HoverPoses;
  TArray<FIsdkGrabPose> SelectPoses;

  void Hover(FIsdkGrabPose Pose);
  void Unhover(int Identifier);
  void Select(FIsdkGrabPose Pose);
  void Unselect(int Identifier);
  void Move(FIsdkGrabPose Pose);
};

UINTERFACE(BlueprintType, Category = "InteractionSDK")
class OCULUSINTERACTION_API UIsdkIGrabbable : public UInterface
{
  GENERATED_BODY()
};

class OCULUSINTERACTION_API IIsdkIGrabbable
{
  GENERATED_BODY()

 public:
  virtual void ProcessPointerEvent(const FIsdkInteractionPointerEvent& Event)
      PURE_VIRTUAL(IIsdkIGrabbable::ProcessPointerEvent, ;);
  virtual FIsdkCancelGrabEventDelegate* GetCancelGrabEventDelegate()
      PURE_VIRTUAL(IIsdkIGrabbable::GetCancelGrabEventDelegate, return {};);
};
