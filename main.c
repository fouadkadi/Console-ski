#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
//----------------------------------Bibliotheque Conio pour l'interface graphique ----------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
typedef enum
{
    BLACK,          /**< black color */
    BLUE,           /**< blue color */
    GREEN,          /**< green color */
    CYAN,           /**< cyan color */
    RED,            /**< red color */
    MAGENTA,        /**< magenta color */
    BROWN,          /**< brown color */
    LIGHTGRAY,      /**< light gray color */
    DARKGRAY,       /**< dark gray color */
    LIGHTBLUE,      /**< light blue color */
    LIGHTGREEN,     /**< light green color */
    LIGHTCYAN,      /**< light cyan color */
    LIGHTRED,       /**< light red color */
    LIGHTMAGENTA,   /**< light magenta color */
    YELLOW,         /**< yellow color */
    WHITE           /**< white color */
} COLORS;

/*@{*/
/**
 * This defines enables you to use all MinGW conio.h functions without
 * underscore.
 */
#define cgets   _cgets
#define cprintf _cprintf
#define cputs   _cputs
#define cscanf  _cscanf

#ifdef UNICODE
    #define cgetws   _cgetws
    #define getwch   _getwch
    #define getwche  _getwche
    #define putwch   _putwch
    #define ungetwch _ungetwch
    #define cputws   _cputws
    #define cwprintf _cwprintf
    #define cwscanf  _cwscanf
#endif
/*@}*/

/**
 * Define alias for _conio_gettext.
 * If you want to use gettext function from some other library
 * (e.g. GNU gettext) you have to define _CONIO_NO_GETTEXT_ so you won't get
 * name conflict.
 */
#ifndef _CONIO_NO_GETTEXT_
  #define gettext _conio_gettext
#endif

#define ScreenClear clrscr

/**
 * @anchor cursortypes
 * @name Cursor types
 * Predefined cursor types. */
/*@{*/
#define _NOCURSOR 0         /**< no cursor */
#define _SOLIDCURSOR 100    /**< cursor filling whole cell */
#define _NORMALCURSOR 20    /**< cursor filling 20 percent of cell height */
/*@}*/

/**
 * Structure holding information about screen.
 * @see gettextinfo
 * @see inittextinfo
 */
struct text_info {
    unsigned char curx;          /**< cursor coordinate x */
    unsigned char cury;          /**< cursor coordinate y */
    unsigned short attribute;    /**< current text attribute */
    unsigned short normattr;     /**< original value of text attribute after
                                      start of the application. If you don't
                                      called the <TT>inittextinfo</TT> on the
                                      beginning of the application, this always
                                      will be black background and light gray
                                      foreground */
    unsigned char screenwidth;   /**< screen width */
    unsigned char screenheight;  /**< screen height */
};

/**
 * Structure used by gettext/puttext.
 * @see _conio_gettext
 * @see puttext
 */
struct char_info {
#ifdef UNICODE
    wchar_t letter;        /**< character value */
#else
    char letter;           /**< character value */
#endif
    unsigned short attr;   /**< attribute value */
};

static int __BACKGROUND = BLACK;
static int __FOREGROUND = LIGHTGRAY;
static struct text_info __text_info = {
    1, 1,
    LIGHTGRAY + (BLACK << 4),
    LIGHTGRAY + (BLACK << 4),
    80, 25
};
static int __CONIO_TOP = 0;
static int __CONIO_LEFT = 0;

static void
__fill_text_info (void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
    __CONIO_LEFT = info.srWindow.Left;
    __CONIO_TOP = info.srWindow.Top;
    __text_info.curx = info.dwCursorPosition.X - __CONIO_LEFT + 1;
    __text_info.cury = info.dwCursorPosition.Y - __CONIO_TOP  + 1;
    __text_info.attribute = info.wAttributes;
    __text_info.screenwidth  = info.srWindow.Right - info.srWindow.Left + 1;
    __text_info.screenheight = info.srWindow.Bottom - info.srWindow.Top + 1;
}

void
gettextinfo (struct text_info * info)
{
    __fill_text_info();
    *info = __text_info;
}

void
inittextinfo (void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    GetConsoleScreenBufferInfo (GetStdHandle(STD_OUTPUT_HANDLE), &info);
    __text_info.normattr = info.wAttributes;
}

void
clrscr (void)
{
    DWORD written;
    int i;

    __fill_text_info();
    for (i = __CONIO_TOP; i < __CONIO_TOP + __text_info.screenheight; i++) {
      FillConsoleOutputAttribute (GetStdHandle (STD_OUTPUT_HANDLE),
        __FOREGROUND + (__BACKGROUND << 4), __text_info.screenwidth,
        (COORD) {__CONIO_LEFT, i},
        &written);
      FillConsoleOutputCharacter (GetStdHandle(STD_OUTPUT_HANDLE), ' ',
        __text_info.screenwidth,
        (COORD) {__CONIO_LEFT, i},
        &written);
    }
    gotoxy (1, 1);
}


void
clreol (void)
{
    COORD coord;
    DWORD written;

    __fill_text_info();
    coord.X = __CONIO_LEFT + __text_info.curx - 1;
    coord.Y = __CONIO_TOP  + __text_info.cury - 1;

    FillConsoleOutputAttribute (GetStdHandle (STD_OUTPUT_HANDLE),
      __FOREGROUND + (__BACKGROUND << 4),
      __text_info.screenwidth - __text_info.curx + 1, coord, &written);
    FillConsoleOutputCharacter (GetStdHandle (STD_OUTPUT_HANDLE),
      ' ', __text_info.screenwidth - __text_info.curx + 1, coord, &written);
    gotoxy (__text_info.curx, __text_info.cury);
}


