#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL/SDL.h>
#define COTE 8
#define VIDE 20
#define SUCCES true
#define ECHEC false
#define R 65536
#define V 256
#define B 1

//parametres
#define LARGEUR 600
#define HAUTEUR 600
#define ESPACE 64
#define EPAISSEUR 4
//fin parametres

#define ORIGINE_X (LARGEUR-ESPACE*COTE)/2
#define ORIGINE_Y (HAUTEUR-ESPACE*COTE)/2

typedef enum {
    BLANC       =   R*255   +   V*255   +   B*255   ,
    NOIR        =   R*0     +   V*0     +   B*0     ,
    GRIS        =   R*127   +   V*127   +   B*127   ,
    ROUGE       =   R*255   +   V*0     +   B*0     ,
    VERT_CLAIR  =   R*127   +   V*255   +   B*0
} couleur_t;

typedef struct {
    int i;
    int j;
} position_t;

typedef struct {
    bool petit;
    bool grand;
} roque_t;

Uint32 get_couleur(SDL_Surface *surface, couleur_t couleur){
    return SDL_MapRGB(surface->format, couleur/R, couleur/V, couleur/B);
}

SDL_Surface *CreerRect(int largeur, int hauteur){
    return SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, largeur, hauteur, 32, 0, 0, 0, 0);
}

void ColorerRect(SDL_Surface *surface, couleur_t couleur){
    SDL_FillRect(surface, NULL, get_couleur(surface, couleur));
}

void Transparence(SDL_Surface *surface, couleur_t couleur){
    SDL_SetColorKey(surface, SDL_SRCCOLORKEY, get_couleur(surface, couleur));
}

void ImporterImagesPieces(SDL_Surface *pions[2][6]){
    char s[12];
    for(int i=0, j; i<2; i++){
        for(j=0; j<6; j++){
            snprintf(s, 12, "../img/%d_%d.bmp", i, j);
            pions[i][j] = SDL_LoadBMP(s);
            Transparence(pions[i][j], GRIS);
        }
    }
}

void CollerSurf(SDL_Surface *surface_dest, SDL_Surface *surface_src, SDL_Rect position){
    SDL_BlitSurface(surface_src, NULL, surface_dest, &position);
}

void regler_fenetre(SDL_Surface *ecran, char *nom_fenetre, char *adr_icone, couleur_t couleur){
    SDL_WM_SetIcon(SDL_LoadBMP(adr_icone), NULL);
    SDL_WM_SetCaption(nom_fenetre, NULL);
    ColorerRect(ecran, couleur);
}

void dessiner_grille(SDL_Surface *ecran, couleur_t couleur){
    SDL_Surface *horizontale = CreerRect(COTE*ESPACE+EPAISSEUR, EPAISSEUR);
    SDL_Surface *verticale = CreerRect(EPAISSEUR, COTE*ESPACE+EPAISSEUR);
    ColorerRect(horizontale, couleur);
    ColorerRect(verticale, couleur);
    SDL_Rect position_h;
    SDL_Rect position_v;
    position_h.x=ORIGINE_X;
    position_v.y=ORIGINE_Y;
    for(int i=0; i<=COTE; i++){
        position_h.y=ORIGINE_Y+i*ESPACE;
        position_v.x=ORIGINE_X+i*ESPACE;
        CollerSurf(ecran, horizontale, position_h);
        CollerSurf(ecran, verticale, position_v);
    }
    SDL_FreeSurface(horizontale);
    SDL_FreeSurface(verticale);
}

void dessiner_cases(SDL_Surface *ecran, SDL_Surface *cases[2]){
    SDL_Rect position;
    for(int i=0, j; i<COTE; i++){
        for(j=0; j<COTE; j++){
            position.x=ORIGINE_X+EPAISSEUR+j*ESPACE;
            position.y=ORIGINE_Y+EPAISSEUR+i*ESPACE;
            CollerSurf(ecran, cases[(j+i%2)%2], position);
        }
    }
}

