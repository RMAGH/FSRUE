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

class FSlateApplication : public FSlateApplicationBase, public FGenericApplicationMessageHandler
{
public:
	SLATE_API virtual ~FSlateApplication();
	const float GetAverageDeltaTime() const { return AverageDeltaTime; }
	const float GetDeltaTime() const { return (float)(CurrentTime - LastTickTime); }
	float GetAverageDeltaTimeForResponsiveness() const { return AverageDeltaTimeForResponsiveness; }
	static SLATE_API void Create();
	static SLATE_API TSharedRef<FSlateApplication> Create(const TSharedRef<class GenericApplication>& InPlatformApplication);
	static SLATE_API TSharedRef<FSlateApplication> InitializeAsStandaloneApplication(const TSharedRef< class FSlateRenderer >& PlatformRenderer);
	static SLATE_API TSharedRef<FSlateApplication> InitializeAsStandaloneApplication(const TSharedRef< class FSlateRenderer >& PlatformRenderer, const TSharedRef<class GenericApplication>& InPlatformApplication);
	static SLATE_API void InitializeCoreStyle();
	static bool IsInitialized() { return CurrentApplication.IsValid(); }
	static FSlateApplication& Get()
	{
		check(CurrentApplication.IsValid());
		check(IsInGameThread() || IsInSlateThread() || IsInAsyncLoadingThread() || IsInParallelLoadingThread());
		return *CurrentApplication;
	}
	static SLATE_API void Shutdown(bool bShutdownPlatform = true);
	static SLATE_API TSharedRef<class FGlobalTabmanager> GetGlobalTabManager();
	static SLATE_API void InitHighDPI(const bool bForceEnable);
	SLATE_API const class FStyleNode* GetRootStyle() const;
	SLATE_API virtual bool InitializeRenderer(TSharedRef<FSlateRenderer> InRenderer, bool bQuietMode = false);
	SLATE_API virtual void InitializeSound(const TSharedRef<ISlateSoundDevice>& InSlateSoundDevice);
	SLATE_API void PlaySound(const FSlateSound& SoundToPlay, int32 UserIndex = 0) const;
	SLATE_API float GetSoundDuration(const FSlateSound& Sound) const;
	IInputInterface* GetInputInterface() const { return PlatformApplication->GetInputInterface(); }
	bool SupportsSystemHelp() const { return PlatformApplication->SupportsSystemHelp(); }
	void ShowSystemHelp() { PlatformApplication->ShowSystemHelp(); }
	ITextInputMethodSystem* GetTextInputMethodSystem() const { return PlatformApplication->GetTextInputMethodSystem(); }
	SLATE_API void SetCursorPos(const FVector2D& MouseCoordinate) override;
	SLATE_API void OverridePlatformTextField(TUniquePtr<IPlatformTextField> PlatformTextField);
	SLATE_API void UsePlatformCursorForCursorUser(bool bUsePlatformCursor);
	SLATE_API void SetPlatformCursorVisibility(bool bNewVisibility);
	SLATE_API void PollGameDeviceState();
	SLATE_API void FinishedInputThisFrame();
	SLATE_API void Tick(ESlateTickType TickType = ESlateTickType::All);
	SLATE_API bool IsTicking() const;
	SLATE_API void PumpMessages();
	SLATE_API bool CanAddModalWindow() const;
	SLATE_API bool CanDisplayWindows() const;
	SLATE_API virtual EUINavigation GetNavigationDirectionFromKey(const FKeyEvent& InKeyEvent) const override;
	SLATE_API virtual EUINavigation GetNavigationDirectionFromAnalog(const FAnalogInputEvent& InAnalogEvent) override;
	SLATE_API virtual EUINavigationAction GetNavigationActionFromKey(const FKeyEvent& InKeyEvent) const override;
	UE_DEPRECATED(4.24, "GetNavigationActionForKey doesn't handle multiple users properly, use GetNavigationActionFromKey instead")
	SLATE_API virtual EUINavigationAction GetNavigationActionForKey(const FKey& InKey) const override;
	SLATE_API void AddModalWindow(TSharedRef<SWindow> InSlateWindow, const TSharedPtr<const SWidget> InParentWidget, bool bSlowTaskWindow = false);
	SLATE_API void SetModalWindowStackStartedDelegate(FModalWindowStackStarted StackStartedDelegate);
	SLATE_API void SetModalWindowStackEndedDelegate(FModalWindowStackEnded StackEndedDelegate);
	SLATE_API TSharedRef<SWindow> AddWindowAsNativeChild(TSharedRef<SWindow> InSlateWindow, TSharedRef<SWindow> InParentWindow, const bool bShowImmediately = true);
	SLATE_API TSharedPtr<IMenu> PushMenu(const TSharedRef<SWidget>& InParentWidget, const FWidgetPath& InOwnerPath, const TSharedRef<SWidget>& InContent, const UE::Slate::FDeprecateVector2DParameter& SummonLocation, const FPopupTransitionEffect& TransitionEffect, const bool bFocusImmediately = true, const UE::Slate::FDeprecateVector2DParameter& SummonLocationSize = FVector2f::ZeroVector, TOptional<EPopupMethod> Method = TOptional<EPopupMethod>(), const bool bIsCollapsedByParent = true, const int32 FocusUserIndex = INDEX_NONE);
	SLATE_API TSharedPtr<IMenu> PushMenu(const TSharedPtr<IMenu>& InParentMenu, const TSharedRef<SWidget>& InContent, const UE::Slate::FDeprecateVector2DParameter& SummonLocation, const FPopupTransitionEffect& TransitionEffect, const bool bFocusImmediately = true, const UE::Slate::FDeprecateVector2DParameter& SummonLocationSize = FVector2f::ZeroVector, const bool bIsCollapsedByParent = true, const int32 FocusUserIndex = INDEX_NONE);
	SLATE_API TSharedPtr<IMenu> PushHostedMenu(const TSharedRef<SWidget>& InParentWidget, const FWidgetPath& InOwnerPath, const TSharedRef<IMenuHost>& InMenuHost, const TSharedRef<SWidget>& InContent, TSharedPtr<SWidget>& OutWrappedContent, const FPopupTransitionEffect& TransitionEffect, EShouldThrottle ShouldThrottle, const bool bIsCollapsedByParent = true);
	SLATE_API TSharedPtr<IMenu> PushHostedMenu(const TSharedPtr<IMenu>& InParentMenu, const TSharedRef<IMenuHost>& InMenuHost, const TSharedRef<SWidget>& InContent, TSharedPtr<SWidget>& OutWrappedContent, const FPopupTransitionEffect& TransitionEffect, EShouldThrottle ShouldThrottle, const bool bIsCollapsedByParent = true);
	SLATE_API bool HasOpenSubMenus(TSharedPtr<IMenu> InMenu) const;
	SLATE_API virtual bool AnyMenusVisible() const override;
	SLATE_API TSharedPtr<IMenu> FindMenuInWidgetPath(const FWidgetPath& InWidgetPath) const;
	SLATE_API TSharedPtr<SWindow> GetVisibleMenuWindow() const;
	SLATE_API TSharedPtr<SWidget> GetMenuHostWidget() const;
	SLATE_API virtual void DismissAllMenus() override;
	SLATE_API void DismissMenu(const TSharedPtr<IMenu>& InFromMenu);
	SLATE_API void DismissMenuByWidget(const TSharedRef<SWidget>& InWidgetInMenu);
	SLATE_API void DestroyWindowImmediately(TSharedRef<SWindow> WindowToDestroy);
	SLATE_API void ExternalModalStart();
	SLATE_API void ExternalModalStop();
	int GetNumExternalModalWindowsActive() const { return NumExternalModalWindowsActive; }
	DECLARE_EVENT_OneParam(FSlateApplication, FSlateTickEvent, float);
	FSlateTickEvent& OnPreTick() { return PreTickEvent; }
	FSlateTickEvent& OnPostTick() { return PostTickEvent; }
	FSimpleMulticastDelegate& OnPreShutdown() { return PreShutdownEvent; }
	DECLARE_EVENT_OneParam(FSlateApplication, FUserRegisteredEvent, int32);
	FUserRegisteredEvent& OnUserRegistered() { return UserRegisteredEvent; }
	DECLARE_EVENT_OneParam(FSlateApplication, FOnWindowBeingDestroyed, const SWindow&);
	FOnWindowBeingDestroyed& OnWindowBeingDestroyed() { return WindowBeingDestroyedEvent; }
	FOnMenuDestroyed& OnMenuBeingDestroyed() { return MenuBeingDestroyedEvent; }
	DECLARE_MULTICAST_DELEGATE_FiveParams(FOnFocusChanging, const FFocusEvent&, const FWeakWidgetPath&, const TSharedPtr<SWidget>&, const FWidgetPath&, const TSharedPtr<SWidget>&);
	FOnFocusChanging& OnFocusChanging() { return FocusChangingDelegate; }
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPreScreenshot, TSharedRef<SWidget>)
	FOnPreScreenshot& OnPreScreenshot() { return PreScreenshotDelegate; }
	SLATE_API void RegisterGameViewport(TSharedRef<SViewport> InViewport);
	SLATE_API void RegisterViewport(TSharedRef<SViewport> InViewport);
	SLATE_API TSharedPtr<SViewport> GetGameViewport() const;
	SLATE_API void UnregisterGameViewport();
	SLATE_API void RegisterVirtualWindow(TSharedRef<SWindow> InWindow);
	SLATE_API void UnregisterVirtualWindow(TSharedRef<SWindow> InWindow);
	SLATE_API void FlushRenderState();
	SLATE_API void SetUserFocusToGameViewport(uint32 UserIndex, EFocusCause ReasonFocusIsChanging = EFocusCause::SetDirectly);
	SLATE_API void SetAllUserFocusToGameViewport(EFocusCause ReasonFocusIsChanging = EFocusCause::SetDirectly);
	SLATE_API void ActivateGameViewport();
	SLATE_API bool GetTransformFullscreenMouseInput() const;
