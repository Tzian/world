#pragma once

#include "world/core/WorldConfig.h"

#include "ITreeWorker.h"
#include "Tree.h"
#include "TreeSkelettonParameters.h"
#include "TreeSkeletton.h"

namespace world {

class WORLDAPI_EXPORT TreeSkelettonGenerator : public ITreeWorker {
    WORLD_WRITE_SUBCLASS_METHOD
public:
    TreeSkelettonGenerator();

    virtual ~TreeSkelettonGenerator();

    TreeSkelettonGenerator *clone() const override;

    void setSeedLocation(const TreeParamd &param);

    void setRootWeight(const TreeParamd &param);

    void setInclination(const TreeParamd &param);

    void setTheta(const TreeParamd &param);

    void setForkingCount(const TreeParami &param);

    void setSize(const TreeParamd &size);

    void setWeight(const TreeParamd &weight);

    void processInstance(TreeInstance &tree, double resolution) override;

    void write(WorldFile &wf) const override;

    void read(const WorldFile &wf) override;

private:
    void forkNode(SkelettonNode<TreeInfo> *node);

    // G�n�ration de nombres al�toires uniforme entre 0 et 1.
    Parameter<double> _rng;

    // Localisation du point de d�part de l'arbre
    TreeParamd _seedLocation;
    // Poids de d�part de l'arbre
    TreeParamd _rootWeight;

    TreeParamd _phi;
    TreeParamd _theta;
    TreeParamd _size;

    TreeParamd _weight;
    // Nombre de branches � chaque division
    TreeParami _count;
};
} // namespace world

// Fonctions sp�cifiques pour les arbres
