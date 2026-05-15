#ifndef JEU_H
#define JEU_H

#include "anneau.h"
#include "avion.h"

/* ------------------------------------------------------------------ */
/* Structure de l'etat du jeu                                          */
/* ------------------------------------------------------------------ */

typedef struct {
    float chrono_secondes;   /* temps ecoule depuis le passage du 1er anneau */
    int   anneau_courant;    /* prochain anneau a franchir (0..NB_ANNEAUX)   */
    int   chrono_actif;      /* 1 si le chrono tourne, 0 avant le 1er anneau */
    int   partie_terminee;   /* 1 si le circuit est complet                  */
    float meilleur_temps;    /* meilleur temps realise, -1.0 si aucun        */
} Jeu;

/* Initialise l'etat du jeu */
void jeu_init(Jeu *j);

/*
 * Remet le jeu a zero : chrono, anneau courant, et teleporte l'avion
 * au point de depart face au premier anneau.
 */
void jeu_reset(Jeu *j, Avion *avion);

/*
 * Met a jour le chronometre, teste le passage de l'avion dans
 * l'anneau courant, detecte la collision avec le tube du tore
 * et la collision avec le sol.
 * En cas de collision, appelle jeu_reset.
 */
void jeu_update(Jeu *j, Avion *avion,
                const Anneau anneaux[NB_ANNEAUX], float dt);

/*
 * Dessine le HUD : chronometre centre en haut de la fenetre,
 * et ecran de fin si la partie est terminee.
 * fenetre_w et fenetre_h sont les dimensions courantes de la fenetre.
 */
void jeu_draw_hud(const Jeu *j, int fenetre_w, int fenetre_h);

#endif