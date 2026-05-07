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

/* ------------------------------------------------------------------ */
/* anneaux_init                                                        */
/* ------------------------------------------------------------------ */

void anneaux_init(Anneau anneaux[NB_ANNEAUX]) {
    float hauteurs[NB_ANNEAUX] = {8.0f, 10.0f, 13.0f, 15.0f, 13.0f, 10.0f,8.0f, 10.0f, 13.0f, 15.0f, 13.0f, 10.0f};
    int i;
    for (i = 0; i < NB_ANNEAUX; i++) {
        /* Angle de position sur le cercle (sens antihoraire vu du dessus) */
        float t = (float)i * (2.0f * PI / (float)NB_ANNEAUX);

        anneaux[i].x = CIRCUIT_RAYON * sinf(t);
        anneaux[i].z = CIRCUIT_RAYON * cosf(t);
        anneaux[i].y = hauteurs[i];

        /* La tangente au cercle en t est perpendiculaire au rayon :
         *   direction de vol = ( cos(t), 0, -sin(t) )
         *   angle_y = atan2(cos(t), -sin(t))                          */
        anneaux[i].angle_y = atan2f(cosf(t), -sinf(t));
    }
}

/* ------------------------------------------------------------------ */
/* anneaux_get_depart                                                  */
/* ------------------------------------------------------------------ */

void anneaux_get_depart(float *start_x, float *start_z,float *start_angle_y){
    Anneau anneaux[NB_ANNEAUX];
    anneaux_init(anneaux);

    float ay = anneaux[0].angle_y;
    *start_x       = anneaux[0].x - sinf(ay) * DEPART_DIST;
    *start_z       = anneaux[0].z - cosf(ay) * DEPART_DIST;
    *start_angle_y = ay;
}

/* ------------------------------------------------------------------ */
/* anneaux_draw                                                        */
/* ------------------------------------------------------------------ */

void anneaux_draw(const Anneau anneaux[NB_ANNEAUX], int anneau_courant) {
    int i;

    float temps_s = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float pulse   = 0.5f + 0.5f * sinf(temps_s * 4.0f);

    for (i = 0; i < NB_ANNEAUX; i++) {
        const Anneau *a = &anneaux[i];

        if (i == 0) {
            glColor3f(0.10f, 0.80f, 0.15f);           /* vert  : depart  */
        } else if (i == NB_ANNEAUX - 1) {
            glColor3f(0.90f, 0.10f, 0.10f);           /* rouge : arrivee */
        } else if (i == anneau_courant) {
            glColor3f(pulse, 1.0f, 1.0f);             /* cyan pulse : cible */
        } else {
            glColor3f(0.70f, 0.55f, 0.07f);           /* dore attenué    */
        }

        glPushMatrix();
        glTranslatef(a->x, a->y, a->z);
        glRotatef(a->angle_y * (180.0f / PI), 0.0f, 1.0f, 0.0f);
        glutSolidTorus(ANNEAU_RAYON_TUBE, ANNEAU_RAYON_TORE,
                       ANNEAU_STACKS, ANNEAU_SLICES);
        glPopMatrix();
    }
}