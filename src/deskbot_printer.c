//
// Created by Ondrej Svoboda on 14.02.2023.
//

#include <stdio.h>
#include "deskbot_printer.h"

static void
printPolygonData (PolygonList polygonList)
{
  fprintf (stdout, "----- POLYGON COUNT %zu  \n", polygonList.used);
  {

    for (int i = 0; i < polygonList.used; i++)
      {
        Polygon polygon = polygonList.array[i];
        fprintf (stdout,
                 "----- POLYGON LAYER layer[%s] rotation[%f] owner[" FORMAT_H
                 "] \n",
                 polygon.layerName, polygon.rotation,
                 ARGS_H (polygon.ownerHandle));

        for (int k = 0; k < polygon.pointCount; k++)
          {
            BITCODE_2BD point = polygon.points[k];
            fprintf (stdout, "----- POINT [x:%f,y:%f] \n", point.x, point.y);
          }
      }
  }
}

static void
printRoomData (Room room)
{
  fprintf (stdout, "-- ROOM name[%s] id[%s] path[%s]\n", room.attribute.name,
           room.attribute.id, room.attribute.path);
  printPolygonData (room.polygons);
}

static void
printSeatData (Seat seat)
{
  fprintf (stdout, "-- SEAT name[%s] id[%s] path[%s]\n", seat.attribute.name,
           seat.attribute.id, seat.attribute.path);
  printPolygonData (seat.polygons);
}

void
printDeskbotData (DeskbotData deskbotData)
{
  for (int i = 0; i < deskbotData.rooms.used; i++)
    {
      printRoomData (deskbotData.rooms.array[i]);
    }

  for (int i = 0; i < deskbotData.seats.used; i++)
    {
      printSeatData (deskbotData.seats.array[i]);
    }
}
