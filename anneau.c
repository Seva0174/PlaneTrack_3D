#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "anneau.h"

#define PI 3.14159265358979f

/* ------------------------------------------------------------------ */
/* Circuit circulaire                                                  */
/*                                                                     */
/* Les NB_ANNEAUX anneaux sont repartis uniformement sur un cercle    */
/* de rayon CIRCUIT_RAYON centre a l'origine.                         */
/*                                                                     */
/* Pour chaque anneau, angle_y est la tangente locale au cercle,      */
/* de sorte que le portail soit perpendiculaire a la direction de vol. */
/* ------------------------------------------------------------------ */

#define CIRCUIT_RAYON  80.0f
#define DEPART_DIST    20.0f


// creer le circuit  d'anneau en forme d'un cercle
void anneaux_init(Anneau anneaux[NB_ANNEAUX]) {
    float hauteurs[NB_ANNEAUX] = {8.0f, 10.0f, 13.0f, 15.0f, 13.0f, 10.0f,8.0f, 10.0f, 13.0f, 15.0f, 13.0f, 10.0f};
    for (int i = 0; i < NB_ANNEAUX; i++) {
        // renvoie la bonne partie du cercle trigo decoupe en part egal d'angle 
        float t = (float)i * (2.0f * PI / (float)NB_ANNEAUX);

        anneaux[i].x = CIRCUIT_RAYON * sinf(t);
        anneaux[i].z = CIRCUIT_RAYON * cosf(t);
        anneaux[i].y = hauteurs[i];

        // oriente le trou de l'anneau correctement pour creer un parcours
        anneaux[i].angle_y = atan2f(cosf(t), -sinf(t));
    }
}

/*
 * Retourne la position et l'angle de depart de l'avion pour qu'il
 * soit oriente face au premier anneau avec une distance de securite.
 */
void anneaux_get_depart(float *start_x, float *start_z,float *start_angle_y){
    Anneau anneaux[NB_ANNEAUX];
    anneaux_init(anneaux);

    float ay = anneaux[0].angle_y;
    *start_x       = anneaux[0].x - sinf(ay) * DEPART_DIST;
    *start_z       = anneaux[0].z - cosf(ay) * DEPART_DIST;
    *start_angle_y = ay;
}


void anneaux_draw(const Anneau anneaux[NB_ANNEAUX], int anneau_courant) {
    float temps_s = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float pulse   = 0.5f + 0.5f * sinf(temps_s * 4.0f);

    for (int i = 0; i < NB_ANNEAUX; i++) {
        const Anneau *a = &anneaux[i];
        //choix couleur
        if (i == 0) {
            glColor3f(0.10f, 0.80f, 0.15f);           // vert  :depart
        } else if (i == NB_ANNEAUX - 1) {
            glColor3f(0.90f, 0.10f, 0.10f);           // rouge : arrivee 
        } else if (i == anneau_courant) {
            glColor3f(pulse, 1.0f, 1.0f);             // cyan pulse : cible 
        } else {
            glColor3f(0.70f, 0.55f, 0.07f);           // dore 
        }
        //dessine l'anneau
        glPushMatrix();
        //coordoné ou dessine
        glTranslatef(a->x, a->y, a->z);
        //orientation du dessin ce qui rend le tore debout et la direction des trous du tore
        glRotatef(a->angle_y * (180.0f / PI), 0.0f, 1.0f, 0.0f);
        // dessin du tore
        glutSolidTorus(ANNEAU_RAYON_TUBE, ANNEAU_RAYON_TORE,ANNEAU_STACKS, ANNEAU_SLICES);
        glPopMatrix();
    }
}