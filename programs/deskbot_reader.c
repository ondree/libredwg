//
// Created by Ondrej Svoboda on 09.02.2023.
//
#include "config.h"
#include <stdlib.h>
#include <string.h>
// strings.h or string.h
#ifdef AX_STRCASECMP_HEADER

#include AX_STRCASECMP_HEADER

#endif

#include <math.h>
#include <stdbool.h>

#ifdef HAVE_VALGRIND_VALGRIND_H
#  include <valgrind/valgrind.h>
#endif

#include "dwg.h"
#include "deskbot_reader.h"
#include "bits.h"
#include "dwg_bounding_box.h"
#include "deskbot_printer.h"

#ifndef DISABLE_DXF
#  ifndef DISABLE_JSON

#  endif
#endif

char *base_layer;
char *default_base_layer = "080202_BEAUGEB_AWAND";

static char *entityTextValue(Dwg_Data *data, BITCODE_TV value) {
    if (data->header.version > R_2007)
        return bit_convert_TU((BITCODE_TU) value);
    else
        return value;
}

int countChar(const char *str, char symbol) {
    int count = 0;
    if (str == NULL) return 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] == symbol) {
            count++;
        }
    }
    return count;
}

static void insertData(DeskbotData *deskbotData, Attribute *attribute,
                       PolygonList *polygonList) {
    const char path_divider = '/';
    int dividers = countChar(attribute->path, path_divider);
    switch (dividers) {
        case 3: {
            Seat seat;
            seat.attribute = *attribute;
            seat.polygons = *polygonList;
            insertSeatList(&deskbotData->seats, seat);
        }
            break;
        case 2: {
            Room room;
            room.attribute = *attribute;
            room.polygons = *polygonList;
            insertRoomList(&deskbotData->rooms, room);
        }
            break;
        default:
            break;
    }
}

static bool layerLWNamesWithPrefix(Dwg_Entity_LWPOLYLINE entity, const char *seatLayer,
                                   const char *roomLayer) {
    BITCODE_TV name = entity.parent->layer->obj->tio.object->tio.LAYER->name;
    return strstr(name, seatLayer) != NULL || strstr(name, roomLayer) != NULL;
}

static bool layerNamesWithPrefix(Dwg_Entity_POLYLINE_2D entity, const char *seatLayer,
                                 const char *roomLayer) {
    char *name = entity.parent->layer->obj->tio.object->tio.LAYER->name;
//    fprintf(stderr, "Checking layer: %s\n", name);
    if (strcmp(name, "1") != 0)
        fprintf(stderr, "Checking layer: %s\n", name);
    return strstr(name, seatLayer) != NULL || strstr(name, roomLayer) != NULL;
}

static BITCODE_RD countRotation(Polygon polygon) {
    // Needs to have 4 points to be able to calculate rotation
    if (polygon.pointCount == 4) {
        BITCODE_2BD point1 = polygon.points[0];
        BITCODE_2BD point4 = polygon.points[3];
        double difX = point4.x - point1.x;
        double difY = point4.y - point1.y;
        double rotation = -atan2(difY, difX) * (180 / M_PI);
        if (rotation < 0)
            return rotation + 360;
        else
            return rotation;
    } else
        return 0;
}

static void loadVertex(Dwg_Data *data, Dwg_Entity_POLYLINE_2D *entity,
                       PolygonList *polygonList, DeskbotInsert insert, Dwg_Handle handle) {
    BITCODE_2BD *points = malloc(entity->num_owned * sizeof(BITCODE_2BD));
    BITCODE_3BD min = data->header_vars.EXTMIN;
    BITCODE_3BD max = data->header_vars.EXTMAX;
    for (int i = 0; i < entity->num_owned; i++) {
        Dwg_Object *object = entity->vertex[i]->obj;
        Dwg_Entity_VERTEX_2D *vertex = object->tio.entity->tio.VERTEX_2D;
        if (vertex != NULL) {
            // calculate coordinates for Deskbot, needs to invert Y values
            points[i].x = (vertex->point.x + insert.insertPoint.x - min.x) / 100;
            points[i].y = (max.y - (vertex->point.y + insert.insertPoint.y)) / 100;
        }
    }
    Polygon polygon;
    polygon.ownerHandle = entity->parent->ownerhandle->handleref;
    polygon.handle = handle;
    polygon.points = points;
    polygon.pointCount = entity->num_owned;
    polygon.layerName = entity->parent->layer->obj->tio.object->tio.LAYER->name;
    polygon.rotation = countRotation(polygon);
    insertPolygonList(polygonList, polygon);
}

