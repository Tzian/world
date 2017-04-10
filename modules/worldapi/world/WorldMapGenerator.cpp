#include "WorldMapGenerator.h"

#include <utility>
#include <list>
#include <tuple>

#include "../maths/mathshelper.h"
#include "../maths/interpolation.h"

using namespace arma;
using namespace maths;
using namespace img;

// MODULES (� d�placer ?)

WorldMapGeneratorModule::WorldMapGeneratorModule(WorldMapGenerator * parent) : 
	_parent(parent) {

}

cube & WorldMapGeneratorModule::reliefMap(WorldMap & map) const {
	return map._reliefMap;
}

std::mt19937 & WorldMapGeneratorModule::rng() const {
	return _parent->_rng;
}


// -----
ReliefMapGenerator::ReliefMapGenerator(WorldMapGenerator * parent) : 
	WorldMapGeneratorModule(parent) {

}


// -----
const float CustomWorldRMGenerator::PIXEL_UNIT = 10;

CustomWorldRMGenerator::CustomWorldRMGenerator(WorldMapGenerator * parent, float biomeDensity) : 
	ReliefMapGenerator(parent),
	_biomeDensity(biomeDensity) {
	
}

void CustomWorldRMGenerator::generate(WorldMap & map) const {
	cube & relief = reliefMap(map);

	// Nombre de biomes � g�n�rer.
	uint32_t size = relief.n_rows * relief.n_cols;
	uint32_t biomeCount = (uint32_t)(_biomeDensity * (float) size / (PIXEL_UNIT * PIXEL_UNIT));


	// -> Cr�ation de la grille pour le placement des points de mani�re al�atoire,
	// mais avec une distance minimum.

	// Calcul des dimensions de la grille
	float minDistance = PIXEL_UNIT / 2.0 * sqrt(_biomeDensity);
	
	uint32_t sliceCount = maths::max<uint32_t>((uint32_t)((float)relief.n_rows / minDistance), 1);
	float sliceSize = (float)relief.n_rows / (float)sliceCount;

	uint32_t caseCount = maths::max<uint32_t>((uint32_t)((float)relief.n_cols / minDistance), 1);
	float caseSize = (float)relief.n_cols / (float)caseCount;

	// Pr�paration de la grille
	typedef std::pair<vec2d, vec2d> point; // pour plus de lisibilit�
	std::vector<std::vector<point>> pointsMap; pointsMap.reserve(sliceCount);
	std::vector<std::pair<uint32_t, uint32_t>> grid; grid.reserve(sliceCount * caseCount);

	for (int x = 0; x < sliceCount; x++) {
		pointsMap.emplace_back();

		std::vector<point> & slice = pointsMap[x];
		slice.reserve(caseCount);

		for (int y = 0; y < caseCount; y++) {
			grid.emplace_back(x, y);
			slice.emplace_back(vec2d(-1, -1), vec2d(0, 0));
		}
	}
	
	// G�n�ration des points
	std::uniform_real_distribution<double> rand(0.0, 1.0);

	for (int i = 0; i < biomeCount; i++) { // TODO dans les cas limites la grille peut se vider totalement
		// G�n�ration des coordonn�es des points
		uint32_t randIndex = (uint32_t)(rand(rng()) * grid.size());
		std::pair<uint32_t, uint32_t> randPoint = grid.at(randIndex);
		grid.erase(grid.begin() + randIndex);

		uint32_t x = randPoint.first;
		uint32_t y = randPoint.second;

		// Calcul des limites dans lesquelles on peut avoir un point
		double limNegX = 0;
		double limPosX = sliceSize;
		double limNegY = 0;
		double limPosY = caseSize;

		if (x > 0) {
			auto negXCase = pointsMap[x - 1][y];

			if (negXCase.first.x >= 0) {
				limNegX = negXCase.first.x;
			}
		}
		if (x < sliceCount - 1) {
			auto posXCase = pointsMap[x + 1][y];

			if (posXCase.first.x >= 0) {
				limPosX = posXCase.first.x;
			}
		}
		if (y > 0) {
			auto negYCase = pointsMap[x][y - 1];

			if (negYCase.first.y >= 0) {
				limNegY = negYCase.first.y;
			}
		}
		if (y < caseCount - 1) {
			auto posYCase = pointsMap[x][y + 1];

			if (posYCase.first.y >= 0) {
				limPosY = posYCase.first.y;
			}
		}

		// � partir des limites on peut d�terminer la position random du point
		double randX = rand(rng());
		double randY = rand(rng());

		// TODO ces deux param�tres sont random.
		float elevation = rand(rng());
		float diff = rand(rng()) * elevation;

		pointsMap[x][y] = std::make_pair<vec2d, vec2d>(
			vec2d(
				(float)(randX * (limPosX - limNegX) + limNegX + x * sliceSize),
				(float)(randY * (limPosY - limNegY) + limNegY + y * caseSize)),
			vec2d(elevation, diff));
	}


	// -> Interpolation des valeurs des points pour reconstituer une map
	
	// Cr�ation des interpolateur
	IDWInterpolator<vec2d, vec2d> interpolator(6);

	// On pr�pare les donn�es � interpoler.
	for (auto & slice : pointsMap) {
		for (auto & pt : slice) {
			if (pt.first.x >= 0) {
				interpolator.addData(pt.first, pt.second);
			}
		}
	}

	// On remplit la grille � l'aide de l'interpolateur. And enjoy.
	for (uint32_t x = 0; x < relief.n_rows; x++) {
		for (uint32_t y = 0; y < relief.n_cols; y++) {
			vec2d pt((double)x + 0.5, (double)y + 0.5);
			vec2d result = interpolator.getData(pt);
			relief.at(x, y, 0) = result.x;
			relief.at(x, y, 1) = result.y;
		}
	}
}


// WorldMapGenerator

WorldMapGenerator::WorldMapGenerator(uint32_t sizeX, uint32_t sizeY) :
	_rng(time(NULL)),
	_sizeX(sizeX), _sizeY(sizeY),
	_reliefMap(nullptr) {

}

WorldMapGenerator::~WorldMapGenerator() {

}

std::unique_ptr<WorldMap> WorldMapGenerator::generate() {
	std::unique_ptr<WorldMap> map = std::make_unique<WorldMap>(_sizeX, _sizeY);

	if (_reliefMap != nullptr) _reliefMap->generate(*map);

	return map;
}
