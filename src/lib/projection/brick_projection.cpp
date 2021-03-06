#include <projection/brick_projection.h>

#include <projection/utilities.h>

#include <algorithms/util_algorithms.h>

#include <CGAL/Boolean_set_operations_2.h>

#include <list>
#include <iterator>
#include <algorithm>

#include <stdexcept>

namespace city
{
    namespace projection
    {
        BrickPrint::BrickPrint(void)
        {}
        BrickPrint::BrickPrint(FacePrint const& face_projection)
            : bounding_box(face_projection.bbox()),
              projected_facets(std::vector<FacePrint>{{face_projection}})
        {}
        BrickPrint::BrickPrint(OGRLayer* projection_layer)
        {
            projection_layer->ResetReading();

            OGRFeature* ogr_facet;
            while((ogr_facet = projection_layer->GetNextFeature()) != nullptr)
            {
                FacePrint facet(ogr_facet, projection_layer->GetLayerDefn());
                projected_facets.push_back(facet);
                OGRFeature::DestroyFeature(ogr_facet);
                bounding_box += facet.bbox();
            }
        }
        BrickPrint::BrickPrint(BrickPrint const& other)
            : bounding_box(other.bounding_box),
              projected_facets(other.projected_facets)
        {}
        BrickPrint::BrickPrint(BrickPrint && other)
            : bounding_box(std::move(other.bounding_box)),
              projected_facets(std::move(other.projected_facets))
        {}
        BrickPrint::~BrickPrint(void)
        {}

        void BrickPrint::swap(BrickPrint & other)
        {
            using std::swap;

            swap(bounding_box, other.bounding_box);
            swap(projected_facets, other.projected_facets);
        }
            
        BrickPrint & BrickPrint::operator =(BrickPrint const& other)
        {
            bounding_box = other.bounding_box;
            projected_facets = other.projected_facets;

            return *this;
        }

        BrickPrint & BrickPrint::operator =(BrickPrint && other)
        {
            bounding_box = std::move(other.bounding_box);
            projected_facets = std::move(other.projected_facets);

            return *this;
        }

        Bbox_2 const& BrickPrint::bbox(void) const noexcept
        {
            return bounding_box;
        }
        std::size_t BrickPrint::size(void) const noexcept
        {
            return projected_facets.size();
        }

        BrickPrint::iterator BrickPrint::begin(void) noexcept
        {
            return projected_facets.begin();
        }
        BrickPrint::iterator BrickPrint::end(void) noexcept
        {
            return projected_facets.end();
        }
        BrickPrint::const_iterator BrickPrint::begin(void) const noexcept
        {
            return projected_facets.begin();
        }
        BrickPrint::const_iterator BrickPrint::end(void) const noexcept
        {
            return projected_facets.end();
        }
        BrickPrint::const_iterator BrickPrint::cbegin(void) const noexcept
        {
            return projected_facets.cbegin();
        }
        BrickPrint::const_iterator BrickPrint::cend(void) const noexcept
        {
            return projected_facets.cend();
        }
        bool BrickPrint::is_empty(void) const noexcept
        {
            return projected_facets.empty();
        }


        bool BrickPrint::contains(Point_2 const& point) const
        {
            return std::any_of(
                std::begin(projected_facets),
                std::end(projected_facets),
                [&point](FacePrint const& facet)
                {
                    return facet.contains(point);
                }
            );
        }
        bool BrickPrint::contains(InexactPoint_2 const& inexact_point) const
        {
            InexactToExact to_exact;
            return contains(to_exact(inexact_point));
        }
        bool BrickPrint::in_domain(Point_2 const& point) const
        {
            return CGAL::do_overlap(bounding_box, point.bbox());
        }
        bool BrickPrint::contains(FacePrint const& facet) const
        {
            Polygon_set ps;
            for(auto const& _facet : projected_facets)
                ps.join(_facet.get_polygon());
            ps.intersection(facet.get_polygon());
            std::vector<Polygon_with_holes> _inter;
            ps.polygons_with_holes(std::back_inserter(_inter));
            return _inter.size() == 1 && _inter.at(0) == facet.get_polygon();
        }
        bool BrickPrint::overlaps(Polygon const& polygon) const
        {
            Polygon_set ps;
            for(auto const& _facet : projected_facets)
                ps.join(_facet.get_polygon());
            ps.intersection(polygon);
            return !ps.is_empty();
        }
        bool BrickPrint::overlaps(Polygon_with_holes const& polygon) const
        {
            Polygon_set ps;
            for(auto const& _facet : projected_facets)
                ps.join(_facet.get_polygon());
            ps.intersection(polygon);
            return !ps.is_empty();
        }
        bool BrickPrint::overlaps(FacePrint const& facet) const
        {
            return overlaps(facet.get_polygon());
        }
        bool BrickPrint::is_under(FacePrint const& facet) const
        {
            bool under(false);

            if(facet.is_perpendicular() || facet.is_degenerate())
                under = true;
            else
            {
                Point_2 point(
                    CGAL::centroid(
                        facet.outer_boundary()[0],
                        facet.outer_boundary()[1],
                        facet.outer_boundary()[2]
                    )
                );
                under = contains(facet) && facet.get_height(point) <= get_height(point); /** there are no intersections */
            }
            return under;
        }