void
delline (void)
{
    COORD coord;
    SMALL_RECT rect;
    CHAR_INFO fillchar;

    __fill_text_info();
    coord.X = __CONIO_LEFT;
    coord.Y = __CONIO_TOP + __text_info.cury - 1;
    rect.Left = __CONIO_LEFT;
    rect.Top = __CONIO_TOP + __text_info.cury;
    rect.Right = __CONIO_LEFT + __text_info.screenwidth - 1;
    rect.Bottom = __CONIO_TOP + __text_info.screenheight - 1;
    fillchar.Attributes = __FOREGROUND + (__BACKGROUND << 4);
#ifdef UNICODE
    fillchar.Char.UnicodeChar = L' ';
    ScrollConsoleScreenBufferW (GetStdHandle (STD_OUTPUT_HANDLE),
      &rect, NULL, coord, &fillchar);
#else
    fillchar.Char.AsciiChar = ' ';
    ScrollConsoleScreenBufferA (GetStdHandle (STD_OUTPUT_HANDLE),
      &rect, NULL, coord, &fillchar);
#endif


    gotoxy (__text_info.curx, __text_info.cury);
}

void
insline (void)
{
    COORD coord;
    SMALL_RECT rect;
    CHAR_INFO fillchar;

    __fill_text_info();
    coord.X = __CONIO_LEFT;
    coord.Y = __CONIO_TOP + __text_info.cury;
    rect.Left = __CONIO_LEFT;
    rect.Top = __CONIO_TOP + __text_info.cury - 1;
    rect.Right = __CONIO_LEFT + __text_info.screenwidth - 1;
    rect.Bottom = __CONIO_TOP + __text_info.screenheight - 2;
    fillchar.Attributes = __FOREGROUND + (__BACKGROUND << 4);
#ifdef UNICODE
    fillchar.Char.UnicodeChar = L' ';
    ScrollConsoleScreenBufferW (GetStdHandle (STD_OUTPUT_HANDLE),
      &rect, NULL, coord, &fillchar);
#else
    fillchar.Char.AsciiChar = ' ';
    ScrollConsoleScreenBufferA (GetStdHandle (STD_OUTPUT_HANDLE),
      &rect, NULL, coord, &fillchar);
#endif

    gotoxy (__text_info.curx, __text_info.cury);
}

void
movetext (int left, int top, int right, int bottom, int destleft, int desttop)
{
    struct char_info * buffer;

    buffer = malloc ((right - left + 1) * (bottom - top + 1) * sizeof(struct char_info));
    gettext (left, top, right, bottom, buffer);
    puttext (destleft, desttop, destleft + right - left, desttop + bottom - top, buffer);
    free(buffer);
}

void
_conio_gettext (int left, int top, int right, int bottom,
  struct char_info * buf)
{
    int i;
    SMALL_RECT r;
    CHAR_INFO* buffer;
    COORD size;

    __fill_text_info();
    r = (SMALL_RECT) {__CONIO_LEFT + left - 1, __CONIO_TOP + top - 1,
      __CONIO_LEFT + right - 1, __CONIO_TOP + bottom - 1};
    size.X = right - left + 1;
    size.Y = bottom - top + 1;
    buffer = malloc (size.X * size.Y * sizeof(CHAR_INFO));

    ReadConsoleOutput (GetStdHandle (STD_OUTPUT_HANDLE),
      (PCHAR_INFO) buffer, size, (COORD) {0, 0}, &r);

    for (i = 0; i < size.X * size.Y; i++)
    {
#ifdef UNICODE
        buf[i].letter = buffer[i].Char.UnicodeChar;
#else
        buf[i].letter = buffer[i].Char.AsciiChar;
#endif
        buf[i].attr = buffer[i].Attributes;
    }
    free (buffer);
}

void
puttext (int left, int top, int right, int bottom, struct char_info * buf)
{
    int i;
    SMALL_RECT r;
    CHAR_INFO* buffer;
    COORD size;

    __fill_text_info();
    r = (SMALL_RECT) {__CONIO_LEFT + left - 1, __CONIO_TOP + top - 1,
      __CONIO_LEFT + right - 1, __CONIO_TOP + bottom - 1};
    size.X = right - left + 1;
    size.Y = bottom - top + 1;
    buffer = malloc (size.X * size.Y * sizeof(CHAR_INFO));

    for (i = 0; i < size.X * size.Y; i++)
    {
#ifdef UNICODE
        buffer[i].Char.UnicodeChar = buf[i].letter;
#else
        buffer[i].Char.AsciiChar = buf[i].letter;
#endif
        buffer[i].Attributes = buf[i].attr;
    }

    WriteConsoleOutput (GetStdHandle (STD_OUTPUT_HANDLE),
      buffer, size, (COORD) {0, 0}, &r);
    free (buffer);
}

void
gotoxy(int x, int y)
{
  COORD c;

  c.X = __CONIO_LEFT + x - 1;
  c.Y = __CONIO_TOP  + y - 1;
  SetConsoleCursorPosition (GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void
cputsxy (int x, int y, char * str)
{
    gotoxy (x, y);
    cputs (str);
}

void
putchxy (int x, int y, char ch)
{
    gotoxy (x, y);
    putch (ch);
}

void
_setcursortype (int type)
{
    CONSOLE_CURSOR_INFO Info;

    if (type == 0) {
        Info.bVisible = FALSE;
    } else {
      Info.dwSize = type;
      Info.bVisible = TRUE;
    }
    SetConsoleCursorInfo (GetStdHandle (STD_OUTPUT_HANDLE),
      &Info);
}


void
textattr (int _attr)
{
    __FOREGROUND = _attr & 0xF;
    __BACKGROUND = _attr >> 4;
    SetConsoleTextAttribute (GetStdHandle(STD_OUTPUT_HANDLE), _attr);
}

void
normvideo (void)
{
    textattr (__text_info.normattr);
}

void
textbackground (int color)
{
    __BACKGROUND = color;
    SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE),
      __FOREGROUND + (color << 4));
}


void
textcolor (int color)
{
    __FOREGROUND = color;
    SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE),
      color + (__BACKGROUND << 4));
}


int
wherex (void)
{
    __fill_text_info();
    return __text_info.curx;
}


