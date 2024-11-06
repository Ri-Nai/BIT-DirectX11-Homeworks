#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance),
	m_CameraMode(CameraMode::FirstPerson),
	m_CBFrame(),
	m_CBOnResize(),
	m_CBRarely()
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

	// 初始化鼠标，键盘不需要
	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);

	return true;
}

void GameApp::OnResize()
{
	D3DApp::OnResize();

	// 摄像机变更显示
	if (m_pCamera != nullptr)
	{
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_CBOnResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());
		
		D3D11_MAPPED_SUBRESOURCE mappedData;
		HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
		memcpy_s(mappedData.pData, sizeof(CBChangesOnResize), &m_CBOnResize, sizeof(CBChangesOnResize));
		m_pd3dImmediateContext->Unmap(m_pConstantBuffers[2].Get(), 0);
	}
}

void GameApp::UpdateScene(float dt)
{
	// 更新鼠标事件，获取相对偏移量
	Mouse::State mouseState = m_pMouse->GetState();
	Mouse::State lastMouseState = m_MouseTracker.GetLastState();

	Keyboard::State keyState = m_pKeyboard->GetState();
	m_KeyboardTracker.Update(keyState);

	// 获取子类
	auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
	auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);

	if (keyState.IsKeyDown(Keyboard::W))
		cam1st->MoveForward(dt * 3.0f);
	if (keyState.IsKeyDown(Keyboard::S))
		cam1st->MoveForward(dt * -3.0f);
	if (keyState.IsKeyDown(Keyboard::A))
		cam1st->Strafe(dt * -3.0f);
	if (keyState.IsKeyDown(Keyboard::D))
		cam1st->Strafe(dt * 3.0f);
	if (keyState.IsKeyDown(Keyboard::Q))
		cam1st->Roll(dt * 1.0f);
	if (keyState.IsKeyDown(Keyboard::E))
		cam1st->Roll(dt * -1.0f);

	// 视野旋转，防止开始的差值过大导致的突然旋转
	cam1st->Pitch(mouseState.y * dt * 1.25f);
	cam1st->Yaw(mouseState.x * dt * 1.25f);

	// 更新观察矩阵
	m_pCamera->UpdateViewMatrix();
	XMStoreFloat4(&m_CBFrame.eyePos, m_pCamera->GetPositionXM());
	m_CBFrame.view = XMMatrixTranspose(m_pCamera->GetViewXM());

	// 重置滚轮值
	m_pMouse->ResetScrollWheelValue();


	// 切换灯光
	auto set_color = [&](XMFLOAT4 color, float factor)
	{
		return XMFLOAT4(color.x * factor, color.y * factor, color.z * factor, 1.0f);
	};
	if (m_KeyboardTracker.IsKeyReleased(Keyboard::D1))
	{
		auto color = XMFLOAT4(0.95703125, 0.87109375, 0.52734375, 1.0f);
		m_CBFrame.pointLight[0].position = XMFLOAT3(0.0f, 2.0f, 0.0f);
		m_CBFrame.pointLight[0].ambient = set_color(color, 0.2);
		m_CBFrame.pointLight[0].diffuse = set_color(color, 2.0);
		m_CBFrame.pointLight[0].specular = set_color(color, 1.5);
		m_CBFrame.pointLight[0].att = XMFLOAT3(1.0f, 0.1f, 0.01f);
		m_CBFrame.pointLight[0].range = 8.0f;

		m_PointLightDirection[0] = XMFLOAT3(0.0f, 0.0f, 0.0f);

		m_CBFrame.numPointLight = !m_CBFrame.numPointLight;
	}
	if (m_KeyboardTracker.IsKeyReleased(Keyboard::D2))
	{
		auto color = XMFLOAT4(0.50390625, 0.97265625, 0.984375, 1.0f);
		m_CBFrame.spotLight[0].ambient = set_color(color, 0.2);
		m_CBFrame.spotLight[0].diffuse = set_color(color, 2.0);
		m_CBFrame.spotLight[0].specular = set_color(color, 1.5);
		m_CBFrame.spotLight[0].att = XMFLOAT3(1.0f, 0.05f, 0.005f);
		m_CBFrame.spotLight[0].range = 40.0f;
		m_CBFrame.spotLight[0].spot = 20.0f;

		m_CBFrame.numSpotLight = !m_CBFrame.numSpotLight;
	}
	if (m_KeyboardTracker.IsKeyReleased(Keyboard::OemPlus))
	{
		auto idx = m_CBFrame.numDirLight;
		auto color = XMFLOAT4(0.85546875, 0.7421875, 0.984375, 1.0f);

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(0.0f, 1.0f);
		auto direction = XMFLOAT3(dis(gen), dis(gen), dis(gen));
		auto d_normalized = XMVector3Normalize(XMLoadFloat3(&direction));
		XMStoreFloat3(&direction, d_normalized);

		m_CBFrame.dirLight[idx].ambient = set_color(color, 0.2);
		m_CBFrame.dirLight[idx].diffuse = set_color(color, 1.0);
		m_CBFrame.dirLight[idx].specular = set_color(color, 1.0);
		m_CBFrame.dirLight[idx].direction = direction;
		m_CBFrame.numDirLight++;
	}
	if (m_KeyboardTracker.IsKeyReleased(Keyboard::OemMinus))
	{
		m_CBFrame.numDirLight > 0 ? m_CBFrame.numDirLight-- : 0;
	}

	// 更新光源
	if (m_CBFrame.numPointLight)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(-0.5f, 0.5f);
		auto delta = XMVectorSet(dis(gen), 0.0f, dis(gen), 0.0f);
		auto dir = XMLoadFloat3(&m_PointLightDirection[0]);
		dir = XMVector3Normalize(dir + delta);
		XMStoreFloat3(&m_PointLightDirection[0], dir);

		auto pos = XMLoadFloat3(&m_CBFrame.pointLight[0].position);
		pos += dir * dt * 2.0f;
		XMStoreFloat3(&m_CBFrame.pointLight[0].position, pos);
	}
	if (m_CBFrame.numSpotLight)
	{
		m_CBFrame.spotLight[0].position = m_pCamera->GetPosition();
		m_CBFrame.spotLight[0].direction = m_pCamera->GetLook();
	}


	m_Worlds.clear();
	// m_Worlds[0] 和 [1] 存储 母字符 与 子字符 的世界矩阵 
	m_Worlds.resize(2);
	angle += dt;

	for (int i = -size; i <= size; i++)
	{
		for (int j = -size; j <= size; j++)
		{
			float length = abs(i) + abs(j);
			float scale = (sinf(0.05 * angle * 3.0f + i + j) + 1) * 0.25;
			auto mScale = XMMatrixScaling(scale, scale, scale);

			auto mRotateSelf = XMMatrixRotationX(angle + i + j);
			auto mRotateCommon = XMMatrixRotationY(angle * length * 0.05);
			auto mTranslateXY = XMMatrixTranslation(i * 2.0, 0.0, j * 2.0);
			auto mTranslateZ = XMMatrixTranslation(0, pow((11.5 - length), 2) * cos(angle * 0.6) * 0.015, 0);

			auto mTranslate = mTranslateXY * mRotateCommon * mTranslateZ;

			// 计算出变换然后存储到世界矩阵的数组中
			m_Worlds[0].push_back(mScale * mRotateSelf * mTranslate);
			// 每个子字符是从同一个起点开始的，所以设置种子每次一致
			// 疑似可以通过初始化一个起始位置，但是这样的话代码结构又要大改
			srand(length + i * j);

			scale *= 0.6;
			auto mTranslateChild = XMMatrixTranslation(3, 3, 0);
			for (int k = 0; k < 6; ++k)
			{
				auto mScaleChild = XMMatrixScaling(scale, scale, scale);
				auto mRotateChild = XMMatrixRotationX(rand()) * XMMatrixRotationY(rand()) * XMMatrixRotationZ(rand() + angle);
				// 计算出变换然后存储到世界矩阵的数组中
				m_Worlds[1].push_back(mTranslateChild * mRotateSelf * mScaleChild * mRotateChild * mTranslate);
			}

		}
	}

	
	// 退出程序，这里应向窗口发送销毁信息
	if (keyState.IsKeyDown(Keyboard::Escape))
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
	
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesEveryFrame), &m_CBFrame, sizeof(CBChangesEveryFrame));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//
	// 绘制几何模型
	//
	for (int i = 0; i < m_Models.size(); ++i)
	{
		for (int j = 0; j < m_Worlds[i].size(); j++)
		{
			auto world = m_Worlds[i][j];
			m_Models[i].SetWorldMatrix(world);
			m_Models[i].SetMaterial(m_Materials[i][j]);
			m_Models[i].Draw(m_pd3dImmediateContext.Get());
		}
	}

	HR(m_pSwapChain->Present(0, 0));
}


