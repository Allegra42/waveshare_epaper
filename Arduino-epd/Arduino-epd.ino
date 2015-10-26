#include "epd.h"

void tekst ( void )
{
  // bufor tekstu dla znakw chinskich
  epd_set_color ( BLACK, WHITE ) ; // czarne litery, biale tlo
  epd_clear ( ) ; // czyszczenie ekranu
 
  epd_set_en_font ( ASCII48 ) ; // uastawiamy czcionki zwykle o rozmiarze 32
  
  epd_disp_string ( "Das Teil ist mega cool..." , 0 , 0 ) ; // wyswietlamy powitanie
  epd_disp_string ( "---....++**###&&%$§" , 100 , 300 ) ;
  
  epd_udpate ( ) ;
}
void setup ( void )
{
  epd_init ( ) ; // inicjalizacja
  epd_wakeup ( ) ; // pobudka
  epd_set_memory ( MEM_NAND ) ; // wybor pamieci NAND
}
void loop ( void )
{
  tekst ( ) ;
  while ( 1 ) { }
}
