/*
Description :   This is an example of usage of the KDTree class. It does not
                compile, but should give you the idea of how to integrate the
                KDTree in your code
*/


#include "kdtree.h"

/* 
    Your Photon class implementation, which stores each 
    photon walk interaction 
*/
class YourPhoton {
    Vec3D position_;    // 3D point of the interaction
    ...
    // It returns the axis i position (x, y or z)
    float position(std::size_t i) const { return position_[i]}
    ...    
}

/* 
    An additional struct that allows the KD-Tree to access your photon position
*/
struct PhotonAxisPositition {
    float operator()(const YourPhoton& p, std::size_t i) const {
        return p.position(i);
    }
};

/* 
    The KD-Tree ready to work in 3 dimensions, with YourPhoton s, under a 
    brand-new name: YourPhotonMap 
*/
using YourPhotonMap = nn::KDTree<YourPhoton,3,PhotonAxisPosition>;


/*
    Example function to generate the photon map with the given photons
*/
YourPhotonMap generation_of_photon_map(...){
    std::list<YourPhoton> photons = ...;        // Create a list of photons
    map = YourPhotonMap(photons, PhotonAxisPosition())
    return map
}

/*
    Example method to search for the nearest neighbors of the photon map
*/
void search_nearest(YourPhotonMap map, ...){
    // Position to look for the nearest photons
    Vec3D query_position = ...;    

    // Maximum number of photons to look for
    unsigned long nphotons_estimate = ...;

    // Maximum distance to look for photons
    float radius_estimate = ...;

    // nearest is the nearest photons returned by the KDTree
    auto nearest = map.nearest_neighbors(query_position,
                                         nphotons_estimate,
                                         radius_estimate)
}



