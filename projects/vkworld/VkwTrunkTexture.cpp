#include "VkwTrunkTexture.h"

#include <world/tree/Tree.h>
#include <world/assets/ImageUtils.h>

namespace world {

WORLD_REGISTER_CHILD_CLASS(ITreeWorker, VkwTrunkTexture, "VkwTrunkTexture")

VkwTrunkTexture::VkwTrunkTexture(const std::string &shaderName)
        : _texGen(128, 128, shaderName) {}

void VkwTrunkTexture::processInstance(TreeInstance &tree, double resolution) {
    if (resolution >= 7) {
        tree.trunkTexture() = Image(128, 128, ImageType::RGB);
        // TODO make texture generator usable multiple times
        _texGen.generateTextureAsync();
        _texGen.getGeneratedImage(tree.trunkTexture());
    }
}

// TODO transmit parameters
VkwTrunkTexture *VkwTrunkTexture::clone() const {
    return new VkwTrunkTexture();
}

} // namespace world