        double BrickPrint::get_height(Point_2 const& point) const
        {
            double height(0.);
            if(contains(point))
                height  = std::accumulate(
                    std::begin(projected_facets),
                    std::end(projected_facets),
                    height,
                    [&point](double & result_height, FacePrint const& facet)
                    {
                        return result_height + facet.get_height(point);
                    }
                );
            return height;
        }
        double BrickPrint::get_height(InexactPoint_2 const& inexact_point) const
        {
            double height(0.);
            if(contains(inexact_point))
                height  = std::accumulate(
                    std::begin(projected_facets),
                    std::end(projected_facets),
                    height,
                    [&inexact_point](double & result_height, FacePrint const& facet)
                    {
                        return result_height + facet.get_height(inexact_point);
                    }
                );
            return height;
        }

        std::vector<double> BrickPrint::areas(void) const
        {
            std::vector<double> results(projected_facets.size());
            std::transform(
                std::begin(projected_facets),
                std::end(projected_facets),
                std::begin(results),
                [](FacePrint const& facet)
                {
                    return facet.area();
                }
            );
            return results;
        }
        double BrickPrint::area(void) const
        {
            auto results = areas();

            return std::accumulate(
                std::begin(results),
                std::end(results),
                0.,
                std::plus<double>()
            );
        }
        std::vector<double> BrickPrint::edge_lengths(void) const
        {
            Polygon_set ps;
            for(auto const& _facet : projected_facets)
                ps.join(_facet.get_polygon());
            
            std::list<Polygon_with_holes> footprint_polygons;
            ps.polygons_with_holes(std::back_inserter(footprint_polygons));

            auto size = std::accumulate(
                std::begin(footprint_polygons),
                std::end(footprint_polygons),
                std::size_t(0),
                [](std::size_t const _size, Polygon_with_holes const& footprint)
                {
                    return _size + footprint.outer_boundary().size();
                }
            );

            std::vector<double> result;
            result.reserve(size);
            for(auto const& footprint : footprint_polygons)
            {
                auto buffer = ::city::edge_lengths(footprint.outer_boundary());
                result.insert(std::end(result), std::begin(buffer), std::end(buffer));
            }
            return result;
        }
        double BrickPrint::circumference(void) const
        {
            auto lengths = edge_lengths();
    
            return std::accumulate(
                std::begin(lengths),
                std::end(lengths),
                0.,
                std::plus<double>()
            );
        }

        void BrickPrint::filter(void)
        {
            projected_facets.erase(
                std::remove_if(
                    std::begin(projected_facets),
                    std::end(projected_facets),
                    [](FacePrint const& facet)
                    {
                        return facet.is_empty() || facet.is_degenerate();
                    }
                ),
                std::end(projected_facets)
            );
        }

        BrickPrint & BrickPrint::operator +=(FacePrint const& lfacet)
        {
            if(projected_facets.empty())
                projected_facets.push_back(lfacet);
            else
            {
                /* If lfacet does not intersect the surface we push it directly*/
                if(!overlaps(lfacet))
                    projected_facets.push_back(lfacet);
                else
                {
                    /* If lfacet is under the surface we loose it*/
                    if(!is_under(lfacet))
                    {
                        std::vector<FacePrint> result;
                        BrickPrint lfacets(lfacet);

                        for(auto const& rfacet : projected_facets)
                        {
                            auto temp = lfacets.occlusion(rfacet);
                            result.insert(std::end(result), std::begin(temp), std::end(temp));
                        }
                        result.insert(std::end(result), std::begin(lfacets.projected_facets), std::end(lfacets.projected_facets));
                        projected_facets = std::move(result);
                    }
                }
            }
            bounding_box += lfacet.bbox();

            return *this;
        }
        BrickPrint & BrickPrint::operator +=(BrickPrint const& other)
        {
            filter();

            for(auto const& facet : other.projected_facets)
            {
                if(!facet.is_empty() && !facet.is_degenerate())
                    operator +=(facet);
            }

            filter();

            return *this;
        }