void placer_pieces(SDL_Surface *ecran, SDL_Surface *pions[2][6], int grille[COTE][COTE], bool joueur, bool rotation){
    SDL_Rect position;
    for(int i=0; i<COTE; i++){
        for(int j=0; j<COTE; j++){
            position.x=ORIGINE_X+EPAISSEUR+((!joueur||!rotation)*j+((joueur&&rotation)*(7-j)))*ESPACE;
            position.y=ORIGINE_Y+EPAISSEUR+((!joueur||!rotation)*i+((joueur&&rotation)*(7-i)))*ESPACE;
            if(grille[i][j] != VIDE){
                CollerSurf(ecran, pions[grille[i][j]/10][grille[i][j]%10], position);
            }
        }
    }
}

void liberer_surfaces(SDL_Surface *cases[2], SDL_Surface *case_mouv, SDL_Surface *case_select, SDL_Surface *pions[2][6]){
    for(int i=0, j; i<2; i++){
        SDL_FreeSurface(cases[i]);
        for(j=0; j<6; j++){
            SDL_FreeSurface(pions[i][j]);
        }
    }
    SDL_FreeSurface(case_mouv);
    SDL_FreeSurface(case_select);
}

void initialiser_grille_jeu(int grille_jeu[COTE][COTE]){
    for(int i=0, j; i<COTE; i++){
        for(j=0; j<COTE; j++){
            switch(i){
            case 0:
            case 7:
                if(j<5){
                    grille_jeu[i][j]=10*(1-i/7)+j;
                } else {
                    grille_jeu[i][j]=10*(1-i/7)+7-j;
                }
                break;
            case 1:
            case 6:
                grille_jeu[i][j]=17-2*i;
                break;
            default:
                grille_jeu[i][j]=VIDE;
                break;
            }
        }
    }
}

void reinit_mouv(bool grille_mouv[COTE][COTE]){
    for(int i=0, j; i<COTE; i++){
        for(j=0; j<COTE; j++){
            grille_mouv[i][j]=false;
        }
    }
}

void zero_en_passant_joueur(bool en_passant_joueur[COTE]){
    for(int i=0; i<COTE; i++){
        en_passant_joueur[i]=false;
    }
}

void zero_en_passant(bool en_passant[2][COTE]){
    for(int i=0; i<2; i++){
        zero_en_passant_joueur(en_passant[i]);
    }
}

void droit_mouv(int grille_jeu[COTE][COTE], bool grille_mouv[COTE][COTE], int i, int j, bool joueur){
    int b, h, g, d;
    for(b=i+1; (b<COTE) && (grille_jeu[b][j]==VIDE); b++){
        grille_mouv[b][j]=true;
    }
    if((b<COTE) && (grille_jeu[b][j]/10==!joueur)){
        grille_mouv[b][j]=true;
    }
    for(h=i-1; (h>=0) && (grille_jeu[h][j]==VIDE); h--){
        grille_mouv[h][j]=true;
    }
    if((h>=0) && (grille_jeu[h][j]/10==!joueur)){
        grille_mouv[h][j]=true;
    }
    for(g=j-1; (g>=0) && (grille_jeu[i][g]==VIDE); g--){
        grille_mouv[i][g]=true;
    }
    if((g>=0) && (grille_jeu[i][g]/10==!joueur)){
        grille_mouv[i][g]=true;
    }
    for(d=j+1; (d<COTE) && (grille_jeu[i][d]==VIDE); d++){
        grille_mouv[i][d]=true;
    }
    if((d<COTE) && (grille_jeu[i][d]/10==!joueur)){
        grille_mouv[i][d]=true;
    }
}

