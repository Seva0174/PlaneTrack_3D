#define K 8
#define VIDE 0
#define PLEIN 1
#define SUBDIV -1

typedef int element;

typedef struct s_karbre
{
    element e;
    struct s_karbre *fils[K];

} *karbre;

karbre kArbreVide();
karbre kConsArbre(element e, ...);
karbre kFils(int ieme, karbre A);
element kRacine(karbre A);
int kEstVide(karbre A);
void kAfficher(karbre A);
void kAfficher_bis(karbre A, int prof);