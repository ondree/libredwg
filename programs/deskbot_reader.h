//
// Created by Ondrej Svoboda on 09.02.2023.
//

#ifndef LIBREDWG_DESKBOT_READER_H
#  define LIBREDWG_DESKBOT_READER_H

#endif // LIBREDWG_DESKBOT_READER_H

extern char *base_layer;
extern char *default_base_layer;
extern int vector_point_scale;
extern int overall_point_scale;

void loadDeskbotData(Dwg_Data *data, const char *string, const char *string1);