void diagonal_mouv(int grille_jeu[COTE][COTE], bool grille_mouv[COTE][COTE], int i, int j, bool joueur){
    int bg, bd, hg, hd;
    for(bg=1; (i+bg<COTE) && (j-bg>=0) && (grille_jeu[i+bg][j-bg]==VIDE); bg++){
        grille_mouv[i+bg][j-bg]=true;
    }
    if((i+bg<COTE) && (j-bg>=0) && (grille_jeu[i+bg][j-bg]/10==!joueur)){
        grille_mouv[i+bg][j-bg]=true;
    }
    for(bd=1; (i+bd<COTE) && (j+bd<COTE) && (grille_jeu[i+bd][j+bd]==VIDE); bd++){
        grille_mouv[i+bd][j+bd]=true;
    }
    if((i+bd<COTE) && (j+bd<COTE) && (grille_jeu[i+bd][j+bd]/10==!joueur)){
        grille_mouv[i+bd][j+bd]=true;
    }
    for(hg=1; (i-hg>=0) && (j-hg>=0) && (grille_jeu[i-hg][j-hg]==VIDE); hg++){
        grille_mouv[i-hg][j-hg]=true;
    }
    if((i-hg>=0) && (j-hg>=0) && (grille_jeu[i-hg][j-hg]/10==!joueur)){
        grille_mouv[i-hg][j-hg]=true;
    }
    for(hd=1; (i-hd>=0) && (j+hd<COTE) && (grille_jeu[i-hd][j+hd]==VIDE); hd++){
        grille_mouv[i-hd][j+hd]=true;
    }
    if((i-hd>=0) && (j+hd<COTE) && (grille_jeu[i-hd][j+hd]/10==!joueur)){
        grille_mouv[i-hd][j+hd]=true;
    }
}

void cavalier_mouv(int grille_jeu[COTE][COTE], bool grille_mouv[COTE][COTE], int i, int j, bool joueur){
    if((i-2>=0) && (j-1>=0) && (grille_jeu[i-2][j-1]/10!=joueur)){
        grille_mouv[i-2][j-1]=true;
    }
    if((i-2>=0) && (j+1<COTE) && (grille_jeu[i-2][j+1]/10!=joueur)){
        grille_mouv[i-2][j+1]=true;
    }
    if((i-1>=0) && (j-2>=0) && (grille_jeu[i-1][j-2]/10!=joueur)){
        grille_mouv[i-1][j-2]=true;
    }
    if((i-1>=0) && (j+2<COTE) && (grille_jeu[i-1][j+2]/10!=joueur)){
        grille_mouv[i-1][j+2]=true;
    }
    if((i+1<COTE) && (j-2>=0) && (grille_jeu[i+1][j-2]/10!=joueur)){
        grille_mouv[i+1][j-2]=true;
    }
    if((i+1<COTE) && (j+2<COTE) && (grille_jeu[i+1][j+2]/10!=joueur)){
        grille_mouv[i+1][j+2]=true;
    }
    if((i+2<COTE) && (j-1>=0) && (grille_jeu[i+2][j-1]/10!=joueur)){
        grille_mouv[i+2][j-1]=true;
    }
    if((i+2<COTE) && (j+1<COTE) && (grille_jeu[i+2][j+1]/10!=joueur)){
        grille_mouv[i+2][j+1]=true;
    }
}

void unite_mouv(int grille_jeu[COTE][COTE], bool grille_mouv[COTE][COTE], int i, int j, bool joueur){
    if((i-1>=0) && (j-1>=0) && (grille_jeu[i-1][j-1]/10!=joueur)){
        grille_mouv[i-1][j-1]=true;
    }
    if((i-1>=0) && (grille_jeu[i-1][j]/10!=joueur)){
        grille_mouv[i-1][j]=true;
    }
    if((i-1>=0) && (j+1<COTE) && (grille_jeu[i-1][j+1]/10!=joueur)){
        grille_mouv[i-1][j+1]=true;
    }
    if((j-1>=0) && (grille_jeu[i][j-1]/10!=joueur)){
        grille_mouv[i][j-1]=true;
    }
    if((j+1<COTE) && (grille_jeu[i][j+1]/10!=joueur)){
        grille_mouv[i][j+1]=true;
    }
    if((i+1<COTE) && (j-1>=0) && (grille_jeu[i+1][j-1]/10!=joueur)){
        grille_mouv[i+1][j-1]=true;
    }
    if((i+1<COTE) && (grille_jeu[i+1][j]/10!=joueur)){
        grille_mouv[i+1][j]=true;
    }
    if((i+1<COTE) && (j+1<COTE) && (grille_jeu[i+1][j+1]/10!=joueur)){
        grille_mouv[i+1][j+1]=true;
    }
}

