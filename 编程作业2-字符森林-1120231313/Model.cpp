#include "Model.h"
using namespace DirectX;

Model::Model(const std::string& filePath) {
	readObj(filePath);
}

Model::~Model() {
	delete[] vertices;
	delete[] indices;
}
size_t Model::getVertexCount() const
{
	return vertexCount;
}
size_t Model::getIndexCount() const
{
	return indexCount;
}
void Model::readObj(const std::string& filePath) {
	std::ifstream file(filePath);
	std::random_device rd;  // ���ڻ�ȡ�������
	std::mt19937 gen(rd()); // �������������
	std::uniform_real_distribution<float> dis(0.0f, 1.0f); // [0, 1) �ľ��ȷֲ�

	std::string line;
	std::vector<std::string> s_vertices, s_indices; // �洢ÿ�е�����

	while (std::getline(file, line)) {
		if (!line.empty()) {
			if (line.size() > 1 && line[0] == 'v' && line[1] != 'n')
				s_vertices.push_back(line.substr(2));
			else if (line[0] == 'f')
				s_indices.push_back(line.substr(2));
		}
	}
	file.close();

	vertexCount = s_vertices.size();
	indexCount = 3 * s_indices.size();
	vertices = new GameApp::VertexPosColor[vertexCount];
	indices = new WORD[indexCount];

	for (size_t i = 0; i < s_vertices.size(); ++i) {
		std::istringstream data(s_vertices[i]);
		double x, y, z;
		data >> x >> y >> z;
		// ���Ա任����
		x *= 20, y *= 20, z *= 20;
		vertices[i].pos = XMFLOAT3(x, y, z);
		// �����ɫ
		vertices[i].color = XMFLOAT4(dis(gen), dis(gen), dis(gen), dis(gen));
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
}
