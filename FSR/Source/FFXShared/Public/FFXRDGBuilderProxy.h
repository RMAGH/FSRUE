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

class FRDGBuilder : public FRDGScopeState
{
	struct FAsyncDeleter
	{
		TUniqueFunction<void()> Function;
		UE::Tasks::FTask Prerequisites;
		static UE::Tasks::FTask LastTask;
		RENDERCORE_API ~FAsyncDeleter();
	} AsyncDeleter;
	FRDGAllocatorScope RootAllocatorScope;
public:
	RENDERCORE_API FRDGBuilder(FRHICommandListImmediate& RHICmdList, FRDGEventName Name = {}, ERDGBuilderFlags Flags = ERDGBuilderFlags::None, EShaderPlatform ShaderPlatform = GMaxRHIShaderPlatform);
	FRDGBuilder(const FRDGBuilder&) = delete;
	RENDERCORE_API ~FRDGBuilder();
	FRDGTexture* FindExternalTexture(FRHITexture* Texture) const;
	FRDGTexture* FindExternalTexture(IPooledRenderTarget* ExternalPooledTexture) const;
	FRDGBuffer* FindExternalBuffer(FRHIBuffer* Buffer) const;
	FRDGBuffer* FindExternalBuffer(FRDGPooledBuffer* ExternalPooledBuffer) const;
	RENDERCORE_API FRDGTextureRef RegisterExternalTexture(const TRefCountPtr<IPooledRenderTarget>& ExternalPooledTexture, ERDGTextureFlags Flags = ERDGTextureFlags::None);
	RENDERCORE_API FRDGTextureRef RegisterExternalTexture(const TRefCountPtr<IPooledRenderTarget>& ExternalPooledTexture, const TCHAR* NameIfNotRegistered, ERDGTextureFlags Flags = ERDGTextureFlags::None);
	RENDERCORE_API FRDGBufferRef RegisterExternalBuffer(const TRefCountPtr<FRDGPooledBuffer>& ExternalPooledBuffer, ERDGBufferFlags Flags = ERDGBufferFlags::None);
	RENDERCORE_API FRDGBufferRef RegisterExternalBuffer(const TRefCountPtr<FRDGPooledBuffer>& ExternalPooledBuffer, ERDGBufferFlags Flags, ERHIAccess AccessFinal);
	FRDGTextureRef CreateTexture(const FRDGTextureDesc& Desc, const TCHAR* Name, ERDGTextureFlags Flags = ERDGTextureFlags::None);
	FRDGBufferRef CreateBuffer(const FRDGBufferDesc& Desc, const TCHAR* Name, ERDGBufferFlags Flags = ERDGBufferFlags::None);
	FRDGBufferRef CreateBuffer(const FRDGBufferDesc& Desc, const TCHAR* Name, FRDGBufferNumElementsCallback&& NumElementsCallback, ERDGBufferFlags Flags = ERDGBufferFlags::None);
	FRDGTextureSRVRef CreateSRV(const FRDGTextureSRVDesc& Desc);
	FRDGBufferSRVRef CreateSRV(const FRDGBufferSRVDesc& Desc);
	inline FRDGBufferSRVRef CreateSRV(FRDGBufferRef Buffer, EPixelFormat Format) { return CreateSRV(FRDGBufferSRVDesc(Buffer, Format)); }
	FRDGTextureUAVRef CreateUAV(const FRDGTextureUAVDesc& Desc, ERDGUnorderedAccessViewFlags Flags = ERDGUnorderedAccessViewFlags::None);
	inline FRDGTextureUAVRef CreateUAV(FRDGTextureRef Texture, ERDGUnorderedAccessViewFlags Flags = ERDGUnorderedAccessViewFlags::None, EPixelFormat Format = PF_Unknown) { return CreateUAV(FRDGTextureUAVDesc(Texture, 0, Format), Flags); }
	FRDGBufferUAVRef CreateUAV(const FRDGBufferUAVDesc& Desc, ERDGUnorderedAccessViewFlags Flags = ERDGUnorderedAccessViewFlags::None);
	inline FRDGBufferUAVRef CreateUAV(FRDGBufferRef Buffer, EPixelFormat Format, ERDGUnorderedAccessViewFlags Flags = ERDGUnorderedAccessViewFlags::None) { return CreateUAV(FRDGBufferUAVDesc(Buffer, Format), Flags); }
	template <typename ParameterStructType>
	TRDGUniformBufferRef<ParameterStructType> CreateUniformBuffer(const ParameterStructType* ParameterStruct);
	void* Alloc(uint64 SizeInBytes, uint32 AlignInBytes = 16);
	template <typename PODType>
	PODType* AllocPOD();
	template <typename PODType>
	PODType* AllocPODArray(uint32 Count);
	template <typename PODType>
	TArrayView<PODType> AllocPODArrayView(uint32 Count);
	template <typename ObjectType, typename... TArgs>
	ObjectType* AllocObject(TArgs&&... Args);
	template <typename ObjectType>
	TArray<ObjectType, SceneRenderingAllocator>& AllocArray();
	template <typename ParameterStructType>
	ParameterStructType* AllocParameters();
	template <typename ParameterStructType>
	ParameterStructType* AllocParameters(const ParameterStructType* StructToCopy);
	template <typename BaseParameterStructType>
	BaseParameterStructType* AllocParameters(const FShaderParametersMetadata* ParametersMetadata);
	template <typename BaseParameterStructType>
	TStridedView<BaseParameterStructType> AllocParameters(const FShaderParametersMetadata* ParametersMetadata, uint32 NumStructs);
	void AddPostExecuteCallback(TUniqueFunction<void()>&& Callback)
	{
		check(Callback);
		PostExecuteCallbacks.Emplace(Forward<TUniqueFunction<void()>&&>(Callback));
	}
	template <typename ParameterStructType, typename ExecuteLambdaType>
	FRDGPassRef AddPass(FRDGEventName&& Name, const ParameterStructType* ParameterStruct, ERDGPassFlags Flags, ExecuteLambdaType&& ExecuteLambda);
	template <typename ExecuteLambdaType>
	FRDGPassRef AddPass(FRDGEventName&& Name, const FShaderParametersMetadata* ParametersMetadata, const void* ParameterStruct, ERDGPassFlags Flags, ExecuteLambdaType&& ExecuteLambda);
	template <typename ExecuteLambdaType>
	FRDGPassRef AddPass(FRDGEventName&& Name, ERDGPassFlags Flags, ExecuteLambdaType&& ExecuteLambda);
	template <typename ParameterStructType, typename LaunchLambdaType>
	FRDGPassRef AddDispatchPass(FRDGEventName&& Name, const ParameterStructType* ParameterStruct, ERDGPassFlags Flags, LaunchLambdaType&& LaunchLambda);
	void SetPassWorkload(FRDGPass* Pass, uint32 Workload);
	RENDERCORE_API void AddPassDependency(FRDGPass* Producer, FRDGPass* Consumer);
	void AddDispatchHint();
	template <typename TaskLambda>
	UE::Tasks::FTask AddSetupTask(TaskLambda&& Task, bool bCondition = true, ERDGSetupTaskWaitPoint WaitPoint = ERDGSetupTaskWaitPoint::Compile);
	template <typename TaskLambda>
	UE::Tasks::FTask AddSetupTask(TaskLambda&& Task, UE::Tasks::ETaskPriority Priority, bool bCondition = true, ERDGSetupTaskWaitPoint WaitPoint = ERDGSetupTaskWaitPoint::Compile);
	template <typename TaskLambda>
	UE::Tasks::FTask AddSetupTask(TaskLambda&& Task, UE::Tasks::FPipe* Pipe, UE::Tasks::ETaskPriority Priority = UE::Tasks::ETaskPriority::Normal, bool bCondition = true, ERDGSetupTaskWaitPoint WaitPoint = ERDGSetupTaskWaitPoint::Compile);
	template <typename TaskLambda, typename PrerequisitesCollectionType>
	UE::Tasks::FTask AddSetupTask(TaskLambda&& Task, PrerequisitesCollectionType&& Prerequisites, UE::Tasks::ETaskPriority Priority = UE::Tasks::ETaskPriority::Normal, bool bCondition = true, ERDGSetupTaskWaitPoint WaitPoint = ERDGSetupTaskWaitPoint::Compile);
	template <typename TaskLambda, typename PrerequisitesCollectionType>
	UE::Tasks::FTask AddSetupTask(TaskLambda&& Task, UE::Tasks::FPipe* Pipe, PrerequisitesCollectionType&& Prerequisites, UE::Tasks::ETaskPriority Priority = UE::Tasks::ETaskPriority::Normal, bool bCondition = true, ERDGSetupTaskWaitPoint WaitPoint = ERDGSetupTaskWaitPoint::Compile);
	template <typename TaskLambda>
	UE::Tasks::FTask AddCommandListSetupTask(TaskLambda&& Task, bool bCondition = true, ERDGSetupTaskWaitPoint WaitPoint = ERDGSetupTaskWaitPoint::Compile);
	template <typename TaskLambda>
	UE::Tasks::FTask AddCommandListSetupTask(TaskLambda&& Task, UE::Tasks::ETaskPriority Priority, bool bCondition = true, ERDGSetupTaskWaitPoint WaitPoint = ERDGSetupTaskWaitPoint::Compile);
	template <typename TaskLambda>
	UE::Tasks::FTask AddCommandListSetupTask(TaskLambda&& Task, UE::Tasks::FPipe* Pipe, UE::Tasks::ETaskPriority Priority = UE::Tasks::ETaskPriority::Normal, bool bCondition = true, ERDGSetupTaskWaitPoint WaitPoint = ERDGSetupTaskWaitPoint::Compile);
	template <typename TaskLambda, typename PrerequisitesCollectionType>
	UE::Tasks::FTask AddCommandListSetupTask(TaskLambda&& Task, PrerequisitesCollectionType&& Prerequisites, UE::Tasks::ETaskPriority Priority = UE::Tasks::ETaskPriority::Normal, bool bCondition = true, ERDGSetupTaskWaitPoint WaitPoint = ERDGSetupTaskWaitPoint::Compile);
	template <typename TaskLambda, typename PrerequisitesCollectionType>
	UE::Tasks::FTask AddCommandListSetupTask(TaskLambda&& Task, UE::Tasks::FPipe* Pipe, PrerequisitesCollectionType&& Prerequisites, UE::Tasks::ETaskPriority Priority = UE::Tasks::ETaskPriority::Normal, bool bCondition = true, ERDGSetupTaskWaitPoint WaitPoint = ERDGSetupTaskWaitPoint::Compile);
	inline bool IsParallelSetupEnabled() const { return ParallelSetup.bEnabled; }
	inline bool IsAsyncComputeEnabled() const { return bSupportsAsyncCompute; }
	RENDERCORE_API void SetFlushResourcesRHI();
	void SkipInitialAsyncComputeFence();
	void QueueBufferUpload(FRDGBufferRef Buffer, const void* InitialData, uint64 InitialDataSize, ERDGInitialDataFlags InitialDataFlags = ERDGInitialDataFlags::None);
	template <typename ElementType>
	inline void QueueBufferUpload(FRDGBufferRef Buffer, TArrayView<ElementType, int32> Container, ERDGInitialDataFlags InitialDataFlags = ERDGInitialDataFlags::None) { QueueBufferUpload(Buffer, Container.GetData(), Container.Num() * sizeof(ElementType), InitialDataFlags); }
	void QueueBufferUpload(FRDGBufferRef Buffer, const void* InitialData, uint64 InitialDataSize, FRDGBufferInitialDataFreeCallback&& InitialDataFreeCallback);
	template <typename ElementType>
	inline void QueueBufferUpload(FRDGBufferRef Buffer, TArrayView<ElementType, int32> Container, FRDGBufferInitialDataFreeCallback&& InitialDataFreeCallback) { QueueBufferUpload(Buffer, Container.GetData(), Container.Num() * sizeof(ElementType), InitialDataFreeCallback); }
	void QueueBufferUpload(FRDGBufferRef Buffer, FRDGBufferInitialDataFillCallback&& InitialDataFillCallback);
	void QueueBufferUpload(FRDGBufferRef Buffer, FRDGBufferInitialDataCallback&& InitialDataCallback, FRDGBufferInitialDataSizeCallback&& InitialDataSizeCallback);
	void QueueBufferUpload(FRDGBufferRef Buffer, FRDGBufferInitialDataCallback&& InitialDataCallback, FRDGBufferInitialDataSizeCallback&& InitialDataSizeCallback, FRDGBufferInitialDataFreeCallback&& InitialDataFreeCallback);
	void QueueCommitReservedBuffer(FRDGBufferRef Buffer, uint64 CommitSizeInBytes);
	void QueueTextureExtraction(FRDGTextureRef Texture, TRefCountPtr<IPooledRenderTarget>* OutPooledTexturePtr, ERDGResourceExtractionFlags Flags = ERDGResourceExtractionFlags::None);
	void QueueTextureExtraction(FRDGTextureRef Texture, TRefCountPtr<IPooledRenderTarget>* OutPooledTexturePtr, ERHIAccess AccessFinal, ERDGResourceExtractionFlags Flags = ERDGResourceExtractionFlags::None);
	void QueueBufferExtraction(FRDGBufferRef Buffer, TRefCountPtr<FRDGPooledBuffer>* OutPooledBufferPtr);
	void QueueBufferExtraction(FRDGBufferRef Buffer, TRefCountPtr<FRDGPooledBuffer>* OutPooledBufferPtr, ERHIAccess AccessFinal);
	RENDERCORE_API const TRefCountPtr<IPooledRenderTarget>& ConvertToExternalTexture(FRDGTextureRef Texture);
	RENDERCORE_API const TRefCountPtr<FRDGPooledBuffer>& ConvertToExternalBuffer(FRDGBufferRef Buffer);
	RENDERCORE_API FRHIUniformBuffer* ConvertToExternalUniformBuffer(FRDGUniformBufferRef UniformBuffer);
	const TRefCountPtr<IPooledRenderTarget>& GetPooledTexture(FRDGTextureRef Texture) const;
	const TRefCountPtr<FRDGPooledBuffer>& GetPooledBuffer(FRDGBufferRef Buffer) const;
	void SetTextureAccessFinal(FRDGTextureRef Texture, ERHIAccess Access);
	void SetBufferAccessFinal(FRDGBufferRef Buffer, ERHIAccess Access);
	RENDERCORE_API void UseExternalAccessMode(FRDGViewableResource* Resource, ERHIAccess ReadOnlyAccess, ERHIPipeline Pipelines = ERHIPipeline::Graphics);
	void UseExternalAccessMode(TArrayView<FRDGViewableResource* const> Resources, ERHIAccess ReadOnlyAccess, ERHIPipeline Pipelines = ERHIPipeline::Graphics)
	{
		for (FRDGViewableResource* Resource : Resources) { UseExternalAccessMode(Resource, ReadOnlyAccess, Pipelines); }
	}
	RENDERCORE_API void UseInternalAccessMode(FRDGViewableResource* Resource);
	inline void UseInternalAccessMode(TArrayView<FRDGViewableResource* const> Resources)
	{
		for (FRDGViewableResource* Resource : Resources) { UseInternalAccessMode(Resource); }
	}
	RENDERCORE_API void FlushSetupQueue();
	RENDERCORE_API void Execute();
	static RENDERCORE_API void TickPoolElements();
	static RENDERCORE_API bool IsImmediateMode();
	static RENDERCORE_API void WaitForAsyncDeleteTask();
	static RENDERCORE_API const UE::Tasks::FTask& GetAsyncDeleteTask();
	static RENDERCORE_API void WaitForAsyncExecuteTask();
	static RENDERCORE_API const UE::Tasks::FTask& GetAsyncExecuteTask();
	FRDGBlackboard Blackboard;
#if RDG_DUMP_RESOURCES
	static RENDERCORE_API FString BeginResourceDump(const TCHAR* Cmd = TEXT(""), const TCHAR* Context = TEXT(""));
	static RENDERCORE_API bool IsDumpingFrame();
#else
	static bool IsDumpingFrame() { return false; }
#endif
#if WITH_MGPU
	void EnableForceCopyCrossGPU() { bForceCopyCrossGPU = true; }
#endif
private:
	static const char* const kDefaultUnaccountedCSVStat;
	const FRDGEventName BuilderName;
	FRDGPass* ProloguePass = nullptr;
	FRDGPass* EpiloguePass = nullptr;
	bool bInitialAsyncComputeFence = GSupportsEfficientAsyncCompute;
	bool bSupportsAsyncCompute = false;
	bool bSupportsRenderPassMerge = false;
	uint32 AsyncComputePassCount = 0;
	uint32 RasterPassCount = 0;
	TArray<FRDGDispatchPass*, FRDGArrayAllocator> DispatchPasses;
	RENDERCORE_API ERDGPassFlags OverridePassFlags(const TCHAR* PassName, ERDGPassFlags Flags) const;
	inline FRDGPass* GetProloguePass() const { return ProloguePass; }
	inline FRDGPassHandle GetProloguePassHandle() const { return FRDGPassHandle(0); }
	inline FRDGPassHandle GetEpiloguePassHandle() const
	{
		checkf(EpiloguePass, TEXT("The handle is not valid until the epilogue has been added to the graph during execution."));
		return Passes.Last();
	}
	FRHIRenderPassInfo GetRenderPassInfo(const FRDGPass* Pass) const;
	template <typename ParameterStructType, typename ExecuteLambdaType>
	FRDGPass* AddPassInternal(FRDGEventName&& Name, const FShaderParametersMetadata* ParametersMetadata, const ParameterStructType* ParameterStruct, ERDGPassFlags Flags, ExecuteLambdaType&& ExecuteLambda);
	void MarkResourcesAsProduced(FRDGPass* Pass);
	RENDERCORE_API FRDGPass* SetupEmptyPass(FRDGPass* Pass);
	RENDERCORE_API FRDGPass* SetupParameterPass(FRDGPass* Pass);
	void SetupPassInternals(FRDGPass* Pass);
	void SetupPassResources(FRDGPass* Pass);
	void SetupPassDependencies(FRDGPass* Pass);
	void Compile();
	void CompilePassOps(FRDGPass* Pass);
	void ExecuteSerialPass(FRHIComputeCommandList& RHICmdListPass, FRDGPass* Pass);
	static void ExecutePass(FRHIComputeCommandList& RHICmdListPass, FRDGPass* Pass);
	static void ExecutePassPrologue(FRHIComputeCommandList& RHICmdListPass, FRDGPass* Pass);
	static void ExecutePassEpilogue(FRHIComputeCommandList& RHICmdListPass, FRDGPass* Pass);
	static void PushPreScopes(FRHIComputeCommandList& RHICmdListPass, FRDGPass* FirstPass);
	static void PushPassScopes(FRHIComputeCommandList& RHICmdListPass, FRDGPass* Pass);
	static void PopPassScopes(FRHIComputeCommandList& RHICmdListPass, FRDGPass* Pass);
	static void PopPreScopes(FRHIComputeCommandList& RHICmdListPass, FRDGPass* LastPass);
	FRDGPassRegistry Passes;
	FRDGTextureRegistry Textures;
	FRDGBufferRegistry Buffers;
	FRDGViewRegistry Views;
	FRDGUniformBufferRegistry UniformBuffers;
	struct FExtractedTexture
	{
		FExtractedTexture() = default;
		FExtractedTexture(FRDGTexture* InTexture, TRefCountPtr<IPooledRenderTarget>* InPooledTexture) : Texture(InTexture), PooledTexture(InPooledTexture) {}
		FRDGTexture* Texture{};
		TRefCountPtr<IPooledRenderTarget>* PooledTexture{};
	};
	TArray<FExtractedTexture, FRDGArrayAllocator> ExtractedTextures;
	struct FExtractedBuffer
	{
		FExtractedBuffer() = default;
		FExtractedBuffer(FRDGBuffer* InBuffer, TRefCountPtr<FRDGPooledBuffer>* InPooledBuffer) : Buffer(InBuffer), PooledBuffer(InPooledBuffer) {}
		FRDGBuffer* Buffer{};
		TRefCountPtr<FRDGPooledBuffer>* PooledBuffer{};
	};
	TArray<FExtractedBuffer, FRDGArrayAllocator> ExtractedBuffers;
	Experimental::TRobinHoodHashMap<FRHITexture*, FRDGTexture*, DefaultKeyFuncs<FRHITexture*>, FRDGArrayAllocator> ExternalTextures;
	Experimental::TRobinHoodHashMap<FRHIBuffer*, FRDGBuffer*, DefaultKeyFuncs<FRHIBuffer*>, FRDGArrayAllocator> ExternalBuffers;
	TArray<FRDGBuffer*, FRDGArrayAllocator> NumElementsCallbackBuffers;
	IRHITransientResourceAllocator* TransientResourceAllocator = nullptr;
	bool bSupportsTransientTextures = false;
	bool bSupportsTransientBuffers = false;
	bool IsTransient(FRDGTextureRef Texture) const;
	bool IsTransient(FRDGBufferRef Buffer) const;
	bool IsTransientInternal(FRDGViewableResource* Resource, bool bFastVRAM) const;
	struct FCollectResourceOp
	{
		enum class EOp : uint8 { Allocate, Deallocate };
		static FCollectResourceOp Allocate(FRDGBufferHandle BufferHandle) { return FCollectResourceOp(BufferHandle.GetIndex(), ERDGViewableResourceType::Buffer, EOp::Allocate); }
		static FCollectResourceOp Allocate(FRDGTextureHandle TextureHandle) { return FCollectResourceOp(TextureHandle.GetIndex(), ERDGViewableResourceType::Texture, EOp::Allocate); }
		static FCollectResourceOp Deallocate(FRDGBufferHandle BufferHandle) { return FCollectResourceOp(BufferHandle.GetIndex(), ERDGViewableResourceType::Buffer, EOp::Deallocate); }
		static FCollectResourceOp Deallocate(FRDGTextureHandle TextureHandle) { return FCollectResourceOp(TextureHandle.GetIndex(), ERDGViewableResourceType::Texture, EOp::Deallocate); }
		FCollectResourceOp() = default;
		FCollectResourceOp(uint32 InResourceIndex, ERDGViewableResourceType InResourceType, EOp InOp) : ResourceIndex(InResourceIndex), ResourceType(static_cast<uint32>(InResourceType)), Op(static_cast<uint32>(InOp)) {}
		EOp GetOp() const { return static_cast<EOp>(Op); }
		ERDGViewableResourceType GetResourceType() const { return static_cast<ERDGViewableResourceType>(ResourceType); }
		FRDGTextureHandle GetTextureHandle() const
		{
			check(GetResourceType() == ERDGViewableResourceType::Texture);
			return FRDGTextureHandle(ResourceIndex);
		}
		FRDGBufferHandle GetBufferHandle() const
		{
			check(GetResourceType() == ERDGViewableResourceType::Buffer);
			return FRDGBufferHandle(ResourceIndex);
		}
		uint32 ResourceIndex : 30;
		uint32 ResourceType : 1;
		uint32 Op : 1;
	};
	using FCollectResourceOpArray = TArray<FCollectResourceOp, FRDGArrayAllocator>;
	struct FCollectResourceContext
	{
		FCollectResourceOpArray TransientResources;
		FCollectResourceOpArray PooledTextures;
		FCollectResourceOpArray PooledBuffers;
		TArray<FRDGUniformBufferHandle, FRDGArrayAllocator> UniformBuffers;
		TArray<FRDGViewHandle, FRDGArrayAllocator> Views;
		FRDGUniformBufferBitArray UniformBufferMap;
		FRDGViewBitArray ViewMap;
	};
	Experimental::TRobinHoodHashMap<FRDGPooledTexture*, FRDGTexture*, DefaultKeyFuncs<FRDGPooledTexture*>, FConcurrentLinearArrayAllocator> PooledTextureOwnershipMap;
	Experimental::TRobinHoodHashMap<FRDGPooledBuffer*, FRDGBuffer*, DefaultKeyFuncs<FRDGPooledBuffer*>, FConcurrentLinearArrayAllocator> PooledBufferOwnershipMap;
	void FinalizeDescs();
	void CollectAllocations(FCollectResourceContext& Context, FRDGPass* Pass);
	void CollectAllocateTexture(FCollectResourceContext& Context, ERHIPipeline PassPipeline, FRDGPassHandle PassHandle, FRDGTexture* Texture);
	void CollectAllocateBuffer(FCollectResourceContext& Context, ERHIPipeline PassPipeline, FRDGPassHandle PassHandle, FRDGBuffer* Buffer);
	void CollectDeallocations(FCollectResourceContext& Context, FRDGPass* Pass);
	void CollectDeallocateTexture(FCollectResourceContext& Context, ERHIPipeline PassPipeline, FRDGPassHandle PassHandle, FRDGTexture* Texture, uint32 ReferenceCount);
	void CollectDeallocateBuffer(FCollectResourceContext& Context, ERHIPipeline PassPipeline, FRDGPassHandle PassHandle, FRDGBuffer* Buffer, uint32 ReferenceCount);
	void AllocateTransientResources(TConstArrayView<FCollectResourceOp> Ops);
	void AllocatePooledTextures(FRHICommandListBase& RHICmdList, TConstArrayView<FCollectResourceOp> Ops);
	void AllocatePooledBuffers(FRHICommandListBase& RHICmdList, TConstArrayView<FCollectResourceOp> Ops);
	void CreateViews(FRHICommandListBase& RHICmdList, TConstArrayView<FRDGViewHandle> ViewsToCreate);
	void CreateUniformBuffers(TConstArrayView<FRDGUniformBufferHandle> UniformBuffersToCreate);
	TRefCountPtr<IPooledRenderTarget> AllocatePooledRenderTargetRHI(FRHICommandListBase& RHICmdList, FRDGTextureRef Texture);
	TRefCountPtr<FRDGPooledBuffer> AllocatePooledBufferRHI(FRHICommandListBase& RHICmdList, FRDGBufferRef Buffer);
	void SetExternalPooledRenderTargetRHI(FRDGTexture* Texture, IPooledRenderTarget* RenderTarget);
	void SetPooledTextureRHI(FRDGTexture* Texture, FRDGPooledTexture* PooledTexture);
	void SetTransientTextureRHI(FRDGTexture* Texture, FRHITransientTexture* TransientTexture);
	void SetDiscardPass(FRDGTexture* Texture, FRHITransientTexture* TransientTexture);
	void SetExternalPooledBufferRHI(FRDGBuffer* Buffer, const TRefCountPtr<FRDGPooledBuffer>& PooledBuffer);
	void SetPooledBufferRHI(FRDGBuffer* Buffer, FRDGPooledBuffer* PooledBuffer);
	void SetTransientBufferRHI(FRDGBuffer* Buffer, FRHITransientBuffer* TransientBuffer);
	void InitViewRHI(FRHICommandListBase& RHICmdList, FRDGView* View);
	void InitBufferViewRHI(FRHICommandListBase& RHICmdList, FRDGBufferSRV* SRV);
	void InitBufferViewRHI(FRHICommandListBase& RHICmdList, FRDGBufferUAV* UAV);
	void InitTextureViewRHI(FRHICommandListBase& RHICmdList, FRDGTextureSRV* SRV);
	void InitTextureViewRHI(FRHICommandListBase& RHICmdList, FRDGTextureUAV* UAV);
	TMap<FRDGBarrierBatchBeginId, FRDGBarrierBatchBegin*, FRDGSetAllocator> BarrierBatchMap;
	TArray<FRHITrackedAccessInfo, FRDGArrayAllocator> EpilogueResourceAccesses;
	TArray<TRefCountPtr<IPooledRenderTarget>, FRDGArrayAllocator> ActivePooledTextures;
	TArray<TRefCountPtr<FRDGPooledBuffer>, FRDGArrayAllocator> ActivePooledBuffers;
	FRDGTransitionCreateQueue TransitionCreateQueue;
	FRDGTextureSubresourceState ScratchTextureState;
	FRDGSubresourceState PrologueSubresourceState;
	void CompilePassBarriers();
	void CollectPassBarriers();
	void CollectPassBarriers(FRDGPassHandle PassHandle);
	void CreatePassBarriers();
	void FinalizeResources();
	void AddFirstTextureTransition(FRDGTextureRef Texture);
	void AddFirstBufferTransition(FRDGBufferRef Buffer);
	void AddLastTextureTransition(FRDGTextureRef Texture);
	void AddLastBufferTransition(FRDGBufferRef Buffer);
	void AddCulledReservedCommitTransition(FRDGBufferRef Buffer);
	template <typename FilterSubresourceLambdaType>
	void AddTextureTransition(FRDGTextureRef Texture, FRDGTextureSubresourceState& StateBefore, FRDGTextureSubresourceState& StateAfter, FilterSubresourceLambdaType&& FilterSubresourceLambda);
	void AddTextureTransition(FRDGTextureRef Texture, FRDGTextureSubresourceState& StateBefore, FRDGTextureSubresourceState& StateAfter) { AddTextureTransition(Texture, StateBefore, StateAfter, [](FRDGSubresourceState*, int32) { return true; }); }
	template <typename FilterSubresourceLambdaType>
	void AddBufferTransition(FRDGBufferRef Buffer, FRDGSubresourceState*& StateBefore, FRDGSubresourceState* StateAfter, FilterSubresourceLambdaType&& FilterSubresourceLambda);
	void AddBufferTransition(FRDGBufferRef Buffer, FRDGSubresourceState*& StateBefore, FRDGSubresourceState* StateAfter) { AddBufferTransition(Buffer, StateBefore, StateAfter, [](FRDGSubresourceState*) { return true; }); }
	void AddTransition(FRDGViewableResource* Resource, FRDGSubresourceState StateBefore, FRDGSubresourceState StateAfter, FRDGTransitionInfo TransitionInfo);
	void AddAliasingTransition(FRDGPassHandle BeginPassHandle, FRDGPassHandle EndPassHandle, FRDGViewableResource* Resource, const FRHITransientAliasingInfo& Info);
#ifdef FFXRDGBUILDER_DELETE
	FRDGPassHandle GetEpilogueBarrierPassHandle(FRDGPassHandle Handle) { return Passes[Handle]->EpilogueBarrierPass; }
	FRDGPassHandle GetPrologueBarrierPassHandle(FRDGPassHandle Handle) { return Passes[Handle]->PrologueBarrierPass; }
	FRDGPass* GetEpilogueBarrierPass(FRDGPassHandle Handle) { return Passes[GetEpilogueBarrierPassHandle(Handle)]; }
	FRDGPass* GetPrologueBarrierPass(FRDGPassHandle Handle) { return Passes[GetPrologueBarrierPassHandle(Handle)]; }
	void AddToPrologueBarriersToEnd(FRDGPassHandle Handle, FRDGBarrierBatchBegin& BarriersToBegin)
	{
		FRDGPass* Pass = GetPrologueBarrierPass(Handle);
		Pass->GetPrologueBarriersToEnd(Allocators.Transition).AddDependency(&BarriersToBegin);
	}
	void AddToEpilogueBarriersToEnd(FRDGPassHandle Handle, FRDGBarrierBatchBegin& BarriersToBegin)
	{
		FRDGPass* Pass = GetEpilogueBarrierPass(Handle);
		Pass->GetEpilogueBarriersToEnd(Allocators.Transition).AddDependency(&BarriersToBegin);
	}
	template <typename FunctionType>
	void AddToPrologueBarriers(FRDGPassHandle PassHandle, FunctionType Function)
	{
		FRDGPass* Pass = GetPrologueBarrierPass(PassHandle);
		FRDGBarrierBatchBegin& BarriersToBegin = Pass->GetPrologueBarriersToBegin(Allocators.Transition, TransitionCreateQueue);
		Function(BarriersToBegin);
		Pass->GetPrologueBarriersToEnd(Allocators.Transition).AddDependency(&BarriersToBegin);
	}
	template <typename FunctionType>
	void AddToEpilogueBarriers(FRDGPassHandle PassHandle, FunctionType Function)
	{
		FRDGPass* Pass = GetEpilogueBarrierPass(PassHandle);
		FRDGBarrierBatchBegin& BarriersToBegin = Pass->GetEpilogueBarriersToBeginFor(Allocators.Transition, TransitionCreateQueue, Pass->GetPipeline());
		Function(BarriersToBegin);
		Pass->GetEpilogueBarriersToEnd(Allocators.Transition).AddDependency(&BarriersToBegin);
	}
	FRHITransientAllocationFences GetAllocateFences(FRDGViewableResource* Resource) const;
	FRHITransientAllocationFences GetDeallocateFences(FRDGViewableResource* Resource) const;
	inline ERHIPipeline GetPassPipeline(FRDGPassHandle PassHandle) const { return Passes[PassHandle]->Pipeline; }
	FRDGSubresourceState* AllocSubresource(const FRDGSubresourceState& Other);
	FRDGSubresourceState* AllocSubresource();
#endif
	struct FAsyncSetupOp
	{
		enum class EType : uint8 { SetupPassResources, CullRootBuffer, CullRootTexture, ReservedBufferCommit };
		static FAsyncSetupOp SetupPassResources(FRDGPass* Pass)
		{
			FAsyncSetupOp Op(EType::SetupPassResources);
			Op.Pass = Pass;
			return Op;
		}
		static FAsyncSetupOp CullRootBuffer(FRDGBuffer* Buffer)
		{
			FAsyncSetupOp Op(EType::CullRootBuffer);
			Op.Buffer = Buffer;
			return Op;
		}
		static FAsyncSetupOp CullRootTexture(FRDGTexture* Texture)
		{
			FAsyncSetupOp Op(EType::CullRootTexture);
			Op.Texture = Texture;
			return Op;
		}
		static FAsyncSetupOp ReservedBufferCommit(FRDGBuffer* Buffer, uint64 CommitSizeInBytes)
		{
			FAsyncSetupOp Op(EType::ReservedBufferCommit, CommitSizeInBytes);
			Op.Buffer = Buffer;
			return Op;
		}
		EType GetType() const { return (EType)Type; }
		uint64 Type : 8;
		uint64 Payload : 48;
		FAsyncSetupOp(EType InType, uint64 InPayload = 0) : Type((uint8)InType), Payload(InPayload) { check(InPayload < (1ull << 48ull)); }
		union { FRDGPass* Pass; FRDGBuffer* Buffer; FRDGTexture* Texture; };
	};
	struct FAsyncSetupQueue
	{
		void Push(FAsyncSetupOp Op)
		{
			UE::TScopeLock Lock(Mutex);
			Ops.Emplace(Op);
		}
		UE::FMutex Mutex;
		TArray<FAsyncSetupOp, FRDGArrayAllocator> Ops;
		UE::Tasks::FTask LastTask;
		bool bEnabled = false;
	} AsyncSetupQueue;
	void LaunchAsyncSetupQueueTask();
	void ProcessAsyncSetupQueue();
#ifdef FFXRDGBUILDER_DELETE
	FRDGBufferReservedCommitHandle AcquireReservedCommitHandle(FRDGBuffer* Buffer)
	{
		FRDGBufferReservedCommitHandle Handle;
		if (Buffer->PendingCommitSize > 0)
		{
			Handle = FRDGBufferReservedCommitHandle(ReservedBufferCommitSizes.Num());
			ReservedBufferCommitSizes.Emplace(Buffer->PendingCommitSize);
			Buffer->PendingCommitSize = 0;
		}
		return Handle;
	}
#endif
	uint64 GetReservedCommitSize(FRDGBufferReservedCommitHandle Handle) { return Handle.IsValid() ? ReservedBufferCommitSizes[Handle.GetIndex()] : 0; }
	TArray<uint64, FRDGArrayAllocator> ReservedBufferCommitSizes;
	TArray<FRDGPass*, FRDGArrayAllocator> CullPassStack;
	bool AddCullingDependency(FRDGProducerStatesByPipeline& LastProducers, const FRDGProducerState& NextState, ERHIPipeline NextPipeline);
	void AddCullRootBuffer(FRDGBuffer* Buffer);
	void AddCullRootTexture(FRDGTexture* Texture);
	void AddLastProducersToCullStack(const FRDGProducerStatesByPipeline& LastProducers);
	void FlushCullStack();
	struct
	{
		TStaticArray<TArray<UE::Tasks::FTask, FRDGArrayAllocator>, (int32)ERDGSetupTaskWaitPoint::MAX> Tasks;
		bool bEnabled = false;
		int8 TaskPriorityBias = 0;
		UE::Tasks::ETaskPriority GetTaskPriority(UE::Tasks::ETaskPriority TaskPriority) const { return (UE::Tasks::ETaskPriority)FMath::Clamp((int32)TaskPriority - TaskPriorityBias, 0, (int32)UE::Tasks::ETaskPriority::Count - 1); }
	} ParallelSetup;
	void WaitForParallelSetupTasks(ERDGSetupTaskWaitPoint WaitPoint);
	bool bCompiling = false;
	bool bParallelCompileEnabled = false;
	struct FParallelExecute
	{
		TArray<FParallelPassSet, FRDGArrayAllocator> ParallelPassSets;
		TOptional<UE::Tasks::FTaskEvent> TasksAwait;
		TOptional<UE::Tasks::FTaskEvent> TasksAsync;
		TOptional<UE::Tasks::FTaskEvent> DispatchTaskEventAwait;
		TOptional<UE::Tasks::FTaskEvent> DispatchTaskEventAsync;
		ERDGPassTaskMode TaskMode = ERDGPassTaskMode::Inline;
		bool IsEnabled() const { return TaskMode != ERDGPassTaskMode::Inline; }
		static UE::Tasks::FTask LastAsyncExecuteTask;
	} ParallelExecute;
	void SetupParallelExecute(TStaticArray<void*, MAX_NUM_GPUS> const& QueryBatchData);
	void SetupDispatchPassExecute();
	struct FUploadedBuffer
	{
		FUploadedBuffer() = default;
		FUploadedBuffer(FRDGBuffer* InBuffer, const void* InData, uint64 InDataSize) : Buffer(InBuffer), Data(InData), DataSize(InDataSize) {}
		FUploadedBuffer(FRDGBuffer* InBuffer, FRDGBufferInitialDataFillCallback&& InDataFillCallback) : Buffer(InBuffer), DataFillCallback(MoveTemp(InDataFillCallback)) {}
		FUploadedBuffer(FRDGBuffer* InBuffer, const void* InData, uint64 InDataSize, FRDGBufferInitialDataFreeCallback&& InDataFreeCallback) : bUseFreeCallbacks(true), Buffer(InBuffer), Data(InData), DataSize(InDataSize), DataFreeCallback(MoveTemp(InDataFreeCallback)) {}
		FUploadedBuffer(FRDGBuffer* InBuffer, FRDGBufferInitialDataCallback&& InDataCallback, FRDGBufferInitialDataSizeCallback&& InDataSizeCallback) : bUseDataCallbacks(true), Buffer(InBuffer), DataCallback(MoveTemp(InDataCallback)), DataSizeCallback(MoveTemp(InDataSizeCallback)) {}
		FUploadedBuffer(FRDGBuffer* InBuffer, FRDGBufferInitialDataCallback&& InDataCallback, FRDGBufferInitialDataSizeCallback&& InDataSizeCallback, FRDGBufferInitialDataFreeCallback&& InDataFreeCallback) : bUseDataCallbacks(true), bUseFreeCallbacks(true), Buffer(InBuffer), DataCallback(MoveTemp(InDataCallback)), DataSizeCallback(MoveTemp(InDataSizeCallback)), DataFreeCallback(MoveTemp(InDataFreeCallback)) {}
		bool bUseDataCallbacks = false;
		bool bUseFreeCallbacks = false;
		FRDGBuffer* Buffer{};
		const void* Data{};
		uint64 DataSize{};
		FRDGBufferInitialDataCallback DataCallback;
		FRDGBufferInitialDataSizeCallback DataSizeCallback;
		FRDGBufferInitialDataFreeCallback DataFreeCallback;
		FRDGBufferInitialDataFillCallback DataFillCallback;
	};
	TArray<FUploadedBuffer, FRDGArrayAllocator> UploadedBuffers;
	void SubmitBufferUploads(FRHICommandList& InRHICmdList, UE::Tasks::FTaskEvent* AllocateUploadBuffersTask = nullptr);
	TArray<FRDGViewableResource*, FRDGArrayAllocator> AccessModeQueue;
	TSet<FRDGViewableResource*, DefaultKeyFuncs<FRDGViewableResource*>, FRDGSetAllocator> ExternalAccessResources;
	RENDERCORE_API void FlushAccessModeQueue();
	TArray<TUniqueFunction<void()>, FRDGArrayAllocator> PostExecuteCallbacks;
	FGraphEventArray WaitOutstandingTasks;
	bool bFlushResourcesRHI = false;
	FRHICommandListScopedExtendResourceLifetime ExtendResourceLifetimeScope;
	void BeginFlushResourcesRHI();
	void EndFlushResourcesRHI();
	struct FAuxiliaryPass
	{
		uint8 Clobber = 0;
		uint8 Visualize = 0;
		uint8 Dump = 0;
		uint8 FlushAccessModeQueue = 0;
		bool IsDumpAllowed() const { return Dump == 0; }
		bool IsVisualizeAllowed() const { return Visualize == 0; }
		bool IsClobberAllowed() const { return Clobber == 0; }
		bool IsFlushAccessModeQueueAllowed() const { return FlushAccessModeQueue == 0; }
		bool IsActive() const { return Clobber > 0 || Visualize > 0 || Dump > 0 || FlushAccessModeQueue > 0; }
	} AuxiliaryPasses;
	void SetupAuxiliaryPasses(FRDGPass* Pass);
#if RDG_DUMP_RESOURCES
	void DumpNewGraphBuilder();
	void DumpResourcePassOutputs(const FRDGPass* Pass);
#endif
#if RDG_ENABLE_DEBUG
	RENDERCORE_API void VisualizePassOutputs(const FRDGPass* Pass);
	RENDERCORE_API void ClobberPassOutputs(const FRDGPass* Pass);
#endif
#if WITH_MGPU
	bool bForceCopyCrossGPU = false;
	void ForceCopyCrossGPU();
#endif
	IF_RDG_ENABLE_TRACE(FRDGTrace Trace);
#if RDG_ENABLE_DEBUG
	FRDGUserValidation UserValidation;
	FRDGBarrierValidation BarrierValidation;
#endif
	friend FRDGTrace;
	friend FRDGAsyncComputeBudgetScopeGuard;
	friend FRDGScopedCsvStatExclusive;
	friend FRDGScopedCsvStatExclusiveConditional;
	friend FRDGDispatchPassBuilder;
};
