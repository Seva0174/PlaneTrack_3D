#ifndef AVION_H
#define AVION_H

typedef struct {
    float x, y, z;           /* position dans le monde */

    float dir_x, dir_y, dir_z;   /* vecteur direction (normalise) */
    float up_x,  up_y,  up_z;    /* vecteur up (roulis) */

    float vitesse;            /* vitesse d'avancement */
    float tangage;            /* commande tangage  : -1.0 a +1.0 */
    float roulis;             /* commande roulis   : -1.0 a +1.0 */
} Avion;

/* Initialise l'avion a une position et orientation par defaut */
void avion_init(Avion *a);

/* Met a jour la position et l'orientation selon les commandes et dt (secondes) */
void avion_update(Avion *a, float dt);

/* Dessine le modele 3D de l'avion */
void avion_draw(const Avion *a);

#endif