int
wherey (void)
{
    __fill_text_info();
    return __text_info.cury;
}

char *
getpass (const char * prompt, char * str)
{
    int maxlength = str[0];
    int length = 0;
    int ch = 0;
    int x, y;

    cputs(prompt);
    __fill_text_info();
    x = __text_info.curx;
    y = __text_info.cury;

    while (ch != '\r') {
        ch = getch();
        switch (ch) {
            case '\r' : /* enter */
                break;
            case '\b' : /* backspace */
                if (length > 0) putchxy (x + --length, y, ' ');
                gotoxy (x + length, y);
                break;
            default:
                if (length < maxlength) {
                    putchxy (x + length, y, '*');
                    str[2 + length++] = ch;
                }
        }
    }

    str[1] = length;
    str[2 + length] = '\0';
    return &str[2];
}

void
highvideo (void)
{
    if (__FOREGROUND < DARKGRAY) textcolor(__FOREGROUND + 8);
}

void
lowvideo (void)
{
    if (__FOREGROUND > LIGHTGRAY) textcolor(__FOREGROUND - 8);
}

void
delay (int ms)
{
    Sleep(ms);
}

void
switchbackground (int color)
{
    struct char_info* buffer;
    int i;

    buffer = malloc(__text_info.screenwidth * __text_info.screenheight *
      sizeof(struct char_info));
    _conio_gettext(1, 1, __text_info.screenwidth, __text_info.screenheight,
      buffer);
    for (i = 0; i < __text_info.screenwidth * __text_info.screenheight; i++) {
        unsigned short attr = buffer[i].attr & 0xF;
        buffer[i].attr = (color << 4) | attr;
    }
    puttext(1, 1, __text_info.screenwidth, __text_info.screenheight, buffer);
    free(buffer);
}

void
flashbackground (int color, int ms)
{
    struct char_info* buffer;

    buffer = malloc(__text_info.screenwidth * __text_info.screenheight *
      sizeof(struct char_info));
    _conio_gettext(1, 1, __text_info.screenwidth, __text_info.screenheight,
      buffer);
    switchbackground(color);
    delay(ms);
    puttext(1, 1, __text_info.screenwidth, __text_info.screenheight, buffer);
    free(buffer);
}

void
clearkeybuf (void)
{
    while (kbhit()) {
        getch();
    }
}

#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------------------------------------------------

// *****************************************Declaration des structures **************************************************

typedef struct champrski champrski;
struct champrski //Champs du vecteur de reservation.
{ int deb;
  int fin;
  int taille ;
};
typedef struct rski rski;
struct rski // Structure pour le vecteur de reservation.
{ champrski tab[500];
 int nbres; // Nombre de reservation.
};

typedef struct VSkieur VSkieur;
struct VSkieur //Champs du vecteur skieur.
{
    char Nom[25];
    char adresse[50];
    int psk; //Nombre de paires de skis lou�es.
    rski *reservect; //Pointeur sur la structure contenant le vecteur de reservation .
};
typedef struct vecteur vecteur;
struct vecteur //Vecteur skieur.
 { VSkieur T[500];
   int nbr;
 };
typedef struct InfoSkis InfoSkis;
struct InfoSkis //Sous-liste de reservation.
{
    int deb;
    int fin;
    int ind;   /* indice d'elements du vecteur skieur*/
    InfoSkis *ressuiv;
};

typedef struct PaireDeSkis PaireDeSkis;
struct PaireDeSkis // Liste principale.
{
    int taille;
    InfoSkis* loc;
    PaireDeSkis* skisuiv;
};
//----Strucutre supplementaire pour l'affichage -------

typedef struct chaine chaine;
struct chaine
 {
     char  mots[100];
 };

//***********************************************Mod�le Llc revisit�***********************************************

void allouerski (PaireDeSkis **element)
{
   *element=malloc(sizeof(PaireDeSkis));
}
void allouerloc (InfoSkis **element)
{
    *element=malloc(sizeof(InfoSkis));
}
void aff_adrski(PaireDeSkis *p, PaireDeSkis *q)
{
    p->skisuiv=q;
}
void aff_adrloc(InfoSkis *p, InfoSkis *q)
{
    p->ressuiv=q;
}
void aff_valski(PaireDeSkis *p,int x)
{
    p->taille=x;
}
void aff_valloc(InfoSkis *p,int d, int f)
{
    p->deb=d;
    p->fin=f;
}
PaireDeSkis *suivantski(PaireDeSkis *p)
{
    return p->skisuiv;
}
InfoSkis *suivantloc(InfoSkis* p)
{
  return p->ressuiv;
}
int Taille(PaireDeSkis *p)
{
    return p->taille;
}

//*********************Chargement du stock (liste principale) avec 5 pour chaque taille [100->300]*********************
PaireDeSkis *chargementski()
{

    int i,z,t,n;
    int cpt=0;
    PaireDeSkis *P=NULL,*T=NULL,*S=NULL;
    allouerski(&P);
    aff_valski(P,100);
    P->loc=NULL;
    T=P;
    for(t=100;t<320;t+=20)
    {
        if (t==100)
            i=1;
        else
        i=0;
        while (i<5)
        {
            allouerski(&S);
            aff_valski(S,t);
            aff_adrski(P,S);// Liaison de S et P ie: de S et T(P �tant un pointeur de meme valeur que T apr�s la copie)
            P->loc=NULL;
            P=S; //Passer � l'element suivant
            i++;
        }

    }
    aff_adrski(P,NULL);// Fin de la Liste
    P->loc=NULL;
    return T;
}
/************ Affichage du stock***********/
void afficheski(PaireDeSkis *P)
{
    while (P!=NULL)
    {
        printf("%d->",Taille(P));
        P=suivantski(P);
    }

}

void afficheloc(InfoSkis* P)//Afiche la sous liste de reservation
{
    while (P!=NULL)
        {   textcolor(15);
            textbackground(3);
            printf("%d-%d",P->deb,P->fin);
            gotoxy(wherex()+2,wherey());//Procedure pour situer le curseur au moement de l'afichage.
            P=suivantloc(P);
        }
        printf("\n\n");
}

