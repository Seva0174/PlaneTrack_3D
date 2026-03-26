#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include "avion.h"

#define VITESSE_BASE      15.0f   /* unites/seconde */
#define SENSIBILITE_ROT    1.8f   /* radians/seconde par unite de commande */

/* ------------------------------------------------------------------ */
/* Utilitaires vectoriels internes                                     */
/* ------------------------------------------------------------------ */

static void vec_normalize(float *x, float *y, float *z)
{
    float len = sqrtf((*x)*(*x) + (*y)*(*y) + (*z)*(*z));
    if (len < 1e-6f) return;
    *x /= len;
    *y /= len;
    *z /= len;
}

static void vec_cross(float ax, float ay, float az,
                      float bx, float by, float bz,
                      float *rx, float *ry, float *rz)
{
    *rx = ay*bz - az*by;
    *ry = az*bx - ax*bz;
    *rz = ax*by - ay*bx;
}

/* ------------------------------------------------------------------ */
/* avion_init                                                          */
/* ------------------------------------------------------------------ */

void avion_init(Avion *a)
{
    a->x = 0.0f;
    a->y = 5.0f;
    a->z = 0.0f;

    /* L'avion part vers -Z (vers l'interieur de la scene) */
    a->dir_x = 0.0f;
    a->dir_y = 0.0f;
    a->dir_z = -1.0f;

    a->up_x = 0.0f;
    a->up_y = 1.0f;
    a->up_z = 0.0f;

    a->vitesse  = VITESSE_BASE;
    a->tangage  = 0.0f;
    a->roulis   = 0.0f;
}

/* ------------------------------------------------------------------ */
/* avion_update                                                        */
/* ------------------------------------------------------------------ */

void avion_update(Avion *a, float dt)
{
    float right_x, right_y, right_z;

    /* Vecteur droite = dir x up */
    vec_cross(a->dir_x, a->dir_y, a->dir_z,
              a->up_x,  a->up_y,  a->up_z,
              &right_x, &right_y, &right_z);
    vec_normalize(&right_x, &right_y, &right_z);

    /* --- Roulis : rotation autour de dir --- */
    if (a->roulis != 0.0f) {
        float angle = a->roulis * SENSIBILITE_ROT * dt;
        float c = cosf(angle), s = sinf(angle);

        float nx = c*a->up_x + s*right_x;
        float ny = c*a->up_y + s*right_y;
        float nz = c*a->up_z + s*right_z;
        a->up_x = nx;
        a->up_y = ny;
        a->up_z = nz;
        vec_normalize(&a->up_x, &a->up_y, &a->up_z);

        /* Recalcul du vecteur droite apres roulis */
        vec_cross(a->dir_x, a->dir_y, a->dir_z,
                  a->up_x,  a->up_y,  a->up_z,
                  &right_x, &right_y, &right_z);
        vec_normalize(&right_x, &right_y, &right_z);
    }

    /* --- Tangage : rotation autour du vecteur droite --- */
    if (a->tangage != 0.0f) {
        float angle = a->tangage * SENSIBILITE_ROT * dt;
        float c = cosf(angle), s = sinf(angle);

        float ndx = c*a->dir_x + s*a->up_x;
        float ndy = c*a->dir_y + s*a->up_y;
        float ndz = c*a->dir_z + s*a->up_z;
        a->dir_x = ndx;
        a->dir_y = ndy;
        a->dir_z = ndz;
        vec_normalize(&a->dir_x, &a->dir_y, &a->dir_z);

        float nux = c*a->up_x - s*a->dir_x;
        float nuy = c*a->up_y - s*a->dir_y;
        float nuz = c*a->up_z - s*a->dir_z;
        a->up_x = nux;
        a->up_y = nuy;
        a->up_z = nuz;
        vec_normalize(&a->up_x, &a->up_y, &a->up_z);
    }

    /* --- Avancement --- */
    a->x += a->dir_x * a->vitesse * dt;
    a->y += a->dir_y * a->vitesse * dt;
    a->z += a->dir_z * a->vitesse * dt;
}

/* ------------------------------------------------------------------ */
/* avion_draw                                                          */
/*                                                                     */
/* Modele minimaliste : fuselage + ailes + empennage                  */
/* Tout est dessine dans le repere local de l'avion,                  */
/* on applique juste une translation + orientation via la matrice.    */
/* ------------------------------------------------------------------ */

void avion_draw(const Avion *a)
{
    /* Vecteur droite = dir x up */
    float rx = a->dir_y * a->up_z - a->dir_z * a->up_y;
    float ry = a->dir_z * a->up_x - a->dir_x * a->up_z;
    float rz = a->dir_x * a->up_y - a->dir_y * a->up_x;

    /*
     * Matrice de rotation column-major OpenGL :
     * [right | up | -dir | 0]
     * On negat dir car OpenGL regarde vers -Z par defaut.
     */
    float m[16] = {
         rx,          ry,          rz,          0.0f,
         a->up_x,     a->up_y,     a->up_z,     0.0f,
        -a->dir_x,   -a->dir_y,   -a->dir_z,    0.0f,
         a->x,        a->y,        a->z,         1.0f
    };

    glPushMatrix();
    glMultMatrixf(m);

    /* Le fuselage est modelise sur l'axe X, mais la direction de vol
     * est -Z dans le repere local. On tourne de 90 deg autour de Y
     * pour que le nez pointe vers l'avant. */
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

    /* --- Fuselage (corps allonge sur X) --- */
    glColor3f(0.85f, 0.85f, 0.90f);
    glPushMatrix();
    glScalef(2.4f, 0.4f, 0.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /* --- Cabine (bosse sur le dessus, vers l'avant) --- */
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