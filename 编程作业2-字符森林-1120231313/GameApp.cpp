#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
using namespace DirectX;

const D3D11_INPUT_ELEMENT_DESC VertexPosColor::inputLayout[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance), m_CBuffer()
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	if (!InitEffect())
		return false;

	if (!InitResource())
		return false;

	return true;
}

void GameApp::OnResize()
{
	D3DApp::OnResize();
}

void GameApp::UpdateScene(float dt)
{
	worlds.clear();
	worlds.resize(2);
	angle += dt;
	
	for (int i = -size; i <= size; i++)
	{
		for (int j = -size; j <= size; j++)
		{
			float length = abs(i) + abs(j);
			float scale = (sinf(0.2 * angle * 3.0f + length) + 1.3f) * 1.3;
			auto mScale = XMMatrixScaling(scale, scale, scale);
			auto mRotateSelf = XMMatrixRotationX(angle + i + j);
			auto mRotateCommon = XMMatrixRotationZ(angle * length * 0.02);
			auto mTranslateXY = XMMatrixTranslation(i * 10, j * 10, 0);
			auto mTranslateZ = XMMatrixTranslation(0, 0, pow((12 - length), 2) * cos(angle * 0.6));
			auto mTranslate = mTranslateXY * mRotateCommon * mTranslateZ;
			worlds[0].push_back(XMMatrixTranspose(mScale * mRotateSelf * mTranslate));
			srand(length);
			
			
			scale *= 0.6;
			auto mTranslateChild = XMMatrixTranslation(3, 3, 0);
			for (int k = 0; k < 6; ++k)
			{
				auto mScaleChild = XMMatrixScaling(scale, scale, scale);
				auto mRotateChild = XMMatrixRotationX(rand()) * XMMatrixRotationY(rand()) * XMMatrixRotationZ(rand() + angle);
				worlds[1].push_back(XMMatrixTranspose(mTranslateChild * mRotateSelf * mScaleChild * mRotateChild * mTranslate));
			}
			
		}
	}
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);
	static float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	// RGBA = (0,0,0,255)
	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&black));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	for (size_t i = 0; i < models.size(); ++i)
		for (const auto &world : worlds[i])
		{
			m_CBuffer.world = world;
			models[i]->Draw(m_pd3dImmediateContext, m_CBuffer, m_pConstantBuffer);
		}
	//models[0]->Draw(m_pd3dImmediateContext, m_CBuffer, m_pConstantBuffer);
	//models[1]->Draw(m_pd3dImmediateContext, m_CBuffer, m_pConstantBuffer);

	HR(m_pSwapChain->Present(0, 0));
}


bool GameApp::InitEffect()
{
	ComPtr<ID3DBlob> blob;

	// 创建顶点着色器
	HR(CreateShaderFromFile(L"HLSL\\Cube_VS.cso", L"HLSL\\Cube_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf()));
	// 创建顶点布局
	HR(m_pd3dDevice->CreateInputLayout(VertexPosColor::inputLayout, ARRAYSIZE(VertexPosColor::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout.GetAddressOf()));

	// 创建像素着色器
	HR(CreateShaderFromFile(L"HLSL\\Cube_PS.cso", L"HLSL\\Cube_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf()));

	return true;
}


bool GameApp::InitResource()
{
	
	std::vector<std::string> modelFiles = {
		"Ning.obj",
		"Jie.obj",
	};

	for (const auto& modelFile : modelFiles)
		models.push_back(Model::LoadModel(m_pd3dDevice, modelFile)); // 假设 models 是一个 Model 指针的向量

	// 设置常量缓冲区描述
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建常量缓冲区
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer.GetAddressOf()));

	// 初始化常量缓冲区的值
	m_CBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
		XMVectorSet(40, 40, -50, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 0, -1, 0.0f)
	));
	m_CBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));

	// 设置图元类型，设定输入布局
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout.Get());
	// 将着色器绑定到渲染管线
	m_pd3dImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
	// 将更新好的常量缓冲区绑定到顶点着色器
	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());

	m_pd3dImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);

	D3D11SetDebugObjectName(m_pVertexLayout.Get(), "VertexPosColorLayout");
	D3D11SetDebugObjectName(m_pConstantBuffer.Get(), "ConstantBuffer");
	D3D11SetDebugObjectName(m_pVertexShader.Get(), "Cube_VS");
	D3D11SetDebugObjectName(m_pPixelShader.Get(), "Cube_PS");
	return true;
}
