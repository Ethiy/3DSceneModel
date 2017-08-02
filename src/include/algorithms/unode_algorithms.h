#pragma once

#include <geometry_definitions.h>
#include <scene/unode.h>

#include <vector>


namespace urban
{
    /*! Computes border length*/
    double border_length(scene::UNode & unode);

    /*! Applies affine transformations to unodes*/
    scene::UNode & affine_transform(scene::UNode &, const Affine_transformation_3 &);
    /*! Translate unodes*/
    scene::UNode & translate(scene::UNode &, const Vector_3 &);
    /*! Scale unodes*/
    scene::UNode & scale(scene::UNode &, double);
    /*! Rotate unodes*/
    scene::UNode & rotate(scene::UNode &, const Vector_3 &, double);
    /*! Rotate unodes*/
    scene::UNode & rotate(scene::UNode &, const std::map<double, Vector_3> &);

    /*! Computes plane equations*/
    void plane_equations(scene::UNode &);

    /** Prunes surfaces of unode.
     * @param unode unode to prune
     * @return scene::UNode with pruned surfaces
     */
    scene::UNode & prune(scene::UNode & unode);
    /** Prunes surfaces of unodes.
     * @param unode a vector of unodes to prune
     * @return a vector of unodes with pruned surfaces
     */
    std::vector<scene::UNode> & prune(std::vector<scene::UNode> & unode);

    /** Computes unode surface area*/
    double area(scene::UNode & unode);
}