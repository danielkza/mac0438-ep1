#ifndef CICLISTA_H
#define CICLISTA_H

/* Estrutura de informação */
typedef struct
{
  int id;
  int pos;
  int lap;
} cycler_info;

/* Funções */
void *cycler(void *);

#endif