        std::vector<FacePrint> BrickPrint::occlusion(FacePrint const& lfacet)
        {
            filter();

            std::vector<FacePrint>  lhs,
                                    rhs;

            for(auto const& rfacet : projected_facets)
            {
                std::vector<Polygon_with_holes> intersections;
                CGAL::intersection(lfacet.get_polygon(), rfacet.get_polygon(), std::back_inserter(intersections));
                if(intersections.empty())
                {
                    lhs.push_back(lfacet);
                    rhs.push_back(rfacet);
                }
                else
                {
                    Polygon_set _lhs(lfacet.get_polygon()),
                                _rhs(rfacet.get_polygon());
                
                    for(auto const& intersection : intersections)
                    {
                        Point_2 sample_point = CGAL::centroid(intersection.outer_boundary()[0], intersection.outer_boundary()[1], intersection.outer_boundary()[2]);

                        if(lfacet.get_plane_height(sample_point) > rfacet.get_plane_height(sample_point))
                            _rhs.difference(intersection);
                        else
                            _lhs.difference(intersection);
                    }

                    lhs = unpack(lhs, _lhs, lfacet.get_id(), lfacet.get_plane());
                    rhs = unpack(rhs, _rhs, rfacet.get_id(), rfacet.get_plane());
                }
            }
            projected_facets = rhs;

            return lhs;
        }

        void BrickPrint::to_ogr(OGRLayer* projection_layer, shadow::Point const& reference_point, bool labels) const
        {
            OGRFieldDefn* facet_id = new OGRFieldDefn("Id", OFTInteger64);
            projection_layer->CreateField(facet_id);

            OGRFieldDefn* plane_coefficient_a = new OGRFieldDefn("coeff_a", OFTReal);
            projection_layer->CreateField(plane_coefficient_a);
            OGRFieldDefn* plane_coefficient_b = new OGRFieldDefn("coeff_b", OFTReal);
            projection_layer->CreateField(plane_coefficient_b);
            OGRFieldDefn* plane_coefficient_c = new OGRFieldDefn("coeff_c", OFTReal);
            projection_layer->CreateField(plane_coefficient_c);
            OGRFieldDefn* plane_coefficient_d = new OGRFieldDefn("coeff_d", OFTReal);
            projection_layer->CreateField(plane_coefficient_d);
            if(labels)
            {
                OGRFieldDefn* unqualified_errors = new OGRFieldDefn("Unq_Errors", OFTString);
                projection_layer->CreateField(unqualified_errors);
                OGRFieldDefn* building_errors = new OGRFieldDefn("Bul_Errors", OFTString);
                projection_layer->CreateField(building_errors);
                OGRFieldDefn* facets_errors = new OGRFieldDefn("Fac_Errors", OFTString);
                projection_layer->CreateField(facets_errors);
            }

            for(auto const& facet : projected_facets)
            {
                OGRFeature* ogr_facet = facet.to_ogr(projection_layer->GetLayerDefn(), reference_point, labels);
                if(projection_layer->CreateFeature(ogr_facet) != OGRERR_NONE)
                    throw std::runtime_error("GDAL could not insert the facet in vector image!");
                OGRFeature::DestroyFeature(ogr_facet);
            }
        }

        bool BrickPrint::equal_facets(BrickPrint const& other) const
        {
            bool equality(projected_facets.size() == other.projected_facets.size());
            if(equality)
            {
                equality = std::is_permutation(
                    std::begin(projected_facets),
                    std::end(projected_facets),
                    std::begin(other.projected_facets)
                );
            }
            return equality;
        }

        std::ostream & operator <<(std::ostream & os, BrickPrint const& brick_projection)
        {
            os << "Bounding box: " << brick_projection.bounding_box << std::endl
               << "Face Projections: " << brick_projection.projected_facets.size() << std::endl;
            std::copy(std::begin(brick_projection.projected_facets), std::end(brick_projection.projected_facets), std::ostream_iterator<FacePrint>(os, "\n"));
            return os;
        }

        BrickPrint operator +(BrickPrint const& lhs, BrickPrint const& rhs)
        {
            BrickPrint result(lhs);
            return result += rhs;
        }
        bool operator ==(BrickPrint const& lhs, BrickPrint const& rhs)
        {
            return  lhs.equal_facets(rhs);
        }
        bool operator !=(BrickPrint const& lhs, BrickPrint const& rhs)
        {
            return !(lhs == rhs);
        }
    }

    void swap(projection::BrickPrint & lhs, projection::BrickPrint & rhs)
    {
        lhs.swap(rhs);
    }

    std::vector<double> areas(projection::BrickPrint const& brick_projection)
    {
        return brick_projection.areas();
    }
    double area(projection::BrickPrint const& brick_projection)
    {
        return brick_projection.area();
    }
    std::vector<double> edge_lengths(projection::BrickPrint const& brick_projection)
    {
        return brick_projection.edge_lengths();
    }
    double circumference(projection::BrickPrint const& brick_projection)
    {
        return brick_projection.circumference();
    }
}
