#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Geometry.h"
#include "LightHelper.h"
#include "Camera.h"
#include <random>

#include "DDSTextureLoader.h"	
#include "WICTextureLoader.h"

class GameApp : public D3DApp
{
public:

	struct CBChangesEveryDrawing
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInvTranspose;
		Material material;
		DirectX::XMFLOAT4 color;

		DirectX::XMFLOAT2 texOffset;
		DirectX::XMFLOAT2 texScale;
	};

	struct CBChangesEveryFrame
	{
		DirectX::XMMATRIX view;
		DirectX::XMFLOAT4 eyePos;
		DirectionalLight dirLight[10];
		PointLight pointLight[10];
		SpotLight spotLight[10];
		int numDirLight;
		int numPointLight;
		int numSpotLight;
	};

	struct CBChangesOnResize
	{
		DirectX::XMMATRIX proj;
	};

	struct CBChangesRarely
	{
		float pad[4];		// 打包保证16字节对齐
	};

	// 一个尽可能小的游戏对象类
	class GameObject
	{
	public:
		GameObject();

		// 获取位置
		DirectX::XMFLOAT3 GetPosition() const;
		// 设置缓冲区
		template<class VertexType, class IndexType>
		void SetBuffer(ID3D11Device * device, const Geometry::MeshData<VertexType, IndexType>& meshData);
		// 设置材质
		void SetMaterial(const Material& material);
		// 设置颜色
		void SetColor(const DirectX::XMFLOAT4& color);
		// 设置纹理
		void SetTexture(ID3D11ShaderResourceView * texture);
		// 设置矩阵
		void SetWorldMatrix(const DirectX::XMFLOAT4X4& world);
		void XM_CALLCONV SetWorldMatrix(DirectX::XMMATRIX world);
		// 设置纹理坐标偏移
		void SetTexOffset(const DirectX::XMFLOAT2& offset);
		// 绘制
		void Draw(ID3D11DeviceContext * deviceContext);

		// 设置调试对象名
		// 若缓冲区被重新设置，调试对象名也需要被重新设置
		void SetDebugObjectName(const std::string& name);
	private:
		DirectX::XMFLOAT4X4 m_WorldMatrix;				    // 世界矩阵
		Material m_Material;								// 物体材质
		DirectX::XMFLOAT4 m_Color;							// 颜色
		ComPtr<ID3D11ShaderResourceView> m_pTexture;		// 纹理
		ComPtr<ID3D11Buffer> m_pVertexBuffer;				// 顶点缓冲区
		ComPtr<ID3D11Buffer> m_pIndexBuffer;				// 索引缓冲区
		UINT m_VertexStride;								// 顶点字节大小
		UINT m_IndexCount;								    // 索引数目	
		DirectX::XMFLOAT2 m_TexOffset;						// 纹理坐标偏移
		DirectX::XMFLOAT2 m_TexScale;						// 纹理坐标缩放
	};

	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson, Free };
	
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	bool InitEffect();
	bool InitResource();

private:
	// 定义了方阵的大小
	static constexpr int size = 12;
	// 定义了游戏至此的角度
	float angle = 0;
	ComPtr<ID3D11InputLayout> m_pVertexLayoutPosNormalColor;	// 无材质顶点输入布局
	ComPtr<ID3D11InputLayout> m_pVertexLayoutPosNormalTex;		// 有材质顶点输入布局
	ComPtr<ID3D11Buffer> m_pConstantBuffers[4];				    // 常量缓冲区

	std::vector<GameObject> m_Models;							// 所有模型
	std::vector<std::vector<DirectX::XMMATRIX>> m_Worlds;		// 所有模型的世界矩阵
	std::vector<std::vector<Material>> m_Materials;				// 所有模型的材质
	std::vector<std::vector<DirectX::XMFLOAT4>> m_Colors;		// 所有模型的颜色
	GameObject m_Plane;											// 平面

	ComPtr<ID3D11RasterizerState> m_pRasterizerState;			// 光栅化状态

	ComPtr<ID3D11VertexShader> m_pVertexShader3D;				// 用于3D的顶点着色器
	ComPtr<ID3D11VertexShader> m_pPlaneVS3D;					// 用于平面的顶点着色器
	ComPtr<ID3D11PixelShader> m_pPixelShader3D;				    // 用于3D的像素着色器
	ComPtr<ID3D11PixelShader> m_pPlanePS3D;						// 用于平面的像素着色器
	ComPtr<ID3D11GeometryShader> m_pGeometryShader3D;			// 用于3D的几何着色器

	CBChangesEveryFrame m_CBFrame;							    // 该缓冲区存放仅在每一帧进行更新的变量
	CBChangesOnResize m_CBOnResize;							    // 该缓冲区存放仅在窗口大小变化时更新的变量
	CBChangesRarely m_CBRarely;								    // 该缓冲区存放不会再进行修改的变量

	ComPtr<ID3D11SamplerState> m_pSamplerState;				    // 采样器状态

	std::shared_ptr<Camera> m_pCamera;						    // 摄像机
	CameraMode m_CameraMode;									// 摄像机模式

	DirectX::XMFLOAT3 m_PointLightDirection[10];				// 点光源运动方向
};


#endif