//***********************************************Parcours de Listes*******************************************************

int nbskis (PaireDeSkis* p)
{
    int cpt=0;
    while (p!=NULL)
    {
         cpt++;
        (p)=suivantski(p);
    }
    return cpt;
}
PaireDeSkis* skit(PaireDeSkis* p,int t)
{
    int i;
    PaireDeSkis* z=p;
    for(i=1;i<=nbskis(z);i++)
    {
        if (Taille(p)==t)
        {
            return p;
        }
        else p=suivantski(p);

    }
    if (p==NULL)
    {
        return NULL;
    }
}
int nbjloc(PaireDeSkis* p)
{
    int s=0;
     while(p->loc!=NULL && p!=NULL)
     {
         s=((p->loc->fin)-(p->loc->deb)+1)+s;
         (p->loc)=suivantloc(p->loc);
     }
     return s;
}
//************************************************Parcours de Vecteurs***************************************************
int chercheskieur(char mots[25],vecteur Tab) //Retourne l'indice, du client dans le vecteur de reservation
{
    int cpt,i;
    cpt=0;
    for(i=0;i<(Tab.nbr);i++)
     {
        if (strcmp(mots,Tab.T[i].Nom)==0)
        {
            return i;
            cpt++;
        }
     }
    if(cpt==0)
    {
        return Tab.nbr;
    }
}
void chargeindice(vecteur *v,char Nome[25],char adr[50] ,int i) //Charge le vecteur skieur.
{   printf("\n");    textbackground(6);textcolor(15);

    strcpy((v->T+i)->Nom,Nome);
    strcpy((v->T+i)->adresse,adr);
    v->nbr++;
}
//-----------------------------------------------------Insertion----------------------------------------------------------
//*******************Procedure verifiant si la location peut �tre r�alis�e, renvoyant un bool�en **************************
void testloc(PaireDeSkis* p,InfoSkis* q,int t, int d , int f,int *v,InfoSkis **actu,InfoSkis **suiv, PaireDeSkis **x)
{
    int res=0;
    PaireDeSkis *z=NULL;

    if (p!=NULL){
  do
  {
   if((q)!=NULL)
    {
    if (f<(q->deb))// Test sur le debut ie : (Insertion avant la t�te)
    {
        res=1;
        *actu=NULL;
        *suiv=q;
    }
    else
    {
        while((q)!=NULL)
        {
          if((suivantloc(q))!=NULL)
          {
            if (((q->fin)<d) && (f<(suivantloc(q)->deb))) //Teste sur le milieu ie: ( Inserer entre deux maillons)
            {
                res=1;
                *actu=q;
                *suiv=suivantloc(q);
            }
          }
            else
            {
              if((q->fin)<d)// Test sur la fin (Insertion apr�s la queue)
              {
                  res=1;
                  *actu=q;
                  *suiv=NULL;

              }
            }
            q=suivantloc(q);

        }
    }
    p=suivantski(p);
    p=skit(p,t);
    if (p!=NULL)
    q=p->loc;

   }
   else
   {
       z=p;
      res=1;
       *x=z;
   }

  }while(res==0 && p!=NULL);
    }
  *v=res;// Le bool�en
}
int location(PaireDeSkis* p,int t,int a, int b,char nom[25],vecteur vct)
{
    int v;
    InfoSkis *actu=NULL;
    InfoSkis *suiv=NULL;
    PaireDeSkis *w=NULL;
    InfoSkis* q=NULL;

    if (p!=NULL)
     q=p->loc;

    testloc(p,q,t,a,b,&v,&actu,&suiv,&w);
    if (v==1)// la location peut etre r�alis�
    {
        if (w!=NULL && w->loc==NULL)
        {

        insertete(w,a,b,actu,suiv,w,vct,nom);
        }
        else
        {
          insertete(p,a,b,actu,suiv,w,vct,nom);
        }
    }
    //Cas de v=0 pris en consideration dans la procedure "reservation".
    return v;
}
void insertete(PaireDeSkis* p, int a, int b, InfoSkis* actu, InfoSkis* suiv,PaireDeSkis *w,vecteur vct,char nom[25])
{

InfoSkis *S;

        if (w!=NULL && w->loc==NULL) //La sous_liste de reservation n'a pas encore �t� cr�e,ie: 1er location de la taille en question
        {
            allouerloc(&w->loc);
            aff_valloc(w->loc,a,b);
            aff_adrloc(w->loc,NULL);
            w->loc->ind=chercheskieur(nom,vct);

        }
        else
        {

            if (actu==NULL)// Inserer avant la t�te.
            {
                allouerloc(&S);
                aff_valloc(S,a,b);
                aff_adrloc(S,suiv);
                p->loc=S;
                p->loc->ind=chercheskieur(nom,vct);

            }
            else if(suiv==NULL)// Insertion au milieu
            {

               allouerloc(&S);
               aff_valloc(S,a,b);
               aff_adrloc(S,suiv);
               aff_adrloc(actu,S);
               S->ind=chercheskieur(nom,vct);

            }
            else // Insertion � la fin
            {
                allouerloc(&S);
                aff_valloc(S,a,b);
                aff_adrloc(S,suiv);
                aff_adrloc(actu,S);
                S->ind=chercheskieur(nom,vct);

            }
        }
}

//*******************************************R�s�rvation(plusieurs locations) ********************************************

