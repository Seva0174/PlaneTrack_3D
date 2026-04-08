#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "anneau.h"

#define PI 3.14159265358979f

/* ------------------------------------------------------------------ */
/* Description du circuit en forme de huit                            */
/*                                                                     */
/* Le parcours se compose de deux boucles elliptiques qui se          */
/* croisent au centre (origine).                                       */
/*                                                                     */
/*   Boucle gauche  (anneaux 0 a 5)  : centre a (-60, 0)             */
/*   Boucle droite  (anneaux 6 a 11) : centre a (+60, 0)             */
/*                                                                     */
/* Le croisement se fait autour de z=0, x proche de 0.               */
/*                                                                     */
/* Pour chaque anneau, angle_y est la tangente locale a la            */
/* trajectoire, de sorte que le portail soit toujours perpendiculaire */
/* a la direction de vol attendue.                                     */
/*                                                                     */
/* Convention : angle_y est l'angle de la direction de VOL (vers ou   */
/* l'avion doit aller). Le tore est ensuite oriente perpendiculairement*/
/* a cette direction dans anneaux_draw.                               */
/* ------------------------------------------------------------------ */

/* Centre et demi-axes des deux boucles (plan XZ) */
#define BOUCLE_CX_G   (-60.0f)
#define BOUCLE_CZ_G     (0.0f)
#define BOUCLE_RX_G    (55.0f)
#define BOUCLE_RZ_G    (45.0f)

#define BOUCLE_CX_D    (60.0f)
#define BOUCLE_CZ_D     (0.0f)
#define BOUCLE_RX_D    (55.0f)
#define BOUCLE_RZ_D    (45.0f)

/* Distance de depart de l'avion avant le premier anneau */
#define DEPART_DIST    60.0f

/* ------------------------------------------------------------------ */
/* Utilitaire : point et tangente sur une ellipse parametrique        */
/*                                                                     */
/*   x(t) = cx + rx * cos(t)                                          */
/*   z(t) = cz + rz * sin(t)                                          */
/*                                                                     */
/* La tangente (direction de vol) est :                               */
/*   dx/dt = -rx * sin(t)                                             */
/*   dz/dt =  rz * cos(t)                                             */
/*                                                                     */
/* angle_y = atan2(dx, dz)  (angle dans le plan XZ, convention OpenGL)*/
/* ------------------------------------------------------------------ */

static void point_ellipse(float cx, float cz, float rx, float rz,
                          float t,
                          float *x, float *z, float *angle_y)
{
    float dx = -rx * sinf(t);
    float dz =  rz * cosf(t);
    *x       = cx + rx * cosf(t);
    *z       = cz + rz * sinf(t);
    *angle_y = atan2f(dx, dz);
}

/* ------------------------------------------------------------------ */
/* anneaux_init                                                        */
/*                                                                     */
/* Circuit en huit : 6 anneaux sur la boucle gauche (sens horaire     */
/* vu du dessus, t de PI/2 a -PI/2) puis 6 anneaux sur la boucle     */
/* droite (sens antihoraire, t de -PI/2 a PI/2).                      */
/*                                                                     */
/* Les hauteurs varient legerement pour donner du relief.             */
/* ------------------------------------------------------------------ */

void anneaux_init(Anneau anneaux[NB_ANNEAUX]){
    float x, z, ay;
    float t;
    /* Hauteurs des 12 anneaux */
    float hauteurs[NB_ANNEAUX] = {
        10.0f, 12.0f, 14.0f, 12.0f, 10.0f,  8.0f,
        10.0f, 12.0f, 14.0f, 12.0f, 10.0f,  8.0f
    };

    /* --- Boucle gauche : 6 anneaux, t parcourt de PI/2 vers 3*PI/2
     *     (sens horaire vu du dessus, c'est-a-dire x decroit d'abord) */
    int i;
    for (i = 0; i < 6; i++){
        t = (PI / 2.0f) + (float)i * (2.0f * PI / 6.0f);
        point_ellipse(BOUCLE_CX_G, BOUCLE_CZ_G,
                      BOUCLE_RX_G, BOUCLE_RZ_G,
                      t, &x, &z, &ay);
        anneaux[i].x       = x;
        anneaux[i].y       = hauteurs[i];
        anneaux[i].z       = z;
        anneaux[i].angle_y = ay;
    }

    /* --- Boucle droite : 6 anneaux, t parcourt de -PI/2 vers PI/2
     *     (sens antihoraire vu du dessus, x croit d'abord) */
    for (i = 0; i < 6; i++){
        t = -(PI / 2.0f) + (float)i * (2.0f * PI / 6.0f);
        point_ellipse(BOUCLE_CX_D, BOUCLE_CZ_D,
                      BOUCLE_RX_D, BOUCLE_RZ_D,
                      t, &x, &z, &ay);
        anneaux[6 + i].x       = x;
        anneaux[6 + i].y       = hauteurs[6 + i];
        anneaux[6 + i].z       = z;
        anneaux[6 + i].angle_y = ay;
    }
}

