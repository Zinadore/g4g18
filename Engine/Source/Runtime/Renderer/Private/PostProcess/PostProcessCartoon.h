/*=============================================================================
	PostProcessCartoon.h: Post processing cartoon effect implementation.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "RendererInterface.h"
#include "PostProcess/RenderingCompositionGraph.h"

// derives from TRenderingCompositePassBase<InputCount, OutputCount>
class FRCPassPostProcessCartoon : public TRenderingCompositePassBase<2, 1>
{
public:
	FRCPassPostProcessCartoon(FRHICommandList& RHICmdList);


	// interface FRenderingCompositePass ---------

	virtual void Process(FRenderingCompositePassContext& Context) override;
	virtual void Release() override { delete this; }
	virtual FPooledRenderTargetDesc ComputeOutputDesc(EPassOutputId InPassOutputId) const override;

};