void reservation(PaireDeSkis *p,int nombredereservation,char client[25] ,char adr[50],vecteur *vgrand )
{
    int i,taille,db,fn,cpt=0;
    int h=0;


    if (chercheskieur(client,*vgrand)==vgrand->nbr)// Si c'est un nouveau client alors l'inscrire dans le vecteur.
     {
        chargeindice(vgrand,client,adr,vgrand->nbr);
     }

    for(i=0;i<nombredereservation;i++)
     {  label1:textbackground(15);
         system("cls");
         bande();
         textcolor(0);
         gotoxy(15,5);
        textbackground(5);
        printf("Parametres de la reservation numero %d :\n",i+1);
        gotoxy(20,8);
        textbackground(7);
        printf("Taille : ");

        gotoxy(25,10);
        textbackground(7);
        printf("Debut :");

        gotoxy(25,13);
        textbackground(7);
        printf("Fin :");
        gotoxy(35,8);
       textbackground(6);
      scanf("%d",&taille);
      gotoxy(35,10);
      scanf("%d",&db);
      gotoxy(35,13);
      scanf("%d",&fn);


      if(location(skit(p,taille),taille,db,fn,client,*vgrand)==1)// Location r�alisable
       {

        cpt++;
        gotoxy(25,wherey()+2);
        textbackground(10);
        printf("R%cservation r%cussie\n",130,130);
        if (cpt==1)
        {

            (vgrand->T+chercheskieur(client,*vgrand))->reservect=malloc(sizeof(rski));//allouer un vecteur de reservation
        }

        (vgrand->T+chercheskieur(client,*vgrand))->reservect->nbres=cpt;//nombre de reservations
        ((vgrand->T+chercheskieur(client,*vgrand))->reservect->tab+cpt-1)->deb=db;//debut
        ((vgrand->T+chercheskieur(client,*vgrand))->reservect->tab+cpt-1)->fin=fn;//fin
        ((vgrand->T+chercheskieur(client,*vgrand))->reservect->tab+cpt-1)->taille=taille;//taille
       }

      else
       {
           if(skit(p,taille)==NULL)// Taille non r�pertori�e.
           {  gotoxy(35,wherey()+8); textbackground(4);
           textcolor(15);

               puts(" Desole,Il n'ya pas de paire de cette taille ");
           }
           else
           {  gotoxy(20,wherey()+2);
           textbackground(12);
           textcolor(15);
               puts(" Paire indisponible \n");
               affichedispo(p,taille);
               textbackground(8);
               textcolor(15);
               gotoxy(4,wherey()+2);
               printf("Voulez-vous reserver pour une date alternative ?\n\n Tapez 1 - oui\t\t 0 - non\n");
               gotoxy(wherex()+4,wherey()+1);
               scanf("%d",&h);
                if (h==1)
                {   textbackground(15);
                    system("cls");
                goto label1;


                }

           }
       }
                    gotoxy(30,wherey()+3);
                    textbackground(3);
                    system("pause");
    }
    if(cpt!=0)
    {
        ((vgrand->T+chercheskieur(client,*vgrand))->psk)= cpt ;//nombre de paires de ski dans le grand vecteur de clients
    }
    else{

    vgrand->nbr --;} //Le client n'a pas r�ussie a faire au moin une resrvation.
}
//*******************************************************Annulation ************************************************************
void annuleres(char client[50], vecteur *vgrand, PaireDeSkis* q)
{
    int i,j=0;
    InfoSkis* z=NULL;
    PaireDeSkis *m;
    InfoSkis *l,*tete,*suiv=NULL;
    m=q;


        while(q!=NULL && j<vgrand->T[chercheskieur(client,*vgrand)].reservect->nbres)
        {
            l=NULL;
            z=q->loc;
            while(z!=NULL && j<vgrand->T[chercheskieur(client,*vgrand)].reservect->nbres )
            {

                if (q->loc->ind==chercheskieur(client,*vgrand) && l==NULL)//Suppression au d�but
                {

                    j++;
                    q->loc=suivantloc(q->loc);
                    if(q->loc!=NULL)
                    aff_adrloc(q->loc,suivantloc(q->loc));

                }
                else
                {
                    l=z;
                    if(suivantloc(l)!=NULL && suivantloc(l)->ind==chercheskieur(client,*vgrand) )//Au milieu
                    {

                        j++;
                        suiv=suivantloc(l);
                        aff_adrloc(l,suivantloc(suivantloc(l)));
                        free(suiv);
                    }

                    else if(suivantloc(l)==NULL && l->fin==chercheskieur(client,*vgrand) )//A la fin
                    {
                            j++;
                            aff_adrloc(l,NULL);
                            free(q->loc);
                    }
                }

                z=suivantloc(z);
            }

                q=suivantski(q);

        }

while(m!=NULL)
{
    tete=m->loc;
    while(tete!=NULL)
    {
        if((tete->ind)>chercheskieur(client,*vgrand))
            {
                tete->ind--;
            }
        tete=suivantloc(tete);

    }

  m=suivantski(m);
}

    for(i=chercheskieur(client,*vgrand);i<vgrand->nbr;i++)// Decalage vers le haut dans le vecteur de reservation
    {
         vgrand->T[i]=vgrand->T[i+1];
    }
  vgrand->nbr--;
}
//********************* Affiche les jours disponible d'une paire de ski de taille t *************************************************
void affichedispo(PaireDeSkis *p,int taille)
{ int cpt,gpt ,i,y;InfoSkis *tetesousliste;
    cpt =1;
    gpt=1;
    InfoSkis* q=NULL;

    while(skit(p,taille)!=NULL)
    {
        textcolor(0);
    textbackground(15);
    q=p->loc;
    tetesousliste=q;

    p=skit(p,taille);
    q=p->loc;
   //si la sous-liste est vide on dit que le ski est libre de 1 a  151
    if((q)==NULL){ gotoxy(8,wherey());printf(" Le ski numero %d de taille %d est disponible du jour 1 au jour 151\n",cpt,taille);}
    // sinon on recherche les jours libres
    else
    {
            while((q)!=NULL){

           if(((q==tetesousliste))||(gpt==1))
           {gpt=0;
           if(((q->deb)>1)){ gotoxy(8,wherey());
           printf("Le ski numero %d de taille %d est disponible du jour 1 au jour %d \n",cpt,taille,((q->deb)-1));

                 }}



               if((suivantloc(q))!=NULL)
                {
                   if(  ((suivantloc(q)->deb)-(q->fin))>1)

                   {gotoxy(8,wherey());

                    printf("Le ski numero %d de taille %d est disponible du jour %d au jour %d \n",cpt,taille,((q->fin)+1),((suivantloc(q)->deb)-1));

                   }
               }
              else
              {

                  if(((q->fin)<151))
                {gotoxy(8,wherey());
                  printf("Le ski numero %d de taille %d est disponible du jour %d au jour 151 \n",cpt,taille,((q->fin)+1));
                }
              }


        (q)=suivantloc(q); // passer au maillon suivant de la sous-liste
      }

    }
    cpt++;//ccompte le le numero du ski de taille damand�e en court de traitment
      y=wherey()+1;
           for(i=0;i<16;i++){
        gotoxy(2*i+8,y);
        textbackground(15);
        textcolor(i);
        printf("%c",175);}
        printf("\n");
        textcolor(0);
        textbackground(15);

p=suivantski(p);// passe au maillon suivant de la liste
gpt=1;
}

}
//--------------------------------------Partie affichage -----------------------------------------------------------------------
int choix (void){
    int i,x=0,y=0,ch;
    chaine lechoix[]={ "      >>      Faire une reservation      <<      ",
                       "      >>      Annuler une reservation    <<      ",
                       "   >>      Afficher les jours disponibles   <<   ",
                       "   >>     Autres  fonctions et procedures   <<   ",
                       "            >>          Exit            <<       ",};

ch=1;
 do{
  for(i=0;i<5;i++)
    {if(i==ch){
        textbackground(11);
        textcolor(9);
         printdirection(lechoix[i].mots,"right",35,(22+i+1),0);
         }
     else{     textbackground(8);
               textcolor(0);
         printdirection(lechoix[i].mots,"right",35,(22+i+1),0);
         }

    }
    x=readkey();
    switch(x){
     case 336 : ch = (ch+1)%5;
                break;
     case 328: ch = (ch+4)%5;
               break;
     case 13 :
         y=1;
        break;

       }

 }while(y!=1);
 return ch+1;
                      }
 //-------------------------------------------------------------------------------------------------------------------
    int choix2 (void){
    int i,x=0,y=0,ch;
    chaine lechoix[]={ "   >>       Sous liste des reservations     <<   ",
                       "      >>        Liste des clients        <<      ",
                       "   >>             Nombre de skis            <<   ",
                       "   >>           Retour en arriere           <<   ",
                       "          >>          Exit            <<         ",};

ch=1;
 do{
  for(i=0;i<5;i++)
    {if(i==ch){
        textbackground(11);
        textcolor(9);
         printdirection(lechoix[i].mots,"right",35,(22+i+1),0);
         }
     else{     textbackground(7);
               textcolor(0);
         printdirection(lechoix[i].mots,"right",35,(22+i+1),0);
         }

    }
    x=readkey();
    switch(x){
     case 336 : ch = (ch+1)%5;
                break;
     case 328: ch = (ch+4)%5;
               break;
     case 13 :
         y=1;
        break;

       }

 }while(y!=1);
 return ch+1;
                      }