/* ------------------------------------------------------------------ */
/* anneaux_get_depart                                                  */
/*                                                                     */
/* L'avion demarre a DEPART_DIST unites derriere le premier anneau,  */
/* oriente exactement dans la direction du trou (angle_y[0]).         */
/* ------------------------------------------------------------------ */

void anneaux_get_depart(float *start_x, float *start_z,
                        float *start_angle_y)
{
    Anneau anneaux[NB_ANNEAUX];
    anneaux_init(anneaux);

    /* Direction inverse de la direction de vol du 1er anneau */
    float ay = anneaux[0].angle_y;
    *start_x       = anneaux[0].x - sinf(ay) * DEPART_DIST;
    *start_z       = anneaux[0].z - cosf(ay) * DEPART_DIST;
    *start_angle_y = ay;
}

/* ------------------------------------------------------------------ */
/* anneaux_draw                                                        */
/*                                                                     */
/* Dessine tous les anneaux.                                           */
/* Couleurs : vert (0), dore (1..NB_ANNEAUX-2), rouge (NB_ANNEAUX-1) */
/*                                                                     */
/* Orientation du tore :                                               */
/*   glutSolidTorus genere le tore dans le plan XY (axe Z).           */
/*   On applique d'abord angle_y (rotation autour de Y) pour pointer  */
/*   le tore dans la direction de vol, puis une rotation de 90 deg    */
/*   autour de Y pour que le plan du tore soit perpendiculaire a la   */
/*   direction de vol : le trou fait alors face a l'avion.            */
/* ------------------------------------------------------------------ */

void anneaux_draw(const Anneau anneaux[NB_ANNEAUX]){
    int i;

    for (i = 0; i < NB_ANNEAUX; i++){
        const Anneau *a = &anneaux[i];

        if (i == 0){
            glColor3f(0.10f, 0.80f, 0.15f);   /* vert  : depart  */
        } else if (i == NB_ANNEAUX - 1){
            glColor3f(0.90f, 0.10f, 0.10f);   /* rouge : arrivee */
        } else {
            glColor3f(1.00f, 0.78f, 0.10f);   /* dore  : milieu  */
        }

        glPushMatrix();

        /* Translation au centre de l'anneau */
        glTranslatef(a->x, a->y, a->z);

        /* Orientation du tore :
         *
         * glutSolidTorus genere un tore dans le plan XY avec l'axe Z.
         * Cela signifie que le trou regarde deja selon Z : un avion
         * volant selon +Z peut traverser sans aucune rotation.
         *
         * Le tore est donc DEJA vertical et traversable.
         * Il suffit d'une rotation Ry(angle_y) pour l'orienter face
         * a la direction de vol tangente au circuit.
         *
         * IMPORTANT : ne pas ajouter de Rx(90) car cela coucherait
         * le tore a plat dans le plan XZ (axe Y = vertical) et
         * l'avion ne pourrait plus y rentrer en volant.
         */
        glRotatef(a->angle_y * (180.0f / PI), 0.0f, 1.0f, 0.0f);

        glutSolidTorus(ANNEAU_RAYON_TUBE, ANNEAU_RAYON_TORE,
                       ANNEAU_STACKS, ANNEAU_SLICES);

        glPopMatrix();
    }
}