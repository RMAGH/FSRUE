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

#include "FFXFrameInterpolationSlate.h"
#include "RenderingThread.h"

//------------------------------------------------------------------------------------------------------
// Helper definitions.
//------------------------------------------------------------------------------------------------------
struct FSlateReleaseDrawBufferCommandString
{
	static const TCHAR* TStr() { return TEXT("FSlateReleaseDrawBufferCommand"); }
};

struct FFISlateReleaseDrawBufferCommand final : public FRHICommand < FFISlateReleaseDrawBufferCommand, FSlateReleaseDrawBufferCommandString >
{
	FSlateDrawBuffer* DrawBuffer;

	FFISlateReleaseDrawBufferCommand(FSlateDrawBuffer* InDrawBuffer)
		: DrawBuffer(InDrawBuffer)
	{}

	void Execute(FRHICommandListBase& CmdList)
	{
		DrawBuffer->Unlock();
	}

	static void ReleaseDrawBuffer(FRHICommandListImmediate& RHICmdList, FSlateDrawBuffer* InDrawBuffer)
	{
		if (!RHICmdList.Bypass())
		{
			ALLOC_COMMAND_CL(RHICmdList, FFISlateReleaseDrawBufferCommand)(InDrawBuffer);
		}
		else
		{
			FFISlateReleaseDrawBufferCommand Cmd(InDrawBuffer);
			Cmd.Execute(RHICmdList);
		}
	}
};

FDelegateHandle OnSlateWindowRenderedHandle;
FDelegateHandle OnSlateWindowDestroyedHandle;
FDelegateHandle OnPreResizeWindowBackBufferHandle;
FDelegateHandle OnPostResizeWindowBackBufferHandle;
FDelegateHandle OnBackBufferReadyToPresentHandle;

//------------------------------------------------------------------------------------------------------
// Implementation for the SlateRenderer override that allows for more draw buffers.
//------------------------------------------------------------------------------------------------------
FFXFrameInterpolationSlateRenderer::FFXFrameInterpolationSlateRenderer(TSharedRef<FSlateRenderer> InUnderlyingRenderer) : FSlateRenderer(InUnderlyingRenderer->GetFontServices()), UnderlyingRenderer(InUnderlyingRenderer), FreeBufferIndex(0), ResourceVersion(0)
{
	SlateWindowRendered = MoveTemp(InUnderlyingRenderer->OnSlateWindowRendered());
	OnBackBufferReadyToPresentDelegate = MoveTemp(InUnderlyingRenderer->OnBackBufferReadyToPresent());
	InUnderlyingRenderer->OnSlateWindowRendered().Clear();
	InUnderlyingRenderer->OnBackBufferReadyToPresent().Clear();
	OnSlateWindowRenderedHandle = InUnderlyingRenderer->OnSlateWindowRendered().AddRaw(this, &FFXFrameInterpolationSlateRenderer::OnSlateWindowRenderedThunk);
	OnBackBufferReadyToPresentHandle = InUnderlyingRenderer->OnBackBufferReadyToPresent().AddRaw(this, &FFXFrameInterpolationSlateRenderer::OnBackBufferReadyToPresentThunk);
	FSlateApplication::Get().OnPreShutdown().AddLambda([InUnderlyingRenderer]()
	{
		InUnderlyingRenderer->OnSlateWindowRendered().Remove(OnSlateWindowRenderedHandle);
		InUnderlyingRenderer->OnBackBufferReadyToPresent().Remove(OnBackBufferReadyToPresentHandle);
	});
}

FFXFrameInterpolationSlateRenderer::~FFXFrameInterpolationSlateRenderer()
{
	UnderlyingRenderer->OnSlateWindowRendered().Remove(OnSlateWindowRenderedHandle);
	UnderlyingRenderer->OnBackBufferReadyToPresent().Remove(OnBackBufferReadyToPresentHandle);
}

bool FFXFrameInterpolationSlateRenderer::Initialize()
{
    return true;
}

void FFXFrameInterpolationSlateRenderer::Destroy()
{
    UnderlyingRenderer->Destroy();
}

