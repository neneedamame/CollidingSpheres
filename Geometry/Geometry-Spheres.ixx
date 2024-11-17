module;
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Surface_mesh_default_criteria_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
export module Geometry:Spheres;
import :Geometry;

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polyhedron_3<K> Polyhedron;
typedef K::FT FT;
typedef K::Point_3 Point;
typedef CGAL::Surface_mesh<Point> Surface_mesh;

// 定义隐式函数：单位球体的方程
FT sphere_function(const Point& p) {
	return CGAL::squared_distance(p, Point(0, 0, 0)) - 1;
}

typedef CGAL::Implicit_surface_3<K, decltype(&sphere_function)> Surface;
typedef CGAL::Surface_mesh_default_criteria_3<CGAL::Surface_mesh_traits_generator_3<K>/*CGAL::Surface_mesh_traits_3<K>*/> Criteria;

export namespace xcs
{
	class Spheres : public Geometry
	{
	public:
		double radius = 1;
		void GenerateMesh() override 
        {


		}
	};
}