
/* -------------------------------------------------------------------- */
/*   Quick hack to get a GEM conform Mandelbrot                         */
/*   My first "real" GEM program to be honest, so bear with me ;)       */
/*                                                                      */
/*   V1.0 - Axel Muhr                                                   */
/* -------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#include <aes.h>
#include <vdi.h>

#include <ext.h>
#include <tos.h>
#include <math.h>
#include <time.h>

#include <setjmp.h>  /* Transputer handling stuff */

#include "trproc.h"
#include "scancode.h"

/* -------------------------------------------------------------------- */
/*       Konstantendefinitionen.                                        */
/* -------------------------------------------------------------------- */

#define DESK                  0

#define W_KIND                NAME|CLOSER|FULLER|MOVER|SIZER
#define EV_KIND               MU_MESAG|MU_KEYBD

#define WINDOW_NAME           " GEM Transputer Mandelbrot "

#define min(a, b)             ((a) < (b) ? (a) : (b))
#define max(a, b)             ((a) > (b) ? (a) : (b))

/* -------------------------------------------------------------------- */
/*    Typendefinition.                                                  */
/* -------------------------------------------------------------------- */

typedef enum
{
    FALSE,
    TRUE
} 
boolean;

/* -------------------------------------------------------------------- */
/*    Funktionsprototypen.                                              */
/* -------------------------------------------------------------------- */

void    gem_prg( void );
void    hndl_window( void );
void    do_redraw( int w_handle, int x, int y, int w, int h );
void    new_size( int w_handle, int x, int y, int w, int h );
void    close_window( int w_handle );
void    clipping( int x, int y, int w, int h, int mode );
void 	mandelbrot(void);
void 	t_mandelbrot(void);

boolean rc_intersect( GRECT *r1, GRECT *r2 );
boolean open_window( void );

/* -------------------------------------------------------------------- */
/*       Global variables                                           */
/* -------------------------------------------------------------------- */

int     Msgbuff[8];                 /* Buffer fr Mitteilungen.         */

int     W_handle,                   /* Variablen zum Arbeiten mit einem */
        Wx, Wy, Ww, Wh;             /* Fenster - bei mehreren Fenstern  */
boolean W_fulled;                   /* ist ein Feld mit diesen Variabl. */
                                    /* zu vereinbaren.                  */
int     pxarray[128];               /* Feld fr Clipkoordinaten etc.    */
boolean Done = FALSE;               /* 'Mach weiter'-Flag.              */

