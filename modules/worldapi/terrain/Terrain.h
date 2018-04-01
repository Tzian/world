#pragma once

#include <worldapi/worldapidef.h>

#include <armadillo/armadillo>
#include <vector>
#include <memory>

#include "../maths/Interpolation.h"
#include "../maths/MathsHelper.h"
#include "../maths/BoundingBox.h"
#include "worldapi/assets/Mesh.h"
#include "worldapi/assets/Image.h"

namespace world {

	/* NB : Toutes les ressources g�n�r�es par cette classe utilisent le COLUMN MAJOR ORDER,
	c'est � dire que la premi�re colonne de matrice armadillo correspondra � la premi�re
	ligne d'un fichier image correspondant.
	-> x indique l'indice de la ligne,
	-> y indique l'indice de la colonne.*/

	class WORLDAPI_EXPORT TerrainGenerator;

	/** Cette classe permet de manipuler des terrains planaires avec plusieurs niveaux
	de d�tails.
	Les terrains d�finis par cette classe sont d�finis par des surfaces. A chaque point
	(x, y) du plan, correspond une ou plusieurs altitude(s) z(i), suivant si le terrain
	est simple couche ou multi couche. */
	class WORLDAPI_EXPORT Terrain {

	public :
		explicit Terrain(int size);

		explicit Terrain(const arma::Mat<double> &data);

		Terrain(const Terrain &terrain);

		Terrain(Terrain &&terrain);

		virtual ~Terrain();

		Terrain &operator=(const Terrain &terrain);

		void setBounds(double xmin, double ymin, double zmin, double xmax, double ymax, double zmax);

		const BoundingBox &getBoundingBox() const;

		int getSize() const {
			return (int) _array.n_rows;
		}

		double &operator()(int x, int y);

		/** Gives the height from the height map case which is the
		 * nearest to (x, y).  */
		double getRawHeight(double x, double y) const;

		double getInterpolatedHeight(double x, double y, const Interpolation::interpFunc &func) const;

		/** Gives the height of the terrain at the specified point.
		 * The height given by this method corresponds to the exact
		 * height of the terrain mesh at the point (x, y), given
		 * that it's xy bounds are [0,1] on both axis.
		 * @param x coordinate in terrain-coordinates space, ie. from
		 * 0 to 1.
		 * @param y see above */
		double getExactHeightAt(double x, double y) const;

		// ------ IO

		//Interfa�age avec les fichiers .obj
		Mesh *convertToMesh() const;

		Mesh *convertToMesh(double sizeX, double sizeY, double sizeZ) const;

		Mesh *
		convertToMesh(double offsetX, double offsetY, double offsetZ, double sizeX, double sizeY, double sizeZ) const;

		//M�thodes pour la conversion du terrain en image.
		Image convertToImage() const;

		//Raw map TODO raw data stream ?
		char *getRawData(int &rawDataSize, float height = 1, float offset = 0) const;

		int getRawDataSize() const;

		Image getTexture() const;

		const Image &texture() const;

	private :

		BoundingBox _bbox;
		arma::Mat<double> _array;
		std::unique_ptr<Image> _texture;

		// ------

		friend class TerrainGenerator;

		friend class PerlinTerrainGenerator;

		friend class TerrainManipulator;

		vec2i getPixelPos(double x, double y) const;
	};
}
