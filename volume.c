#include "volume.h"

// prend en param la boule et le point en question
int point_dans_boule(int cx_boule, int cy_boule, int cz_boule, int r, int x, int y, int z)
{
    int dx = cx_boule - x;
    int dy = cy_boule - y;
    int dz = cz_boule - z;
    // distance entre le centre de la boule et le point
    int distance;
    distance = dx * dx + dy * dy + dz * dz;

    return distance <= r * r;
}

// prend en param la boule et le cube
// 1 si intersection
// 0 sinon
int intersection_boule_cube(int cx_boule, int cy_boule, int cz_boule, int r, int x1, int y1, int z1, int x2, int y2, int z2)
{
    int cx_cube = (x1 + x2) / 2;
    int cy_cube = (y1 + y2) / 2;
    int cz_cube = (z1 + z2) / 2;
    int dx = cx_boule - cx_cube;
    int dy = cy_boule - cy_cube;
    int dz = cz_boule - cz_cube;
    int distance_cube_boule;
    int rayon_total;
    // On aproxime le cube par une boule,
    // on calcule la distance entre le centre de la boule
    // et le centre du cube aproximé par une boule
    rayon_total = r + (sqrt(3) * (x2 - x1) / 2);
    distance_cube_boule = (dx * dx) + (dy * dy) + (dz * dz);

    return distance_cube_boule <= rayon_total * rayon_total;
}

// prend en param la boule et le cube
int cube_dans_boule(int cx_boule, int cy_boule, int cz_boule, int r, int x1, int y1, int z1, int x2, int y2, int z2)
{
    // pour chaque sommet du cube verifier qu'il soit dans la boule
    // pour chaque sommet calculer sa distance avec le centre de la boule
    // si cette distance < rayon de la boule alors le sommet est dans la boule

    return (point_dans_boule(cx_boule, cy_boule, cz_boule, r, x1, y1, z1)    // 1er
            && point_dans_boule(cx_boule, cy_boule, cz_boule, r, x2, y1, z1) // 2ème
            && point_dans_boule(cx_boule, cy_boule, cz_boule, r, x2, y2, z1) // 3ème
            && point_dans_boule(cx_boule, cy_boule, cz_boule, r, x1, y2, z1) // 4ème

            && point_dans_boule(cx_boule, cy_boule, cz_boule, r, x1, y1, z2) // 5ème
            && point_dans_boule(cx_boule, cy_boule, cz_boule, r, x2, y1, z2) // 6ème
            && point_dans_boule(cx_boule, cy_boule, cz_boule, r, x2, y2, z2) // 7ème
            && point_dans_boule(cx_boule, cy_boule, cz_boule, r, x1, y2, z2) // 8ème
    );
}

karbre boule2arbre_bis(int cx_boule, int cy_boule, int cz_boule, int r, int x1, int y1, int z1, int x2, int y2, int z2, int prof)
{

    if (!intersection_boule_cube(cx_boule, cy_boule, cz_boule, r, x1, y1, z1, x2, y2, z2))
    {
        // les deux boules (boule et cube aproximé par une boule) s'intersecte pas
        return kConsArbre(VIDE,
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide());
    }

    if (cube_dans_boule(cx_boule, cy_boule, cz_boule, r, x1, y1, z1, x2, y2, z2))
    {
        return kConsArbre(PLEIN,
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide());
    }

    if (prof == PROF_MAX ||
        (x2 - x1 <= 1 && y2 - y1 <= 1 && z2 - z1 <= 1))
    {
        return kConsArbre(VIDE,
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide());
    }

    return kConsArbre(SUBDIV, boule2arbre_bis(cx_boule, cy_boule, cz_boule, r, x1, (y1 + y2) / 2, z1, (x1 + x2) / 2, y2, (z1 + z2) / 2, prof + 1),
                      boule2arbre_bis(cx_boule, cy_boule, cz_boule, r, (x1 + x2) / 2, (y1 + y2) / 2, z1, x2, y2, (z1 + z2) / 2, prof + 1),
                      boule2arbre_bis(cx_boule, cy_boule, cz_boule, r, (x1 + x2) / 2, y1, z1, x2, (y1 + y2) / 2, (z1 + z2) / 2, prof + 1),
                      boule2arbre_bis(cx_boule, cy_boule, cz_boule, r, x1, y1, z1, (x1 + x2) / 2, (y1 + y2) / 2, (z1 + z2) / 2, prof + 1),

                      boule2arbre_bis(cx_boule, cy_boule, cz_boule, r, x1, (y1 + y2) / 2, (z1 + z2) / 2, (x1 + x2) / 2, y2, z2, prof + 1),
                      boule2arbre_bis(cx_boule, cy_boule, cz_boule, r, (x1 + x2) / 2, (y1 + y2) / 2, (z1 + z2) / 2, x2, y2, z2, prof + 1),
                      boule2arbre_bis(cx_boule, cy_boule, cz_boule, r, (x1 + x2) / 2, y1, (z1 + z2) / 2, x2, (y1 + y2) / 2, z2, prof + 1),
                      boule2arbre_bis(cx_boule, cy_boule, cz_boule, r, x1, y1, (z1 + z2) / 2, (x1 + x2) / 2, (y1 + y2) / 2, z2, prof + 1));
}

