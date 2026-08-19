// This file is part of the FSR Upscaling Unreal Engine Plugin.
//
// Copyright (c) 2023-2025 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once
#include "CoreMinimal.h"
#include "Rendering/SlateRenderer.h"
#include "Rendering/SlateDrawBuffer.h"
#include "Slate/SlateTextures.h"
#include "Widgets/Accessibility/SlateAccessibleMessageHandler.h"
#include "Framework/Application/SlateApplication.h"
#include "FFXShared.h"
#include "FFXSlateApplication.h"

//------------------------------------------------------------------------------------------------------
// Slate override code that allows Frame Interpolation to re-render and present both the Interpolated and Real frame.
//------------------------------------------------------------------------------------------------------
class FFXFrameInterpolationSlateRenderer : public FSlateRenderer
{
	static const uint32 NumDrawBuffers = 6;
public:
	FFXFrameInterpolationSlateRenderer(TSharedRef<FSlateRenderer> InUnderlyingRenderer);
	virtual ~FFXFrameInterpolationSlateRenderer();
public:
	void OnSlateWindowRenderedThunk(SWindow& Window) { return SlateWindowRendered.Broadcast(Window); }
	void OnBackBufferReadyToPresentThunk(SWindow& Window, class ISlateViewportProvider& Provider) { return OnBackBufferReadyToPresentDelegate.Broadcast(Window, Provider); }
	virtual bool Initialize() override;
	virtual void Destroy() override;
	virtual FSlateDrawBuffer& AcquireDrawBuffer() override;
	virtual void ReleaseDrawBuffer(FSlateDrawBuffer& InWindowDrawBuffer) override;
	virtual void OnWindowDestroyed(const TSharedRef<SWindow>& InWindow) override;
	virtual void RequestResize(const TSharedPtr<SWindow>& Window, uint32 NewWidth, uint32 NewHeight) override;
	virtual void CreateViewport(const TSharedRef<SWindow> Window) override;
	virtual void UpdateFullscreenState(const TSharedRef<SWindow> Window, uint32 OverrideResX, uint32 OverrideResY) override;
	virtual void SetSystemResolution(uint32 Width, uint32 Height) override;
	virtual void RestoreSystemResolution(const TSharedRef<SWindow> InWindow) override;
	virtual void DrawWindows(FSlateDrawBuffer& InWindowDrawBuffer) override;
	virtual void FlushCommands() const override;
	virtual void Sync() const override;
	virtual void ReleaseDynamicResource(const FSlateBrush& InBrush) override;
	virtual void RemoveDynamicBrushResource(TSharedPtr<FSlateDynamicImageBrush> BrushToRemove) override;
	virtual FIntPoint GenerateDynamicImageResource(const FName InTextureName) override;
	virtual bool GenerateDynamicImageResource(FName ResourceName, uint32 Width, uint32 Height, const TArray< uint8 >& Bytes) override;
	virtual bool GenerateDynamicImageResource(FName ResourceName, FSlateTextureDataRef TextureData) override;
	virtual FSlateResourceHandle GetResourceHandle(const FSlateBrush& Brush, FVector2f LocalSize, float DrawScale) override;
	virtual bool CanRenderResource(UObject& InResourceObject) const override;
	virtual void* GetViewportResource(const SWindow& Window) override;
	virtual ISlateViewportProvider* GetViewportProvider(const SWindow& Window) override;
	virtual void SetColorVisionDeficiencyType(EColorVisionDeficiency Type, int32 Severity, bool bCorrectDeficiency, bool bShowCorrectionWithDeficiency) override;
	virtual FSlateUpdatableTexture* CreateUpdatableTexture(uint32 Width, uint32 Height) override;
	virtual FSlateUpdatableTexture* CreateSharedHandleTexture(void* SharedHandle) override;
	virtual void ReleaseUpdatableTexture(FSlateUpdatableTexture* Texture) override;
	virtual ISlateAtlasProvider* GetTextureAtlasProvider() override;
	virtual FCriticalSection* GetResourceCriticalSection() override;
	virtual void ReleaseAccessedResources(bool bImmediatelyFlush) override;
	virtual int32 GetCurrentSceneIndex() const override;
	virtual void SetCurrentSceneIndex(int32 InIndex) override;
	virtual void ClearScenes() override;
protected:
	virtual void RegisterCurrentScene_Impl(FSceneInterface* Scene) override;
public:
	EPixelFormat GetSlateRecommendedColorFormat() override;
	virtual void DestroyCachedFastPathRenderingData(struct FSlateCachedFastPathRenderingData* InRenderingData) override;
	virtual void DestroyCachedFastPathElementData(FSlateCachedElementData* InCachedElementData) override;
	virtual void EndFrame() const override;
	virtual void AddWidgetRendererUpdate(const struct FRenderThreadUpdateContext& Context, bool bDeferredRenderTargetUpdate) override;
#if WITH_MULTI_VIEW_SLATE_WINDOW_SUPPORT
	virtual void CreateNativeLayer(int32 NewNativeLayer, SWindow& InWindow, void* NativeViewHandle) override;
	virtual void DeleteNativeLayer(int32 OldNativeLayer, SWindow& InWindow) override;
#endif
	virtual void ReloadTextureResources() override;
	virtual void LoadStyleResources(const ISlateStyle& Style) override;
	virtual bool AreShadersInitialized() const override;
	virtual void PrepareToTakeScreenshot(const FIntRect& Rect, TArray<FColor>* OutColorData, SWindow* ScreenshotWindow) override;
	virtual void PrepareToTakeHDRScreenshot(const FIntRect& Rect, TArray<FLinearColor>* OutColorData, SWindow* ScreenshotWindow) override;
	virtual void OnVirtualDesktopSizeChanged(const FDisplayMetrics& NewDisplayMetric) override;
	virtual ISlateAtlasProvider* GetFontAtlasProvider() override;
	virtual void CopyWindowsToVirtualScreenBuffer(const TArray<FString>& KeypressBuffer) override;
	virtual void MapVirtualScreenBuffer(FMappedTextureBuffer* OutImageData) override;
	virtual void UnmapVirtualScreenBuffer() override;
	virtual bool HasLostDevice() const override;
private:
	FSlateDrawBuffer DrawBuffers[NumDrawBuffers];
	TArray<TSharedPtr<FSlateDynamicImageBrush>> DynamicBrushesToRemove[NumDrawBuffers];
private:
	TSharedPtr<FSlateRenderer> UnderlyingRenderer;
	uint32 FreeBufferIndex;
	uint32 ResourceVersion;
};