#if WITH_SLATE_DEBUGGING
	SLATE_API void TryDumpNavigationConfig(TSharedPtr<FNavigationConfig> InNavigationConfig) const;
#endif 
	SLATE_API bool SetUserFocus(uint32 UserIndex, const TSharedPtr<SWidget>& WidgetToFocus, EFocusCause ReasonFocusIsChanging = EFocusCause::SetDirectly);
	SLATE_API void SetAllUserFocus(const TSharedPtr<SWidget>& WidgetToFocus, EFocusCause ReasonFocusIsChanging = EFocusCause::SetDirectly);
	SLATE_API void ClearUserFocus(uint32 UserIndex, EFocusCause ReasonFocusIsChanging = EFocusCause::SetDirectly);
	SLATE_API void ClearAllUserFocus(EFocusCause ReasonFocusIsChanging = EFocusCause::SetDirectly);
	SLATE_API void SetPendingNavigationContext(uint32 UserIndex, TSharedPtr<ISlateMetaData> InContext);
	template <typename T>
	TSharedPtr<T> GetPendingNavigationContext(uint32 UserIndex) const
	{
		if (const TSharedPtr<ISlateMetaData>* Found = PendingNavigationContexts.Find(UserIndex))
		{
			if ((*Found)->IsOfType<T>())
			{
				return StaticCastSharedPtr<T>(*Found);
			}
		}
		return nullptr;
	}
	SLATE_API void ClearPendingNavigationContext(uint32 UserIndex);
	SLATE_API bool SetKeyboardFocus(const TSharedPtr<SWidget>& OptionalWidgetToFocus, EFocusCause ReasonFocusIsChanging = EFocusCause::SetDirectly);
	SLATE_API void ClearKeyboardFocus(const EFocusCause ReasonFocusIsChanging = EFocusCause::SetDirectly);
	DECLARE_EVENT_OneParam(FSlateApplication, FOnRegularWindowMovedOrResized, TSharedRef<SWindow>);
	FOnRegularWindowMovedOrResized& OnRegularWindowMovedOrResized() { return OnRegularWindowMovedOrResizedEvent; }
#if WITH_EDITOR
	DECLARE_EVENT_OneParam(FSlateApplication, FOnApplicationPreInputKeyDownListener, const FKeyEvent&);
	FOnApplicationPreInputKeyDownListener& OnApplicationPreInputKeyDownListener() { return OnApplicationPreInputKeyDownListenerEvent; }
	DECLARE_EVENT_OneParam(FSlateApplication, FOnApplicationMousePreInputButtonDownListener, const FPointerEvent&);
	FOnApplicationMousePreInputButtonDownListener& OnApplicationMousePreInputButtonDownListener() { return OnApplicationMousePreInputButtonDownListenerEvent; }
	DECLARE_EVENT_OneParam(FSlateApplication, FOnWindowDPIScaleChanged, TSharedRef<SWindow>);
	FOnWindowDPIScaleChanged& OnWindowDPIScaleChanged() { return OnWindowDPIScaleChangedEvent; }
	FOnWindowDPIScaleChanged& OnSystemSignalsDPIChanged() { return OnSignalSystemDPIChangedEvent; }
