#include "mesh.h"

Mesh::Mesh() : m_status(0) {}

const Vertex& Mesh::GetVertex(size_t idx) const
{
	return m_vertices[idx];
}

s32 Mesh::GetIndex(size_t i) const
{
	return m_indices[i];
}

size_t Mesh::GetVertexCount() const
{
	return m_vertices.GetSize();
}

size_t Mesh::GetFaceCount() const
{
	return m_indices.GetSize() / 3;
}

void Mesh::PushVertex(const Vertex& vert)
{
	m_vertices.PushBack(vert);
}

void Mesh::PushVertex(Vertex&& vert)
{
	m_vertices.PushBack(std::move(vert));
}

void Mesh::PushIndex(s32 idx)
{
	m_indices.PushBack(idx);
}

void Mesh::SetHasNormals()
{
	m_status |= HAS_NORMALS;
}

void Mesh::SetHasTexcoords()
{
	m_status |= HAS_TEXCOORDS;
}

bool Mesh::GetHasNormals() const
{
    return (m_status & HAS_NORMALS) > 0;
}

bool Mesh::GetHasTexcoords() const
{
    return (m_status & HAS_TEXCOORDS) > 0;
}
void Mesh::Draw(Renderer* renderer) const
{
	const s32 count_tris = (s32)GetFaceCount();	
	for (s32 i = 0; i < count_tris; ++i)
	{
		// Assembly.
		const s32 base = 3 * i;
		const Vertex& vert0 = GetVertex(GetIndex(base));
		const Vertex& vert1 = GetVertex(GetIndex(base + 1));
		const Vertex& vert2 = GetVertex(GetIndex(base + 2));

		VertexAttributes1 v0, v1, v2;
		v0 = { vert0.position, vert0.normal, vert0.tex_coords };
		v1 = { vert1.position, vert1.normal, vert1.tex_coords };
		v2 = { vert2.position, vert2.normal, vert2.tex_coords };

        renderer->RenderTriangle(&v0, &v1, &v2);
    }
}

ers::vec3 calculate_tangent(const Vertex& vert0, const Vertex& vert1, const Vertex& vert2)
{
	ers::vec3 d0 = vert1.position - vert0.position;
	ers::vec3 d1 = vert2.position - vert1.position;

	ers::vec2 ds(vert1.tex_coords.x() - vert0.tex_coords.x(), vert2.tex_coords.x() - vert1.tex_coords.x());
	ers::vec3 dt(vert1.tex_coords.y() - vert0.tex_coords.y(), vert2.tex_coords.y() - vert1.tex_coords.y());

	f32 det = ds.x() * dt.y() - ds.y() * dt.x();
	if (ers::equals_epsilon_approx(det, 0.0f, 1.0e-6f)) return ers::vec3(0.0f);

	det = 1.0f / det; dt *= det;
	return d0 * dt.y() - d1 * dt.x();
}

void calculate_tbn_vectors(
	const Vertex& vert0, const Vertex& vert1, const Vertex& vert2,
	ers::vec3& t, ers::vec3& b, ers::vec3& n	
)
{
	const ers::vec3 d0 = vert1.position - vert0.position;
	const ers::vec3 d1 = vert2.position - vert1.position;
	const ers::vec2 ds(vert1.tex_coords.x() - vert0.tex_coords.x(), vert2.tex_coords.x() - vert1.tex_coords.x());
	ers::vec3 dt(vert1.tex_coords.y() - vert0.tex_coords.y(), vert2.tex_coords.y() - vert1.tex_coords.y());

	f32 det = ds.x() * dt.y() - ds.y() * dt.x();	
	ers::mat3 result(1.0f);
	if (!ers::equals_epsilon_approx(det, 0.0f, 1.0e-6f))
	{
		det = 1.0f / det;
		dt *= det;
		n = ers::normalize(ers::cross(d0, d1));
		t = ers::normalize(d0 * dt.y() - d1 * dt.x());
		b = ers::normalize(ers::cross(n, t));
	}
	else
	{		
		t = ers::vec3(1.0f, 0.0f, 0.0f);
		b = ers::vec3(0.0f, 1.0f, 0.0f);
		n = ers::vec3(0.0f, 0.0f, 1.0f);
	}
}