bool GameApp::InitEffect()
{
	ComPtr<ID3DBlob> blob;

	// 创建顶点着色器(3D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_VS_3D.cso", L"HLSL\\Basic_VS_3D.hlsl", "VS_3D", "vs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pVertexShader3D.GetAddressOf()));
	// 创建顶点布局(3D)
	HR(m_pd3dDevice->CreateInputLayout(VertexPosNormalColor::inputLayout, ARRAYSIZE(VertexPosNormalColor::inputLayout),
		blob->GetBufferPointer(), blob->GetBufferSize(), m_pVertexLayout3D.GetAddressOf()));

	// 创建像素着色器(3D)
	HR(CreateShaderFromFile(L"HLSL\\Basic_PS_3D.cso", L"HLSL\\Basic_PS_3D.hlsl", "PS_3D", "ps_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pPixelShader3D.GetAddressOf()));

	HR(CreateShaderFromFile(L"HLSL\\Basic_GS_3D.cso", L"HLSL\\Basic_GS_3D.hlsl", "GS_3D", "gs_5_0", blob.ReleaseAndGetAddressOf()));
	HR(m_pd3dDevice->CreateGeometryShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, m_pGeometryShader3D.GetAddressOf()));

	return true;
}

bool GameApp::InitResource()
{
	// ******************
	// 设置常量缓冲区描述
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建用于VS和PS的常量缓冲区
	cbd.ByteWidth = sizeof(CBChangesEveryDrawing);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesEveryFrame);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesOnResize);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[2].GetAddressOf()));
	cbd.ByteWidth = sizeof(CBChangesRarely);
	HR(m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[3].GetAddressOf()));
	// ******************
	// 初始化游戏对象
	ComPtr<ID3D11ShaderResourceView> texture;

	// 初始化模型
	const std::vector<std::string> model_paths = {
		"Ning.obj",
		"Jie.obj"
	};
	for (const auto& path : model_paths)
	{
		GameObject model;
		model.SetBuffer(m_pd3dDevice.Get(), Geometry::CreateModel(path));
		m_Models.push_back(model);
	}

	// 初始化模型的世界矩阵
	m_Worlds.resize(m_Models.size());
	auto init_world = [&](std::vector<XMMATRIX>& world, int size)
	{
		for (int i = -size; i <= size; i++)
			for (int j = -size; j <= size; j++)
			{
				auto mScaleMatrix = XMMatrixScaling(.5, .5, .5);
				auto mRotationMatrix = XMMatrixRotationY(0.0f);
				auto mTranslationXYMatrix = XMMatrixTranslation(i * 2.0f, 0.0f, j * 2.0f);
				auto W = mScaleMatrix * mRotationMatrix * mTranslationXYMatrix;
				world.push_back(W);
			}
	};
	init_world(m_Worlds[0], size);
	init_world(m_Worlds[1], size * 6);

	// 初始化模型材质
	m_Materials.resize(m_Models.size());
	auto init_mat = [&](std::vector<Material>& mats, int size)
	{
		for (int i = 0; i < (2 * size + 1) * (2 * size + 1); i++)
		{
			Material material;
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> dis(0.0f, 1.0f);
			auto factor = dis(gen);
			auto color = XMFLOAT3(factor, factor, factor);

			material.ambient = XMFLOAT4(color.x, color.y, color.z, dis(gen));
			material.diffuse = XMFLOAT4(color.x * 0.8, color.y * 0.8, color.z * 0.8, dis(gen));
			material.specular = XMFLOAT4(color.x * 0.1, color.y * 0.1, color.z * 0.1, dis(gen));

			mats.push_back(material);
		}
	};
	init_mat(m_Materials[0], size);
	init_mat(m_Materials[1], size * 6);
		
	// 初始化采样器状态
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	HR(m_pd3dDevice->CreateSamplerState(&sampDesc, m_pSamplerState.GetAddressOf()));

	
	// ******************
	// 初始化常量缓冲区的值
	// 初始化每帧可能会变化的值
	m_CameraMode = CameraMode::Free;
	auto camera = std::shared_ptr<FirstPersonCamera>(new FirstPersonCamera);
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	XMFLOAT3 pos = XMFLOAT3(1.0f, 1.0f, 0.5f);
	XMFLOAT3 to = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	pos.y += 3;
	camera->LookTo(pos, to, up);

	// 初始化仅在窗口大小变动时修改的值
	m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
	m_CBOnResize.proj = XMMatrixTranspose(m_pCamera->GetProjXM());

	// 初始化不会变化的值
	// 灯光
	auto color = XMFLOAT4(0.95703125, 0.87109375, 0.52734375, 1.0f);
	auto set_color = [&](float factor)
	{
		return XMFLOAT4(color.x * factor, color.y * factor, color.z * factor, 1.0f);
	};
	m_CBFrame.pointLight[0].position = XMFLOAT3(0.0f, 2.0f, 0.0f);
	m_CBFrame.pointLight[0].ambient = set_color(0.2);
	m_CBFrame.pointLight[0].diffuse = set_color(2.0);
	m_CBFrame.pointLight[0].specular = set_color(1.5);
	m_CBFrame.pointLight[0].att = XMFLOAT3(1.0f, 0.1f, 0.01f);
	m_CBFrame.pointLight[0].range = 8.0f;

	m_PointLightDirection[0] = XMFLOAT3(0.0f, 0.0f, 0.0f);
	// 光源数量
	m_CBFrame.numDirLight = 0;
	m_CBFrame.numPointLight = 1;
	m_CBFrame.numSpotLight = 0;

	// 更新不容易被修改的常量缓冲区资源
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesOnResize), &m_CBOnResize, sizeof(CBChangesOnResize));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[2].Get(), 0);

	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[3].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesRarely), &m_CBRarely, sizeof(CBChangesRarely));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[3].Get(), 0);

	// ******************
	// 给渲染管线各个阶段绑定好所需资源
	// 设置图元类型，设定输入布局
	m_pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pd3dImmediateContext->IASetInputLayout(m_pVertexLayout3D.Get());
	// 默认绑定3D着色器
	m_pd3dImmediateContext->VSSetShader(m_pVertexShader3D.Get(), nullptr, 0);
	// 预先绑定各自所需的缓冲区，其中每帧更新的缓冲区需要绑定到两个缓冲区上
	m_pd3dImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
	m_pd3dImmediateContext->VSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	m_pd3dImmediateContext->VSSetConstantBuffers(2, 1, m_pConstantBuffers[2].GetAddressOf());

	m_pd3dImmediateContext->GSSetShader(m_pGeometryShader3D.Get(), nullptr, 0);
	m_pd3dImmediateContext->GSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	m_pd3dImmediateContext->GSSetConstantBuffers(2, 1, m_pConstantBuffers[2].GetAddressOf());

	m_pd3dImmediateContext->PSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
	m_pd3dImmediateContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	m_pd3dImmediateContext->PSSetConstantBuffers(3, 1, m_pConstantBuffers[3].GetAddressOf());
	m_pd3dImmediateContext->PSSetShader(m_pPixelShader3D.Get(), nullptr, 0);
	m_pd3dImmediateContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

	// ******************
	// 设置调试对象名
	//
	D3D11SetDebugObjectName(m_pVertexLayout3D.Get(), "VertexPosNormalColorLayout");
	D3D11SetDebugObjectName(m_pConstantBuffers[0].Get(), "CBDrawing");
	D3D11SetDebugObjectName(m_pConstantBuffers[1].Get(), "CBFrame");
	D3D11SetDebugObjectName(m_pConstantBuffers[2].Get(), "CBOnResize");
	D3D11SetDebugObjectName(m_pConstantBuffers[3].Get(), "CBRarely");
	D3D11SetDebugObjectName(m_pVertexShader3D.Get(), "Basic_VS_3D");
	D3D11SetDebugObjectName(m_pPixelShader3D.Get(), "Basic_PS_3D");
	D3D11SetDebugObjectName(m_pSamplerState.Get(), "SSLinearWrap");

	return true;
}

