/*=============================================================================
	PostProcessCartoon.cpp: Post processing cartoon effect implementation.
=============================================================================*/

#include "PostProcess/PostProcessCartoon.h"
 #include "StaticBoundShaderState.h"
 #include "SceneUtils.h"
 #include "PostProcess/SceneFilterRendering.h"
 #include "SceneRendering.h"
 #include "PipelineStateCache.h"


class FPostProcessCartoonVS : public FGlobalShader {
	DECLARE_SHADER_TYPE(FPostProcessCartoonVS, Global);

	FPostProcessCartoonVS() { }
	FPostProcessCartoonVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}

	void SetParameters(const FRenderingCompositePassContext& Context)
	{
		const FVertexShaderRHIParamRef ShaderRHI = GetVertexShader();

		FGlobalShader::SetParameters<FViewUniformShaderParameters>(Context.RHICmdList, ShaderRHI, Context.View.ViewUniformBuffer);

		const FPooledRenderTargetDesc* SceneColor = Context.Pass->GetInputDesc(ePId_Input0);

		if (!SceneColor)
		{
			// input is not hooked up correctly
			return;
		}

		const FPooledRenderTargetDesc* SceneDepth = Context.Pass->GetInputDesc(ePId_Input1);

		if (!SceneDepth)
		{
			// input is not hooked up correctly
			return;
		}
	}

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return true;
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
 	{
 		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM4);
 	}
};

IMPLEMENT_SHADER_TYPE(, FPostProcessCartoonVS, TEXT("/Engine/Private/PostProcessCartoon.usf"), TEXT("PPCartoonVS"), SF_Vertex);


/** Encapsulates the post processing cartoon pixel shader. */
 class FPostProcessCartoonPS : public FGlobalShader
 {
 	DECLARE_SHADER_TYPE(FPostProcessCartoonPS, Global);

 	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
 	{
 		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM4);
 	}

 	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
 	{
 		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
 	}

 	/** Default constructor. */
	FPostProcessCartoonPS() {}

 public:
 	FPostProcessPassParameters PostprocessParameter;

 	/** Initialization constructor. */
	FPostProcessCartoonPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
 		: FGlobalShader(Initializer)
 	{
		PostprocessParameter.Bind(Initializer.ParameterMap);
 	}

	template <typename TRHICmdList>
	void SetParameters(TRHICmdList& RHICmdList, const FRenderingCompositePassContext& Context)
	{
		const FPixelShaderRHIParamRef ShaderRHI = GetPixelShader();

		const FViewInfo& View = Context.View;

		FGlobalShader::SetParameters<FViewUniformShaderParameters>(RHICmdList, ShaderRHI, View.ViewUniformBuffer);

		PostprocessParameter.SetPS(RHICmdList, ShaderRHI, Context, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());

		const FPooledRenderTargetDesc* SceneColor = Context.Pass->GetInputDesc(ePId_Input0);

		if (!SceneColor)
		{
			// input is not hooked up correctly
			return;
		}

		const FPooledRenderTargetDesc* SceneDepth = Context.Pass->GetInputDesc(ePId_Input1);

		if (!SceneDepth)
		{
			// input is not hooked up correctly
			return;
		}
	}

 	// FShader interface.
 	virtual bool Serialize(FArchive& Ar) override
 	{
 		bool bShaderHasOutdatedParameters = FGlobalShader::Serialize(Ar);
 		Ar << PostprocessParameter;
 		return bShaderHasOutdatedParameters;
 	}

 	static const TCHAR* GetSourceFilename()
 	{
 		return TEXT("/Engine/Private/PostProcessCartoon.usf");
 	}

 	static const TCHAR* GetFunctionName()
 	{
 		return TEXT("PPCartoonPS");
 	}
 };

 IMPLEMENT_SHADER_TYPE(, FPostProcessCartoonPS, TEXT("/Engine/Private/PostProcessCartoon.usf"), TEXT("PPCartoonPS"), SF_Pixel);

static TAutoConsoleVariable<int32> CVarCartoonEffect(
	TEXT("r.CartoonPass"),
	0,
	TEXT("Enables a cartoon like filter.\n")
	TEXT("1 = Enable\n")
	TEXT("0 = Disable\n"),
	ECVF_RenderThreadSafe);

void FRCPassPostProcessCartoon::Process(FRenderingCompositePassContext& Context)
{
	const FPooledRenderTargetDesc* SceneColor = GetInputDesc(ePId_Input0);

	if(!SceneColor)
	{
		// input is not hooked up correctly
		return;
	}

	const FPooledRenderTargetDesc* SceneDepth = GetInputDesc(ePId_Input1);

	if(!SceneDepth) {
		return;
	}


	const FViewInfo& View = Context.View;
	const FSceneViewFamily& ViewFamily = *(View.Family);

	const FSceneRenderTargetItem& DestRenderTarget = PassOutputs[0].RequestSurface(Context);

	FIntRect SrcRect = Context.SceneColorViewRect;
	FIntRect DestRect = Context.GetSceneColorDestRect(DestRenderTarget);

	FIntPoint SrcSize = SceneColor->Extent;
	FIntPoint DestSize = PassOutputs[0].RenderTargetDesc.Extent;

	SCOPED_DRAW_EVENTF(Context.RHICmdList, PostProcessCartoon, TEXT("PostProcessCartoon %dx%d"), DestRect.Width(), DestRect.Height());

	// Set the view family's render target/viewport.
	SetRenderTarget(Context.RHICmdList, DestRenderTarget.TargetableTexture, FTextureRHIRef());
	Context.SetViewportAndCallRHI(0, 0, 0.0f, DestSize.X, DestSize.Y, 1.0f );

	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	Context.RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

	// set the state
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

	TShaderMapRef<FPostProcessCartoonVS> VertexShader(Context.GetShaderMap());
	TShaderMapRef<FPostProcessCartoonPS> PixelShader(Context.GetShaderMap());

	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
	GraphicsPSOInit.PrimitiveType = PT_TriangleList;

	SetGraphicsPipelineState(Context.RHICmdList, GraphicsPSOInit);

	PixelShader->SetParameters(Context.RHICmdList, Context);
	VertexShader->SetParameters(Context);

	DrawPostProcessPass(Context.RHICmdList,
		DestRect.Min.X, DestRect.Min.Y,
		DestRect.Width(), DestRect.Height(),
		SrcRect.Min.X, SrcRect.Min.Y,
		SrcRect.Width(), SrcRect.Height(),
		DestSize,
		SrcSize,
		*VertexShader,
		View.StereoPass,
		Context.HasHmdMesh(),
		EDRF_Default);

	Context.RHICmdList.CopyToResolveTarget(DestRenderTarget.TargetableTexture, DestRenderTarget.ShaderResourceTexture, FResolveParams());
}

FPooledRenderTargetDesc FRCPassPostProcessCartoon::ComputeOutputDesc(EPassOutputId InPassOutputId) const
{
	FPooledRenderTargetDesc Ret = GetInput(ePId_Input0)->GetOutput()->RenderTargetDesc;

	Ret.Reset();
	Ret.DebugName = TEXT("PostProcessCartoon");

	return Ret;
}