void pion_attaq_mouv(int grille_jeu[COTE][COTE], bool grille_mouv[COTE][COTE], int i, int j, bool joueur){
    if((i-1+2*joueur>=0) && (i-1+2*joueur<COTE) && (j>=0) && (grille_jeu[i-1+2*joueur][j-1]/10==!joueur)){
        grille_mouv[i-1+2*joueur][j-1]=true;
    }
    if((i-1+2*joueur>=0) && (i-1+2*joueur<COTE) && (j<COTE) && (grille_jeu[i-1+2*joueur][j+1]/10==!joueur)){
        grille_mouv[i-1+2*joueur][j+1]=true;
    }
}

void pion_mouv(int grille_jeu[COTE][COTE], bool grille_mouv[COTE][COTE], int i, int j, bool en_passant[2][COTE], bool joueur){
    if((i-1+2*joueur>=0) && (i-1+2*joueur<COTE) && (grille_jeu[i-1+2*joueur][j]==VIDE)){
        grille_mouv[i-1+2*joueur][j]=true;
        if((i==6-5*joueur) && (i-2+4*joueur>=0) && (i-2+4*joueur<COTE) && (grille_jeu[i-2+4*joueur][j]==VIDE)){
            grille_mouv[i-2+4*joueur][j]=true;
            en_passant[joueur][j]=true;
        }
    }
    if(i==3+joueur){
        if((j-1>=0) && (grille_jeu[i][j-1]==5+10*!joueur) && (en_passant[!joueur][j-1])){
            grille_mouv[i-1+2*joueur][j-1]=true;
        }
        if((j+1<COTE) && (grille_jeu[i][j+1]==5+10*!joueur) && (en_passant[!joueur][j+1])){
            grille_mouv[i-1+2*joueur][j+1]=true;
        }
    }
    pion_attaq_mouv(grille_jeu, grille_mouv, i, j, joueur);
}

bool menace(int grille_jeu[COTE][COTE], int i, int j, bool joueur){
    bool grille_mouv[COTE][COTE]={NULL};
    reinit_mouv(grille_mouv);
    bool en_passant[2][COTE];
    zero_en_passant(en_passant);
    for(int i2=0, j2; i2<COTE; i2++){
        for(j2=0; j2<COTE; j2++){
            switch(grille_jeu[i2][j2]+10*joueur){
            case 10:
                droit_mouv(grille_jeu, grille_mouv, i2, j2, !joueur);
                break;
            case 11:
                cavalier_mouv(grille_jeu, grille_mouv, i2, j2, !joueur);
                break;
            case 12:
                diagonal_mouv(grille_jeu, grille_mouv, i2, j2, !joueur);
                break;
            case 13:
                droit_mouv(grille_jeu, grille_mouv, i2, j2, !joueur);
                diagonal_mouv(grille_jeu, grille_mouv, i2, j2, !joueur);
                break;
            case 14:
                unite_mouv(grille_jeu, grille_mouv, i2, j2, !joueur);
                break;
            case 15:
                pion_attaq_mouv(grille_jeu, grille_mouv, i2, j2, !joueur);
                break;
            }
        }
    }
    return grille_mouv[i][j];
}

bool echec(int grille_jeu[COTE][COTE], bool joueur){
    int i=0, j=0;
    while(grille_jeu[i][j]!=4+10*joueur){
        j++;
        if(j==COTE){
            j=0;
            i++;
        }
    }
    return menace(grille_jeu, i, j, joueur);
}