#endif 
	SLATE_API FModifierKeysState GetModifierKeys() const;
	SLATE_API void ResetToDefaultInputSettings();
	SLATE_API void ResetToDefaultPointerInputSettings();
	void SetHandleDeviceInputWhenApplicationNotActive(bool bAllow) { bHandleDeviceInputWhenApplicationNotActive = bAllow; }
	bool GetHandleDeviceInputWhenApplicationNotActive() const { return bHandleDeviceInputWhenApplicationNotActive; }
	SLATE_API virtual void* GetMouseCaptureWindow() const;
	SLATE_API void ReleaseAllPointerCapture();
	UE_DEPRECATED(4.23, "ReleaseMouseCapture has been renamed to ReleaseAllPointerCapture()")
	SLATE_API void ReleaseMouseCapture();
	SLATE_API void ReleaseAllPointerCapture(int32 UserIndex);
	UE_DEPRECATED(4.23, "ReleaseMouseCaptureForUser has been renamed to ReleaseAllPointerCapture(int32 UserIndex)")
	SLATE_API void ReleaseMouseCaptureForUser(int32 UserIndex);
	SLATE_API TSharedPtr<SWindow> GetActiveModalWindow() const;
	SLATE_API void SetExitRequestedHandler(const FSimpleDelegate& OnExitRequestedHandler);
	SLATE_API bool GeneratePathToWidgetUnchecked(TSharedRef<const SWidget> InWidget, FWidgetPath& OutWidgetPath, EVisibility VisibilityFilter = EVisibility::Visible) const;
	SLATE_API void GeneratePathToWidgetChecked(TSharedRef<const SWidget> InWidget, FWidgetPath& OutWidgetPath, EVisibility VisibilityFilter = EVisibility::Visible) const;
	SLATE_API virtual TSharedPtr<SWindow> FindWidgetWindow(TSharedRef<const SWidget> InWidget) const override;
	UE_DEPRECATED(4.23, "The FindWidgetWindow method that takes an FWidgetPath has been deprecated.  If you dont need the widget path, use FindWidgetWindow(MyWidget) instead.  If you need the path use GeneratePathToWidget")
	SLATE_API TSharedPtr<SWindow> FindWidgetWindow(TSharedRef<const SWidget> InWidget, FWidgetPath& OutWidgetPath) const;
	bool IsUsingHighPrecisionMouseMovment() const { return PlatformApplication.IsValid() ? PlatformApplication->IsUsingHighPrecisionMouseMode() : false; }
	bool IsUsingTrackpad() const { return PlatformApplication.IsValid() ? PlatformApplication->IsUsingTrackpad() : false; }
	bool IsMouseAttached() const { return PlatformApplication.IsValid() ? PlatformApplication->IsMouseAttached() : false; }
	bool IsGamepadAttached() const { return PlatformApplication.IsValid() ? PlatformApplication->IsGamepadAttached() : false; }
	SLATE_API void SetWidgetReflector(const TSharedRef<IWidgetReflector>& WidgetReflector);
	void SetWidgetReflectorSourceAccessDelegate(FAccessSourceCode AccessDelegate) { SourceCodeAccessDelegate = AccessDelegate; }
	void SetWidgetReflectorQuerySourceAccessDelegate(FQueryAccessSourceCode QueryAccessDelegate) { QuerySourceCodeAccessDelegate = QueryAccessDelegate; }
	void SetWidgetReflectorAssetAccessDelegate(FAccessAsset AccessDelegate) { AssetAccessDelegate = AccessDelegate; }
	void SetWidgetReflectorDebugAccessDelegate(FAccessDebugObject AccessDelegate) { DebugObjectAccessDelegate = AccessDelegate; }
	void SetApplicationScale(float InScale) { Scale = InScale; }
	virtual void GetInitialDisplayMetrics(FDisplayMetrics& OutDisplayMetrics) const { PlatformApplication->GetInitialDisplayMetrics(OutDisplayMetrics); }
	SLATE_API bool IsDragDropping() const;
	SLATE_API bool IsDragDroppingAffected(const FPointerEvent& InPointerEvent) const;
	SLATE_API TSharedPtr<class FDragDropOperation> GetDragDroppingContent() const;
	SLATE_API void CancelDragDrop();
	const TAttribute<bool>& GetNormalExecutionAttribute() const { return NormalExecutionGetter; }
	bool IsNormalExecution() const { return !GIntraFrameDebuggingGameThread; }
	bool InKismetDebuggingMode() const { return GIntraFrameDebuggingGameThread; }
	SLATE_API void EnterDebuggingMode();
	SLATE_API void LeaveDebuggingMode(bool bLeavingDebugForSingleStep = false);
#if WITH_EDITOR
	struct FScopedPreventDebuggingMode
	{
		UE_NODISCARD_CTOR SLATE_API FScopedPreventDebuggingMode(FText Reason);
		SLATE_API ~FScopedPreventDebuggingMode();
	private:
		int32 Id;
	};
#endif
	SLATE_API virtual UE::Slate::FDeprecateVector2DResult CalculatePopupWindowPosition(const FSlateRect& InAnchor, const UE::Slate::FDeprecateVector2DParameter& InSize, bool bAutoAdjustForDPIScale = true, const UE::Slate::FDeprecateVector2DParameter& InProposedPlacement = FVector2f::ZeroVector, const EOrientation Orientation = Orient_Vertical, const EPopupLayoutMode = EPopupLayoutMode::Menu) const;
	SLATE_API virtual UE::Slate::FDeprecateVector2DResult CalculateTooltipWindowPosition(const FSlateRect& InAnchorRect, const UE::Slate::FDeprecateVector2DParameter& InSize, bool bAutoAdjustForDPIScale, EPopupCursorOverlapMode CursorOverlapMode = EPopupCursorOverlapMode::PreventOverlap) const;
	SLATE_API bool IsWindowInDestroyQueue(TSharedRef<SWindow> Window) const;
	SLATE_API bool IsRunningAtTargetFrameRate() const;
	SLATE_API bool AreMenuAnimationsEnabled() const;
	UE_DEPRECATED(5.0, "Enable Window Animations is no longer used and is a no-op so calling this function is no longer necessary.")
	SLATE_API void EnableMenuAnimations(const bool bEnableAnimations);
	SLATE_API void SetPlatformApplication(const TSharedRef<class GenericApplication>& InPlatformApplication);
	SLATE_API void OverridePlatformApplication(TSharedPtr<class GenericApplication> InPlatformApplication);
	UE_DEPRECATED(4.26, "SetAppIcon has been deprecated.  Set \"AppIcon\" in your applications style to override the icon")
	SLATE_API void SetAppIcon(const FSlateBrush* const InAppIcon);
	void ExternalUIChange(bool bIsOpening) { bIsExternalUIOpened = bIsOpening; }
	SLATE_API void ShowVirtualKeyboard(bool bShow, int32 UserIndex, TSharedPtr<IVirtualKeyboardEntry> TextEntryWidget = nullptr);
	SLATE_API bool AllowMoveCursor();
	SLATE_API FSlateRect GetWorkArea(const FSlateRect& InRect) const;
	virtual void NativeApp_ShowKeyboard(bool bShow, FString InitialString = "", int32 SelectionStart = -1, int32 SelectionEnd = -1) {}
	SLATE_API bool SupportsSourceAccess() const;
	SLATE_API void GotoLineInSource(const FString& FileName, int32 LineNumber) const;
	FPopupSupport& GetPopupSupport() { return PopupSupport; }
	SLATE_API void ForceRedrawWindow(const TSharedRef<SWindow>& InWindowToDraw);
	SLATE_API bool TakeScreenshot(const TSharedRef<SWidget>& Widget, TArray<FColor>& OutColorData, FIntVector& OutSize);
	SLATE_API bool TakeHDRScreenshot(const TSharedRef<SWidget>& Widget, TArray<FLinearColor>& OutColorData, FIntVector& OutSize);
	SLATE_API bool TakeScreenshot(const TSharedRef<SWidget>& Widget, const FIntRect& InnerWidgetArea, TArray<FColor>& OutColorData, FIntVector& OutSize);
	SLATE_API bool TakeHDRScreenshot(const TSharedRef<SWidget>& Widget, const FIntRect& InnerWidgetArea, TArray<FLinearColor>& OutColorData, FIntVector& OutSize);
	inline TSharedPtr<const FSlateUser> GetUser(int32 UserIndex) const { return Users.IsValidIndex(UserIndex) ? Users[UserIndex] : nullptr; }
	inline TSharedPtr<FSlateUser> GetUser(int32 UserIndex) { return Users.IsValidIndex(UserIndex) ? Users[UserIndex] : nullptr; }
	SLATE_API TSharedPtr<FSlateUser> GetUser(FPlatformUserId PlatformUser);
	inline TSharedPtr<const FSlateUser> GetUser(const FInputEvent& InputEvent) const { return GetUser(InputEvent.GetUserIndex()); }
	inline TSharedPtr<FSlateUser> GetUser(const FInputEvent& InputEvent) { return GetUser(InputEvent.GetUserIndex()); }
	inline TSharedPtr<FSlateUser> GetUserFromControllerId(int32 ControllerId)
	{
		TOptional<int32> UserIndex = GetUserIndexForController(ControllerId);
		if (UserIndex.IsSet())
		{
			return GetUser(UserIndex.GetValue());
		}
		return nullptr;
	}
	inline TSharedPtr<const FSlateUser> GetUserFromControllerId(int32 ControllerId) const
	{
		TOptional<int32> UserIndex = GetUserIndexForController(ControllerId);
		if (UserIndex.IsSet())
		{
			return GetUser(UserIndex.GetValue());
		}
		return nullptr;
	}
	SLATE_API TSharedPtr<FSlateUser> GetUserFromPlatformUser(FPlatformUserId PlatformUser);
	SLATE_API TSharedPtr<const FSlateUser> GetUserFromPlatformUser(FPlatformUserId PlatformUser) const;
	inline TSharedPtr<const FSlateUser> GetCursorUser() const
	{
		TSharedPtr<const FSlateUser> SlateUser = GetUser(CursorUserIndex);
		check(SlateUser.IsValid());
		return SlateUser;
	}
	inline TSharedPtr<FSlateUser> GetCursorUser()
	{
		TSharedPtr<FSlateUser> SlateUser = GetUser(CursorUserIndex);
		check(SlateUser.IsValid());
		return SlateUser;
	}
	SLATE_API TSharedRef<FSlateVirtualUserHandle> FindOrCreateVirtualUser(int32 VirtualUserIndex);
	SLATE_API void UnregisterUser(int32 UserIndex);
	SLATE_API void ForEachUser(TFunctionRef<void(FSlateUser&)> InPredicate, bool bIncludeVirtualUsers = false);
	UE_DEPRECATED(4.23, "ForEachUser now provides an FSlateUser& parameter to the lambda instead of an FSlateUser*")
	SLATE_API void ForEachUser(TFunctionRef<void(FSlateUser*)> InPredicate, bool bIncludeVirtualUsers = false);
	inline static double GetFixedDeltaTime() { return FixedDeltaTime; }
	static SLATE_API void SetFixedDeltaTime(double InSeconds);
