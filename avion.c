#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include "avion.h"
#include "anneau.h"

#define VITESSE_BASE      20.0f
#define VITESSE_MONTEE    10.0f
#define VITESSE_VIRAGE     1.8f
#define PI              3.14159265358979f

/*
 * Angles cibles (en degres) atteints quand la commande est a fond.
 * Le lerp interpole entre 0 et ces valeurs selon monter/virer.
 */
#define TANGAGE_MAX    25.0f   /* nez en haut quand monter =  1 */
#define ROULIS_MAX     35.0f   /* aile gauche en haut quand virer = 1 */

/*
 * Vitesse de lissage du lerp (fraction de l'ecart corrige par seconde).
 * Plus la valeur est grande, plus la transition est rapide.
 *   8.0 => l'avion atteint ~99% de l'angle cible en ~0.6 s
 */
#define LERP_VITESSE    8.0f

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

    a->tangage_visuel = 0.0f;
    a->roulis_visuel  = 0.0f;

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

    /* Remet les angles visuels a zero pour ne pas heriter de la frame precedente */
    a->tangage_visuel = 0.0f;
    a->roulis_visuel  = 0.0f;
}

/* ------------------------------------------------------------------ */
/* avion_update                                                        */
/* ------------------------------------------------------------------ */

void avion_update(Avion *a, float dt){
    float delta[16];
    float tmp[16];

    /* --- Physique (inchangee) --- */

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

    /* --- Angles visuels : lerp vers la cible ---
     *
     * L'angle cible est proportionnel a la commande (entre -1 et +1).
     * Le facteur de lissage est : k = 1 - exp(-LERP_VITESSE * dt)
     * ce qui garantit une convergence independante du framerate.
     *
     * Tangage :
     *   monter =  1 => cible =  TANGAGE_MAX (nez en haut)
     *   monter = -1 => cible = -TANGAGE_MAX (nez en bas)
     *   monter =  0 => cible =  0 (retour au neutre)
     *
     * Roulis :
     *   virer =  1 (droite) => cible = -ROULIS_MAX (aile droite baisse)
     *   virer = -1 (gauche) => cible =  ROULIS_MAX (aile gauche baisse)
     *   virer =  0          => cible =  0
     */
    {
        float k = 1.0f - expf(-LERP_VITESSE * dt);

        float tangage_cible = a->monter *  TANGAGE_MAX;
        float roulis_cible  = a->virer  * -ROULIS_MAX;

        a->tangage_visuel += (tangage_cible - a->tangage_visuel) * k;
        a->roulis_visuel  += (roulis_cible  - a->roulis_visuel)  * k;
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

    /* Orientation du modele : le fuselage est dessine le long de X,
     * on le fait pointer dans la direction +Z (sens de marche) */
    glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);

    /* --- Inclinaisons visuelles ---
     *
     * On applique d'abord le tangage (rotation autour de Z local,
     * qui correspond a l'axe lateral de l'avion apres le -90 ci-dessus),
     * puis le roulis (rotation autour de X local, axe longitudinal).
     *
     * L'ordre assure que les deux inclinaisons se combinent naturellement :
     * en virage monte, l'avion pique et s'incline simultanement.
     */
    glRotatef(a->tangage_visuel, 0.0f, 0.0f, 1.0f);  /* tangage : axe lateral  */
    glRotatef(a->roulis_visuel,  1.0f, 0.0f, 0.0f);  /* roulis  : axe longitudinal */

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