void enregistrer_mouv_piece(int grille_jeu[COTE][COTE], bool grille_mouv[COTE][COTE], int i, int j, roque_t roque[2], bool en_passant[2][COTE], bool joueur){
    switch(grille_jeu[i][j]%10){
    case 0:
        droit_mouv(grille_jeu, grille_mouv, i, j, joueur);
        break;
    case 1:
        cavalier_mouv(grille_jeu, grille_mouv, i, j, joueur);
        break;
    case 2:
        diagonal_mouv(grille_jeu, grille_mouv, i, j, joueur);
        break;
    case 3:
        droit_mouv(grille_jeu, grille_mouv, i, j, joueur);
        diagonal_mouv(grille_jeu, grille_mouv, i, j, joueur);
        break;
    case 4:
        unite_mouv(grille_jeu, grille_mouv, i, j, joueur);
        if(roque[joueur].petit && (grille_jeu[7*!joueur][5]==VIDE) && (grille_jeu[7*!joueur][6]==VIDE) && !echec(grille_jeu, joueur) && !menace(grille_jeu, 7*!joueur, 5, joueur) && !menace(grille_jeu, 7*!joueur, 6, joueur) && !menace(grille_jeu, 7*!joueur, 7, joueur)){
            grille_mouv[7*!joueur][6] = true;
        }
        if(roque[joueur].grand && (grille_jeu[7*!joueur][1]==VIDE) && (grille_jeu[7*!joueur][2]==VIDE) && (grille_jeu[7*!joueur][3]==VIDE) && !echec(grille_jeu, joueur) && !menace(grille_jeu, 7*!joueur, 0, joueur) && !menace(grille_jeu, 7*!joueur, 1, joueur) && !menace(grille_jeu, 7*!joueur, 2, joueur) && !menace(grille_jeu, 7*!joueur, 3, joueur)){
            grille_mouv[7*!joueur][2] = true;
        }
        break;
    case 5:
        pion_mouv(grille_jeu, grille_mouv, i, j, en_passant, joueur);
        break;
    }
}

void supprimer_mouv_echec(int grille_jeu[COTE][COTE], bool grille_mouv[COTE][COTE], int i, int j, bool joueur){
    int aux;
    for(int i2=0, j2; i2<COTE; i2++){
        for(j2=0; j2<COTE;j2++){
            if(grille_mouv[i2][j2]){
                aux = grille_jeu[i2][j2];
                grille_jeu[i2][j2] = grille_jeu[i][j];
                grille_jeu[i][j] = VIDE;
                if(echec(grille_jeu, joueur)){
                    grille_mouv[i2][j2] = false;
                }
                grille_jeu[i][j] = grille_jeu[i2][j2];
                grille_jeu[i2][j2] = aux;
            }
        }
    }
}

bool enregistrer_mouv(int grille_jeu[COTE][COTE], bool grille_mouv[COTE][COTE], int i, int j, roque_t roque[2], bool en_passant[2][COTE], bool joueur){
    if(i>=0 && i<COTE && j>=0 && j<COTE && grille_jeu[i][j]/10==joueur){
        reinit_mouv(grille_mouv);
        enregistrer_mouv_piece(grille_jeu, grille_mouv, i, j, roque, en_passant, joueur);
        supprimer_mouv_echec(grille_jeu, grille_mouv, i, j, joueur);
        return SUCCES;
    } else {
        return ECHEC;
    }
}

bool mouv_vide(bool grille_mouv[COTE][COTE]){
    bool vide = false;
    int i=0, j=0;
    while(!vide && !grille_mouv[i][j]){
        j++;
        if(j==COTE){
            j=0;
            i++;
        }
        if(i==COTE){
            vide = true;
        }
    }
    return vide;
}

