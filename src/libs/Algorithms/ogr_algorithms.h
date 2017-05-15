#pragma once

#include "../geometry_definitions.h"

#include <ogr_geometry.h>

namespace urban
{
    /**
     * construct OGRPoint from a CGAL Point_2
     * @param point a CGAL Point_2
     * @return a pointer to an OGR Point
     */
    OGRPoint* to_ogr(const Point_2 & point);

    /**
     * construct OGRPolygon from a CGAL Polygon
     * @param polygon a CGAL Polygon
     * @return a pointer to an OGR Linear Ring
     */
    OGRLinearRing* to_ogr(const Polygon & polygon);
    
    /**
     * construct OGRPolygon from a CGAL Polygon with holes
     * @param polygon a CGAL Polygon with holes
     * @return a pointer to an OGR polygon
     */
    OGRPolygon* to_ogr(const Polygon_with_holes & polygon_with_holes);
    
    /**
     * construct Point_2 from a OGRPoint
     * @param ogr_point a pointer to an OGRPoint
     * @return a Point_2
     */
    Point_2 to_urban(OGRPoint* ogr_point);

    /**
     * construct Polygon from a OGRLinearRing
     * @param ogr_ring a pointer to an OGRLinearRing
     * @return a Polygon
     */
    Polygon to_urban(OGRLinearRing* ogr_ring);
    
    /**
     * construct Polygon with holes from a OGRPolygon
     * @param ogr_polygon a pointer to an OGRPolygon
     * @return a Polygon_with_holes
     */
    Polygon_with_holes to_urban(OGRPolygon* ogr_polygon);
}