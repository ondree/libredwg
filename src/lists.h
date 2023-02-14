//
// Created by Ondrej Svoboda on 14.02.2023.
//

#include <stddef.h>
#include "dwg.h"
#ifndef LIBREDWG_LISTS_H
#  define LIBREDWG_LISTS_H

#endif // LIBREDWG_LISTS_H

typedef struct polygon
{
  BITCODE_2BD *points;
  BITCODE_BL pointCount;
  BITCODE_RD rotation;
  Dwg_Handle ownerHandle;
  char *layerName;
} Polygon;

typedef struct deskbot_insert
{
  BITCODE_3DPOINT insertPoint;
} DeskbotInsert;

typedef struct deskbot_attribute
{
  char *id;
  char *name;
  char *path
} Attribute;

typedef struct
{
  Polygon *array;
  size_t used;
  size_t size;
} PolygonList;

typedef struct deskbot_seat
{
  Attribute attribute;
  PolygonList polygons;
} Seat;

typedef struct deskbot_room
{
  Attribute attribute;
  PolygonList polygons;
} Room;

typedef struct
{
  Seat *array;
  size_t used;
  size_t size;
} SeatList;

typedef struct
{
  Room *array;
  size_t used;
  size_t size;
} RoomList;

typedef struct deskbot_data
{
  SeatList seats;
  RoomList rooms;
} DeskbotData;

void initRoomList (RoomList *a, size_t initialSize);
void insertRoomList (RoomList *a, Room element);
void freeRoomList (RoomList *a);

void initSeatList (SeatList *a, size_t initialSize);
void insertSeatList (SeatList *a, Seat element);
void freeSeatList (SeatList *a);

void initPolygonList (PolygonList *a, size_t initialSize);
void insertPolygonList (PolygonList *a, Polygon element);
void freeSePolygonList (PolygonList *a);