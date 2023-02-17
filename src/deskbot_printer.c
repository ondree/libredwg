//
// Created by Ondrej Svoboda on 14.02.2023.
//

#include <stdio.h>
#include "deskbot_printer.h"

static void
printPolygonData (PolygonList polygonList)
{
  fprintf (stdout, "-- POLYGON COUNT %zu  \n", polygonList.used);
  {

    for (int i = 0; i < polygonList.used; i++)
      {
        Polygon polygon = polygonList.array[i];
        fprintf (stdout,
                 "---- POLYGON layer[%s] rotation[%f] handle [" FORMAT_H
                 "] owner[" FORMAT_H "] \n",
                 polygon.layerName, polygon.rotation, ARGS_H (polygon.handle),
                 ARGS_H (polygon.ownerHandle));

        for (int k = 0; k < polygon.pointCount; k++)
          {
            BITCODE_2BD point = polygon.points[k];
            fprintf (stdout, "------- POINT [x:%f,y:%f] \n", point.x, point.y);
          }
      }
  }
}

static void
printRoomData (Room room)
{
  fprintf (stdout, "\nROOM name[%s] id[%s] path[%s] block[%s]\n",
           room.attribute.name, room.attribute.id, room.attribute.path,
           room.attribute.blockName);
  printPolygonData (room.polygons);
}

static void
printSeatData (Seat seat)
{
  fprintf (stdout, "\nSEAT name[%s] id[%s] path[%s] block[%s]\n",
           seat.attribute.name, seat.attribute.id, seat.attribute.path,
           seat.attribute.blockName);
  printPolygonData (seat.polygons);
}

void
printDeskbotData (DeskbotData data)
{
  for (int i = 0; i < data.rooms.used; i++)
    {
      printRoomData (data.rooms.array[i]);
    }

  for (int i = 0; i < data.seats.used; i++)
    {
      printSeatData (data.seats.array[i]);
    }
}

void
printRoomCSV (FILE *roomFile, Room room)
{
  for (int i = 0; i < room.polygons.used; i++)
    {
      Polygon polygon = room.polygons.array[i];

      for (int k = 0; k < polygon.pointCount; k++)
        {
          BITCODE_2BD point = polygon.points[k];
          int order = k + 1;
          char *enabled = "false";
          int capacity = 0;
          char *placeType = "ROOM";
          char *placeName = room.attribute.name;
          char *equipment = "[]";
          fprintf (roomFile, "%s,%d,%f,%f,%s,%s,%s,%s,%d,%s\n", placeName,
                   order, point.x, point.y, enabled, room.attribute.path,
                   room.attribute.id, placeType, capacity, equipment);
        }
      // Check only first item
      return;
    }
}

void
printSeatCSV (FILE *seatFile, Seat seat)
{
  for (int i = 0; i < seat.polygons.used; i++)
    {
      Polygon polygon = seat.polygons.array[i];
      if (polygon.pointCount == 4)
        {
          // Insert point is stored as second item
          int insertPointIndex = 1;
          BITCODE_2BD point = polygon.points[insertPointIndex];
          char *enabled = "false";
          char *equipment = "[]";
          fprintf (seatFile, "%s,%f,%f,%f,%s,%s,%s\n", enabled,
                   polygon.rotation, point.x, point.y, seat.attribute.path,
                   seat.attribute.name, equipment);
        }
    }
}

void
printCSV (DeskbotData data)
{
  FILE *roomFile = fopen ("rooms.csv", "w");
  if (roomFile != NULL)
    {
      fprintf (roomFile, "Description,order,x,y,enabled,path,name,placeType,"
                         "capacity,equipment\n");
      for (int i = 0; i < data.rooms.used; i++)
        {
          printRoomCSV (roomFile, data.rooms.array[i]);
        }
    }
  fclose (roomFile);

  FILE *seatFile = fopen ("seats.csv", "w");
  fprintf (seatFile, "enabled,rotation,xpos,ypos,path,name,equipment\n");
  for (int i = 0; i < data.seats.used; i++)
    {
      printSeatCSV (seatFile, data.seats.array[i]);
    }
  fclose (seatFile);
}