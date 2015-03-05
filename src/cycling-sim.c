#include <stdbool.h>    
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

int main(int argc, char **argv)
{
    int dist = -1, num_cyclers = -1;
    int use_random_velocity_i = -1;

    if(argc >= 4) {
        dist = atoi(argv[1]);
        num_cyclers = atoi(argv[2]);
  
        if(strcasecmp(argv[3], "v") == 0)
            use_random_velocity_i = 1;
        else if(strcasecmp(argv[3], "u") == 0)
            use_random_velocity_i = 0;
    }

    if(dist <= 249 || num_cyclers <= 3 || use_random_velocity_i < 0) {
        fprintf(stderr, "Uso: %s distância n_ciclistas [v|u]\n", argv[0]);
        return 1;
    }

    bool use_random_velocity = use_random_velocity_i != 0;
    printf("dist: %d\ncyclers: %d\nrandom_velocity: %s\n", dist, num_cyclers,
           use_random_velocity ? "true" : "false");

    return 0;
}
