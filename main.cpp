#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "Mesh.h"

int gridX = 600;
int gridY = 600;
int gridZ = 600;

const double fovy = 50.;
const double clipNear = .01;
const double clipFar = 1000.;
double x = 0;
double z = -2.5;
double y = 0;
int idx = 0;

std::string path = "/Users/rohansawhney/Desktop/developer/C++/instances/kitten.obj";
std::vector<Mesh> submeshes;
std::unordered_map<std::string, std::vector<Mesh*>> instances;
bool success = true;

void findInstances()
{
    for (int i = 0; i < (int)submeshes.size(); i++) {
        submeshes[i].computeFeatures();
        std::string vsa = std::to_string(submeshes[i].volume) + " " +
                          std::to_string(submeshes[i].surfaceArea);
        instances[vsa].push_back(&submeshes[i]);
    }
}

void printInstructions()
{
    std::cerr << "' ': reload"
              << "↑/↓: move in/out\n"
              << "w/s: move up/down\n"
              << "a/d: move left/right\n"
              << "→/←: toggle between instances\n"
              << "escape: exit program\n"
              << std::endl;
}

void loadMeshes()
{
    Mesh mesh;
    success = mesh.read(path, 0);
    if (success) {
        submeshes = mesh.generateSubmeshesEdgeApproach();
        findInstances();
    }
}

void init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
}

void drawFaces()
{
    int c = 0;
    for (auto kv : instances) {
        if (c == idx) {
            glColor4f(0, 1, 0, 0.6);
        } else {
            glColor4f(0, 0, 1, 0.6);
        }
        
        for (int i = 0; i < (int)kv.second.size(); i++) {
            for (FaceCIter f = kv.second[i]->faces.begin(); f != kv.second[i]->faces.end(); f++) {
                
                if (f->isBoundary()) continue;
                
                glBegin(GL_LINE_LOOP);
                HalfEdgeCIter he = f->he;
                do {
                    glVertex3d(he->vertex->position.x(), he->vertex->position.y(), he->vertex->position.z());
                    
                    he = he->next;
                    
                } while (he != f->he);
                
                glEnd();
            }
        }
        c ++;
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    double aspect = (double)viewport[2] / (double)viewport[3];
    gluPerspective(fovy, aspect, clipNear, clipFar);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(0, 0, z, x, y, 0, 0, 1, 0);
    
    if (success) {
        drawFaces();
    }
    
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x0, int y0)
{
    switch (key) {
        case 27:
            exit(0);
        case ' ':
            loadMeshes();
            break;
        case 'a':
            x -= 0.03;
            break;
        case 'd':
            x += 0.03;
            break;
        case 'w':
            y += 0.03;
            break;
        case 's':
            y -= 0.03;
            break;        
    }
    
    glutPostRedisplay();
}

void special(int i, int x0, int y0)
{
    switch (i) {
        case GLUT_KEY_UP:
            z += 0.03;
            break;
        case GLUT_KEY_DOWN:
            z -= 0.03;
            break;
        case GLUT_KEY_LEFT:
            idx --;
            if (idx == -1) {
                idx = (int)instances.size()-1;
            }
            break;
        case GLUT_KEY_RIGHT:
            idx ++;
            if (idx == (int)instances.size()) {
                idx = 0;
            }
            break;
    }
    
    glutPostRedisplay();
}

int main(int argc, char** argv) {

    loadMeshes();
    
    printInstructions();
    glutInitWindowSize(gridX, gridY);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInit(&argc, argv);
    glutCreateWindow("Instances");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMainLoop();
    
    return 0;
}
