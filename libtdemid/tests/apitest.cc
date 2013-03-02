#include <libtdemid/libtdemid.h>
#include <unistd.h>
#include <stdio.h>

int main (int , char **)
{
  printf("Libtdemid test2 . (C) 2000 Antonio Larrosa Jimenez . Malaga (Spain)\n");
  printf("Using libtdemid from a simple C++ application\n");

  KMidSimpleAPI::kMidInit();
  KMidSimpleAPI::kMidLoad("Kathzy.mid");
  KMidSimpleAPI::kMidPlay();

  for (int i=0;i<30;i++)
  {
    printf("%d/30 seconds\n",i+1);
    sleep(1);
  };    

  KMidSimpleAPI::kMidStop();
  KMidSimpleAPI::kMidDestruct();

  return 0;
};