bool echec_et_mat(int grille_jeu[COTE][COTE], bool joueur){
    bool grille_mouv[COTE][COTE] = {NULL};
    roque_t roque[2] = {{false, false}, {false, false}};
    bool en_passant[2][COTE];
    zero_en_passant(en_passant);
    bool pas_mouv = true;
    int i=0, j=0;
    while(i<COTE && pas_mouv){
        if(enregistrer_mouv(grille_jeu, grille_mouv, i, j, roque, en_passant, joueur)){
            pas_mouv = pas_mouv && mouv_vide(grille_mouv);
        }
        j++;
        if(j==COTE){
            j=0;
            i++;
        }
    }
    return pas_mouv;
}

void afficher_mouv(SDL_Surface *ecran, SDL_Surface *case_mouv, SDL_Surface *case_select, bool grille_mouv[COTE][COTE], position_t position, bool joueur, bool rotation){
    SDL_Rect position_surf;
    position_surf.x=ORIGINE_X+EPAISSEUR+ESPACE*((!joueur||!rotation)*position.j+((joueur&&rotation)*(7-position.j)));
    position_surf.y=ORIGINE_Y+EPAISSEUR+ESPACE*((!joueur||!rotation)*position.i+((joueur&&rotation)*(7-position.i)));
    CollerSurf(ecran, case_select, position_surf);
    for(int i=0, j; i<COTE; i++){
        for(j=0; j<COTE; j++){
            if(grille_mouv[i][j]){
                position_surf.x=ORIGINE_X+EPAISSEUR+((!joueur||!rotation)*j+((joueur&&rotation)*(7-j)))*ESPACE;
                position_surf.y=ORIGINE_Y+EPAISSEUR+((!joueur||!rotation)*i+((joueur&&rotation)*(7-i)))*ESPACE;
                CollerSurf(ecran, case_mouv, position_surf);
            }
        }
    }
}

bool bouger_piece(int grille_jeu[COTE][COTE], bool grille_mouv[COTE][COTE], position_t position_new, position_t position_old, roque_t roque[2], bool joueur){
    if(grille_mouv[position_new.i][position_new.j]){
        if((roque[joueur].petit || roque[joueur].grand) && (position_old.i==7*!joueur) && (position_old.j==4)){
            roque[joueur].petit = false;
            roque[joueur].grand = false;
        } else if(roque[joueur].petit && (position_old.i==7*!joueur) && (position_old.j==7)){
            roque[joueur].petit = false;
        } else if(roque[joueur].grand && (position_old.i==7*!joueur) && (position_old.j==0)){
            roque[joueur].grand = false;
        }
        if((grille_jeu[position_old.i][position_old.j]%10==5) && (position_new.i==7*joueur)){
            grille_jeu[position_new.i][position_new.j]=3+10*joueur;
        } else {
            if(grille_jeu[position_old.i][position_old.j]%10==4){
                if(position_new.j-position_old.j==2){
                    grille_jeu[position_old.i][5]=10*joueur;
                    grille_jeu[position_old.i][7]=VIDE;
                    roque[joueur].petit = false;
                } else if(position_old.j-position_new.j==2){
                    grille_jeu[position_old.i][3]=10*joueur;
                    grille_jeu[position_old.i][0]=VIDE;
                    roque[joueur].grand = false;
                }
            } else if(grille_jeu[position_old.i][position_old.j]%10==5) {
                if(abs(position_old.i-position_new.i)==1 && abs(position_old.j-position_new.j)==1 && grille_jeu[position_new.i][position_new.j]==VIDE){
                    grille_jeu[position_new.i+1-2*joueur][position_new.j]=VIDE;
                }
            }
            grille_jeu[position_new.i][position_new.j]=grille_jeu[position_old.i][position_old.j];
        }
        grille_jeu[position_old.i][position_old.j]=VIDE;
        return SUCCES;
    } else {
        return ECHEC;
    }
}