/* This is the Transputer code */
static char mandel_t8[] = {
    0x46,0x21,0xb8,0x90,0x60,0x49,0x21,0xfb,
    0xda,0x24,0xf2,0xd9,0x71,0x79,0xf4,0xd8,
    0x79,0x21,0xfc,0x79,0x21,0xf8,0x40,0xd1,
    0x4b,0xd2,0x79,0x71,0x79,0xfa,0xe0,0x11,
    0x49,0x22,0xf1,0x60,0x4c,0x73,0xfa,0xd4,
    0x22,0xfa,0xd5,0x22,0xf9,0xc0,0xd6,0x25,
    0xf7,0x7a,0x79,0xf4,0xd7,0x40,0x25,0xf4,
    0x12,0x73,0x41,0x23,0xf4,0xf7,0x19,0x73,
    0x72,0xf7,0x40,0x19,0xf2,0xb2,0xf6,0xcc,
    0x00,0x00,0x00,0x61,0xbd,0x21,0x78,0x21,
    0x2a,0x40,0x25,0xf2,0x21,0xd2,0x21,0x2f,
    0x2f,0x2f,0x2f,0x4f,0x24,0xf2,0xf2,0x42,
    0xf0,0x23,0xfb,0x2f,0x2f,0x2f,0x2f,0x4f,
    0x24,0xf2,0xf2,0x41,0xf0,0x23,0xfb,0x21,
    0x2f,0x2f,0x2f,0x2f,0x4f,0x24,0xf2,0xf2,
    0xf1,0xc2,0xa8,0x21,0x2f,0x2f,0x2f,0x2f,
    0x48,0xdc,0x06,0x2f,0x2f,0x2f,0x2f,0x48,
    0xdc,0x1b,0x21,0x74,0x44,0xf7,0x1a,0x21,
    0x74,0x44,0xf7,0x21,0x10,0x21,0x74,0x44,
    0xf7,0x21,0x70,0x46,0x25,0xf2,0xde,0x7e,
    0x44,0xf8,0xdd,0x21,0x78,0x7d,0x25,0xf2,
    0x21,0xd1,0x21,0x72,0x21,0x71,0xf9,0xa4,
    0x21,0x72,0x21,0xd1,0x21,0x71,0x7b,0x25,
    0xf2,0xd9,0x7c,0x79,0xf4,0xdf,0x21,0x71,
    0x24,0xf2,0xf2,0xd8,0x78,0x21,0x74,0x7b,
    0xf7,0x7f,0x44,0x22,0xfc,0xd7,0x79,0x24,
    0xf2,0xf2,0xd6,0x21,0x71,0x7d,0xf4,0xd5,
    0x7d,0x44,0x22,0xfc,0xd4,0x75,0x24,0xf2,
    0xf2,0xd3,0x21,0x70,0x73,0xfa,0xd2,0x76,
    0xd1,0x77,0x44,0xf8,0xd0,0x21,0x74,0x72,
    0xe1,0x21,0x75,0x72,0xe2,0x71,0x72,0xe3,
    0x77,0x72,0xe4,0x7a,0x78,0xf2,0x74,0x73,
    0xfa,0x48,0x21,0xfb,0xf0,0x23,0xfc,0x60,
    0xba,0xd5,0xd0,0xf6,0x71,0x23,0xfc,0x21,
    0xf5,0x21,0xb3,0x22,0xf0,0x50,0x50,0xb4,
    0x01,0x00,0x00,0x84,0x01,0x00,0x00,0x32,
    0x00,0x00,0x00,0x25,0xf7,0x21,0x27,0x9f,
    0x20,0x20,0x20,0x20,0x20,0x21,0xf5,0x12,
    0x73,0x52,0x44,0xf7,0x11,0x73,0x52,0x44,
    0xf7,0x71,0x24,0x20,0x40,0x24,0xfd,0x72,
    0x23,0x20,0x40,0x21,0xf3,0x24,0x20,0x40,
    0xf8,0x21,0x20,0x20,0x20,0x40,0x24,0xf2,
    0xfa,0xf2,0x24,0xf2,0x71,0xfb,0x62,0x07,
    0x10,0x41,0x24,0xfb,0x23,0xf9,0x21,0xf5,
    0x16,0xf3,0x21,0x1f,0x44,0x24,0xf2,0xfa,
    0x44,0xf7,0x21,0x1e,0x44,0x24,0xf2,0xfa,
    0x44,0xf7,0x21,0x1b,0x44,0x24,0xf2,0xfa,
    0x48,0xf7,0x21,0x19,0x44,0x24,0xf2,0xfa,
    0x48,0xf7,0x21,0x17,0x44,0x24,0xf2,0xfa,
    0x48,0xf7,0x21,0x15,0x44,0x24,0xf2,0xfa,
    0x48,0xf7,0x21,0x1d,0x44,0x24,0xf2,0xfa,
    0x44,0xf7,0x21,0x1b,0x21,0x19,0x28,0xfa,
    0x28,0xfa,0x28,0xf9,0x21,0x7f,0x60,0x8f,
    0xdc,0x1c,0x29,0xf8,0x28,0xfc,0x21,0x13,
    0x28,0xf4,0x21,0x15,0x21,0x17,0x28,0xfa,
    0x28,0xfa,0x28,0xf9,0x21,0x7e,0x60,0x8f,
    0xdc,0x1c,0x29,0xf8,0x28,0xfc,0x21,0x11,
    0x28,0xf4,0x21,0x77,0xdd,0x21,0x78,0xde,
    0x40,0xdb,0x21,0x7e,0xdc,0x7c,0x40,0xf9,
    0x2c,0xa1,0x21,0x7b,0xdf,0x21,0x7c,0x21,
    0xd0,0x40,0xd9,0x21,0x7f,0xda,0x7a,0x40,
    0xf9,0x29,0xa4,0x2a,0xf0,0x17,0x28,0xf4,
    0x77,0xd5,0x78,0xd6,0x75,0xd3,0x76,0xd4,
    0x73,0xd1,0x74,0xd2,0x40,0xd0,0x15,0x11,
    0x28,0xfa,0x2a,0xf6,0x22,0x71,0x28,0xfa,
    0x29,0xf4,0xc0,0x24,0xa0,0x21,0x7d,0x70,
    0xf9,0x23,0xaa,0x17,0x13,0x28,0xfa,0x2a,
    0xf8,0x13,0x28,0xf4,0x13,0x13,0x28,0xfa,
    0x2a,0xf6,0x1d,0x2a,0xf6,0x13,0x28,0xf4,
    0x11,0x15,0x28,0xfa,0x28,0xfa,0x28,0xf9,
    0x1f,0x2a,0xf6,0x17,0x28,0xf4,0x17,0x17,
    0x28,0xfa,0x2a,0xf8,0x15,0x28,0xf4,0x13,
    0x13,0x28,0xfa,0x2a,0xf8,0x11,0x28,0xf4,
    0x70,0x81,0xd0,0x64,0x01,0x7b,0x23,0x20,
    0x40,0x21,0xf3,0x24,0x20,0x40,0xf8,0x79,
    0x24,0x20,0x40,0x21,0xf3,0xf5,0x21,0x20,
    0x20,0x20,0x40,0x24,0xf2,0xfa,0xf2,0x70,
    0x21,0x20,0x40,0x21,0xf3,0xf0,0x23,0xfb,
    0x1f,0x21,0x13,0x28,0xfa,0x2a,0xf6,0x1f,
    0x28,0xf4,0x19,0x29,0x44,0x22,0xf1,0x22,
    0x70,0x52,0x7b,0xff,0x22,0x70,0x52,0x21,
    0x7f,0xff,0x21,0x11,0x1d,0x28,0xfa,0x28,
    0xfa,0x28,0xf9,0x1d,0x28,0xf4,0x1b,0x2c,
    0x41,0x22,0xf1,0x22,0x1b,0xf3,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x10,0x40,0x60,
    0xbc,0x60,0x42,0x21,0xfb,0xd3,0x24,0xf2,
    0xd2,0x42,0xd1,0x21,0x4a,0x21,0xfb,0xd0,
    0x10,0x60,0xdd,0x61,0x26,0x4f,0x21,0xfb,
    0x60,0xd9,0x60,0x1a,0x23,0xf9,0x73,0x60,
    0xd6,0x10,0x60,0xd5,0x62,0xb5,0x61,0x29,
    0x01,0xb4,0x22,0xf0,0x50,0x50,0x50
};