protected:
	SLATE_API TSharedRef<FSlateUser> RegisterNewUser(int32 UserIndex, bool bIsVirtual = false);
	SLATE_API TSharedRef<FSlateUser> RegisterNewUser(FPlatformUserId PlatformUserId, bool bIsVirtual = false);
	SLATE_API TSharedRef<FSlateUser> GetOrCreateUser(int32 UserIndex);
	SLATE_API TSharedRef<FSlateUser> GetOrCreateUser(FPlatformUserId PlatformUserId);
	SLATE_API TSharedRef<FSlateUser> GetOrCreateUser(FInputDeviceId DeviceId);
	inline TSharedRef<FSlateUser> GetOrCreateUser(const FInputEvent& InputEvent) { return GetOrCreateUser(InputEvent.GetUserIndex()); }
	friend class FEventRouter;
	SLATE_API FPointerEvent TransformPointerEvent(const FPointerEvent& PointerEvent, const TSharedPtr<SWindow>& Window) const;
	SLATE_API virtual bool DoesWidgetHaveMouseCaptureByUser(const TSharedPtr<const SWidget> Widget, int32 UserIndex, TOptional<int32> PointerIndex) const override;
	SLATE_API virtual bool DoesWidgetHaveMouseCapture(const TSharedPtr<const SWidget> Widget) const override;
	SLATE_API virtual TOptional<EFocusCause> HasUserFocus(const TSharedPtr<const SWidget> Widget, int32 UserIndex) const override;
	SLATE_API virtual TOptional<EFocusCause> HasAnyUserFocus(const TSharedPtr<const SWidget> Widget) const override;
	SLATE_API virtual bool IsWidgetDirectlyHovered(const TSharedPtr<const SWidget> Widget) const override;
	SLATE_API virtual bool ShowUserFocus(const TSharedPtr<const SWidget> Widget) const override;
	SLATE_API TSharedRef<FNavigationConfig> GetRelevantNavConfig(int32 UserIndex) const;
	SLATE_API void OnShutdown();
	SLATE_API void DestroyRenderer();
	SLATE_API void TickTime();
	SLATE_API void TickPlatform(float DeltaTime);
	SLATE_API void TickAndDrawWidgets(float DeltaTime);
	SLATE_API void DrawWindows();
	SLATE_API void PrivateDrawWindows(TSharedPtr<SWindow> DrawOnlyThisWindow = nullptr);
	SLATE_API void DrawPrepass(TSharedPtr<SWindow> DrawOnlyThisWindow);
	SLATE_API void DrawWindowAndChildren(const TSharedRef<SWindow>& WindowToDraw, struct FDrawWindowArgs& DrawWindowArgs);
	SLATE_API void GetAllVisibleChildWindows(TArray< TSharedRef<SWindow> >& OutWindows, TSharedRef<SWindow> CurrentWindow);
	SLATE_API void ThrottleApplicationBasedOnMouseMovement();
	SLATE_API virtual FWidgetPath LocateWidgetInWindow(UE::Slate::FDeprecateVector2DParameter ScreenspaceMouseCoordinate, const TSharedRef<SWindow>& Window, bool bIgnoreEnabledStatus, int32 UserIndex) const override;
	SLATE_API void SetupPhysicalSensitivities();
