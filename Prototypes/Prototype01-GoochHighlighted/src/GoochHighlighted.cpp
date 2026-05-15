#include "GoochHighlighted.h"
#include "MapHelper.hpp"

namespace Diligent
{

namespace HLSL
{
	#include "../assets/structures.fxh"
} // namespace HLSL


	SampleBase* CreateSample()
	{
		return new GoochHighlighted();
	}

	GoochHighlighted::GoochHighlighted()
	{
		m_Camera.SetDefaultSecondaryRotation(QuaternionF::RotationFromAxisAngle(float3{ 0.f, 1.0f, 0.0f }, -PI_F / 2.f));
		m_Camera.SetDistRange(0.1f, 5.f);
		m_Camera.SetDefaultDistance(0.9f);
		m_Camera.ResetDefaults();
	}

	void GoochHighlighted::CreateResourceSignature()
	{
		VERIFY(m_ResourceSignatures.empty(), "Resource signature has already been created");

		PipelineResourceSignatureDescX SignatureDesc{ "SG Render Resource Signature" };

		SignatureDesc
			.SetUseCombinedTextureSamplers(m_pDevice->GetDeviceInfo().IsGLDevice())
			.AddResource(SHADER_TYPE_VS_PS, "cbFrameAttribs", SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE);

	}

	GoochHighlighted::~GoochHighlighted()
	{

	}

	void GoochHighlighted::Initialize(const SampleInitInfo& InitInfo)
	{
		SampleBase::Initialize(InitInfo);
		LoadModel(std::string("../../Common/Assets/Models/Bunny/Bunny.gltf").c_str());

		CreatePipelineState();
		m_Model->PrepareGPUResources(m_pDevice, m_pImmediateContext);
	}

	void GoochHighlighted::LoadModel(const char* Path)
	{
		GLTF::ModelCreateInfo ModelCI;
		ModelCI.FileName = Path;
		ModelCI.ComputeBoundingBoxes = True;
		ModelCI.pResourceManager = nullptr;

		m_Model = std::make_unique<GLTF::Model>(m_pDevice, m_pImmediateContext, ModelCI);
		//Create resource bindings
		ModelResourceBindings ResourceBindings;
		ResourceBindings.MaterialSRB.resize(m_Model->Materials.size());
		for (size_t mat = 0; mat < m_Model->Materials.size(); ++mat)
		{
			RefCntAutoPtr<IShaderResourceBinding>& pMatSRB = ResourceBindings.MaterialSRB[mat];
			
		}



		// Update model transforms
		int sceneID = m_Model->DefaultSceneId;
		m_Model->ComputeTransforms(sceneID, m_Transforms[0]);
		m_ModelAABB = m_Model->ComputeBoundingBox(sceneID, m_Transforms[0]);

		// Center and scale Model
		float MaxDim = 0;
		float3 ModelDim(m_ModelAABB.Max - m_ModelAABB.Min);
		MaxDim = std::max(MaxDim, ModelDim.x);
		MaxDim = std::max(MaxDim, ModelDim.y);
		MaxDim = std::max(MaxDim, ModelDim.z);

		m_SceneScale = (1.0f / std::max(MaxDim, 0.01f)) * 0.5f;
		float3 Translate = -m_ModelAABB.Min - 0.5f * ModelDim;
		float4x4 InvYAxis = float4x4::Identity();
		InvYAxis._22 = -1;

		m_ModelTransform = float4x4::Translation(Translate) * float4x4::Scale(m_SceneScale) * InvYAxis;
		m_Model->ComputeTransforms(sceneID, m_Transforms[0], m_ModelTransform);
		m_ModelAABB = m_Model->ComputeBoundingBox(sceneID, m_Transforms[0]);
		m_Transforms[1] = m_Transforms[0];

	}