/* -------------------------------------------------------------------- */
/*       Extern definierte globale Variable.                            */
/* -------------------------------------------------------------------- */

extern int handle;
extern int gl_wbox,
           gl_hbox;

/* -------------------------------------------------------------------- */
/*       void gem_prg( void );                                          */
/*                                                                      */
/*       Beispiel fr die Verwendung der Funktion evnt_multi( ), sowie  */
/*       diverser Funktionen der wind-Bibliothek in einem GEM-Programm. */
/*                                                                      */
/*       ->                      Nichts.                                */
/*                                                                      */
/*       <-                      Nichts.                                */
/* -------------------------------------------------------------------- */

void gem_prg( void )
{
   int      event,                  /* Ergebnis mit Ereignissen.        */
            mx, my,                 /* Mauskoordinaten.                 */
            mbutton,                /* Mausknopf.                       */
            mstate,                 /* Status des Mausknopfs.           */
            keycode,                /* Scancode einer Tastatureingabe.  */
            mclicks;                /* Anzahl Mausklicks.               */

   vsf_color( handle, 1 );          /* Farbindex setzen.                */
   vsf_interior( handle, 3 );       /* Schraffierte Muster zeichnen.    */

   if ( open_window( ) == TRUE )    /* Ein Fenster zu Beginn ”ffnen.    */
   {
      do
      {
         event = evnt_multi( EV_KIND,
                             1, 1, 1,
                             0, 0, 0, 0, 0,
                             0, 0, 0, 0, 0,
                             Msgbuff,
                             0, 0,  /* Evnt-Timer rausgenommen,1 s warten.*/
                             &mx, &my,
                             &mbutton, &mstate,
                             &keycode, &mclicks);

         wind_update( BEG_UPDATE ); /* Jetzt darf GEM nicht mehr        */
                                    /* alleine zeichnen.                */
         if ( event & MU_MESAG )    /* Auswertung der Ereignisse.       */
         {
            if ( Msgbuff[0] >= WM_REDRAW && WM_NEWTOP >= Msgbuff[0] )
               hndl_window( );      /* Fensteraktion.                   */
         }
         /* else if ( event & MU_TIMER ) 
         {                            
            style++;                 
            if ( style == 13 )
               style = 1;
            vsf_style( handle, style );
            do_redraw( W_handle, Wx, Wy, Ww, Wh );

         } */
         else if( event & MU_KEYBD )
         {
            if ( keycode == CNTRL_C ) /* Mit Control-C kann beendet     */
            {                         /* werden.                        */
               if ( W_handle != -1 )
                  close_window( W_handle );
               Done = TRUE;
            }
         }
         wind_update( END_UPDATE );
      }
      while ( !Done );
   }
}

