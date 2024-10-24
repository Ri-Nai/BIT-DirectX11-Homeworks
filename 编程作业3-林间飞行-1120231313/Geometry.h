//***************************************************************************************
// Geometry.h by X_Jun(MKXJun) (C) 2018-2019 All Rights Reserved.
// Licensed under the MIT License.
//
// 生成常见的几何体网格模型
// Generate common geometry meshes.
//***************************************************************************************

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <vector>
#include <string>
#include <map>
#include "Vertex.h"
#include <wrl/client.h>
#include "d3dUtil.h"

namespace Geometry
{
	// 网格数据
	template<class VertexType = VertexPosNormalTex, class IndexType = WORD>
	struct MeshData
	{
		std::vector<VertexType> vertexVec;	// 顶点数组
		std::vector<IndexType> indexVec;	// 索引数组

		MeshData()
		{
			// 需检验索引类型合法性
			static_assert(sizeof(IndexType) == 2 || sizeof(IndexType) == 4, "The size of IndexType must be 2 bytes or 4 bytes!");
			static_assert(std::is_unsigned<IndexType>::value, "IndexType must be unsigned integer!");
		}
	};
}

namespace Geometry
{
	namespace Internal
	{
		//
		// 以下结构体和函数仅供内部实现使用
		//

		struct VertexData
		{
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT3 normal;
			DirectX::XMFLOAT4 tangent;
			DirectX::XMFLOAT4 color;
			DirectX::XMFLOAT2 tex;
		};

		// 根据目标顶点类型选择性将数据插入
		template<class VertexType>
		inline void InsertVertexElement(VertexType& vertexDst, const VertexData& vertexSrc)
		{
			static std::string semanticName;
			static const std::map<std::string, std::pair<size_t, size_t>> semanticSizeMap = {
				{"POSITION", std::pair<size_t, size_t>(0, 12)},
				{"NORMAL", std::pair<size_t, size_t>(12, 24)},
				{"TANGENT", std::pair<size_t, size_t>(24, 40)},
				{"COLOR", std::pair<size_t, size_t>(40, 56)},
				{"TEXCOORD", std::pair<size_t, size_t>(56, 64)}
			};

			for (size_t i = 0; i < ARRAYSIZE(VertexType::inputLayout); i++)
			{
				semanticName = VertexType::inputLayout[i].SemanticName;
				const auto& range = semanticSizeMap.at(semanticName);
				memcpy_s(reinterpret_cast<char*>(&vertexDst) + VertexType::inputLayout[i].AlignedByteOffset,
					range.second - range.first,
					reinterpret_cast<const char*>(&vertexSrc) + range.first,
					range.second - range.first);
			}
		}
	}
	
	//
	// 几何体方法的实现
	//

	inline MeshData<VertexPosColor, WORD> CreateModel(const std::string& filePath)
	{
		using namespace DirectX;

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

		MeshData<VertexPosColor, WORD> meshData;
		size_t vertexCount = s_vertices.size();
		size_t indexCount = 3 * s_indices.size();
		meshData.vertexVec.resize(vertexCount);
		meshData.indexVec.resize(indexCount);

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

		Internal::VertexData vertexData;

		VertexPosColor* vertices = new VertexPosColor[vertexCount];
		WORD* indices = new WORD[indexCount];
		size_t vIndex = 0, iIndex = 0;

		// 重新赋值顶点并移动到中心
		for (size_t i = 0; i < vertexCount; ++i) {
			vertexData = {
				XMFLOAT3(tempVertices[i].x - centerX, tempVertices[i].y - centerY, tempVertices[i].z - centerZ),
				XMFLOAT3(0.0f, -1.0f, 0.0f),
				XMFLOAT4(-1.0f, 0.0f, 0.0f, 1.0f),
				XMFLOAT4(dis(gen), dis(gen), dis(gen), dis(gen)), // 随机颜色
				XMFLOAT2(0.0f, 1.0f)
			};
			Internal::InsertVertexElement(meshData.vertexVec[vIndex++], vertexData);
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
			meshData.indexVec[iIndex++] = a - 1;
			meshData.indexVec[iIndex++] = b - 1;
			meshData.indexVec[iIndex++] = c - 1;
		}

		delete[] vertices;		// 释放临时顶点数据
		delete[] indices;		// 释放临时索引数据
		return meshData;
	}

}



#endif