public:
	SLATE_API bool ProcessMouseMoveEvent(const FPointerEvent& MouseEvent, bool bIsSynthetic = false);
	SLATE_API bool ProcessMouseButtonDownEvent(const TSharedPtr< FGenericWindow >& PlatformWindow, const FPointerEvent& InMouseEvent);
	SLATE_API bool ProcessMouseButtonUpEvent(const FPointerEvent& MouseEvent);
	SLATE_API bool ProcessMouseButtonDoubleClickEvent(const TSharedPtr< FGenericWindow >& PlatformWindow, const FPointerEvent& InMouseEvent);
	SLATE_API bool ProcessMouseWheelOrGestureEvent(const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent);
	SLATE_API bool ProcessKeyCharEvent(const FCharacterEvent& InCharacterEvent);
	SLATE_API bool ProcessKeyDownEvent(const FKeyEvent& InKeyEvent);
	SLATE_API bool ProcessKeyUpEvent(const FKeyEvent& InKeyEvent);
	SLATE_API bool ProcessAnalogInputEvent(const FAnalogInputEvent& InAnalogInputEvent);
	SLATE_API bool ProcessDragEnterEvent(TSharedRef<SWindow> WindowEntered, const FDragDropEvent& DragDropEvent);
	SLATE_API void ProcessTouchStartedEvent(const TSharedPtr< FGenericWindow >& PlatformWindow, const FPointerEvent& InTouchEvent);
	SLATE_API void ProcessTouchMovedEvent(const FPointerEvent& InTouchEvent);
	SLATE_API void ProcessTouchEndedEvent(const FPointerEvent& InTouchEvent);
	SLATE_API void ProcessMotionDetectedEvent(const FMotionEvent& InMotionEvent);
	SLATE_API bool ProcessWindowActivatedEvent(const FWindowActivateEvent& ActivateEvent);
	SLATE_API void ProcessApplicationActivationEvent(bool InAppActivated);
	bool IsProcessingInput() const { return ProcessingInput > 0; }
	TSharedRef<FNavigationConfig> GetNavigationConfig() const { return NavigationConfig; }
	SLATE_API void SetNavigationConfig(TSharedRef<FNavigationConfig> InNavigationConfig);
	UE_DEPRECATED(4.20, "Returning to a simpler method of registering navigation configs.\nSetNavigationConfig, is what you should use now.  Note: You'll need to store per user state information yourself if you have any, like we do for repeats with the analog stick in FNavigationConfig::UserNavigationState,\nrather than Slate creating a new Navigation Config per user.")
	void SetNavigationConfigFactory(TFunction<TSharedRef<FNavigationConfig>()> InNavigationConfigFactory) {}
	SLATE_API void CloseAllWindowsImmediately();
	SLATE_API void DestroyWindowsImmediately();
	SLATE_API void ProcessExternalReply(const FWidgetPath& CurrentEventPath, const FReply TheReply, const int32 UserIndex = 0, const int32 PointerIndex = 10);
	SLATE_API void ProcessReply(const FWidgetPath& CurrentEventPath, const FReply& TheReply, const FWidgetPath* WidgetsUnderMouse, const FPointerEvent* InMouseEvent, const uint32 UserIndex = 0);
	SLATE_API void QueryCursor();
	SLATE_API void ProcessCursorReply(const FCursorReply& CursorReply);
	SLATE_API void SpawnToolTip(const TSharedRef<IToolTip>& InToolTip, const UE::Slate::FDeprecateVector2DParameter& InSpawnLocation);
	SLATE_API void CloseToolTip();
	SLATE_API void UpdateToolTip(bool bAllowSpawningOfNewToolTips);
	SLATE_API TArray< TSharedRef<SWindow> > GetInteractiveTopLevelWindows();
	SLATE_API void GetAllVisibleWindowsOrdered(TArray< TSharedRef<SWindow> >& OutWindows);
	SLATE_API bool IsFakingTouchEvents() const;
	SLATE_API void SetGameIsFakingTouchEvents(const bool bIsFaking, FVector2D* CursorLocation = nullptr);
	SLATE_API void SetGameAllowsFakingTouchEvents(const bool bAllowFaking);
	SLATE_API void SetUnhandledKeyDownEventHandler(const FOnKeyEvent& NewHandler);
	SLATE_API void SetUnhandledKeyUpEventHandler(const FOnKeyEvent& NewHandler);
	double GetLastUserInteractionTime() const { return LastUserInteractionTime; }
	DECLARE_EVENT_OneParam(FSlateApplication, FSlateLastUserInteractionTimeUpdateEvent, double);
	FSlateLastUserInteractionTimeUpdateEvent& GetLastUserInteractionTimeUpdateEvent() { return LastUserInteractionTimeUpdateEvent; }
	SLATE_API float GetDragTriggerDistance() const;
	SLATE_API float GetDragTriggerDistanceSquared() const;
	SLATE_API bool IsLastDragOnDragEnterDisabled() const;
	SLATE_API bool HasTraveledFarEnoughToTriggerDrag(const FPointerEvent& PointerEvent, const UE::Slate::FDeprecateVector2DParameter ScreenSpaceOrigin) const;
	SLATE_API bool HasTraveledFarEnoughToTriggerDrag(const FPointerEvent& PointerEvent, const UE::Slate::FDeprecateVector2DParameter ScreenSpaceOrigin, EOrientation Orientation) const;
	SLATE_API void SetDragTriggerDistance(float ScreenPixels);
	SLATE_API void ToggleDisableLastDragOnDragEnter(bool bInDisableLastDragOnDragEnter);
	SLATE_API bool RegisterInputPreProcessor(TSharedPtr<class IInputProcessor> InputProcessor);
	SLATE_API bool RegisterInputPreProcessor(TSharedPtr<class IInputProcessor> InputProcessor, const int32 Index);
	SLATE_API bool RegisterInputPreProcessor(TSharedPtr<class IInputProcessor> InputProcessor, const EInputPreProcessorType Type);
	SLATE_API bool RegisterInputPreProcessor(TSharedPtr<class IInputProcessor> InputProcessor, const FInputPreprocessorRegistrationKey& Info);
	SLATE_API void UnregisterInputPreProcessor(TSharedPtr<class IInputProcessor> InputProcessor);
	UE_DEPRECATED(5.5, "This method is deprecated and will use EInputPreProcessorType::Game as a default. For a more accurate search result, please use the new version taking a EInputPreProcessorType")
	SLATE_API int32 FindInputPreProcessor(TSharedPtr<class IInputProcessor> InputProcessor) const;
	SLATE_API int32 FindInputPreProcessor(TSharedPtr<class IInputProcessor> InputProcessor, const EInputPreProcessorType& Type) const;
	SLATE_API void SetCursorRadius(float NewRadius);
	SLATE_API float GetCursorRadius() const;
	SLATE_API void SetAllowTooltips(bool bCanShow);
	SLATE_API bool GetAllowTooltips() const;
	SLATE_API void SetIsFakeTouch(bool bInIsFakingTouch);
	SLATE_API bool GetIsFakeTouch() const;
	bool IsRenderingOffScreen() const { return bRenderOffScreen; }
	virtual bool IsActive() const override { return bAppIsActive; }
	SLATE_API virtual TSharedRef<SWindow> AddWindow(TSharedRef<SWindow> InSlateWindow, const bool bShowImmediately = true) override;
	virtual void ArrangeWindowToFrontVirtual(TArray<TSharedRef<SWindow>>& Windows, const TSharedRef<SWindow>& WindowToBringToFront) override { FSlateWindowHelper::ArrangeWindowToFront(Windows, WindowToBringToFront); }
	virtual bool FindPathToWidget(TSharedRef<const SWidget> InWidget, FWidgetPath& OutWidgetPath, EVisibility VisibilityFilter = EVisibility::Visible) override
	{
		if (!FSlateWindowHelper::FindPathToWidget(GetInteractiveTopLevelWindows(), InWidget, OutWidgetPath, VisibilityFilter))
		{
			return FSlateWindowHelper::FindPathToWidget(SlateVirtualWindows, InWidget, OutWidgetPath, VisibilityFilter);
		}
		return true;
	}
	virtual const double GetCurrentTime() const override { return CurrentTime; }
	SLATE_API virtual TSharedPtr<SWindow> GetActiveTopLevelWindow() const override;
	SLATE_API virtual TSharedPtr<SWindow> GetActiveTopLevelRegularWindow() const override;
	SLATE_API virtual const FSlateBrush* GetAppIcon() const override;
	SLATE_API virtual const FSlateBrush* GetAppIconSmall() const override;
	virtual float GetApplicationScale() const override { return Scale; }
	virtual bool GetSoftwareCursorAvailable() const override { return bSoftwareCursorAvailable; }
	SLATE_API virtual EVisibility GetSoftwareCursorVis() const override;
	SLATE_API virtual UE::Slate::FDeprecateVector2DResult GetCursorPos() const override;
	SLATE_API virtual UE::Slate::FDeprecateVector2DResult GetLastCursorPos() const override;
	SLATE_API virtual UE::Slate::FDeprecateVector2DResult GetCursorSize() const override;
	SLATE_API virtual TSharedPtr<SWidget> GetKeyboardFocusedWidget() const override;
	virtual EWindowTransparency GetWindowTransparencySupport() const override { return PlatformApplication->GetWindowTransparencySupport(); }
protected:
	SLATE_API virtual TSharedPtr< SWidget > GetMouseCaptorImpl() const override;
