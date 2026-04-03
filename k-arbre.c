#include "k-arbre.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

karbre kArbreVide()
{

    return NULL;
}

karbre kConsArbre(element e, ...)
{
    int i;
    va_list params;
    karbre A;

    va_start(params, e);
    if ((A = (karbre)malloc(sizeof(struct s_karbre))) == NULL)
    {
        printf("ERREUR allocation mémoire\n");
        exit(EXIT_FAILURE);
    }

    A->e = e;
    for (i = 0; i < K; i++)
    {
        A->fils[i] = va_arg(params, karbre);
    }
    va_end(params);

    return A;
}

karbre kFils(int ieme, karbre A)
{
    return A->fils[ieme];
}

element kRacine(karbre A)
{
    return A->e;
}

int kEstVide(karbre A)
{
    return A == kArbreVide();
}

void kAfficher(karbre A)
{
    kAfficher_bis(A, 0);
}

void kAfficher_bis(karbre A, int prof)
{
    int i;

    if (kEstVide(A))
        return;

    for (i = 0; i < prof; i++)
        printf("  ");

    if (kRacine(A) == VIDE)
        printf("VIDE\n");
    else if (kRacine(A) == PLEIN)
        printf("PLEIN\n");
    else if (kRacine(A) == SUBDIV)
        printf("SUBDIV\n");

    for (i = 0; i < K; i++)
        kAfficher_bis(kFils(i, A), prof + 1);
}