static void loadAttribute(Dwg_Data *data, Dwg_Entity_ATTRIB *entity, Attribute *attribute) {
    const char *ATTRIBUTE_ID = "Kennzeichen";
    const char *ATTRIBUTE_NAME = "Benennung";
    const char *ATTRIBUTE_PATH = "TPLNR";

    {
        char *tagValue = entityTextValue(data, entity->tag);
        if (strcmp(tagValue, ATTRIBUTE_ID) == 0) {
            attribute->id = entityTextValue(data, entity->text_value);
        } else if (strcmp(tagValue, ATTRIBUTE_NAME) == 0) {
            attribute->name = entityTextValue(data, entity->text_value);
        } else if (strcmp(tagValue, ATTRIBUTE_PATH) == 0) {
            attribute->path = entityTextValue(data, entity->text_value);
        }
    }
}

static void loadAttributes(Dwg_Data *data, Dwg_Entity_INSERT *insertEntity,
                           DeskbotInsert *insert, Attribute *attribute) {
    insert->insertPoint = insertEntity->ins_pt;
    for (int j = 0; j < insertEntity->num_owned; j++) {
        Dwg_Object *object = insertEntity->attribs[j]->obj;
        Dwg_Entity_ATTRIB *attrib = object->tio.entity->tio.ATTRIB;
        if (attrib != NULL) {
            loadAttribute(data, attrib, attribute);
            fprintf(stderr, "Loading attribute ->id %s, ->name %s, ->path %s, ->blockName %s\n", attribute->id,
                    attribute->name, attribute->path, attribute->blockName);
        }
    }
}

static void loadPolyLines(Dwg_Object_BLOCK_HEADER *header, PolygonList *polygonList,
                          const char *seatLayer, const char *roomLayer,
                          DeskbotInsert insert, Dwg_Data *data) {
    for (int j = 0; j < header->num_owned; j++) {
        Dwg_Object *ownerObj = header->entities[j]->obj;
        switch (ownerObj->fixedtype) {
            case DWG_TYPE_POLYLINE_2D: {
                fprintf(stderr, "checking entity of type POLYLINE_2D\n");
                Dwg_Entity_POLYLINE_2D *entity = ownerObj->tio.entity->tio.POLYLINE_2D;
                if (layerNamesWithPrefix(*entity, seatLayer, roomLayer)) {
                    loadVertex(data, entity, polygonList, insert, ownerObj->handle);
                    fprintf(stderr, "Loaded polylines for %s\n",
                            entity->parent->layer->obj->tio.object->tio.LAYER->name);
                }
            }
                break;
            case DWG_TYPE_LWPOLYLINE: {
                fprintf(stderr, "checking entity of type LWPOLYLINE\n");
                Dwg_Entity_LWPOLYLINE *entity = ownerObj->tio.entity->tio.LWPOLYLINE;
                if (layerLWNamesWithPrefix(*entity, seatLayer, roomLayer)) {
//                    loadVertex(data, entity, polygonList, insert, ownerObj->handle);
                    fprintf(stderr, "Loaded polylines for %s\n",
                            entity->parent->layer->obj->tio.object->tio.LAYER->name);
                }
            }
                break;
            case DWG_TYPE_POLYLINE_PFACE:
                fprintf(stderr, "checking entity of type POLYLINE_PFACE\n");
                break;
            case DWG_TYPE_POLYLINE_MESH:
                fprintf(stderr, "checking entity of type POLYLINE_MESH\n");
            default:
                break;
        }
    }
}

char *loadBaseLayer() {
    if (base_layer == NULL)
        return default_base_layer;
    else
        return base_layer;
}

void loadDeskbotData(Dwg_Data *data, const char *seatLayer, const char *roomLayer) {
    forceBoundingBoxForData(data, loadBaseLayer());
    DeskbotData deskbotData;
    initRoomList(&deskbotData.rooms, 5);
    initSeatList(&deskbotData.seats, 5);

    for (int i = 0; i < data->num_objects; i++) {
        Dwg_Object obj = data->object[i];
        if (obj.fixedtype == DWG_TYPE_BLOCK_HEADER) {
            Dwg_Object_BLOCK_HEADER *header = obj.tio.object->tio.BLOCK_HEADER;
            if (header->num_owned > 0 && header->num_inserts > 0) {
                // Deskbot DWG file should always have 1 insert entity
                Dwg_Entity_INSERT *insertEntity = header->inserts[0]->obj->tio.entity->tio.INSERT;
                PolygonList polygonList;
                initPolygonList(&polygonList, 10);
                Attribute attribute;
                attribute.blockName = header->name;
                DeskbotInsert insert;
                loadAttributes(data, insertEntity, &insert, &attribute);
                loadPolyLines(header, &polygonList, seatLayer, roomLayer,
                              insert, data);
                insertData(&deskbotData, &attribute, &polygonList);
            }
        }
    }
    printCSV(deskbotData);
    freeRoomList(&deskbotData.rooms);
    freeSeatList(&deskbotData.seats);
}