void make_quad(Mesh& quad)
{
	make_quad(-0.5f, 0.5f, 1.0f, 1.0f, quad);
}

void make_quad(float xpos, float ypos, float width, float height, Mesh& quad)
{
	const float x2 = xpos + width;
	const float y2 = ypos - height;

	quad.PushVertex({ ers::vec3(xpos, y2, 0.0f), ers::vec3(0.0f, 0.0f, 1.0f), 	ers::vec2(0.0f, 0.0f) });
	quad.PushVertex({ ers::vec3(x2, y2, 0.0f), ers::vec3(0.0f, 0.0f, 1.0f), 	 	ers::vec2(1.0f, 0.0f) });
	quad.PushVertex({ ers::vec3(x2, ypos, 0.0f), ers::vec3(0.0f, 0.0f, 1.0f), 	ers::vec2(1.0f, 1.0f) });
	quad.PushVertex({ ers::vec3(xpos, ypos, 0.0f), ers::vec3(0.0f, 0.0f, 1.0f), 	ers::vec2(0.0f, 1.0f) });
	quad.PushIndex(0);
	quad.PushIndex(1);
	quad.PushIndex(2);
	quad.PushIndex(0);
	quad.PushIndex(2);
	quad.PushIndex(3);

	quad.SetHasNormals();
	quad.SetHasTexcoords();
}

void make_cube(Mesh& cube)
{
	make_cube(ers::vec3(1.0f), cube);
}