karbre boule2arbre(int cx_boule, int cy_boule, int cz_boule, int r)
{
    return boule2arbre_bis(cx_boule, cy_boule, cz_boule, r, 0, 0, 0, pow(2, N), pow(2, N), pow(2, N), 0);
}

karbre intersection(karbre V1, karbre V2)
{
    if (kEstVide(V1) || kEstVide(V2))
        return kArbreVide();

    if (kRacine(V1) == VIDE || kRacine(V2) == VIDE)
        return kConsArbre(VIDE,
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide(),
                          kArbreVide());

    if (kRacine(V1) == PLEIN)
        return V2;
    if (kRacine(V2) == PLEIN)
        return V1;

    return kConsArbre(SUBDIV, intersection(kFils(0, V1), kFils(0, V2)),
                      intersection(kFils(1, V1), kFils(1, V2)),
                      intersection(kFils(2, V1), kFils(2, V2)),
                      intersection(kFils(3, V1), kFils(3, V2)),
                      intersection(kFils(4, V1), kFils(4, V2)),
                      intersection(kFils(5, V1), kFils(5, V2)),
                      intersection(kFils(6, V1), kFils(6, V2)),
                      intersection(kFils(7, V1), kFils(7, V2)));
}

void affiche_cube(int x1, int y1, int z1, int x2, int y2, int z2)
{

    glBegin(GL_QUADS);

    // Face du bas
    glColor3f(0.9, 0.9, 0.9);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y1, z1);
    glVertex3f(x2, y1, z2);
    glVertex3f(x1, y1, z2);

    // Face du haut
    glColor3f(0.8, 0.8, 0.8);
    glVertex3f(x1, y2, z1);
    glVertex3f(x2, y2, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x1, y2, z2);

    // Face avant
    glColor3f(0.7, 0.7, 0.7);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y1, z1);
    glVertex3f(x2, y2, z1);
    glVertex3f(x1, y2, z1);

    // Face arrière
    glColor3f(0.6, 0.6, 0.6);
    glVertex3f(x1, y1, z2);
    glVertex3f(x2, y1, z2);
    glVertex3f(x2, y2, z2);
    glVertex3f(x1, y2, z2);

    // Face gauche
    glColor3f(0.5, 0.5, 0.5);
    glVertex3f(x1, y1, z2);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y2, z1);
    glVertex3f(x1, y2, z2);

    // Face droite
    glColor3f(0.4, 0.4, 0.4);
    glVertex3f(x2, y1, z2);
    glVertex3f(x2, y1, z1);
    glVertex3f(x2, y2, z1);
    glVertex3f(x2, y2, z2);

    glEnd();
}

void affiche_volume_bis(karbre v, int x1, int y1, int z1, int x2, int y2, int z2)
{
    if (kEstVide(v))
        return;

    if (kRacine(v) == VIDE)
    {
        return;
    }
    else if (kRacine(v) == PLEIN)
    {
        // printf("Aaffichage du cube (%d, %d, %d) (%d, %d, %d)\n", x1, y1, z1, x2, y2, z2);
        affiche_cube(x1, y1, z1, x2, y2, z2);
    }
    else if (kRacine(v) == SUBDIV)
    {
        affiche_volume_bis(kFils(0, v), x1, (y1 + y2) / 2, z1, (x1 + x2) / 2, y2, (z1 + z2) / 2);
        affiche_volume_bis(kFils(1, v), (x1 + x2) / 2, (y1 + y2) / 2, z1, x2, y2, (z1 + z2) / 2);
        affiche_volume_bis(kFils(2, v), (x1 + x2) / 2, y1, z1, x2, (y1 + y2) / 2, (z1 + z2) / 2);
        affiche_volume_bis(kFils(3, v), x1, y1, z1, (x1 + x2) / 2, (y1 + y2) / 2, (z1 + z2) / 2);

        affiche_volume_bis(kFils(4, v), x1, (y1 + y2) / 2, (z1 + z2) / 2, (x1 + x2) / 2, y2, z2);
        affiche_volume_bis(kFils(5, v), (x1 + x2) / 2, (y1 + y2) / 2, (z1 + z2) / 2, x2, y2, z2);
        affiche_volume_bis(kFils(6, v), (x1 + x2) / 2, y1, (z1 + z2) / 2, x2, (y1 + y2) / 2, z2);
        affiche_volume_bis(kFils(7, v), x1, y1, (z1 + z2) / 2, (x1 + x2) / 2, (y1 + y2) / 2, z2);
    }
}

void affiche_volume(karbre v)
{
    affiche_volume_bis(v, 0, 0, 0, pow(2, N), pow(2, N), pow(2, N));
}