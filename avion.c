#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include "avion.h"

#define VITESSE_BASE      20.0f   /* unites/seconde                  */
#define VITESSE_MONTEE     10.0f   /* unites/seconde                  */
#define VITESSE_VIRAGE     1.8f   /* radians/seconde                 */
#define PI              3.14159265358979


/* ------------------------------------------------------------------ */
/* Utilitaires matriciels internes (matrices 4x4, column-major)       */
/* ------------------------------------------------------------------ */

/*
 * mat4_identite : charge la matrice identite dans m.
 */
void mat4_identite(float m[16]){
    m[ 0]=1.f; m[ 4]=0.f; m[ 8]=0.f; m[12]=0.f;
    m[ 1]=0.f; m[ 5]=1.f; m[ 9]=0.f; m[13]=0.f;
    m[ 2]=0.f; m[ 6]=0.f; m[10]=1.f; m[14]=0.f;
    m[ 3]=0.f; m[ 7]=0.f; m[11]=0.f; m[15]=1.f;
}

/*
 * mat4_mul : produit c = a * b (toutes column-major).
 * c doit etre different de a et b.
 */
void mat4_mul(const float a[16], const float b[16], float c[16]){
    int i, j, k;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++) {
            c[j*4+i] = 0.f;
            for (k = 0; k < 4; k++)
                c[j*4+i] += a[k*4+i] * b[j*4+k];
        }
}

/*
 * mat4_translation : construit la matrice de translation (tx, ty, tz).
 *
 *   [ 1  0  0  tx ]
 *   [ 0  1  0  ty ]
 *   [ 0  0  1  tz ]
 *   [ 0  0  0   1 ]
 *
 * En column-major : indices 12, 13, 14 portent tx, ty, tz.
 */
void mat4_translation(float tx, float ty, float tz, float m[16]){
    mat4_identite(m);
    m[12] = tx;
    m[13] = ty;
    m[14] = tz;
}

/*
 * mat4_rot_y : construit la matrice de rotation d'angle theta autour de Y.
 *
 *   [ cos   0   sin  0 ]
 *   [   0   1     0  0 ]
 *   [-sin   0   cos  0 ]
 *   [   0   0     0  1 ]
 *
 * En column-major :
 *   col 0 : cos, 0, -sin, 0   => m[0], m[1], m[2],  m[3]
 *   col 1 :   0, 1,    0, 0   => m[4], m[5], m[6],  m[7]
 *   col 2 : sin, 0,  cos, 0   => m[8], m[9], m[10], m[11]
 *   col 3 :   0, 0,    0, 1   => m[12]..m[15]
 */
void mat4_rot_y(float theta, float m[16]){
    float c = cosf(theta);
    float s = sinf(theta);
    mat4_identite(m);
    m[0] =  c;
    m[2] = -s;
    m[8] =  s;
    m[10]=  c;
}

/* ------------------------------------------------------------------ */
/* avion_init                                                          */
/* ------------------------------------------------------------------ */

void avion_init(Avion *a){
    a->vitesse = VITESSE_BASE;
    a->monter  = 0.0f;
    a->virer   = 0.0f;

    /* Position initiale */
    mat4_translation(0.0f, 5.0f, 0.0f, a->trans);

    /* Orientation initiale : rotation de PI autour de Y
     * pour que l'avion parte vers -Z */
    mat4_rot_y(PI, a->rot);
}

/* ------------------------------------------------------------------ */
/* avion_update                PI                                        */
/*                                                                     */
/* Le deplacement est entierement matriciel :                          */
/*   - virage  : on compose rot avec une Ry(delta)                    */
/*   - avance  : on extrait la direction depuis rot, on construit     */
/*               une matrice de translation et on compose avec trans  */
/*   - montee  : idem sur l'axe Y monde                               */
/* ------------------------------------------------------------------ */

void avion_update(Avion *a, float dt){
    float delta[16];
    float tmp[16];

    /* --- Virage : composition de rotations --- */
    if (a->virer != 0.0f) {
        mat4_rot_y(-a->virer * VITESSE_VIRAGE * dt, delta);
        mat4_mul(a->rot, delta, tmp);
        int i;
        for (i = 0; i < 16; i++)
            a->rot[i] = tmp[i];
    }

    /* --- Avancement : direction de vol extraite de rot ---
     *
     * En column-major, la troisieme colonne de rot (col 2) est le
     * vecteur Z transforme, soit la direction de vol :
     *   rot[8]  = composante X
     *   rot[9]  = composante Y  (ignoree : modele naif)
     *   rot[10] = composante Z
     *
     * On construit T(dx, 0, dz) et on compose avec trans.
     */
    {
        float dx = a->rot[8]  * a->vitesse * dt;
        float dz = a->rot[10] * a->vitesse * dt;
        mat4_translation(dx, 0.0f, dz, delta);
        mat4_mul(delta, a->trans, tmp);
        int i;
        for (i = 0; i < 16; i++) a->trans[i] = tmp[i];
    }

    /* --- Montee / descente : translation sur Y monde --- */
    if (a->monter != 0.0f) {
        float dy = a->monter * VITESSE_MONTEE * dt;
        mat4_translation(0.0f, dy, 0.0f, delta);
        mat4_mul(delta, a->trans, tmp);
        int i;
        for (i = 0; i < 16; i++) a->trans[i] = tmp[i];
    }
}

/* ------------------------------------------------------------------ */
/* avion_draw                                                          */
/*                                                                     */
/* On calcule M = trans * rot et on la charge dans OpenGL.            */
/* ------------------------------------------------------------------ */

void avion_draw(const Avion *a){
    float m[16];
    mat4_mul(a->trans, a->rot, m);

    glPushMatrix();
    glMultMatrixf(m);

    /* Le fuselage est modelise sur l'axe X ; on tourne de -90 deg
     * autour de Y pour que le nez pointe dans la direction de vol. */
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

    /* --- Fuselage --- */
    glColor3f(0.85f, 0.85f, 0.90f);
    glPushMatrix();
    glScalef(2.4f, 0.4f, 0.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* --- Cabine --- */
    glColor3f(0.5f, 0.75f, 0.95f);
    glPushMatrix();
    glTranslatef(0.4f, 0.28f, 0.0f);
    glScalef(0.7f, 0.28f, 0.32f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* --- Aile gauche --- */
    glColor3f(0.75f, 0.75f, 0.82f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.85f);
    glScalef(0.9f, 0.06f, 1.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* --- Aile droite --- */
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.85f);
    glScalef(0.9f, 0.06f, 1.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* --- Empennage vertical --- */
    glColor3f(0.80f, 0.30f, 0.30f);
    glPushMatrix();
    glTranslatef(-1.0f, 0.32f, 0.0f);
    glScalef(0.5f, 0.6f, 0.06f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* --- Empennage horizontal gauche --- */
    glColor3f(0.75f, 0.75f, 0.82f);
    glPushMatrix();
    glTranslatef(-1.0f, 0.0f, -0.45f);
    glScalef(0.5f, 0.05f, 0.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* --- Empennage horizontal droit --- */
    glPushMatrix();
    glTranslatef(-1.0f, 0.0f, 0.45f);
    glScalef(0.5f, 0.05f, 0.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
}