FSlateDrawBuffer& FFXFrameInterpolationSlateRenderer::AcquireDrawBuffer()
{
    FreeBufferIndex = (FreeBufferIndex + 1) % NumDrawBuffers;
    FSlateDrawBuffer* Buffer = &DrawBuffers[FreeBufferIndex];
    while (!Buffer->Lock())
    {
        // If the buffer cannot be locked then the buffer is still in use.  If we are here all buffers are in use
        // so wait until one is free.
        if (IsInSlateThread())
        {
            // We can't flush commands on the slate thread, so simply spinlock until we're done
            // this happens if the render thread becomes completely blocked by expensive tasks when the Slate thread is running
            // in this case we cannot tick Slate.
            FPlatformProcess::Sleep(0.001f);
        }
        else
        {
            FlushCommands();
            UE_LOG(LogSlate, Warning, TEXT("Slate: Had to block on waiting for a draw buffer"));
            FreeBufferIndex = (FreeBufferIndex + 1) % NumDrawBuffers;
        }
        Buffer = &DrawBuffers[FreeBufferIndex];
    }
    // Safely remove brushes by emptying the array and releasing references
    DynamicBrushesToRemove[FreeBufferIndex].Empty();
    Buffer->ClearBuffer();
    Buffer->UpdateResourceVersion(ResourceVersion);
    return *Buffer;
}

void FFXFrameInterpolationSlateRenderer::ReleaseDrawBuffer(FSlateDrawBuffer& InWindowDrawBuffer)
{
#if DO_CHECK
    bool bFound = false;
    for (int32 Index = 0; Index < NumDrawBuffers; ++Index)
    {
        if (&DrawBuffers[Index] == &InWindowDrawBuffer)
        {
            bFound = true;
            break;
        }
    }
    ensureMsgf(bFound, TEXT("It release a DrawBuffer that is not a member of the SlateRHIRenderer"));
#endif
    FSlateDrawBuffer* DrawBuffer = &InWindowDrawBuffer;
    ENQUEUE_RENDER_COMMAND(SlateReleaseDrawBufferCommand)(
        [DrawBuffer](FRHICommandListImmediate& RHICmdList)
        {
            FFISlateReleaseDrawBufferCommand::ReleaseDrawBuffer(RHICmdList, DrawBuffer);
        }
    );
}

void FFXFrameInterpolationSlateRenderer::OnWindowDestroyed(const TSharedRef<SWindow>& InWindow)
{
    return UnderlyingRenderer->OnWindowDestroyed(InWindow);
}

void FFXFrameInterpolationSlateRenderer::RequestResize(const TSharedPtr<SWindow>& Window, uint32 NewWidth, uint32 NewHeight)
{
    UnderlyingRenderer->RequestResize(Window, NewWidth, NewHeight);
}

void FFXFrameInterpolationSlateRenderer::CreateViewport(const TSharedRef<SWindow> Window)
{
    UnderlyingRenderer->CreateViewport(Window);
}

void FFXFrameInterpolationSlateRenderer::UpdateFullscreenState(const TSharedRef<SWindow> Window, uint32 OverrideResX, uint32 OverrideResY)
{
    UnderlyingRenderer->UpdateFullscreenState(Window, OverrideResX, OverrideResY);
}

void FFXFrameInterpolationSlateRenderer::SetSystemResolution(uint32 Width, uint32 Height)
{
    UnderlyingRenderer->SetSystemResolution(Width, Height);
}

void FFXFrameInterpolationSlateRenderer::RestoreSystemResolution(const TSharedRef<SWindow> InWindow)
{
    UnderlyingRenderer->RestoreSystemResolution(InWindow);
}

void FFXFrameInterpolationSlateRenderer::DrawWindows(FSlateDrawBuffer& InWindowDrawBuffer)
{
    UnderlyingRenderer->DrawWindows(InWindowDrawBuffer);
}

void FFXFrameInterpolationSlateRenderer::FlushCommands() const
{
    return UnderlyingRenderer->FlushCommands();
}

void FFXFrameInterpolationSlateRenderer::Sync() const
{
    return UnderlyingRenderer->Sync();
}

void FFXFrameInterpolationSlateRenderer::ReleaseDynamicResource(const FSlateBrush& InBrush)
{
    return UnderlyingRenderer->ReleaseDynamicResource(InBrush);
}