/* -------------------------------------------------------------------- */
/*       void hndl_window( void )                     handle windows    */
/*                                                                      */
/*       Behandlen der Windowaktionen.                                  */
/*                                                                      */
/*       ->                      Nichts.                                */
/*                                                                      */
/*       <-                      Nichts.                                */
/* -------------------------------------------------------------------- */

void hndl_window( void )
{

   switch( Msgbuff[0] )
   {
      case WM_REDRAW:               /* Fensterinhalt neu zeichen.       */
         do_redraw( Msgbuff[3], Msgbuff[4], Msgbuff[5], Msgbuff[6], Msgbuff[7] );
      break;

      case WM_CLOSED:               /* Fenster wurde geschlossen.       */
         close_window( W_handle );  /* Programm beendet.                */
         Done = TRUE;
      break;

      case WM_MOVED:                /* Fenster wurde bewegt oder in     */
      case WM_SIZED:                /* seiner Gr”že ver„ndert.          */
         if ( Msgbuff[6] < 10 * gl_wbox )  /* Mindestgr”že fr Fenster. */
            Msgbuff[6] = 10 * gl_wbox;
         if ( Msgbuff[7] < 5 * gl_hbox )
            Msgbuff[7] = 5 * gl_hbox;
         new_size( Msgbuff[3], Msgbuff[4], Msgbuff[5], Msgbuff[6], Msgbuff[7] );
      break;

      case WM_TOPPED:               /* Fenster (wurde) aktiviert.       */
      case WM_NEWTOP:
         wind_set( Msgbuff[3], WF_TOP, 0, 0, 0, 0 );
         wind_get( Msgbuff[3], WF_WORKXYWH, &Wx, &Wy, &Ww, &Wh );
      break;

      case WM_FULLED:               /* Fenster zoomen.                  */
         if (( W_fulled ^= TRUE ))
            wind_get( Msgbuff[3], WF_FULLXYWH, &Msgbuff[4], &Msgbuff[5], &Msgbuff[6], &Msgbuff[7] );
         else
            wind_get( Msgbuff[3], WF_PREVXYWH, &Msgbuff[4], &Msgbuff[5], &Msgbuff[6], &Msgbuff[7] );
         wind_get( Msgbuff[3], WF_WORKXYWH, &Wx, &Wy, &Ww, &Wh );
         new_size( Msgbuff[3], Msgbuff[4], Msgbuff[5], Msgbuff[6], Msgbuff[7] );
      break;
   }
}

