#include "Model.h"

Model::Model(ComPtr<ID3D11Buffer> vertexBuffer, ComPtr<ID3D11Buffer> indexBuffer, UINT indexCount)
	: m_pVertexBuffer(vertexBuffer), m_pIndexBuffer(indexBuffer), m_IndexCount(indexCount) {}

Model::~Model() {
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pIndexBuffer) m_pIndexBuffer->Release();
}

void Model::Draw(ComPtr<ID3D11DeviceContext> context, ConstantBuffer pCBuffer, ComPtr<ID3D11Buffer> pConstantBuffer) const {

	// 更新常量缓冲区数据
	D3D11_MAPPED_SUBRESOURCE mappedData;
	context->Map(pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy_s(mappedData.pData, sizeof(ConstantBuffer), &pCBuffer, sizeof(ConstantBuffer));
	context->Unmap(pConstantBuffer.Get(), 0);
	UINT stride = sizeof(VertexPosColor);
	UINT offset = 0;
	// 绘制模型
	context->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->DrawIndexed(m_IndexCount, 0, 0); // 根据模型的索引计数绘制
}
std::unique_ptr<Model> Model::LoadModel(ComPtr<ID3D11Device> pDevice, const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open OBJ file.");
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(0.0f, 1.0f);

	std::string line;
	std::vector<std::string> s_vertices, s_indices;

	// 存储每行的内容
	while (std::getline(file, line)) {
		if (!line.empty()) {
			if (line.size() > 1 && line[0] == 'v' && line[1] != 'n')
				s_vertices.push_back(line.substr(2));
			else if (line[0] == 'f')
				s_indices.push_back(line.substr(2));
		}
	}
	file.close();

	size_t vertexCount = s_vertices.size();
	size_t indexCount = 3 * s_indices.size();
	VertexPosColor* vertices = new VertexPosColor[vertexCount];
	WORD* indices = new WORD[indexCount];

	float centerX = 0.0f;
	float centerY = 0.0f;
	float centerZ = 0.0f;

	std::vector<XMFLOAT3> tempVertices(vertexCount); // 临时存储顶点

	// 读取顶点并计算质心
	for (size_t i = 0; i < s_vertices.size(); ++i) {
		std::istringstream data(s_vertices[i]);
		float x, y, z;
		data >> x >> y >> z;
		x *= 20; // 线性变换坐标
		y *= 20;
		z *= 20;

		tempVertices[i] = XMFLOAT3(x, y, z); // 存储原始顶点

		centerX += x;
		centerY += y;
		centerZ += z;
	}

	// 计算质心
	centerX /= vertexCount;
	centerY /= vertexCount;
	centerZ /= vertexCount;

	// 重新赋值顶点并移动到中心
	for (size_t i = 0; i < vertexCount; ++i) {
		vertices[i].pos = XMFLOAT3(tempVertices[i].x - centerX, tempVertices[i].y - centerY, tempVertices[i].z - centerZ);
		vertices[i].color = XMFLOAT4(dis(gen), dis(gen), dis(gen), dis(gen)); // 随机颜色
	}
	for (size_t i = 0; i < s_indices.size(); ++i) {
		std::istringstream data(s_indices[i]);
		std::string s_a, s_b, s_c;
		data >> s_a >> s_b >> s_c;
		int a, b, c;
		std::istringstream ss_a(s_a), ss_b(s_b), ss_c(s_c);
		ss_a >> a;
		ss_b >> b;
		ss_c >> c;
		indices[3 * i] = c - 1;
		indices[3 * i + 1] = b - 1;
		indices[3 * i + 2] = a - 1;
	}

	// 创建顶点缓冲区
	ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(VertexPosColor) * vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	HR(pDevice->CreateBuffer(&vbd, &InitData, vertexBuffer.GetAddressOf()));
	delete[] vertices; // 释放临时顶点数据

	// 创建索引缓冲区
	ComPtr<ID3D11Buffer> indexBuffer = nullptr;
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(WORD) * indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	InitData.pSysMem = indices;
	HR(pDevice->CreateBuffer(&ibd, &InitData, indexBuffer.GetAddressOf()));
	delete[] indices; // 释放临时索引数据

	return std::make_unique<Model>(vertexBuffer, indexBuffer, indexCount);
}