void FFXFrameInterpolationSlateRenderer::RemoveDynamicBrushResource(TSharedPtr<FSlateDynamicImageBrush> BrushToRemove)
{
    if (BrushToRemove.IsValid())
    {
        DynamicBrushesToRemove[FreeBufferIndex].Add(BrushToRemove);
    }
}

FIntPoint FFXFrameInterpolationSlateRenderer::GenerateDynamicImageResource(const FName InTextureName)
{
    return UnderlyingRenderer->GenerateDynamicImageResource(InTextureName);
}

bool FFXFrameInterpolationSlateRenderer::GenerateDynamicImageResource(FName ResourceName, uint32 Width, uint32 Height, const TArray< uint8 >& Bytes)
{
    return UnderlyingRenderer->GenerateDynamicImageResource(ResourceName, Width, Height, Bytes);
}

bool FFXFrameInterpolationSlateRenderer::GenerateDynamicImageResource(FName ResourceName, FSlateTextureDataRef TextureData)
{
    return UnderlyingRenderer->GenerateDynamicImageResource(ResourceName, TextureData);
}

FSlateResourceHandle FFXFrameInterpolationSlateRenderer::GetResourceHandle(const FSlateBrush& Brush, FVector2f LocalSize, float DrawScale)
{
    return UnderlyingRenderer->GetResourceHandle(Brush, LocalSize, DrawScale);
}

bool FFXFrameInterpolationSlateRenderer::CanRenderResource(UObject& InResourceObject) const
{
    return UnderlyingRenderer->CanRenderResource(InResourceObject);
}

void* FFXFrameInterpolationSlateRenderer::GetViewportResource(const SWindow& Window)
{
    return UnderlyingRenderer->GetViewportResource(Window);
}

ISlateViewportProvider* FFXFrameInterpolationSlateRenderer::GetViewportProvider(const SWindow& Window)
{
    return UnderlyingRenderer->GetViewportProvider(Window);
}

void FFXFrameInterpolationSlateRenderer::SetColorVisionDeficiencyType(EColorVisionDeficiency Type, int32 Severity, bool bCorrectDeficiency, bool bShowCorrectionWithDeficiency)
{
    UnderlyingRenderer->SetColorVisionDeficiencyType(Type, Severity, bCorrectDeficiency, bShowCorrectionWithDeficiency);
}

FSlateUpdatableTexture* FFXFrameInterpolationSlateRenderer::CreateUpdatableTexture(uint32 Width, uint32 Height)
{
    return UnderlyingRenderer->CreateUpdatableTexture(Width, Height);
}

FSlateUpdatableTexture* FFXFrameInterpolationSlateRenderer::CreateSharedHandleTexture(void* SharedHandle)
{
    return UnderlyingRenderer->CreateSharedHandleTexture(SharedHandle);
}

void FFXFrameInterpolationSlateRenderer::ReleaseUpdatableTexture(FSlateUpdatableTexture* Texture)
{
    return UnderlyingRenderer->ReleaseUpdatableTexture(Texture);
}

ISlateAtlasProvider* FFXFrameInterpolationSlateRenderer::GetTextureAtlasProvider()
{
    return UnderlyingRenderer->GetTextureAtlasProvider();
}

FCriticalSection* FFXFrameInterpolationSlateRenderer::GetResourceCriticalSection()
{
    return UnderlyingRenderer->GetResourceCriticalSection();
}

void FFXFrameInterpolationSlateRenderer::ReleaseAccessedResources(bool bImmediatelyFlush)
{
    UnderlyingRenderer->ReleaseAccessedResources(bImmediatelyFlush);
    if (bImmediatelyFlush)
    {
        ResourceVersion++;
    }
}

int32 FFXFrameInterpolationSlateRenderer::GetCurrentSceneIndex() const
{
    return UnderlyingRenderer->GetCurrentSceneIndex();
}

void FFXFrameInterpolationSlateRenderer::SetCurrentSceneIndex(int32 InIndex)
{
    return UnderlyingRenderer->SetCurrentSceneIndex(InIndex);
}

void FFXFrameInterpolationSlateRenderer::ClearScenes()
{
    return UnderlyingRenderer->ClearScenes();
}

void FFXFrameInterpolationSlateRenderer::RegisterCurrentScene_Impl(FSceneInterface* Scene)
{
}