	void GoochHighlighted::CreatePipelineState()
	{
		// This is the create of the Pipeline State Object that configures all GPU stages
		GraphicsPipelineStateCreateInfo PSOCreateInfo;

		PSOCreateInfo.PSODesc.Name = "Bunny PSO";

		// Standard Rasterization Graphics pipeline.
		PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

		//clang-format off
		PSOCreateInfo.GraphicsPipeline.NumRenderTargets					= 1;
		// Set Render target format which is the format of the swap chains color buffer
		PSOCreateInfo.GraphicsPipeline.RTVFormats[0]					= m_pSwapChain->GetDesc().ColorBufferFormat;
		// Set Depth buffer format which is the format of the swaps chain's depth buffer
		PSOCreateInfo.GraphicsPipeline.DSVFormat						= m_pSwapChain->GetDesc().DepthBufferFormat;
		PSOCreateInfo.GraphicsPipeline.PrimitiveTopology				= PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode			= CULL_MODE_BACK; // Cull Back faces
		PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable		= True;
		// clang-format on


		ShaderCreateInfo ShaderCI;
		// Tell the system that the shader source code is in HLSL.
		// For OpenGL, the engine will convert this into GLSL under the hood.
		ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

		// OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
		ShaderCI.Desc.UseCombinedTextureSamplers = true;

		// Pack matrices in row-major order
		ShaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;

		
		// Required to load shaders from file
		RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
		m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
		ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

		//Create a vertex shader
		RefCntAutoPtr<IShader> pVS;
		{
			ShaderCI.Desc.ShaderType	= SHADER_TYPE_VERTEX;
			ShaderCI.EntryPoint = "main";
			ShaderCI.Desc.Name = "GoochHighlighted VS";
			ShaderCI.FilePath = "goochhighlighted.vsh";
			m_pDevice->CreateShader(ShaderCI, &pVS);


			// Creating a Dynamic Buffer that writes in the CPU to be read by the GPU every frame.
			// We use this to store our transformation 
			BufferDesc CBDesc;
			CBDesc.Name = "VS Constants CB";
			CBDesc.Size = sizeof(HLSL::Constants);
			CBDesc.Usage = USAGE_DYNAMIC;
			CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
			m_pDevice->CreateBuffer(CBDesc, nullptr, &m_VSConstants);
		}
		 
		// Create a Pixel Shader

		RefCntAutoPtr<IShader> pPS;
		{
			ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
			ShaderCI.EntryPoint = "main";
			ShaderCI.Desc.Name = "GoochHighlighted PS";
			ShaderCI.FilePath = "goochhighlighted.psh";
			m_pDevice->CreateShader(ShaderCI, &pPS);
		}

		// Define vertex shader input layout
		LayoutElement LayoutElems[] =
		{
			// Attribute 0 - vertex position
			LayoutElement{0, 0, 3, VT_FLOAT32, False},
			// Attribute 0 - vertex normal
			LayoutElement{1, 0, 3, VT_FLOAT32, False}
		};

		PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
		PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

		PSOCreateInfo.pVS = pVS;
		PSOCreateInfo.pPS = pPS;
		
		// Define defualt variable type
		PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

		m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);

		// Since we did not explicitly specify the type for 'Constants' variable, default
		// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
		// change and are bound directly through the pipeline state object.
		m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbConstants")->Set(m_VSConstants);