GameApp::GameObject::GameObject()
	: m_IndexCount(), m_VertexStride(), m_Material()
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
}

DirectX::XMFLOAT3 GameApp::GameObject::GetPosition() const
{
	return XMFLOAT3(m_WorldMatrix(3, 0), m_WorldMatrix(3, 1), m_WorldMatrix(3, 2));
}

template<class VertexType, class IndexType>
void GameApp::GameObject::SetBuffer(ID3D11Device * device, const Geometry::MeshData<VertexType, IndexType>& meshData)
{
	// 释放旧资源
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	// 设置顶点缓冲区描述
	m_VertexStride = sizeof(VertexType);
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * m_VertexStride;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));


	// 设置索引缓冲区描述
	m_IndexCount = (UINT)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = m_IndexCount * sizeof(IndexType);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = meshData.indexVec.data();
	HR(device->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));



}

void GameApp::GameObject::SetTexture(ID3D11ShaderResourceView * texture)
{
	m_pTexture = texture;
}

void GameApp::GameObject::SetMaterial(const Material& material)
{
	m_Material = material;
}

void GameApp::GameObject::SetWorldMatrix(const XMFLOAT4X4 & world)
{
	m_WorldMatrix = world;
}

void XM_CALLCONV GameApp::GameObject::SetWorldMatrix(XMMATRIX world)
{
	XMStoreFloat4x4(&m_WorldMatrix, world);
}