EPixelFormat FFXFrameInterpolationSlateRenderer::GetSlateRecommendedColorFormat()
{
    return UnderlyingRenderer->GetSlateRecommendedColorFormat();
}

void FFXFrameInterpolationSlateRenderer::DestroyCachedFastPathRenderingData(struct FSlateCachedFastPathRenderingData* InRenderingData)
{
    return UnderlyingRenderer->DestroyCachedFastPathRenderingData(InRenderingData);
}

void FFXFrameInterpolationSlateRenderer::DestroyCachedFastPathElementData(FSlateCachedElementData* InCachedElementData)
{
    return UnderlyingRenderer->DestroyCachedFastPathElementData(InCachedElementData);
}

void FFXFrameInterpolationSlateRenderer::EndFrame() const
{
    return UnderlyingRenderer->EndFrame();
}

void FFXFrameInterpolationSlateRenderer::AddWidgetRendererUpdate(const struct FRenderThreadUpdateContext& Context, bool bDeferredRenderTargetUpdate)
{
    return UnderlyingRenderer->AddWidgetRendererUpdate(Context, bDeferredRenderTargetUpdate);
}

#if WITH_MULTI_VIEW_SLATE_WINDOW_SUPPORT
void FFXFrameInterpolationSlateRenderer::CreateNativeLayer(int32 NewNativeLayer, SWindow& InWindow, void* NativeViewHandle)
{
    UnderlyingRenderer->CreateNativeLayer(NewNativeLayer, InWindow, NativeViewHandle);
}
void FFXFrameInterpolationSlateRenderer::DeleteNativeLayer(int32 OldNativeLayer, SWindow& InWindow)
{
    UnderlyingRenderer->DeleteNativeLayer(OldNativeLayer, InWindow);
}
#endif

void FFXFrameInterpolationSlateRenderer::ReloadTextureResources()
{
    return UnderlyingRenderer->ReloadTextureResources();
}

void FFXFrameInterpolationSlateRenderer::LoadStyleResources(const ISlateStyle& Style)
{
    return UnderlyingRenderer->LoadStyleResources(Style);
}

bool FFXFrameInterpolationSlateRenderer::AreShadersInitialized() const
{
    return UnderlyingRenderer->AreShadersInitialized();
}

void FFXFrameInterpolationSlateRenderer::PrepareToTakeScreenshot(const FIntRect& Rect, TArray<FColor>* OutColorData, SWindow* ScreenshotWindow)
{
    return UnderlyingRenderer->PrepareToTakeScreenshot(Rect, OutColorData, ScreenshotWindow);
}

void FFXFrameInterpolationSlateRenderer::PrepareToTakeHDRScreenshot(const FIntRect& Rect, TArray<FLinearColor>* OutColorData, SWindow* ScreenshotWindow)
{
    return UnderlyingRenderer->PrepareToTakeHDRScreenshot(Rect, OutColorData, ScreenshotWindow);
}

void FFXFrameInterpolationSlateRenderer::OnVirtualDesktopSizeChanged(const FDisplayMetrics& NewDisplayMetric)
{
    return UnderlyingRenderer->OnVirtualDesktopSizeChanged(NewDisplayMetric);
}

ISlateAtlasProvider* FFXFrameInterpolationSlateRenderer::GetFontAtlasProvider()
{
    return UnderlyingRenderer->GetFontAtlasProvider();
}

void FFXFrameInterpolationSlateRenderer::CopyWindowsToVirtualScreenBuffer(const TArray<FString>& KeypressBuffer)
{
    return UnderlyingRenderer->CopyWindowsToVirtualScreenBuffer(KeypressBuffer);
}

void FFXFrameInterpolationSlateRenderer::MapVirtualScreenBuffer(FMappedTextureBuffer* OutImageData)
{
    return UnderlyingRenderer->MapVirtualScreenBuffer(OutImageData);
}

void FFXFrameInterpolationSlateRenderer::UnmapVirtualScreenBuffer()
{
    return UnderlyingRenderer->UnmapVirtualScreenBuffer();
}

bool FFXFrameInterpolationSlateRenderer::HasLostDevice() const
{
    return UnderlyingRenderer->HasLostDevice();
}
