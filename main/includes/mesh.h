#ifndef MESH_H
#define MESH_H

#include "ers/vector.h"
#include "ers/common.h"
#include "ers/vec.h"
#include "ers/matrix.h"
#include "ers/macros.h"
#include "ers/hash_map.h"
#include "software_renderer.h"

struct Vertex {
	ers::vec3 position;
	ers::vec3 normal;
	ers::vec2 tex_coords;

	inline bool operator ==(const Vertex& ver2) const
	{
		return position == ver2.position
			&& normal == ver2.normal
			&& tex_coords == ver2.tex_coords;
	}
};

class Mesh
{
public:
	enum Flags { HAS_NORMALS = 1 << 0, HAS_TEXCOORDS = 1 << 1 };
	Mesh();

	const Vertex& GetVertex(size_t idx) const;
	s32 GetIndex(size_t i) const;

	size_t GetVertexCount() const;
	size_t GetFaceCount() const;

	void PushVertex(const Vertex& vert);
	void PushVertex(Vertex&& vert);
	void PushIndex(s32 idx);

	void SetHasNormals();
	void SetHasTexcoords();

	bool GetHasNormals() const;
	bool GetHasTexcoords() const;

	void Draw(Renderer* renderer) const;

private:
	ers::Vector<Vertex> m_vertices;
	ers::Vector<s32> m_indices;

	u8 m_status; // xxxx xxba: a ->	has normals, b -> has texture coordinates.
};

ers::vec3 calculate_tangent(const Vertex& vert0, const Vertex& vert1, const Vertex& vert2);
void calculate_tbn_vectors(
	const Vertex& vert0, const Vertex& vert1, const Vertex& vert2,
	ers::vec3& t, ers::vec3& b, ers::vec3& n	
);

// Quad centered at (0, 0, 0), with dimensions 1x1, looking at the positive z-direction.
void make_quad(Mesh& quad);

// Quad centered at (xpos, ypos, 0), with dimensions 1x1, looking at the positive z-direction.
void make_quad(f32 xpos, f32 ypos, f32 width, f32 height, Mesh& quad);

// Cube centered at (0, 0, 0).
void make_cube(Mesh& cube);
void make_cube(const ers::vec3& dim, Mesh& cube);

size_t hash(const Vertex& v);
void load_object_file(const char* filename, Mesh& model);

#endif // MESH_H