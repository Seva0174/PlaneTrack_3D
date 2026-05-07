#ifndef JEU_H
#define JEU_H

#include "anneau.h"
#include "avion.h"

/* ------------------------------------------------------------------ */
/* Structure de l'etat du jeu                                          */
/* ------------------------------------------------------------------ */

typedef struct {
    float chrono_secondes;   /* temps ecoule depuis le debut de la partie */
    int   anneau_courant;    /* prochain anneau a franchir (0..NB_ANNEAUX) */
} Jeu;

/* Initialise l'etat du jeu */
void jeu_init(Jeu *j);

/*
 * Met a jour le chronometre et teste le passage de l'avion dans
 * l'anneau courant.
 * Affiche dans le terminal chaque validation.
 */
void jeu_update(Jeu *j, const Avion *avion,const Anneau anneaux[NB_ANNEAUX], float dt);

/*
 * Dessine le HUD : chronometre centre en haut de la fenetre.
 * fenetre_w et fenetre_h sont les dimensions courantes de la fenetre.
 */
void jeu_draw_hud(const Jeu *j, int fenetre_w, int fenetre_h);

#endif