public:
	SLATE_API virtual bool HasAnyMouseCaptor() const override;
	SLATE_API virtual bool HasUserMouseCapture(int32 UserIndex) const override;
	SLATE_API virtual FSlateRect GetPreferredWorkArea() const override;
	SLATE_API virtual bool HasFocusedDescendants(const TSharedRef<const SWidget>& Widget) const override;
	SLATE_API virtual bool HasUserFocusedDescendants(const TSharedRef< const SWidget >& Widget, int32 UserIndex) const override;
	SLATE_API virtual bool IsExternalUIOpened() override;
	SLATE_API virtual FWidgetPath LocateWindowUnderMouse(UE::Slate::FDeprecateVector2DParameter ScreenspaceMouseCoordinate, const TArray<TSharedRef<SWindow>>& Windows, bool bIgnoreEnabledStatus = false, int32 UserIndex = INDEX_NONE) override;
	SLATE_API virtual bool IsWindowHousingInteractiveTooltip(const TSharedRef<const SWindow>& WindowToTest) const override;
	SLATE_API virtual TSharedRef<SImage> MakeImage(const TAttribute<const FSlateBrush*>& Image, const TAttribute<FSlateColor>& Color, const TAttribute<EVisibility>& Visibility) const override;
	SLATE_API virtual TSharedRef<SWidget> MakeWindowTitleBar(const FWindowTitleBarArgs& InArgs, TSharedPtr<IWindowTitleBar>& OutTitleBar) const override;
	SLATE_API virtual TSharedRef<IToolTip> MakeToolTip(const TAttribute<FText>& ToolTipText) override;
	SLATE_API virtual TSharedRef<IToolTip> MakeToolTip(const FText& ToolTipText) override;
	SLATE_API virtual void RequestDestroyWindow(TSharedRef<SWindow> WindowToDestroy) override;
	SLATE_API virtual bool SetKeyboardFocus(const FWidgetPath& InFocusPath, const EFocusCause InCause) override;
	SLATE_API virtual bool SetUserFocus(const uint32 InUserIndex, const FWidgetPath& InFocusPath, const EFocusCause InCause) override;
	SLATE_API virtual bool SetUserFocusAllowingDescendantFocus(const uint32 InUserIndex, const FWidgetPath& InFocusPath, const EFocusCause InCause) override;
	SLATE_API virtual void SetAllUserFocus(const FWidgetPath& InFocusPath, const EFocusCause InCause) override;
	SLATE_API virtual void SetAllUserFocusAllowingDescendantFocus(const FWidgetPath& InFocusPath, const EFocusCause InCause) override;
	SLATE_API virtual TSharedPtr<SWidget> GetUserFocusedWidget(uint32 UserIndex) const override;
	SLATE_API virtual TSharedPtr<SWidget> GetCurrentDebugContextWidget() const override;
	virtual const TArray<TSharedRef<SWindow>> GetTopLevelWindows() const override { return SlateWindows; }
	DECLARE_EVENT_OneParam(FSlateApplication, FApplicationActivationStateChangedEvent, const bool)
	virtual FApplicationActivationStateChangedEvent& OnApplicationActivationStateChanged() { return ApplicationActivationStateChangedEvent; }
	SLATE_API virtual bool ShouldProcessUserInputMessages(const TSharedPtr< FGenericWindow >& PlatformWindow) const override;
	SLATE_API virtual bool OnKeyChar(const TCHAR Character, const bool IsRepeat) override;
	SLATE_API virtual bool OnKeyDown(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat) override;
	SLATE_API virtual bool OnKeyUp(const int32 KeyCode, const uint32 CharacterCode, const bool IsRepeat) override;
	SLATE_API virtual void OnInputLanguageChanged() override;
	SLATE_API virtual bool OnMouseDown(const TSharedPtr< FGenericWindow >& PlatformWindow, const EMouseButtons::Type Button) override;
	SLATE_API virtual bool OnMouseDown(const TSharedPtr< FGenericWindow >& PlatformWindow, const EMouseButtons::Type Button, const FVector2D CursorPos) override;
	SLATE_API virtual bool OnMouseUp(const EMouseButtons::Type Button) override;
	SLATE_API virtual bool OnMouseUp(const EMouseButtons::Type Button, const FVector2D CursorPos) override;
	SLATE_API virtual bool OnMouseDoubleClick(const TSharedPtr< FGenericWindow >& PlatformWindow, const EMouseButtons::Type Button) override;
	SLATE_API virtual bool OnMouseDoubleClick(const TSharedPtr< FGenericWindow >& PlatformWindow, const EMouseButtons::Type Button, const FVector2D CursorPos) override;
	SLATE_API virtual bool OnMouseWheel(const float Delta) override;
	SLATE_API virtual bool OnMouseWheel(const float Delta, const FVector2D CursorPos) override;
	SLATE_API virtual bool OnMouseMove() override;
	SLATE_API virtual bool OnRawMouseMove(const int32 X, const int32 Y) override;
	SLATE_API virtual bool OnCursorSet() override;
	SLATE_API virtual bool OnTouchGesture(EGestureEvent GestureType, const FVector2D& Delta, float WheelDelta, bool bIsDirectionInvertedFromDevice) override;
	SLATE_API virtual bool OnNativeGestureBegin(EGestureEvent GestureType, const FVector2D& Delta, float Value, bool bIsDirectionInvertedFromDevice, FPlatformUserId PlatformUserId, FInputDeviceId DeviceId) override;
	SLATE_API virtual bool OnNativeGestureUpdate(EGestureEvent GestureType, const FVector2D& Delta, float Value, bool bIsDirectionInvertedFromDevice, FPlatformUserId PlatformUserId, FInputDeviceId DeviceId) override;
	SLATE_API virtual bool OnNativeGestureEnd(EGestureEvent GestureType, const FVector2D& Delta, float Value, bool bIsDirectionInvertedFromDevice, FPlatformUserId PlatformUserId, FInputDeviceId DeviceId) override;
	SLATE_API virtual bool OnTouchStarted(const TSharedPtr< FGenericWindow >& Window, const FVector2D& Location, float Force, int32 TouchIndex, FPlatformUserId PlatformUserId, FInputDeviceId DeviceId) override;
	SLATE_API virtual bool OnTouchMoved(const FVector2D& Location, float Force, int32 TouchIndex, FPlatformUserId PlatformUserId, FInputDeviceId DeviceID) override;
	SLATE_API virtual bool OnTouchEnded(const FVector2D& Location, int32 TouchIndex, FPlatformUserId PlatformUserId, FInputDeviceId DeviceID) override;
	SLATE_API virtual bool OnTouchForceChanged(const FVector2D& Location, float Force, int32 TouchIndex, FPlatformUserId PlatformUserId, FInputDeviceId DeviceID) override;
	SLATE_API virtual bool OnTouchFirstMove(const FVector2D& Location, float Force, int32 TouchIndex, FPlatformUserId PlatformUserId, FInputDeviceId DeviceID) override;
	SLATE_API virtual void ShouldSimulateGesture(EGestureEvent Gesture, bool bEnable) override;
	SLATE_API virtual bool OnMotionDetected(const FVector& Tilt, const FVector& RotationRate, const FVector& Gravity, const FVector& Acceleration, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId) override;
	SLATE_API virtual bool OnControllerAnalog(FGamepadKeyNames::Type KeyName, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId, float AnalogValue) override;
	SLATE_API virtual bool OnControllerButtonPressed(FGamepadKeyNames::Type KeyName, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId, bool IsRepeat) override;
	SLATE_API virtual bool OnControllerButtonReleased(FGamepadKeyNames::Type KeyName, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId, bool IsRepeat) override;
	SLATE_API virtual bool OnSizeChanged(const TSharedRef< FGenericWindow >& PlatformWindow, const int32 Width, const int32 Height, bool bWasMinimized = false) override;
	SLATE_API virtual void OnOSPaint(const TSharedRef< FGenericWindow >& PlatformWindow) override;
	SLATE_API virtual FWindowSizeLimits GetSizeLimitsForWindow(const TSharedRef<FGenericWindow>& Window) const override;
	SLATE_API virtual void OnResizingWindow(const TSharedRef< FGenericWindow >& PlatformWindow) override;
	SLATE_API virtual bool BeginReshapingWindow(const TSharedRef< FGenericWindow >& PlatformWindow) override;
	SLATE_API virtual void FinishedReshapingWindow(const TSharedRef< FGenericWindow >& PlatformWindow) override;
	SLATE_API virtual void SignalSystemDPIChanged(const TSharedRef<FGenericWindow>& Window) override;
	SLATE_API virtual void HandleDPIScaleChanged(const TSharedRef<FGenericWindow>& Window) override;
	SLATE_API virtual void OnRegularWindowMovedOrResized(const TSharedRef<FGenericWindow>& Window) override;
	SLATE_API virtual void OnMovedWindow(const TSharedRef< FGenericWindow >& PlatformWindow, const int32 X, const int32 Y) override;
	SLATE_API virtual bool OnWindowActivationChanged(const TSharedRef< FGenericWindow >& PlatformWindow, const EWindowActivation ActivationType) override;
	SLATE_API virtual bool OnApplicationActivationChanged(const bool IsActive) override;
	SLATE_API virtual bool OnConvertibleLaptopModeChanged() override;
	SLATE_API virtual EWindowZone::Type GetWindowZoneForPoint(const TSharedRef< FGenericWindow >& PlatformWindow, const int32 X, const int32 Y) override;
	SLATE_API virtual void OnWindowClose(const TSharedRef< FGenericWindow >& PlatformWindow) override;
	SLATE_API virtual EDropEffect::Type OnDragEnterText(const TSharedRef< FGenericWindow >& Window, const FString& Text) override;
	SLATE_API virtual EDropEffect::Type OnDragEnterFiles(const TSharedRef< FGenericWindow >& Window, const TArray< FString >& Files) override;
	SLATE_API virtual EDropEffect::Type OnDragEnterExternal(const TSharedRef< FGenericWindow >& Window, const FString& Text, const TArray< FString >& Files) override;
	SLATE_API EDropEffect::Type OnDragEnter(const TSharedRef< SWindow >& Window, const TSharedRef<FExternalDragOperation>& DragDropOperation);
	SLATE_API virtual EDropEffect::Type OnDragOver(const TSharedPtr< FGenericWindow >& Window) override;
	SLATE_API virtual void OnDragLeave(const TSharedPtr< FGenericWindow >& Window) override;
	SLATE_API virtual EDropEffect::Type OnDragDrop(const TSharedPtr< FGenericWindow >& Window) override;
	SLATE_API virtual bool OnWindowAction(const TSharedRef< FGenericWindow >& PlatformWindow, const EWindowAction::Type InActionType) override;
	SLATE_API FReply RoutePointerDownEvent(const FWidgetPath& WidgetsUnderPointer, const FPointerEvent& PointerEvent);
	SLATE_API FReply RoutePointerUpEvent(const FWidgetPath& WidgetsUnderPointer, const FPointerEvent& PointerEvent);
	SLATE_API bool RoutePointerMoveEvent(const FWidgetPath& WidgetsUnderPointer, const FPointerEvent& PointerEvent, bool bIsSynthetic);
	SLATE_API FReply RoutePointerDoubleClickEvent(const FWidgetPath& WidgetsUnderPointer, const FPointerEvent& PointerEvent);
	SLATE_API FReply RouteMouseWheelOrGestureEvent(const FWidgetPath& WidgetsUnderPointer, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent = nullptr);
	SLATE_API int32 GetUserIndexForMouse() const;
	SLATE_API int32 GetUserIndexForKeyboard() const;
	SLATE_API FInputDeviceId GetInputDeviceIdForMouse() const;
	SLATE_API FInputDeviceId GetInputDeviceIdForKeyboard() const;
	SLATE_API int32 GetUserIndexForController(int32 ControllerId) const;
	SLATE_API TOptional<int32> GetUserIndexForInputDevice(FInputDeviceId InputDeviceId) const;
	SLATE_API TOptional<int32> GetUserIndexForPlatformUser(FPlatformUserId PlatformUser) const;
	SLATE_API TOptional<int32> GetUserIndexForController(int32 ControllerId, FKey InKey) const;
	SLATE_API void SetInputManager(TSharedRef<ISlateInputManager> InputManager);
	SLATE_API FDelegateHandle RegisterOnWindowActionNotification(const FOnWindowAction& Notification);
	DECLARE_EVENT_OneParam(FSlateApplication, FOnModalLoopTickEvent, float);
	FOnModalLoopTickEvent& GetOnModalLoopTickEvent() { return ModalLoopTickEvent; }
	SLATE_API void UnregisterOnWindowActionNotification(FDelegateHandle Handle);
	SLATE_API void NavigateToWidget(const uint32 UserIndex, const TSharedPtr<SWidget>& NavigationDestination, ENavigationSource NavigationSource = ENavigationSource::FocusedWidget, EUINavigation NavigationType = EUINavigation::Invalid);
	SLATE_API void NavigateFromWidgetUnderCursor(const uint32 InUserIndex, EUINavigation InNavigationType, TSharedRef<SWindow> InWindow);
	SLATE_API EUINavigation NavigateFromWidget(const uint32 InUserIndex, TSharedPtr<SWidget> InNavigationSource, const TArray<EUINavigation>& InNavigationTypes);
	SLATE_API TSharedPtr<SWindow> FindBestParentWindowForDialogs(const TSharedPtr<SWidget>& InWidget, const ESlateParentWindowSearchMethod InParentWindowSearchMethod = ESlateParentWindowSearchMethod::ActiveWindow);
	SLATE_API const void* FindBestParentWindowHandleForDialogs(const TSharedPtr<SWidget>& InWidget, const ESlateParentWindowSearchMethod InParentWindowSearchMethod = ESlateParentWindowSearchMethod::ActiveWindow);
	SLATE_API uint32 GetCurrentDrawId() const;