/* -------------------------------------------------------------------- */
/*       boolean open_window( void );                                   */
/*                                                                      */
/*       ™ffnen eines Fensters. Bei erfolgreichem ™ffnen wird TRUE zu-  */
/*       rckgeliefert.                                                 */
/*                                                                      */
/*       ->                      Nichts.                                */
/*                                                                      */
/*       <-                      TRUE  falls ein Fenster ge”ffnet wer-  */
/*                                     konnte (dann wurden auch dessen  */
/*                                     globale Parameter gesetzt),      */
/*                               FALSE sonst.                           */
/* -------------------------------------------------------------------- */

boolean open_window( void )
{
   int new,
       xdesk, ydesk, wdesk, hdesk;
                                    /* Gr”že Arbeitsfl„che des Desktop. */
   wind_get( DESK, WF_WORKXYWH, &xdesk, &ydesk, &wdesk, &hdesk );
   if (( new = wind_create( W_KIND, xdesk, ydesk, wdesk, hdesk )) < 0 )
   {
      form_alert( 1, "[3][Kann kein Fenster ”ffnen.][Abbruch]" );
      return ( FALSE );
   }
   graf_mouse( M_OFF, 0 );          /* Maus aus.                        */
   wind_set( new, WF_NAME, WINDOW_NAME, 0, 0 ); /* Fenstername setzen.        */
                                    /* ™ffnendes Rechteck zeichnen.     */
   graf_growbox( 0, 0, 0, 0, xdesk, ydesk, xdesk + wdesk - 1, ydesk + hdesk - 16 );
   wind_open( new, xdesk, ydesk, xdesk + wdesk - 1, ydesk + hdesk - 16 );

                                    /* Fenster ”ffnen.                  */
                                    /* Arbeitsfl„che des Fensters best. */
   wind_get( new, WF_WORKXYWH, &Wx, &Wy, &Ww, &Wh );
   W_handle = new;
   W_fulled = FALSE;
   graf_mouse( M_ON, 0 );           /* Maus an.                         */
   graf_mouse( ARROW, 0 );

   return ( TRUE );                 /* ERFOLG!                          */
}

/* -------------------------------------------------------------------- */
/*       void do_redraw( int w_handle, int x, int y, int w, int h );    */
/*                                                                      */
/*       Neu zeichnen des Inhalts eines Fensters.                       */
/*                                                                      */
/*       -> w_handle             Handle des zu schlieženden Fensters.   */
/*          x, y          x-,y-Koordinate des Zeichenbereichs.          */
/*          w, h          H”he und Breite des Zeichenbereichs.          */
/*                                                                      */
/*       <-                      Nichts.                                */
/* -------------------------------------------------------------------- */

void do_redraw( int w_handle, int x, int y, int w, int h )
{
   GRECT t1, t2;
   
   clock_t start, end;
   double duration;
   char buf[100];

   t2.g_x     = pxarray[0] = x;     /* Zeichenkoordinaten setzen.       */
   t2.g_y     = pxarray[1] = y;
   t2.g_w     = w;
   t2.g_h     = h;
   pxarray[2] = x + w - 1;
   pxarray[3] = y + h - 1;

   graf_mouse( M_OFF, 0 );
   wind_get( w_handle, WF_FIRSTXYWH, &t1.g_x, &t1.g_y, &t1.g_w, &t1.g_h );
   while ( t1.g_w && t1.g_h )
   {
      if ( rc_intersect( &t2, &t1 ) == TRUE )
      {                             /* Nur berechnetes Rechteck         */
                                    /* neu zeichen, sonst k”nnten       */
                                    /* andere Bildschirminhalte         */
                                    /* zerst”rt werden.                 */
         clipping( t1.g_x, t1.g_y, t1.g_w, t1.g_h, TRUE );
         /* v_bar( handle, pxarray ); */
      }
      wind_get( w_handle, WF_NEXTXYWH, &t1.g_x, &t1.g_y, &t1.g_w, &t1.g_h );
   }

   	start = clock();
		t_mandelbrot();
	end = clock();


	duration = ((double)(end - start) / 200);
	vswr_mode(handle, MD_TRANS);
	sprintf(buf, "[1][Done!|Calculation took %.3f seconds][Ok]", duration);
	form_alert( 1, buf );
		
   clipping( t1.g_x, t1.g_y, t1.g_w, t1.g_h, FALSE );

   graf_mouse( M_ON, 0 );
}

