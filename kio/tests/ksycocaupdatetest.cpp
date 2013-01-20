#include <kservice.h>

#include <kapplication.h>

int main(int argc, char *argv[])
{
   TDEApplication k(argc,argv,"whatever"); // KMessageBox needs KApp for makeStdCaption

   KService::rebuildKSycoca(0);
   return 0;
}
