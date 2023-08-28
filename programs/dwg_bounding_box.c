//
// Created by Ondrej Svoboda on 16.02.2023.
//

#include "dwg.h"
#include "bits.h"
#include <stdlib.h>

static char *entityTextValue(Dwg_Data *data, BITCODE_TV value) {
    if (data->header.version > R_2007)
        return bit_convert_TU((BITCODE_TU) value);
    else
        return value;
}

static void calculateLimits2For(BITCODE_2RD point, BITCODE_BD *minX, BITCODE_BD *maxX,
                                BITCODE_BD *minY, BITCODE_BD *maxY) {

    if (*minX > point.x) {
        *minX = point.x;
    }
    if (*maxX < point.x) {
        *maxX = point.x;
    }
    if (*minY > point.y) {
        *minY = point.y;
    }
    if (*maxY < point.y) {
        *maxY = point.y;
    }
}

static void calculateLimitsFor(BITCODE_3BD point, BITCODE_BD *minX, BITCODE_BD *maxX,
                               BITCODE_BD *minY, BITCODE_BD *maxY) {

    if (*minX > point.x) {
        *minX = point.x;
    }
    if (*maxX < point.x) {
        *maxX = point.x;
    }
    if (*minY > point.y) {
        *minY = point.y;
    }
    if (*maxY < point.y) {
        *maxY = point.y;
    }
}

static void limitsForLayer(Dwg_Data *data, char *layer) {
    BITCODE_BD xMin = 5000;
    BITCODE_BD xMax = 0;
    BITCODE_BD yMin = 5000;
    BITCODE_BD yMax = 0;
    for (int i = 0; i < data->num_objects; ++i) {
        Dwg_Object object = data->object[i];
        switch (object.fixedtype) {
            case DWG_TYPE_LINE: {
                Dwg_Entity_LINE *line = object.tio.entity->tio.LINE;
                BITCODE_TV layerName = line->parent->layer->obj->tio.object->tio.LAYER->name;
                if (strcmp(layerName, layer) == 0) {
                    if (line->start.x > 150) {
                        fprintf(stderr, "This is outside");
                    }
                    if (line->start.x < 0) {
                        fprintf(stderr, "This is outside");
                    }
                    if (line->end.y > 150) {
                        fprintf(stderr, "This is outside");
                    }
                    if (line->end.y < 0) {
                        fprintf(stderr, "This is outside");
                    }

                    calculateLimitsFor(line->start, &xMin, &xMax, &yMin, &yMax);
                    calculateLimitsFor(line->end, &xMin, &xMax, &yMin, &yMax);
                }
            }
                break;
            case DWG_TYPE_POLYLINE_2D: {
//                fprintf(stderr, "checking entity of type POLYLINE_2D\n");
                Dwg_Entity_POLYLINE_2D *entity = object.tio.entity->tio.POLYLINE_2D;
                BITCODE_TV layerName = entity->parent->layer->obj->tio.object->tio.LAYER->name;
                if (strcmp(layerName, layer) == 0) {
                    for (int j = 0; j < entity->num_owned; j++) {
                        calculateLimitsFor(entity->vertex[i]->obj->tio.entity->tio.VERTEX_2D->point,
                                           &xMin, &xMax, &yMin, &yMax);
                    }
                }
            }
                break;
            case DWG_TYPE_LWPOLYLINE: {
//                fprintf(stderr, "checking entity of type LWPOLYLINE\n");
                Dwg_Entity_LWPOLYLINE *entity = object.tio.entity->tio.LWPOLYLINE;
                BITCODE_TV layerName = entity->parent->layer->obj->tio.object->tio.LAYER->name;
                if (strcmp(layerName, layer) == 0) {
                    for (int j = 0; j < entity->num_points; j++) {
                        calculateLimits2For(entity->points[j], &xMin, &xMax, &yMin, &yMax);
                    }
                }
            }
                break;
            default:
                break;
        }
    }
    Dwg_Header_Variables *variables = &data->header_vars;
    variables->EXTMAX.y = yMax;
    variables->EXTMAX.x = xMax;
    variables->EXTMIN.y = yMin;
    variables->EXTMIN.x = xMin;
//  printf ("Force set limit points: xMin[%f],xMax[%f],yMin[%f],yMax[%f]\n",
//          variables->EXTMIN.x, variables->EXTMAX.x, variables->EXTMIN.y,
//          variables->EXTMAX.y);
}

Dwg_Object_LAYER **dwg_get_layers_custom(const Dwg_Data *dwg) {
    unsigned int i;
    unsigned int num_layers = dwg_get_layer_count(dwg);
    Dwg_Object_LAYER_CONTROL *_ctrl;
    Dwg_Object_LAYER **layers;
    Dwg_Object *obj;

    obj = dwg_get_first_object(dwg, DWG_TYPE_LAYER_CONTROL);
    if (!obj || !obj->tio.object || !obj->tio.object->tio.LAYER_CONTROL)
        return NULL;
    _ctrl = obj->tio.object->tio.LAYER_CONTROL;
    layers = (Dwg_Object_LAYER **) calloc(num_layers, sizeof(Dwg_Object_LAYER *));
    for (i = 0; i < num_layers; i++) {
        layers[i] = _ctrl->entries[i]->obj->tio.object->tio.LAYER;
    }
    return layers;
}

EXPORT void forceBoundingBoxForData(Dwg_Data *data, char *source_layer_name) {
    Dwg_Object_LAYER **layers = dwg_get_layers_custom(data);
    unsigned int num_layers = dwg_get_layer_count(data);

    for (int i = 0; i < num_layers; i++) {
        Dwg_Object_LAYER *layer = layers[i];
        char *layerName = entityTextValue(data, layer->name);
        if (strcmp(layerName, source_layer_name) == 0) {
            limitsForLayer(data, source_layer_name);
            break;
        }
    }
}