//--------------------------------------------------------------------------------------------------------------------------
void bande(void){
    int i;
   for(i=1;i<41;i++){
        textbackground(3);
    printdirection("                      ","right",98,i,0);
   }

   }
void tab(int x ,int y,vecteur *v){//Affichage du tableau Vskieur
   int i;
   system("cls");
        textcolor(15);
        textbackground(4);gotoxy(x+2,y);
        printf(" Client "); gotoxy(x+12,y);
    textbackground(8);
      printf("          NOM        ");
      gotoxy(x+40,y);
      printf(" Adresse  ");
      gotoxy(x+65,y);
      printf(" Nombre de paires louee ");
     delay(80);

    for(i=0;i<v->nbr;i++)

    { textbackground(1);gotoxy(x+2,y+2+2*i);
        printf("   %d   ",i+1); gotoxy(x+12,y+2+2*i);
    textbackground(8);
      printf("   %s      ",v->T[i].Nom);

     gotoxy(x+40,y+2+2*i);
     printf("    %s      ",v->T[i].adresse);
     gotoxy(x+65,y+2+2*i);
     printf("      %d     ",(v->T+i)->psk);
     delay(80);
    }
    gotoxy(wherex()-60,wherey()+4);
system("pause");
    }

void decalrectangle(int much)
{int y;
if(much==1){y=11;}
if(much==2){y=21;}
if(much==3){y=31;}
textbackground(15);
textcolor(15);

gotoxy(1,y-10);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y-9);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y-8);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y-7);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y-6);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y-5);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y-4);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y-3);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y-2);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y-1);
delay(10);
textbackground(3);
textcolor(3);
gotoxy(1,y);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+1);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+2);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+3);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+4);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+5);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+6);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+7);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+8);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+9);

}
void moverect(char mot[4])

{ int i,y;
    if (strcmp(mot,"up")==0){y=35;
    for(i=0;i<25;i++){
    gotoxy(1,y-i);
textcolor(3);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
if(i!=0)
{gotoxy(1,y-i+5);
textbackground(15);
textcolor(15);
  printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
 textbackground(3);
}delay(50);
}}
if(strcmp(mot,"down")==0)
{y=10;
    for(i=0;i<25;i++){
    gotoxy(1,y+i);
textcolor(3);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
if(i!=0)
{gotoxy(1,y+i);
textbackground(15);
textcolor(15);
  printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
 textbackground(3);
}delay(50);
}

}
}
 int readkey(void) //Lit une touche du clavier
{
    int key;
    clearkeybuf();
    key=getch();
    if (key==0){
        key=getch()+256;}
    if( key==224){
        key=getch()+256;}

    return key;
}