#if WITH_EDITORONLY_DATA
	FDragDropCheckingOverride OnDragDropCheckOverride;
#endif
	SLATE_API const TSet<FKey>& GetPressedMouseButtons() const;
private:
	SLATE_API TSharedRef< FGenericWindow > MakeWindow(TSharedRef<SWindow> InSlateWindow, const bool bShowImmediately);
	SLATE_API void PrivateDestroyWindow(const TSharedRef<SWindow>& DestroyedWindow);
	SLATE_API bool AttemptNavigation(const FWidgetPath& NavigationSource, const FNavigationEvent& NavigationEvent, const FNavigationReply& NavigationReply, const FArrangedWidget& BoundaryWidget);
	SLATE_API bool ExecuteNavigation(const FWidgetPath& NavigationSource, TSharedPtr<SWidget> DestinationWidget, const uint32 UserIndex, bool bAlwaysHandleNavigationAttempt);
	SLATE_API void OnMenuDestroyed(const TSharedRef<IMenu>& Menu);
public:
	struct FNavigationResult
	{
		TSharedPtr<SWidget> DestinationWidget;
		bool bAlwaysHandleNavigationAttempt = false;
#if WITH_SLATE_DEBUGGING
		ESlateDebuggingNavigationMethod NavigationMethod = ESlateDebuggingNavigationMethod::Unknown;
#endif
	};
	SLATE_API FNavigationResult CalculateDestinationWidget(const FWidgetPath& NavigationSource, const FNavigationReply& NavigationReply, EUINavigation NavigationType, uint32 UserIndex, const FArrangedWidget& BoundaryWidget) const;