/* -------------------------------------------------------------------- */
/*       void new_size( int w_handle, int x, int y, int w, int h );     */
/*                                                                      */
/*       Fenster an neue Gr”že anpassen.                                */
/*                                                                      */
/*       -> w_handle             Fensterhandle.                         */
/*          x, y, w, h           Auženkoordinaten des Fensters.         */
/*                                                                      */
/*       <-                      Die globalen Variablen, die die Koor-  */
/*                               dinaten der Fensterarbeitsfl„che ent-  */
/*                               halten, werden updated.                */
/* -------------------------------------------------------------------- */

void new_size( int w_handle, int x, int y, int w, int h )
{
   wind_set( w_handle, WF_CURRXYWH, x, y, w, h );
   wind_get( w_handle, WF_WORKXYWH, &Wx, &Wy, &Ww, &Wh );
}

/* -------------------------------------------------------------------- */
/*       void close_window( int w_handle );                             */
/*                                                                      */
/*       Schliežen eines Fensters.                                      */
/*                                                                      */
/*       -> w_handle             Handle des zu schlieženden Fensters.   */
/*                                                                      */
/*       <-                      Nichts.                                */
/* -------------------------------------------------------------------- */

void close_window( int w_handle )
{
   int x, y, w, h;

   wind_get( w_handle, WF_CURRXYWH, &x, &y, &w, &h );
   graf_shrinkbox( 0, 0, 0, 0, x, y, w, h );
   wind_close( w_handle );
   wind_delete( w_handle );
}

/* -------------------------------------------------------------------- */
/*       boolean rc_intersect( GRECT *r1, GRECT *r2 );                  */
/*                                                                      */
/*       Berechnung der Schnittfl„che zweier Rechtecke.                 */
/*                                                                      */
/*       -> r1, r2               Pointer auf Rechteckstruktur.          */
/*                                                                      */
/*       <-                      TRUE  falls sich die Rechtecke         */
/*                                     schneiden,                       */
/*                               FALSE sonst.                           */
/* -------------------------------------------------------------------- */

boolean rc_intersect( GRECT *r1, GRECT *r2 )
{
   int x, y, w, h;

   x = max( r2->g_x, r1->g_x );
   y = max( r2->g_y, r1->g_y );
   w = min( r2->g_x + r2->g_w, r1->g_x + r1->g_w );
   h = min( r2->g_y + r2->g_h, r1->g_y + r1->g_h );

   r2->g_x = x;
   r2->g_y = y;
   r2->g_w = w - x;
   r2->g_h = h - y;

   return ( (boolean) ((w > x) && (h > y) ) );
}

/* -------------------------------------------------------------------- */
/*       void clipping( int x, int y, int w, int h, int mode );         */
/*                                                                      */
/*       Last but not least: (Re-)Set Clipping Rectangle ...            */
/*                                                                      */
/*       -> x                    x-Koordinate der Clipp' Begrenzung.    */
/*          y                    y-Koordinate       - " -          .    */
/*          w                    Breite des begrenzten Rechtecks.       */
/*          h                    H”he         - " -             .       */
/*          mode                 TRUE  - clipping on,                   */
/*                               FALSE - clipping off.                  */
/*                                                                      */
/*       <-                      Nichts.                                */
/* -------------------------------------------------------------------- */

void clipping( int x, int y, int w, int h, int mode )
{
   pxarray[0] = x;
   pxarray[1] = y;
   pxarray[2] = x + w - 1;
   pxarray[3] = y + h - 1;

   vs_clip( handle, mode, pxarray );
}