void GameApp::GameObject::Draw(ID3D11DeviceContext * deviceContext)
{
	// 设置顶点/索引缓冲区
	UINT strides = m_VertexStride;
	UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);
	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	// 获取之前已经绑定到渲染管线上的常量缓冲区并进行修改
	ComPtr<ID3D11Buffer> cBuffer = nullptr;
	deviceContext->VSGetConstantBuffers(0, 1, cBuffer.GetAddressOf());
	CBChangesEveryDrawing cbDrawing;

	// 内部进行转置，这样外部就不需要提前转置了
	XMMATRIX W = XMLoadFloat4x4(&m_WorldMatrix);
	cbDrawing.world = XMMatrixTranspose(W);
	cbDrawing.worldInvTranspose = XMMatrixInverse(nullptr, W);	// 两次转置抵消
	cbDrawing.material = m_Material;

	// 更新常量缓冲区
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesEveryDrawing), &cbDrawing, sizeof(CBChangesEveryDrawing));
	deviceContext->Unmap(cBuffer.Get(), 0);

	// 设置纹理
	//deviceContext->PSSetShaderResources(0, 1, m_pTexture.GetAddressOf());
	// 可以开始绘制
	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

void GameApp::GameObject::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	std::string vbName = name + ".VertexBuffer";
	std::string ibName = name + ".IndexBuffer";
	m_pVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(vbName.length()), vbName.c_str());
	m_pIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(ibName.length()), ibName.c_str());
#else
	UNREFERENCED_PARAMETER(name);
#endif
}
