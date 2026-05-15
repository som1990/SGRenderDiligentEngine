#pragma once

#include <array>

#include "SampleBase.hpp"
#include "GLTFLoader.hpp"
#include "BasicMath.hpp"
#include "TrackballCamera.hpp"

namespace Diligent
{


class GoochHighlighted final : public SampleBase
{
public:
	GoochHighlighted();
	~GoochHighlighted();


	struct ModelResourceBindings
	{
		void Clear()
		{
			MaterialSRB.clear();
		}

		std::vector<RefCntAutoPtr<IShaderResourceBinding>> MaterialSRB;
	};

	virtual void Initialize(const SampleInitInfo& InitInfo) override final;
	
	virtual void Render() override final;
	virtual void Update(double CurrTime, double ElapsedTime, bool DoUpdateUI) override final;

	virtual const Char* GetSampleName() const override final { return "Gooch Highlighted"; }

private:
	void LoadModel(const char* Path);
	void CreatePipelineState();
	void CreateResourceSignature();
	//void CreateVertexBuffer();
	//void CreateIndexBuffer();


private:
	std::unique_ptr<GLTF::Model>			m_Model;
	std::array<GLTF::ModelTransforms, 2>	m_Transforms; // [0] - current frame, [1] - previous frame
	BoundBox								m_ModelAABB;
	float4x4								m_ModelTransform;
	float									m_SceneScale = 1.f;

	RefCntAutoPtr<IPipelineState>			m_pPSO;
	RefCntAutoPtr<IShaderResourceBinding>	m_pSRB;
	RefCntAutoPtr<IBuffer>					m_VSConstants;
	RefCntAutoPtr<IBuffer>					m_BunnyVertexBuffer;
	
	std::vector<RefCntAutoPtr<IPipelineResourceSignature>> m_ResourceSignatures;


	float4x4								m_WorldViewProjMatrix;
	float4x4								m_NormalMatrix;
	float4									m_EyePos;

	TrackballCamera<float>					m_Camera;

	struct PrimitiveRenderInfo
	{
		const GLTF::Primitive& Primitive;
		const GLTF::Node& Node;

		PrimitiveRenderInfo(const GLTF::Primitive& _Primitive,
			const GLTF::Node& _Node) noexcept :
			Primitive{ _Primitive },
			Node{ _Node }
		{
		}
	};
	std::vector<PrimitiveRenderInfo> m_RenderList;

};

} // namespace Diligent