#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include "avion.h"
#include "anneau.h"

#define VITESSE_BASE      20.0f
#define VITESSE_MONTEE    10.0f
#define VITESSE_VIRAGE     1.8f
#define PI              3.14159265358979f

/* ------------------------------------------------------------------ */
/* Utilitaires matriciels internes (matrices 4x4, column-major)       */
/* ------------------------------------------------------------------ */

void mat4_identite(float m[16]){
    m[ 0]=1.f; m[ 4]=0.f; m[ 8]=0.f; m[12]=0.f;
    m[ 1]=0.f; m[ 5]=1.f; m[ 9]=0.f; m[13]=0.f;
    m[ 2]=0.f; m[ 6]=0.f; m[10]=1.f; m[14]=0.f;
    m[ 3]=0.f; m[ 7]=0.f; m[11]=0.f; m[15]=1.f;
}

void mat4_mul(const float a[16], const float b[16], float c[16]){
    int i, j, k;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++) {
            c[j*4+i] = 0.f;
            for (k = 0; k < 4; k++)
                c[j*4+i] += a[k*4+i] * b[j*4+k];
        }
}

void mat4_translation(float tx, float ty, float tz, float m[16]){
    mat4_identite(m);
    m[12] = tx;
    m[13] = ty;
    m[14] = tz;
}

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

    mat4_identite(a->trans);
    mat4_identite(a->rot);
}

/* ------------------------------------------------------------------ */
/* avion_placer_depart                                                 */
/*                                                                     */
/* Place l'avion au point de depart face au premier anneau.           */
/* Factorise le bloc de positionnement present dans main() et         */
/* jeu_reset() pour eviter la duplication de code.                    */
/* ------------------------------------------------------------------ */

void avion_placer_depart(Avion *a){
    float sx, sz, ay;
    float c, s;

    anneaux_get_depart(&sx, &sz, &ay);

    a->trans[12] = sx;
    a->trans[13] = 5.0f;
    a->trans[14] = sz;

    c = cosf(ay);
    s = sinf(ay);

    a->rot[ 0] =  c;
    a->rot[ 2] = -s;
    a->rot[ 4] =  0.0f;
    a->rot[ 5] =  1.0f;
    a->rot[ 6] =  0.0f;
    a->rot[ 8] =  s;
    a->rot[10] =  c;
}

/* ------------------------------------------------------------------ */
/* avion_update                                                        */
/* ------------------------------------------------------------------ */

void avion_update(Avion *a, float dt){
    float delta[16];
    float tmp[16];

    if (a->virer != 0.0f) {
        mat4_rot_y(-a->virer * VITESSE_VIRAGE * dt, delta);
        mat4_mul(a->rot, delta, tmp);
        int i;
        for (i = 0; i < 16; i++)
            a->rot[i] = tmp[i];
    }

    {
        float dx = a->rot[8]  * a->vitesse * dt;
        float dz = a->rot[10] * a->vitesse * dt;
        mat4_translation(dx, 0.0f, dz, delta);
        mat4_mul(delta, a->trans, tmp);
        int i;
        for (i = 0; i < 16; i++) a->trans[i] = tmp[i];
    }

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
/* ------------------------------------------------------------------ */

void avion_draw(const Avion *a){
    float m[16];
    mat4_mul(a->trans, a->rot, m);

    glPushMatrix();
    glMultMatrixf(m);

    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

    /* Fuselage */
    glColor3f(0.85f, 0.85f, 0.90f);
    glPushMatrix();
    glScalef(2.4f, 0.4f, 0.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* Cabine */
    glColor3f(0.5f, 0.75f, 0.95f);
    glPushMatrix();
    glTranslatef(0.4f, 0.28f, 0.0f);
    glScalef(0.7f, 0.28f, 0.32f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* Aile gauche */
    glColor3f(0.75f, 0.75f, 0.82f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.85f);
    glScalef(0.9f, 0.06f, 1.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* Aile droite */
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.85f);
    glScalef(0.9f, 0.06f, 1.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* Empennage vertical */
    glColor3f(0.80f, 0.30f, 0.30f);
    glPushMatrix();
    glTranslatef(-1.0f, 0.32f, 0.0f);
    glScalef(0.5f, 0.6f, 0.06f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* Empennage horizontal gauche */
    glColor3f(0.75f, 0.75f, 0.82f);
    glPushMatrix();
    glTranslatef(-1.0f, 0.0f, -0.45f);
    glScalef(0.5f, 0.05f, 0.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* Empennage horizontal droit */
    glPushMatrix();
    glTranslatef(-1.0f, 0.0f, 0.45f);
    glScalef(0.5f, 0.05f, 0.8f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
}