module;
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
export module Geometry:Geometry;

import <vector>;

export namespace xcs
{
	class Geometry
	{
	public:
		Geometry() = default;
		const std::vector<glm::vec3>& GetVertices() {
			return vertices;
		}
		const std::vector<unsigned int>& GetIndices() {
			return indices;
		}

		virtual ~Geometry() = default;
		virtual void GenerateMesh() = 0;

	protected:
		std::vector<glm::vec3> vertices;
		std::vector<unsigned int> indices;
	};
}