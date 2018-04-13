#include "SimpleTreeDecorator.h"

#include <vector>

#include "math/MathsHelper.h"

namespace world {

	SimpleTreeDecorator::SimpleTreeDecorator(int maxTreesPerChunk)
			: _rng(time(NULL)), _maxTreesPerChunk(maxTreesPerChunk) {

		auto &skeletton = _model.addWorker<TreeSkelettonGenerator>();
		skeletton.setConstantMaxForkingLevel(2);
		skeletton.setConstantForkingCount(3);

		_model.addWorker<TrunkGenerator>(12, 0.25, 0.2, 4);

	}

	void SimpleTreeDecorator::setModel(const world::Tree &model) {
		_model.setup(model);
	}

	void SimpleTreeDecorator::decorate(FlatWorld &world, WorldZone &zone) {
		Chunk &chunk = zone->chunk();

		const double treeResolution = 5;
		if (chunk.getMaxResolution() < treeResolution || treeResolution <= chunk.getMinResolution())
			return;

		vec3d chunkSize = chunk.getSize();
		vec3d offset = zone->getAbsoluteOffset();

		std::vector<vec2d> positions;
		std::uniform_real_distribution<double> distribX(0, chunkSize.x);
		std::uniform_real_distribution<double> distribY(0, chunkSize.y);

		IGround &ground = world.ground();

		for (int i = 0; i < _maxTreesPerChunk; i++) {
			// On g�n�re une position pour l'arbre
			vec2d position(distribX(_rng), distribY(_rng));

			// On v�rifie que les autres arbres ne sont pas trop pr�s
			bool addTree = true;

			for (const vec2d &prevPos : positions) {

				if (prevPos.squaredLength(position) < 6) {
					addTree = false;
					break;
				}
			}

			if (!addTree) continue;

			// D�termination de l'altitude de l'arbre
			double altitude = ground.observeAltitudeAt(zone, position.x, position.y);
			vec3d pos3D(position.x, position.y, altitude - offset.z);

			// We don't generate the tree if the ground level is not in the chunk at this location
			if (pos3D.z < 0 || pos3D.z >= chunkSize.z) {
				continue;
			}
			// std::cout << pos3D << std::endl;

			// Cr�ation de l'arbre
			Tree &tree = chunk.addObject<Tree>();
			tree.setup(_model);
			tree.setPosition3D(pos3D);

			// Remember position
			positions.push_back(position);
		}
	}
}