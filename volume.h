#include "k-arbre.h"
#include <math.h>
#include <OpenGL/gl.h>
#include "GLUT/glut.h"

#define PROF_MAX 6
#define N 5

karbre boule2arbre(int cx_boule, int cy_boule, int cz_boule, int r);
karbre intersection(karbre V1, karbre V2);
void affiche_volume_bis(karbre v, int x1, int y1, int z1, int x2, int y2, int z2);
void affiche_volume(karbre v);
void affiche_cube(int x1, int y1, int z1, int x2, int y2, int z2);