/* -------------------------------------------------------------------- */
/*       Ende der Beispielprogramms fr EVNT_MULTI( ).                  */
/* -------------------------------------------------------------------- */

void t_mandelbrot(void)
{
  char linebuffer[1024];
  int x, y, farbe;
  int check_result;
  char double64[8];		/* a buffer for IEEE64 conversion */
  double left,right,top,bottom;                   /* Parameter */
  int maxiterations;
  int pxy[2];
  char a_buf[100];
	
  left=-2.0; 			/* Default values */
  right=1.0;                   
  top=1.125; 
  bottom=-1.125;
  maxiterations=32;
  
  
  check_result = checkTransputer(1);
  
  if (check_result == 4) {
	puttrraw(mandel_t8, (sizeof(mandel_t8)));  
  } else {
	  	sprintf(a_buf, "[2][No Transputer found at 0x%08X :-(|Exiting...][Ok]", base);
	form_alert( 1, a_buf );
	exit(0);
  }
	
  /* Sending the initial data to the Mandelbrot algorithm */
  
  puttrint(Ww); /* hardcoded for testing*/
  puttrint(Wh);
  /* we have to convert the Pure C 80bit IEEE doubles 
     into the Transputers 64bit doubles.
	 'xdcnf' does that for us...     */
  xdcnv( &left, &double64 );  
  puttrdouble(double64); 
  xdcnv( &right, &double64 ); 
  puttrdouble(double64);
  xdcnv( &top, &double64 ); 
  puttrdouble(double64);
  xdcnv( &bottom, &double64 ); 
  puttrdouble(double64);
    
  puttrint(maxiterations);	
  
   for (y=0; y<Wh; y++) {
    gettrraw(linebuffer, Ww);       /* get a whole line from the Transputer */
    for(x=0; x<Ww; x++) {
 	  if(linebuffer[x] == maxiterations) {
				farbe = BLACK;
	  } else {
				farbe = linebuffer[x] & 15; /* Colors from 1 to 15 */
	  }
	  vsm_color(handle, farbe);
	  pxy[0] = x + Wx;
	  pxy[1] = y + Wy;
	  v_pmarker(handle, 1, pxy);
    }
  }
	
}

/* The native 68k version */
void mandelbrot(void)
{
	int breite, hoehe, tiefe;
	double xa, xe, ya, ye, dx, dy, cx, cy;
	int spalte, zeile, zaehler;
	int farbe;
	double realteil, zx, zy, zxx, zyy;
	int pxy[2];
	
	/* wind_get( w_handle, WF_WORKXYWH, &Wx, &Wy, &Ww, &Wh ); */
	   
	breite = Ww;
	hoehe = Wh;
	tiefe = 32; /*  I suggest to keep it for comparisson reasons */
	xa = -2.00;
	xe = 1;
	ya = 1.125;
	ye = -1.125;
	dx = (xe - xa) / breite;
	dy = (ya - ye) / hoehe;
	
	for (zeile = 0; zeile < hoehe; zeile++)
	{
		cy = ya - zeile * dy;
		for (spalte = 0; spalte < breite; spalte++)
		{
			cx = xa + spalte * dx;
			zx = 0;
			zy = 0;
			zaehler = 0;
	        do
	        {
				zxx = zx * zx;
				zyy = zy * zy;
				realteil = zxx - zyy + cx;
				zy = (zx + zx) * zy + cy;   
				zx = realteil;
				zaehler = zaehler + 1;
	        } while ((zxx + zyy) <= 4.0 && zaehler != tiefe);
			if (zaehler >= tiefe)
			{
				farbe = BLACK;
			} else
			{
				farbe = zaehler & 15; /* Farbnummern von 1 bis 15 */
	        }
	        vsm_color(handle, farbe);
	        pxy[0] = spalte + Wx;
	        pxy[1] = zeile + Wy;
	        v_pmarker(handle, 1, pxy);
		}

	}
}
