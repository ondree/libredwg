//
// Created by Ondrej Svoboda on 14.02.2023.
//

#include <stdlib.h>
#include "lists.h"

void
initRoomList (RoomList *a, size_t initialSize)
{
  a->array = malloc (initialSize * sizeof (Room));
  a->used = 0;
  a->size = initialSize;
}
void
insertRoomList (RoomList *a, Room element)
{
  if (a->used == a->size)
    {
      a->size *= 2;
      a->array = realloc (a->array, a->size * sizeof (element));
    }
  a->array[a->used++] = element;
}
void
freeRoomList (RoomList *a)
{
  free (a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

void
initSeatList (SeatList *a, size_t initialSize)
{
  a->array = malloc (initialSize * sizeof (Seat));
  a->used = 0;
  a->size = initialSize;
}
void
insertSeatList (SeatList *a, Seat element)
{
  if (a->used == a->size)
    {
      a->size *= 2;
      a->array = realloc (a->array, a->size * sizeof (element));
    }
  a->array[a->used++] = element;
}
void
freeSeatList (SeatList *a)
{
  free (a->array);
  a->array = NULL;
  a->used = a->size = 0;
}
//

void
initPolygonList (PolygonList *a, size_t initialSize)
{
  a->array = malloc (initialSize * sizeof (Polygon));
  a->used = 0;
  a->size = initialSize;
}
void
insertPolygonList (PolygonList *a, Polygon element)
{
  if (a->used == a->size)
    {
      a->size *= 2;
      a->array = realloc (a->array, a->size * sizeof (Polygon));
    }
  a->array[a->used++] = element;
}
void
freeSePolygonList (PolygonList *a)
{
  free (a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

