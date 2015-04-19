#include <stdlib.h>

#include "track.h"
#include "cycler.h"

int* track_new(int num_cyclers, int length, cycler_info **cycler_infos)
{
  int *track = malloc(track_length * num_cyclers, sizeof(int));
  cycler_info *cycler_infos = malloc(num_cyclers, sizeof(cycler_info));

  for(int i = 0; i < num_cyclers; i++) {
    cycler_infos[i].id = i;
    cycler_infos[i].pos = rand() % track_length;
    cycler_infos[i].lap = 0;
  }

  for(int i = 0; i < track_length; i++) {

  }

    for(int j = 0, k = 0; j < num_cyclers; j++)
    {
      if(track[j][0] == -1)
      {
        if(k == pos)
        {
          track[j][0] = i;
          rev_vec[i] = j;
          break;
        }
        else
          k++;
      }
    }
  }

  if(debug)
  {
    for(int i = 0; i < num_cyclers; i++)
      printf("%d ", track[i][0]);
    printf("\n");
  }

  return rev_vec;
}