		// Create a shader resource binding object and bind all static resources in it
		m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

	}

	
	void GoochHighlighted::Render()
	{
		ITextureView* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
		ITextureView* pDSV = m_pSwapChain->GetDepthBufferDSV();

		// Clear the back buffer
		float4 ClearColor = { 0.350f, 0.350f, 0.350f, 1.0f };

		m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor.Data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		{
			// Map the buffer and write current world-view-projection matrix
			MapHelper<HLSL::Constants> CBConstants(m_pImmediateContext, m_VSConstants, MAP_WRITE, MAP_FLAG_DISCARD);
			CBConstants->g_WorldViewProj = m_WorldViewProjMatrix;
			CBConstants->g_NormalMatrix = m_NormalMatrix;
			CBConstants->g_EyePos;

		}
		std::array<IBuffer*, 8> pVBs;
		const Uint32 NumVBs = static_cast<Uint32>(m_Model->GetVertexBufferCount());
		for (Uint32 i = 0; i < NumVBs; ++i)
		{
			pVBs[i] = m_Model->GetVertexBuffer(i);
		}
		m_pImmediateContext->SetVertexBuffers(0, NumVBs, pVBs.data(), nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

		if (IBuffer* pIndexBuffer = m_Model->GetIndexBuffer())
		{
			m_pImmediateContext->SetIndexBuffer(pIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}
		
		m_pImmediateContext->SetPipelineState(m_pPSO);
		m_pImmediateContext->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		


		// Draw GLTF Model. Currently we are drawing all models. However here is where we can separate it out
		// based on PBR material and render passes.  
		m_RenderList.clear();

		auto Scene = m_Model->Scenes[m_Model->DefaultSceneId];
		for (const GLTF::Node* pNode : Scene.LinearNodes)
		{
			if (pNode->pMesh == nullptr) 
				continue;

			for (const GLTF::Primitive& primitive : pNode->pMesh->Primitives)
			{
				if (primitive.VertexCount == 0 && primitive.IndexCount == 0)
					continue;

				m_RenderList.emplace_back(primitive, *pNode);

			}

		}

		const Uint32 FirstIndexLocation = m_Model->GetFirstIndexLocation();
		const Uint32 BaseVertex			= m_Model->GetBaseVertex();

		for (const PrimitiveRenderInfo& PrimRI : m_RenderList)
		{
			const GLTF::Node& Node = PrimRI.Node;
			const GLTF::Primitive& primitive = PrimRI.Primitive;
			if (primitive.HasIndices())
			{
				DrawIndexedAttribs drawAttrs{ primitive.IndexCount, VT_UINT32, DRAW_FLAG_VERIFY_ALL };
				drawAttrs.FirstIndexLocation = FirstIndexLocation + primitive.FirstIndex;
				drawAttrs.BaseVertex = BaseVertex + primitive.FirstVertex;
				m_pImmediateContext->DrawIndexed(drawAttrs);

			}
			else
			{
				DrawAttribs drawAttrs{ primitive.VertexCount, DRAW_FLAG_VERIFY_ALL };
				drawAttrs.StartVertexLocation = BaseVertex + primitive.FirstVertex;
				m_pImmediateContext->Draw(drawAttrs);
			}
			
		}

	}
	void GoochHighlighted::Update(double CurrTime, double ElapsedTime, bool DoUpdateUI)
	{
		SampleBase::Update(CurrTime, ElapsedTime, DoUpdateUI);
		
		const float4x4& NodeGlobalMatrix = m_Transforms[0].NodeGlobalMatrices[0];
		//Apply Rotation animation
		const float4x4 NodeTransform = NodeGlobalMatrix * m_ModelTransform * float4x4::RotationY(static_cast<float>(CurrTime) * 1.0f);
		
		// Camera is at (0, 0, -5) looking along the Z axis
		float4x4 View = float4x4::Translation(0.f, 0.0f, 5.0f);

		// Get pretransform matrix that rotates the scene according the surface orientation
		float4x4 SrfPreTransform = GetSurfacePretransformMatrix(float3{ 0, 0, 1 });

		// Get projection matrix adjusted to the current screen orientation
		float4x4 Proj = GetAdjustedProjectionMatrix(PI_F / 4.0f, 0.1f, 100.f);

		// Compute world-view-projection matrix
		m_WorldViewProjMatrix = NodeTransform * View  * Proj;
		m_NormalMatrix = NodeTransform.RemoveTranslation().Inverse().Transpose();
		m_EyePos = float4(0.f, 0.0f, 5.0f,0.0);
	}
}