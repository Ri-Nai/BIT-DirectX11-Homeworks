#ifndef MODEL_H
#define MODEL_H

#include "GameApp.h"
#include "d3dUtil.h"


class Model {
public:
    GameApp::VertexPosColor* vertices = nullptr;
    WORD* indices = nullptr;

    Model(const std::string& filePath);
    ~Model();
    size_t getVertexCount() const;
    size_t getIndexCount() const;

private:
    void readObj(const std::string& filePath);
    size_t vertexCount = 0;
    size_t indexCount = 0;
};

#endif // MODEL_H
