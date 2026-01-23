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
#include "Components/SceneComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DataSources/IsdkIHmdDataSource.h"
#include "Curves/CurveFloat.h"
#include "IsdkRayVisualComponent.generated.h"

UCLASS(
    ClassGroup = (Custom),
    meta = (BlueprintSpawnableComponent, DisplayName = "ISDK Ray Visual Component"))
class OCULUSINTERACTIONPREBUILTS_API UIsdkRayVisualComponent : public USceneComponent
{
  GENERATED_BODY()

 public:
  UIsdkRayVisualComponent();

  /**
   * @brief When true, during BeginPlay this actor will bind the configured input actions to the
   * PlayerController at index 0.
   * If false, a manual call to BindInputActionEvents must be made to bind the input actions.
   */
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = InteractionSDK)
  bool bAutoBindInputActions = true;
  UPROPERTY()
  TScriptInterface<IIsdkIHmdDataSource> HmdDataSource{};

 private:
  UPROPERTY(EditDefaultsOnly, Category = InteractionSDK)
  USkeletalMeshComponent* PinchArrow;
  UPROPERTY(EditDefaultsOnly, Category = InteractionSDK)
  UStaticMeshComponent* Cursor;
  UPROPERTY(EditInstanceOnly, Category = InteractionSDK)
  UCurveFloat* PinchArrowStrengthCurve;

  UPROPERTY()
  UMaterialInstanceDynamic* CursorMaterialInstance;
  UPROPERTY()
  UMaterialInstanceDynamic* PinchArrowMaterialInstance;

  UPROPERTY(
      BlueprintGetter = GetPinchArrowMaterial,
      BlueprintSetter = SetPinchArrowMaterial,
      EditInstanceOnly,
      Category = "InteractionSDK|Pinch Arrow|Material")
  UMaterialInterface* PinchArrowMaterial;
  UPROPERTY(
      BlueprintGetter = GetCursorMaterial,
      BlueprintSetter = SetCursorMaterial,
      EditInstanceOnly,
      Category = "InteractionSDK|Cursor|Material")
  UMaterialInterface* CursorMaterial;

  UPROPERTY(
      BlueprintGetter = GetPinchArrowMesh,
      BlueprintSetter = SetPinchArrowMesh,
      EditInstanceOnly,
      Category = "InteractionSDK|Pinch Arrow")
  USkeletalMesh* PinchArrowMesh;
  UPROPERTY(
      BlueprintGetter = GetCursorMesh,
      BlueprintSetter = SetCursorMesh,
      EditInstanceOnly,
      Category = "InteractionSDK|Cursor")
  UStaticMesh* CursorMesh;

  void UpdatePinchArrow(float InPinchStrength, FTransform PointerPoseTransform);
  void UpdateCursor(float InPinchStrength, FTransform CursorTransform, bool CollisionHitValid);

 protected:
  virtual void BeginPlay() override;
  virtual void OnRegister() override;

 public:
  virtual void OnVisibilityChanged() override;
  virtual void DestroyComponent(bool bPromoteChildren = false) override;
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void UpdateVisual(
      FTransform PointerPoseTransform,
      FTransform CursorTransform,
      bool CollisionHitValid,
      float RaySelectStrength);

  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void Initialize();

  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetCursorMesh(UStaticMesh* Mesh);
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UStaticMesh* GetCursorMesh() const
  {
    return CursorMesh;
  }

  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetCursorMaterial(UMaterialInterface* InMaterial);
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UMaterialInterface* GetCursorMaterial() const
  {
    return CursorMaterial;
  }
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  UMaterialInstanceDynamic* GetCursorMaterialInstance() const
  {
    return CursorMaterialInstance;
  }

  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetPinchArrowMesh(USkeletalMesh* Mesh);
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  USkeletalMesh* GetPinchArrowMesh() const
  {
    return PinchArrowMesh;
  }

  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetPinchArrowMaterial(UMaterialInterface* InMaterial);
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  UMaterialInterface* GetPinchArrowMaterial() const
  {
    return PinchArrowMaterial;
  }
  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  UMaterialInstanceDynamic* GetPinchArrowMaterialInstance() const
  {
    return PinchArrowMaterialInstance;
  }

  UFUNCTION(BlueprintSetter, Category = InteractionSDK)
  void SetTranslucentSortPriority(int32 InTranslucentSortPriority);
  UFUNCTION(BlueprintGetter, Category = InteractionSDK)
  int32 GetTranslucentSortPriority()
  {
    return TranslucentSortPriority;
  }

  UFUNCTION(BlueprintCallable, Category = InteractionSDK)
  void UpdateMaterialInstanceParameters();

  UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "InteractionSDK|Pinch Arrow")
  bool ShowArrow = true;
  UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "InteractionSDK|Pinch Arrow")
  TArray<FName> MorphTargetNames;
  UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "InteractionSDK|Pinch Arrow|Material")
  FLinearColor PinchArrowColor;

  UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "InteractionSDK|Cursor")
  bool ShowCursor = true;

  /**
   * @brief Distance at which the scale of the cursor is 1.0
   */
  UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "InteractionSDK|Cursor")
  float ReferenceDistance = 50.0;
  /**
   * @brief Cursor size range used for the scale change based on the current pinch strength
   */
  UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "InteractionSDK|Cursor")
  FVector2D CursorSizeMinMax = FVector2D(0.02, 0.015);
  UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "InteractionSDK|Cursor|Material")
  FLinearColor CursorAccentColor;
  UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "InteractionSDK|Cursor|Material")
  FLinearColor CursorColor;
  UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "InteractionSDK|Cursor|Material")
  FLinearColor CursorShadowColor;

 private:
  UPROPERTY(
      BlueprintGetter = GetTranslucentSortPriority,
      BlueprintSetter = SetTranslucentSortPriority,
      EditInstanceOnly,
      Category = "InteractionSDK|Cursor|Material")
  int32 TranslucentSortPriority;
};