private:
	SLATE_API FSlateApplication();
	SLATE_API void SetLastUserInteractionTime(const double InCurrentTime);
	SLATE_API bool SetUserFocus(FSlateUser& User, const FWidgetPath& InFocusPath, const EFocusCause InCause);
	SLATE_API void OnVirtualDesktopSizeChanged(const FDisplayMetrics& NewDisplayMetric);
	SLATE_API bool HandleTouchGesture(FInputDeviceId DeviceId, EGestureEvent GestureType, EGesturePhase GesturePhase, const FVector2D& Delta, const float Value, bool bIsDirectionInvertedFromDevice);
	static SLATE_API TSharedPtr< FSlateApplication > CurrentApplication;
	TSet<FKey> PressedMouseButtons;
	bool bAppIsActive;
	bool bSlateWindowActive;
	bool bRenderOffScreen;
	float Scale;
	float DragTriggerDistance;
	TArray< TSharedRef<SWindow> > SlateWindows;
	TArray< TSharedRef<SWindow> > SlateVirtualWindows;
	TWeakPtr<SWindow> ActiveTopLevelWindow;
	TArray< TSharedPtr<SWindow> > ActiveModalWindows;
	TArray< TSharedRef<SWindow> > WindowDestroyQueue;
	FMenuStack MenuStack;
	float CursorRadius;
	uint32 DrawId;
	TArray<TSharedPtr<FSlateUser>> Users;
	TArray<TWeakPtr<FSlateVirtualUserHandle>> VirtualUsers;
	TMap<uint32, TSharedPtr<ISlateMetaData>> PendingNavigationContexts;
	TWeakPtr<SWidget> LastAllUsersFocusWidget;
	EFocusCause LastAllUsersFocusCause;
	TWeakPtr<SWidget> CurrentDebugContextWidget;
	TWeakPtr<SWindow> CurrentDebuggingWindow;
	FThrottleRequest MouseButtonDownResponsivnessThrottle;
	FThrottleRequest UserInteractionResponsivnessThrottle;
	double LastUserInteractionTime;
	double LastUserInteractionTimeForThrottling;
	FSlateLastUserInteractionTimeUpdateEvent LastUserInteractionTimeUpdateEvent;
	double LastMouseMoveTime;
	FPopupSupport PopupSupport;
	TWeakPtr<SViewport> GameViewportWidget;
#if WITH_EDITOR
	TSet<TWeakPtr<SViewport>> AllGameViewports;
	TArray<TPair<FText, int32>> PreventDebuggingModeStack;
#endif
	TWeakPtr<SNotificationItem> DebuggingModeNotificationMessage;
	TSharedPtr<ISlateSoundDevice> SlateSoundDevice;
	double CurrentTime;
	double LastTickTime;
	float AverageDeltaTime;
	float AverageDeltaTimeForResponsiveness;
	FSimpleDelegate OnExitRequested;
	TWeakPtr<IWidgetReflector> WidgetReflectorPtr;
	FAccessSourceCode SourceCodeAccessDelegate;
	FQueryAccessSourceCode QuerySourceCodeAccessDelegate;
	FAccessAsset AssetAccessDelegate;
	FAccessDebugObject DebugObjectAccessDelegate;
	int32 NumExternalModalWindowsActive;
	TArray<FOnWindowAction> OnWindowActionNotifications;
	const class FStyleNode* RootStyleNode;
	bool bRequestLeaveDebugMode;
	bool bLeaveDebugForSingleStep;
	TAttribute<bool> NormalExecutionGetter;
	FModalWindowStackStarted ModalWindowStackStartedDelegate;
	FModalWindowStackEnded ModalWindowStackEndedDelegate;
	bool bIsExternalUIOpened;
	FThrottleRequest ThrottleHandle;
	bool DragIsHandled;
	TUniquePtr<IPlatformTextField> SlateTextField;
	bool bIsFakingTouch;
	bool bIsGameFakingTouch;
	bool bIsFakingTouched;
	bool bAllowFakingTouch;
	bool bHandleDeviceInputWhenApplicationNotActive;
	FOnKeyEvent UnhandledKeyDownEventHandler;
	FOnKeyEvent UnhandledKeyUpEventHandler;
	bool bTouchFallbackToMouse;
	bool bSoftwareCursorAvailable;
	bool bMenuAnimationsEnabled;
	const FSlateBrush* AppIcon;
	FApplicationActivationStateChangedEvent ApplicationActivationStateChangedEvent;
	FSlateRect VirtualDesktopRect;
	TSharedRef<FNavigationConfig> NavigationConfig;
#if WITH_EDITOR
	TSharedRef<FNavigationConfig> EditorNavigationConfig;
#endif
	TBitArray<FDefaultBitArrayAllocator> SimulateGestures;
	FSlateTickEvent PreTickEvent;
	FSlateTickEvent PostTickEvent;
	FSimpleMulticastDelegate PreShutdownEvent;
	FUserRegisteredEvent UserRegisteredEvent;
	FOnWindowBeingDestroyed WindowBeingDestroyedEvent;
	FOnMenuDestroyed MenuBeingDestroyedEvent;
	FOnModalLoopTickEvent ModalLoopTickEvent;
	FOnFocusChanging FocusChangingDelegate;
	FOnPreScreenshot PreScreenshotDelegate;
	FCriticalSection SlateTickCriticalSection;
	int32 ProcessingInput;
	bool bSynthesizedCursorMove = false;
	bool bIsTicking = false;
	bool bDisableLastDragOnDragEnter = false;
	uint64 PlatformMouseMovementEvents = 0;
	FVector2f LastPlatformCursorPosition = FVector2f::ZeroVector;
	static SLATE_API double FixedDeltaTime;
	class InputPreProcessorsHelper
	{
	public:
		void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor);
		bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent);
		bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent);
		bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent);
		bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent);
		bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent);
		bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent);
		bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent);
		bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& WheelEvent, const FPointerEvent* GestureEvent);
		bool HandleMotionDetectedEvent(FSlateApplication& SlateApp, const FMotionEvent& MotionEvent);
		bool Add(const FInputPreprocessorRegistration& Registration);
		void Remove(TSharedPtr<IInputProcessor> InputProcessor);
		void RemoveAll();
		int32 Find(TSharedPtr<IInputProcessor> InputProcessor, const EInputPreProcessorType& Type) const;
	private:
		bool PreProcessInput(ESlateDebuggingInputEvent InputEvent, TFunctionRef<bool(IInputProcessor&)> InputProcessFunc);
		void AddInternal(const FInputPreprocessorRegistration& Registration);
		using FProcessorTypeStorage = TArray<TSharedPtr<IInputProcessor>>;
		using FInputProcessorStorage = TSparseArray<FProcessorTypeStorage, TInlineSparseArrayAllocator<(uint32)EInputPreProcessorType::Count>>;
		FInputProcessorStorage InputPreProcessors;
		TArray<TSharedPtr<IInputProcessor>> InputPreProcessorsIteratorList;
		bool bIsIteratingPreProcessors = false;
		TArray<TSharedPtr<IInputProcessor>> ProcessorsPendingRemoval;
		TArray<FInputPreprocessorRegistration> ProcessorsPendingAddition;
	};
	InputPreProcessorsHelper InputPreProcessors;
	TSharedRef<ISlateInputManager> InputManager;
	FOnRegularWindowMovedOrResized OnRegularWindowMovedOrResizedEvent;
#if WITH_EDITOR
	FOnApplicationPreInputKeyDownListener OnApplicationPreInputKeyDownListenerEvent;
	FOnApplicationMousePreInputButtonDownListener OnApplicationMousePreInputButtonDownListenerEvent;
	FOnWindowDPIScaleChanged OnSignalSystemDPIChangedEvent;
	FOnWindowDPIScaleChanged OnWindowDPIScaleChangedEvent;
#endif 
};
