//
// Created by Ondrej Svoboda on 14.02.2023.
//

#include "lists.h"

#ifndef LIBREDWG_DESKBOT_PRINTER_H
#  define LIBREDWG_DESKBOT_PRINTER_H

#endif // LIBREDWG_DESKBOT_PRINTER_H

// TODO change output format?
extern char final_file_name[255];
extern char *file_name;
extern char *default_file_name;

void printDeskbotData(DeskbotData data);

void printCSV(DeskbotData data);