int main(int argc, char * argv[]){
    //chargement de la SDL, creation et reglage de la fenetre
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface *ecran = SDL_SetVideoMode(LARGEUR, HAUTEUR, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    regler_fenetre(ecran, "Jeu d'echecs", "img/icone.bmp", BLANC);

    //creation cases et pieces
    SDL_Surface *cases[2] = {
        CreerRect(ESPACE-EPAISSEUR, ESPACE-EPAISSEUR),
        CreerRect(ESPACE-EPAISSEUR, ESPACE-EPAISSEUR)
    };
    SDL_Surface *case_mouv = CreerRect(ESPACE-EPAISSEUR, ESPACE-EPAISSEUR);
    SDL_Surface *case_select = CreerRect(ESPACE-EPAISSEUR, ESPACE-EPAISSEUR);
    SDL_Surface *pions[2][6] = {NULL};

    //coloration cases et importation images pieces
    ColorerRect(cases[0], BLANC);
    ColorerRect(cases[1], GRIS);
    ColorerRect(case_mouv, ROUGE);
    ColorerRect(case_select, VERT_CLAIR);
    ImporterImagesPieces(pions);

    //variables
    bool continuer = true;
    bool rotation = true;
    if(argc<3){
        if((argv[1] != NULL) && (strcmp("-r", argv[1])==0)){
            rotation = false;
        }
    } else {
        continuer = false;
    }
    int grille_jeu[COTE][COTE];
    bool grille_mouv[COTE][COTE];
    bool poser = false;
    bool joueur = 0;
    position_t position;
    position_t position_old;
    roque_t roque[2] = {{true, true}, {true, true}};
    bool en_passant[2][COTE];
    zero_en_passant(en_passant);
    SDL_Event event;

    //jeu
    initialiser_grille_jeu(grille_jeu);
    dessiner_grille(ecran, NOIR);
    dessiner_cases(ecran, cases);
    placer_pieces(ecran, pions, grille_jeu, joueur, rotation);
    SDL_Flip(ecran);
    while(continuer){
        SDL_WaitEvent(&event);
        switch(event.type){
        case SDL_QUIT:
            continuer = false;
        case SDL_MOUSEBUTTONDOWN:
            if(event.button.button==SDL_BUTTON_LEFT){
                position.i = (event.button.y-ORIGINE_Y)/ESPACE;
                position.j = (event.button.x-ORIGINE_X)/ESPACE;
                if(rotation && joueur){
                    position.i = 7-position.i;
                    position.j = 7-position.j;
                }
                if(!poser && !echec_et_mat(grille_jeu, joueur) && enregistrer_mouv(grille_jeu, grille_mouv, position.i, position.j, roque, en_passant, joueur)){
                    afficher_mouv(ecran, case_mouv, case_select, grille_mouv, position, joueur, rotation);
                    placer_pieces(ecran, pions, grille_jeu, joueur, rotation);
                    SDL_Flip(ecran);
                    position_old = position;
                    poser = true;
                } else if(poser) {
                    if(!memcmp(&position, &position_old, sizeof(position_t))){
                        dessiner_cases(ecran, cases);
                        placer_pieces(ecran, pions, grille_jeu, joueur, rotation);
                        SDL_Flip(ecran);
                        poser = false;
                    } else if(bouger_piece(grille_jeu, grille_mouv, position, position_old, roque, joueur)){
                        dessiner_cases(ecran, cases);
                        placer_pieces(ecran, pions, grille_jeu, joueur, rotation);
                        SDL_Flip(ecran);
                        poser = false;
                        joueur = !joueur;
                        if(rotation){
                            dessiner_cases(ecran, cases);
                            placer_pieces(ecran, pions, grille_jeu, joueur, rotation);
                            SDL_Flip(ecran);
                        }
                        zero_en_passant_joueur(en_passant[joueur]);
                    }
                }
            }
        }
    }
    liberer_surfaces(cases, case_mouv, case_select, pions);
    SDL_Quit();
    return 0;
}
