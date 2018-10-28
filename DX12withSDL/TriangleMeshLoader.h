#pragma once
namespace GAL
{
    class Mesh;

    class TriangleMeshLoader : public entt::resource_loader<TriangleMeshLoader, Mesh>
    {
    public:
        TriangleMeshLoader();
        ~TriangleMeshLoader();

        std::shared_ptr<Mesh> load(float size, float red, float green, float blue) const;

    };
} //namespace GAL;

