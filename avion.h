#ifndef AVION_H
#define AVION_H

/*
 * Matrices 4x4 stockees en column-major (convention OpenGL) :
 *
 *   index memoire :  0  4  8  12
 *                    1  5  9  13
 *                    2  6 10  14
 *                    3  7 11  15
 *
 * Matrice de translation T :
 *   [ 1  0  0  tx ]      en memoire : t[12]=tx, t[13]=ty, t[14]=tz
 *   [ 0  1  0  ty ]
 *   [ 0  0  1  tz ]
 *   [ 0  0  0   1 ]
 *
 * Matrice de rotation R (Ry) :
 *   [ cos  0  sin  0 ]
 *   [   0  1    0  0 ]
 *   [-sin  0  cos  0 ]
 *   [   0  0    0  1 ]
 *
 * La transformation finale est M = T * R :
 * on applique d'abord la rotation, puis la translation.
 */
typedef struct {
    float trans[16];      /* matrice de translation 4x4 (column-major) */
    float rot[16];        /* matrice de rotation    4x4 (column-major) */
    float vitesse;        /* vitesse d'avancement  (unites/seconde)    */

    /* commandes : -1.0 a +1.0 */
    float monter;         /* Z : monte,  S : descend                   */
    float virer;          /* Q : gauche, D : droite                    */

    /*
     * Angles visuels (en degres) : purement cosmetiques, ils n'affectent
     * pas la physique ni la detection de collision.
     * Mis a jour par avion_update() via un lerp, appliques dans avion_draw().
     *
     *   tangage_visuel > 0  => nez pointe vers le haut
     *   tangage_visuel < 0  => nez pointe vers le bas
     *   roulis_visuel  > 0  => aile gauche en haut (virage a droite)
     *   roulis_visuel  < 0  => aile droite en haut (virage a gauche)
     */
    float tangage_visuel; /* inclinaison nez haut/bas, en degres       */
    float roulis_visuel;  /* inclinaison ailes gauche/droite, en degres*/
} Avion;

/* Initialise l'avion a une position et orientation par defaut */
void avion_init(Avion *a);

/*
 * Place l'avion au point de depart face au premier anneau.
 * Utilise anneaux_get_depart() pour obtenir la position et l'angle.
 * Appelee au lancement et a chaque reset.
 */
void avion_placer_depart(Avion *a);

/* Met a jour les matrices selon les commandes et dt (secondes) */
void avion_update(Avion *a, float dt);

/* Dessine le modele 3D de l'avion */
void avion_draw(const Avion *a);

#endif