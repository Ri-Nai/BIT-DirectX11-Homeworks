#ifndef MODEL_H
#define MODEL_H

#include "d3dUtil.h"
#include "DXTrace.h"
#include <wrl/client.h>
using namespace DirectX;

class Model {

	// ʹ��ģ�����(C++11)��������
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	Model(ComPtr<ID3D11Buffer> vertexBuffer, ComPtr<ID3D11Buffer> indexBuffer, UINT indexCount);
	~Model();

	void Draw(ComPtr<ID3D11DeviceContext> pContext, ConstantBuffer pCBuffer, ComPtr<ID3D11Buffer> pConstantBuffer) const;
	static Model* LoadModel(ComPtr<ID3D11Device> pDevice, const std::string& filePath);

private:
	ComPtr<ID3D11Buffer> m_pVertexBuffer; // ���㻺����
	ComPtr<ID3D11Buffer> m_pIndexBuffer;  // ����������
	UINT m_IndexCount;              // ��������
};


#endif // MODEL_H