void blancln(int nombreln)
{ int i;
for(i=0;i<nombreln;i++){printf(" \n");}
}
void printdirection(char mot[120],char direction[6],int x ,int y ,int delai)//Impression dans une direction donn�e
{int i;

    if(strcmp(direction,"up")==0)
{for(i=0;i<strlen(mot);i++)
 {   gotoxy(x,y-i);
     printf("%c",mot[i]);
     delay(delai);

 }
}
    if(strcmp(direction,"down")==0)
    { for(i=0;i<strlen(mot);i++)
    {gotoxy(x,y+i);
     printf("%c",mot[i]);
     delay(delai);
    }

    }
    if(strcmp(direction,"right")==0)
    { for(i=0;i<strlen(mot);i++)
    {gotoxy(x+i,y);
    printf("%c",mot[i]);
    delay(delai);
    }}
    if(strcmp(direction,"left")==0)
    { for(i=0;i<strlen(mot);i++)
    {gotoxy(x-i,y);
    printf("%c",mot[strlen(mot)-i-1]);
    delay(delai);
    }

    }
textbackground(15);}

void  basemenu(void)
{int i,y,a=0,z,d,b;
system("mode con lines=40 cols=120");
system("cls");
system("COLOR F3");
        blancln(5);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
moverect("up");
textbackground(3);
textcolor(15);
y=7;
gotoxy(35,y);
 printf("%c%c%c%c%c   %c%c%c%c%c   %c%c%c%c%c%c%c%c   %c%c%c%c     %c%c%c%c   %c%c%c   %c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219); gotoxy(35,y+1);
 printf("%c%c%c%c%c%c %c%c%c%c%c%c   %c%c%c%c%c%c%c%c   %c%c%c%c%c    %c%c%c%c   %c%c%c   %c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219); gotoxy(35,y+2);
 printf("%c%c%c%c%c%c%c%c%c%c%c%c%c   %c%c%c        %c%c%c%c%c%c   %c%c%c%c   %c%c%c   %c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219); gotoxy(35,y+3);
 printf("%c%c%c%c%c%c%c%c%c%c%c%c%c   %c%c%c%c%c%c%c%c   %c%c%c%c%c%c%c  %c%c%c%c   %c%c%c   %c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219); gotoxy(35,y+4);
 printf("%c%c%c  %c%c%c  %c%c%c   %c%c%c%c%c%c%c%c   %c%c%c%c%c%c%c%c %c%c%c%c   %c%c%c   %c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219); gotoxy(35,y+5);
 printf("%c%c%c   %c   %c%c%c   %c%c%c        %c%c%c%c  %c%c%c%c%c%c%c   %c%c%c   %c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219); gotoxy(35,y+6);
 printf("%c%c%c       %c%c%c   %c%c%c%c%c%c%c%c   %c%c%c%c    %c%c%c%c%c   %c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219); gotoxy(35,y+7);
 printf("%c%c%c       %c%c%c   %c%c%c%c%c%c%c%c   %c%c%c%c     %c%c%c%c    %c%c%c%c%c%c%c ",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
gotoxy(30,20);
textcolor(3);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(30,22);delay(50);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(30,23);delay(50);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(30,24);delay(50);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(30,25);delay(50);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(30,26);delay(50);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(30,27);delay(50);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(30,28);delay(50);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(30,30);delay(50);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);
textcolor(3);
z=86;
d=20;
textbackground(15);
b=0;  textbackground(b);

printdirection("                                                     \n","left",z,d+2,0);textbackground(b);
printdirection("                                                     \n","left",z,d+3,0);textbackground(b);
printdirection("                                                     \n","left",z,d+4,0);textbackground(b);
printdirection("                                                     \n","left",z,d+5,0);textbackground(b);
printdirection("                                                     \n","left",z,d+6,0);textbackground(b);
printdirection("                                                     \n","left",z,d+7,0);textbackground(b);
printdirection("                                                     \n","left",z,d+8,0);

}
//-----------------------------------------------------------------------------------------------
void intro(void){int y;
system("mode con lines=40 cols=120");
system("COLOR F3");
 y=1;
 textcolor(3);
  gotoxy(1,y);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+1);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+2);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+3);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+4);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+5);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+6);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+7);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+8);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);gotoxy(1,y+9);
delay(500);
  decalrectangle(1);
  //**********************************************************************************************************************
    textbackground(15);

        printdirection("Ecole Nationale Superieur en Informatique d'Alger ","right",40,1,45);

        printdirection("Annee Universitaire 2016/2017","right",49,2,45) ;
        decalrectangle(2);
textbackground(15);
gotoxy(35,10);
 printf(" %c%c%c%c%c%c%c%c%c%c             %c%c%c%c%c%c%c%c%c           %c%c%c%c\n",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219) ;delay(50);gotoxy(35,11);
 printf("     %c%c                 %c%c     %c%c             %c%c\n",219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219,219);delay(50);gotoxy(35,12);
 printf("     %c%c                 %c%c     %c%c             %c%c\n",219,219,219,219,219,219,219,219,219,219);delay(50);gotoxy(35,13);
 printf("     %c%c                 %c%c%c%c%c%c%c%c%c             %c%c\n",219,219,219,219,219,219,219,219,219,219,219,219,219);delay(50);gotoxy(35,14);
 printf("     %c%c                 %c%c                    %c%c\n",219,219,219,219,219,219,219);delay(50);gotoxy(35,15);
 printf("     %c%c                 %c%c                  %c%c%c%c%c%c\n",219,219,219,219,219,219,219,219,219,219,219);
delay(300);
 decalrectangle(3);
textbackground(15);
textcolor(0);
 gotoxy(14,20);

printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",201,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,187);gotoxy(14,21);
printf("%c                                                                                            %c\n",186,186);gotoxy(14,22);
printf("%c                                                                                            %c\n",186,186);gotoxy(14,23);
printf("%c                                                                                            %c\n",186,186);gotoxy(14,24);
printf("%c                                                                                            %c\n",186,186);gotoxy(14,25);
printf("%c                                                                                            %c\n",186,186);gotoxy(14,26);
printf("%c                                                                                            %c\n",186,186);gotoxy(14,27);
printf("%c                                                                                            %c\n",186,186);gotoxy(14,28);
textbackground(15);
printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",200,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,205,188);
printdirection(">","down",20,22,60);
printdirection("Realise par : ","left",35,22,60);