void make_cube(const ers::vec3& dim, Mesh& cube)
{
	// Produce points:
	// 
	// 7------8 
	// |\     |\
	// | 4------3
	// 6-|----5 |
	//  \|     \|
	//   1------2
	//
	// The normal of the 1 - 2 - 3 - 4 quad is assumed to be
	// in the positive z direction.
	ers::vec3 p3 = dim * 0.5f;
	ers::vec3 p2 = p3; p2.y() -= dim.y();
	ers::vec3 p1 = p2; p1.x() -= dim.x();
	ers::vec3 p4 = p1; p4.y() += dim.y();
	ers::vec3 p5 = p2; p5.z() -= dim.z();
	ers::vec3 p6 = p1; p6.z() -= dim.z();
	ers::vec3 p7 = p4; p7.z() -= dim.z();
	ers::vec3 p8 = p3; p8.z() -= dim.z();
	// UV coords.
	ers::vec2 uv1(0.0f, 0.0f);
	ers::vec2 uv2(1.0f, 0.0f);
	ers::vec2 uv3(1.0f, 1.0f);
	ers::vec2 uv4(0.0f, 1.0f);
	// Face 1: 1/2/3/4 (positive-z).
	ers::vec3 current_normal(0.0f, 0.0f, 1.0f);
	cube.PushVertex({ p1, current_normal, uv1 });
	cube.PushVertex({ p2, current_normal, uv2 });
	cube.PushVertex({ p3, current_normal, uv3 });
	cube.PushVertex({ p4, current_normal, uv4 });
	cube.PushIndex(0); cube.PushIndex(1); cube.PushIndex(2);
	cube.PushIndex(0); cube.PushIndex(2); cube.PushIndex(3);

	// Face 2: 5/6/7/8 (negative-z).
	current_normal.z() = -current_normal.z();
	cube.PushVertex({ p5, current_normal, uv1 });
	cube.PushVertex({ p6, current_normal, uv2 });
	cube.PushVertex({ p7, current_normal, uv3 });
	cube.PushVertex({ p8, current_normal, uv4 });
	cube.PushIndex(4); cube.PushIndex(5); cube.PushIndex(6);
	cube.PushIndex(4); cube.PushIndex(6); cube.PushIndex(7);
	// Face 3: 2/5/8/3 (positive-x).
	current_normal.z() = 0.0f; current_normal.x() = 1.0f;
	cube.PushVertex({ p2, current_normal, uv1 });
	cube.PushVertex({ p5, current_normal, uv2 });
	cube.PushVertex({ p8, current_normal, uv3 });
	cube.PushVertex({ p3, current_normal, uv4 });
	cube.PushIndex(8); cube.PushIndex(9); cube.PushIndex(10);
	cube.PushIndex(8); cube.PushIndex(10); cube.PushIndex(11);
	// Face 4: 6/1/4/7 (negative-x).
	current_normal.x() = -current_normal.x();
	cube.PushVertex({ p6, current_normal, uv1 });
	cube.PushVertex({ p1, current_normal, uv2 });
	cube.PushVertex({ p4, current_normal, uv3 });
	cube.PushVertex({ p7, current_normal, uv4 });
	cube.PushIndex(12); cube.PushIndex(13); cube.PushIndex(14);
	cube.PushIndex(12); cube.PushIndex(14); cube.PushIndex(15);
	// Face 5: 4/3/8/7 (positive-y).
	current_normal.x() = 0.0f; current_normal.y() = 1.0f;
	cube.PushVertex({ p4, current_normal, uv1 });
	cube.PushVertex({ p3, current_normal, uv2 });
	cube.PushVertex({ p8, current_normal, uv3 });
	cube.PushVertex({ p7, current_normal, uv4 });
	cube.PushIndex(16); cube.PushIndex(17); cube.PushIndex(18);
	cube.PushIndex(16); cube.PushIndex(18); cube.PushIndex(19);

	// Face 6: 2/1/6/5 (negative-y).
	current_normal.y() = -current_normal.y();
	cube.PushVertex({ p2, current_normal, uv1 });
	cube.PushVertex({ p1, current_normal, uv2 });
	cube.PushVertex({ p6, current_normal, uv3 });
	cube.PushVertex({ p5, current_normal, uv4 });
	cube.PushIndex(20); cube.PushIndex(21); cube.PushIndex(22);
	cube.PushIndex(20); cube.PushIndex(22); cube.PushIndex(23);	

	cube.SetHasNormals();
	cube.SetHasTexcoords();
}

