#include <stdio.h>
#include "timestamp.h"
#include "string.h"

/* Functions */

/**
 * Função responsável pela recuperação do horário local.
 * Parâmetros: struct timeval * timeNow: Estrutura de dados utilizada para gravação do horário local.
 */ 
void getTime (struct timeval * timeNow) {
    gettimeofday(timeNow, NULL);
}

/**
 * Função responsável pela geração de um formato mais amigável de horário.
 * Parâmetros: struct timeval * time: Estrutura de dados com as informações de horário.
 *             char * formattedTime: String utilizada para gravação de formato amigável de horário.
 */ 
void getFormattedTime (struct timeval * time, char * formattedTime) {
    struct tm *localTime;
    localTime = localtime(&time->tv_sec);

    // Gera informações de dia, mês, ano, hora, minutos e segundos
    strftime(formattedTime, FORMATTED_TIME_SIZE, "%Y:%m:%dT%H:%M:%S", localTime);
    strcat(formattedTime, ".");

    // Gera informações de milisegundos.
    int milisseconds = (int)time->tv_usec / 1000;
    char milissecondsText[6];
    sprintf(milissecondsText, "%dZ", milisseconds);

    // Concatena as informações em uma string única.
    strcat(formattedTime, milissecondsText);
} 