printdirection(" d ","down",60,23,60);
printdirection("sa ","down",59,23,60);
printdirection("eu ","down",58,23,60);
printdirection("yo4 ","down",57,23,60);
printdirection("LF0 ","down",56,23,60);
printdirection("||| ","down",54,23,60);
printdirection("n e ","down",52,23,60);
printdirection("u p ","down",51,23,60);
printdirection("oiu ","down",50,23,60);
printdirection("rdo ","down",49,23,60);
printdirection("mar ","down",48,23,60);
printdirection("AKG ","down",47,23,60);
printdirection(" | Semestre 02","right",80,24,30);
printdirection("> Encadree par :       Dr M.Boulakradeche ","right",20,27,45);
textbackground(15);
gotoxy(wherex()-14,wherey()+3);
system("pause");
}
void fin(void){
    system("COLOR F3");
    system("cls");
    decalrectangle(1);
    textbackground(15);
    textcolor(0);
    printdirection("           Nous vous remercions d'avoir utiliser notre programme    ","right",20,2,30 );
delay(1000);
decalrectangle(2);
decalrectangle(3);
gotoxy(15,10);
    textbackground(3);
    textcolor(15);
    printdirection("Pour toute remarque contactez nous sur :","right",wherex()+25,wherey(),20);textbackground(3);
    printdirection("       ","down",60,wherey()+4,15);
     textbackground(0);textcolor(15);
    printdirection(" Amroun Lyes ","right",24,16,15);
        textbackground(15);textcolor(0);
     printdirection(" e-mail: gl_amroun@esi.dz ","right",17,18,15);
     textbackground(0);textcolor(15);
    printdirection(" Kadi Fouad ","right",85,16,15);
        textbackground(15);textcolor(0);
     printdirection(" e-mail: gf_kadi@esi.dz ","right",77,18,15);


       gotoxy(wherex(),wherey()+2);
       textcolor(15);
       system("pause");





}
//-------------------------------------------------Affichage "partie switch"---------------------------------------------------------------
void menu(PaireDeSkis *p,vecteur *v)
{int taile,nb,paire; PaireDeSkis* q; char nom[25];char adr[50];
    p=chargementski();
    deb:
system("cls");
basemenu();

switch(choix()){
    case 1: system("cls");
    decalrectangle(3);
            textcolor(15);
            gotoxy(15,4);
            textbackground(3);
            puts("> Votre nom s.v.p : ") ;gotoxy(20,10);
            puts("> Votre adresse s.v.p : ");
            gotoxy(20,16);
            puts("> Nombre de reservations :");
            gotoxy(50,4);
            textbackground(15);
            textcolor(0);
            scanf("%s",&nom);
            gotoxy(50,10);
            scanf("%s",&adr);

            gotoxy(50,16);

            textbackground(15);
            scanf("%d",&nb);
            reservation(p,nb,nom,adr,v);
            textbackground(3);
            textcolor(15);
            printdirection("<< **** On vous souhaite un bon sejour **** >>","right",30,wherey()+4,15);
            gotoxy(30,wherey()+3);
            textbackground(15);
            textcolor(0);
            system("pause");
            break;
    case 2: system("cls");
             bande();
            gotoxy(46,20);
            textbackground(4);
            textcolor(15);
            printdirection(" Votre nom s.v.p :","right",35,15,20);
             textbackground(15);
            textcolor(3);
            gotoxy(wherex()+3,wherey());
            scanf("%s",&nom);
            if(v->T[chercheskieur(nom,*v)].psk!=0)
            annuleres(nom,v,p);
            else printf("  \n\n\tClient inexistant  ");
            textbackground(15);
            textcolor(3);
            printdirection(" <<** Votre demande a etait enregistree avec succes","right **>>",30,30,0);
            gotoxy(30,35);
            system("pause");

        break;
    case 3: system("cls");
    bande();
           textbackground(3);
           textcolor(15);
           printdirection("Introduire la taille :","right",10,2,15);
           gotoxy(wherex()+2,wherey());
            textbackground(15);
            textcolor(8);
           scanf("%d",&taile);
           gotoxy(wherex(),wherey()+2);
           affichedispo(p,taile);
           gotoxy(30,36);
           textbackground(3);
           textcolor(0);
           system("pause");
    break;
    case 4: deb2:
            system("cls");
            basemenu();
                switch(choix2()){
                                 case 1: system("cls");
                                 decalrectangle(3);
                                 textbackground(11);
                                 textcolor(15);
                                 printdirection(" Introduire la taille du ski :","right",5,2,15);
                                 gotoxy(wherex()+2,2);textcolor(11);
                                  scanf("%d",&paire);
                                 q=p;
                                 q=skit(q,paire);
                                 while(q!=NULL){
                                      gotoxy(1,wherey()+2);
                                      afficheloc(q->loc);
                                      q=suivantski(q);
                                      q=skit(q,paire);}
                                      gotoxy(15,wherey()+3);
                                      system("pause");


                                 break;
                                 case 2: tab(10,6,v) ;
                                         break;
                                 case 3: system("cls");
                                      moverect("down");
                                     gotoxy(45,10);
                                    textbackground(9);
                                    printf(" Le nombre de ski est de :  %d\n",nbskis(p));
                                    textbackground(15);
                                    gotoxy(15,15);
                                    system("pause");
                                    break;
                                case 4: goto ret;
                                case 5: goto end ;

                        }
                        goto deb2;
                 ret:
            break;
    case 5: goto end ;


}
goto deb;
end: fin() ;

}
int main()
{
system("mode con lines=40 cols=120");
    vecteur  *vo;int paire;
    PaireDeSkis *q;
vo=malloc(sizeof(vecteur));
      vo->nbr=0;
    PaireDeSkis*mat=chargementski();
    intro();
    menu(mat,vo);
    return 0;
}
