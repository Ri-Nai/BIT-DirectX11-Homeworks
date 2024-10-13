#ifndef MODEL_H
#define MODEL_H

#include "d3dUtil.h"
#include "DXTrace.h"
#include <wrl/client.h>
using namespace DirectX;

class Model {

	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	Model(ComPtr<ID3D11Buffer> vertexBuffer, ComPtr<ID3D11Buffer> indexBuffer, UINT indexCount);
	~Model();

	void Draw(ComPtr<ID3D11DeviceContext> pContext, ConstantBuffer pCBuffer, ComPtr<ID3D11Buffer> pConstantBuffer) const;
	static Model* LoadModel(ComPtr<ID3D11Device> pDevice, const std::string& filePath);

private:
	ComPtr<ID3D11Buffer> m_pVertexBuffer; // 顶点缓冲区
	ComPtr<ID3D11Buffer> m_pIndexBuffer;  // 索引缓冲区
	UINT m_IndexCount;              // 索引数量
};


#endif // MODEL_H
