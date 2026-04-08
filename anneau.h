#ifndef ANNEAU_H
#define ANNEAU_H

/* ------------------------------------------------------------------ */
/* Structure d'un anneau                                               */
/*                                                                     */
/* Chaque anneau est un tore vertical que l'avion doit traverser.     */
/* Le 1er anneau est vert, le dernier est rouge, les autres dores.    */
/* ------------------------------------------------------------------ */

#define NB_ANNEAUX      12

/* Rayon du tube du tore */
#define ANNEAU_RAYON_TUBE   0.5f
/* Rayon du tore (distance centre tore - centre tube) */
#define ANNEAU_RAYON_TORE   3.0f
/* Tessellation du tore */
#define ANNEAU_SLICES       32
#define ANNEAU_STACKS       16

typedef struct {
    float x;
    float y;       /* hauteur du centre du tore                       */
    float z;
    float angle_y; /* orientation du portail autour de Y, en radians  */
                   /* le trou du tore est face a la direction angle_y  */
} Anneau;

/* Initialise le tableau des anneaux avec un parcours en forme de huit */
void anneaux_init(Anneau anneaux[NB_ANNEAUX]);

/* Dessine tous les anneaux */
void anneaux_draw(const Anneau anneaux[NB_ANNEAUX]);

/*
 * Retourne la position et l'angle de depart de l'avion pour qu'il
 * soit oriente face au premier anneau avec une distance de securite.
 */
void anneaux_get_depart(float *start_x, float *start_z,
                        float *start_angle_y);

#endif