size_t hash(const Vertex& v)
{
	size_t seed = 0;
	seed ^= ers::hash(v.position.x()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= ers::hash(v.position.y()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= ers::hash(v.position.z()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= ers::hash(v.normal.x()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= ers::hash(v.normal.y()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= ers::hash(v.normal.y()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= ers::hash(v.tex_coords.x()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	seed ^= ers::hash(v.tex_coords.y()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

	return seed;
}

void load_object_file(const char* filename, Mesh& model)
{
	FILE* f = fopen(filename, "r");
	ERS_PANICF(f != nullptr, "load_object_file: %s", filename);

	// 1st pass: scan coordinates.
	ers::Vector<ers::vec3> geom;
	ers::Vector<ers::vec3> norm;
	ers::Vector<ers::vec2> tex;
	while (!feof(f))
	{
		char ch = fgetc(f);
		if (ch == 'v')
		{
			ch = fgetc(f);
			if (ch == 't')
			{
				ers::vec2 v;
				fscanf(f, " %f %f", &(v.x()), &(v.y()));
				tex.PushBack(v);
			}
			else if (ch == 'n')
			{
				//ERS_HERE();
				ers::vec3 v;
				fscanf(f, " %f %f %f", &(v.x()), &(v.y()), &(v.z()));
				norm.PushBack(v);
			}
			else
			{
				ers::vec3 v;
				fscanf(f, "%f %f %f", &(v.x()), &(v.y()), &(v.z()));
				geom.PushBack(v);
			}
		}
		else
		{
			while (ch != '\n' && ch != EOF)
				ch = fgetc(f);
		}
	}

	int rc = fseek(f, 0, SEEK_SET);
	if (rc != 0)
	{
		fclose(f);
		exit(EXIT_FAILURE);
	}

	// 2nd pass: scan indices. Assumes indices for all 3 types of coordinates
	// (position, normal, texture coordinates) exist.
	ers::Vector<Vertex> verts;
	ers::Vector<s32> pos_idx;
	ers::Vector<s32> norm_idx;
	ers::Vector<s32> tex_idx;

	while (!feof(f))
	{
		char ch = fgetc(f);
		if (ch == 'f')
		{
			s32 pa, pb, pc;
			s32 na, nb, nc;
			s32 ta, tb, tc;

			fscanf(
				f, 
				" %d/%d/%d %d/%d/%d %d/%d/%d", 
				&pa, &ta, &na,
				&pb, &tb, &nb,
				&pc, &tc, &nc);

			pos_idx.PushBack(pa - 1); pos_idx.PushBack(pb - 1); pos_idx.PushBack(pc - 1);
			tex_idx.PushBack(ta - 1); tex_idx.PushBack(tb - 1); tex_idx.PushBack(tc - 1);
			norm_idx.PushBack(na - 1); norm_idx.PushBack(nb - 1); norm_idx.PushBack(nc - 1);			
		}
		else
		{
			while (ch != '\n' && ch != EOF)
				ch = fgetc(f);
		}
	}

	fclose(f);

	// Create vertex structs by using the correct indices.
	// Only assumption is that each face has 3 indices per type.
	// It accomodates negative indices and absent texture/normal indices.
	// Any more and it won't work correctly.
	// Note: This is not necessary here, since we assume they all exist,
	// but it breaks if I mess with it, so it stays atm.
	if (norm.GetSize() > 0) model.SetHasNormals();
	if (tex.GetSize() > 0) model.SetHasTexcoords();

	size_t vert_cap = (pos_idx.GetSize() > norm_idx.GetSize()) ? pos_idx.GetSize() : norm_idx.GetSize();
	vert_cap = (vert_cap > tex_idx.GetSize()) ? vert_cap : tex_idx.GetSize();

	verts.Reserve(vert_cap);
	for (size_t i = 0; i < vert_cap; i++)
	{
		Vertex ver;
		if (geom.GetSize() > 0 && pos_idx.GetSize() > 0)
		{
			if (pos_idx[i] < 0)
				pos_idx[i] = static_cast<s32>(vert_cap) - pos_idx[i];
			ver.position = geom[pos_idx[i]];
		}

		if (norm.GetSize() > 0)
		{
			if (norm_idx.GetSize() > 0)
			{
				if (norm_idx[i] < 0)
					norm_idx[i] = static_cast<s32>(vert_cap) - norm_idx[i];
				ver.normal = norm[norm_idx[i]];
			}
			else
			{
				ver.normal = norm[pos_idx[i]];
			}
		}

		if (tex.GetSize() > 0)
		{
			if (tex_idx.GetSize() > 0)
			{
				if (tex_idx[i] < 0)
					tex_idx[i] = static_cast<s32>(vert_cap) - tex_idx[i];
				ver.tex_coords = tex[tex_idx[i]];
			}
			else
			{
				ver.tex_coords = tex[pos_idx[i]];
			}
		}

		verts.PushBack(ver);
	}

	// Find repeated vertices to construct an element buffer.		
	ers::HashMap<Vertex, s32> m(verts.GetSize());
	for (s32 i = 0; i < (s32)vert_cap; i++)
	{
		auto it = m.Find(verts[i]);
		if (it.second == true)
		{
			model.PushIndex(it.first->second);
		}
		else
		{
			model.PushVertex(verts[i]);

			s32 last_vert = (s32)(model.GetVertexCount() - 1);
			m.Insert({ verts[i], last_vert });
			model.PushIndex(last_vert);
		}
	}	
}