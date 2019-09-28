#include <string.h>
#include <string>

#include "map.h"
#include "graphics/graphics.h"
#include "game/tile/tile.h"

#define VERTICES_PER_TILE 6
#define VCOMP_PER_VERTEX 2
#define UV_PER_VERTEX 2
using namespace game;

game::Map::Map (int width, int height) {
    this->width = width;
    this->height = height;
    tiles = NULL;
}
game::Map::~Map () {
    delete[] tiles;
}
Tile* game::Map::getTile (int x, int y) {
    return tiles[x + width * y];
}
void game::Map::setTiles (Tile** tiles) {
    this->tiles = tiles;
}
Tile** game::Map::getTiles () {
    return tiles;
}
int game::Map::getWidth () {
    return width;
}
int game::Map::getHeight () {
    return height;
}
void game::Map::initGraphicsData (graphics::Graphics* graphics, std::string shader) {
    int numVertices = 0;
    for (int i = 0; i < width * height; i++) {
        if (tiles[i]->shouldDraw()) {
            numVertices += VERTICES_PER_TILE;
        }
    }

    float* vertexData = new float[numVertices * VCOMP_PER_VERTEX];
    float* uvData = new float[numVertices * UV_PER_VERTEX];
    float* vertexPtr = vertexData;
    float* uvPtr = uvData;
    for (int i = 0; i < width * height; i++) {
        if (tiles[i]->shouldDraw()) {
            memcpy(vertexData, tiles[i]->getVertexCoords(i % width, i / width), VERTICES_PER_TILE * VCOMP_PER_VERTEX * sizeof(float));
            memcpy(uvData, tiles[i]->getUvs(), VERTICES_PER_TILE * UV_PER_VERTEX * sizeof(float));
            vertexPtr += VERTICES_PER_TILE * VCOMP_PER_VERTEX;
            uvPtr += VERTICES_PER_TILE * UV_PER_VERTEX;
        }
    }
    graphicsData = graphics->loadStaticData(vertexData, uvData, numVertices, numVertices * VCOMP_PER_VERTEX, 
        numVertices * UV_PER_VERTEX, shader);
    
    delete[] vertexData;
    delete[] uvData;
}
void game::Map::unloadGraphicsData (graphics::Graphics* graphics) {
    graphics->unloadStaticData(graphicsData);
    graphicsData = NULL;
}