#include "Mesh.h"
#include "MeshIO.h"
#include <stack>
#include <list>

Mesh::Mesh()
{
    
}

Mesh::Mesh(const Mesh& mesh)
{
    *this = mesh;
}

bool Mesh::read(const std::string& fileName, const int mode)
{
    std::ifstream in(fileName.c_str());

    if (!in.is_open()) {
        std::cerr << "Error: Could not open file for reading" << std::endl;
        return false;
    }
    
    bool readSuccessful = false;
    if ((readSuccessful = MeshIO::read(in, *this, mode))) {
        normalize();
    }
    
    return readSuccessful;
}

bool Mesh::write(const std::string& fileName) const
{
    std::ofstream out(fileName.c_str());
    
    if (!out.is_open()) {
        std::cerr << "Error: Could not open file for writing" << std::endl;
        return false;
    }
    
    MeshIO::write(out, *this);
    
    return false;
}

std::vector<Mesh> Mesh::generateSubmeshesEdgeApproach()
{
    std::vector<Mesh> meshes;
    
    int seen = 0;
    int meshSize = 0;
    while (seen != vertices.size()) {
        Mesh mesh;
        meshes.push_back(mesh);
        
        std::stack<HalfEdgeIter> stack;
        
        VertexIter v;
        for (v = vertices.begin(); v != vertices.end(); v++) {
            if (!v->seen) {
                v->seen = true;
                seen ++;
                break;
            }
        }
        meshes[meshSize].vertices.push_back(*v);
        
        do {
            HalfEdgeIter he = v->he;
            do {
                if (!he->seen) {
                    he->seen = true;
                    stack.push(he);
                    meshes[meshSize].halfEdges.push_back(*he);
                    
                    if (!he->edge->seen) {
                        he->edge->seen = true;
                        meshes[meshSize].edges.push_back(*he->edge);
                    }
                    
                    if (!he->face->seen) {
                        he->face->seen = true;
                        meshes[meshSize].faces.push_back(*he->face);
                    }
                }
                
                he = he->flip->next;
            } while (he != v->he);
            
            v = stack.top()->flip->vertex;
            if (!v->seen) {
                v->seen = true;
                seen ++;
                meshes[meshSize].vertices.push_back(*v);
            }
            
            stack.pop();
        } while (!stack.empty());
        
        meshSize ++;
    }
    
    return meshes;
}

double signedVolume(const Eigen::Vector3d& p1, const Eigen::Vector3d& p2, const Eigen::Vector3d& p3)
{
    double v321 = p3.x() * p2.y() * p1.z();
    double v231 = p2.x() * p3.y() * p1.z();
    double v312 = p3.x() * p1.y() * p2.z();
    double v132 = p1.x() * p3.y() * p2.z();
    double v213 = p2.x() * p1.y() * p3.z();
    double v123 = p1.x() * p2.y() * p3.z();
    
    return (-v321 + v231 + v312 - v132 - v213 + v123) / 6.0;
}

double triArea(const Eigen::Vector3d& p1, const Eigen::Vector3d& p2, const Eigen::Vector3d& p3)
{
    return ((p1 - p2).cross(p3 - p2)).norm() / 2.0;
}

void Mesh::computeFeatures()
{
    for (FaceCIter f = faces.begin(); f != faces.end(); f++) {
        
        HalfEdgeCIter he = f->he;
        VertexIter v1 = he->vertex;
        
        he = he->next;
        VertexIter v2 = he->vertex;
        
        he = he->next;
        VertexIter v3 = he->vertex;
     
        volume += signedVolume(v1->position, v2->position, v3->position);
        surfaceArea += triArea(v1->position, v2->position, v3->position);
    }
    
    volume = fabs(volume);
}

void Mesh::normalize()
{
    // compute center of mass
    Eigen::Vector3d cm = Eigen::Vector3d::Zero();
    for (VertexCIter v = vertices.begin(); v != vertices.end(); v++) {
        cm += v->position;
    }
    cm /= (double)vertices.size();
    
    // translate to origin
    for (VertexIter v = vertices.begin(); v != vertices.end(); v++) {
        v->position -= cm;
    }
    
    // determine radius
    double rMax = 0;
    for (VertexCIter v = vertices.begin(); v != vertices.end(); v++) {
        rMax = std::max(rMax, v->position.norm());
    }
    
    // rescale to unit sphere
    for (VertexIter v = vertices.begin(); v != vertices.end(); v++) {
        v->position /= rMax;
    }
}
