//
// Created by Ondrej Svoboda on 16.02.2023.
//

#include "dwg.h"
#include "bits.h"

static char *entityTextValue(Dwg_Data *data, BITCODE_TV value) {
    if (data->header.version > R_2007)
        return bit_convert_TU((BITCODE_TU) value);
    else
        return value;
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

static void limitsForLayer(Dwg_Data *data, Dwg_Object_LAYER *layer) {
    BITCODE_BD xMin = 5000;
    BITCODE_BD xMax = 0;
    BITCODE_BD yMin = 5000;
    BITCODE_BD yMax = 0;
    for (int i = 0; i < data->num_objects; ++i) {
        Dwg_Object object = data->object[i];
        switch (object.fixedtype) {
            case DWG_TYPE_LINE: {
                Dwg_Entity_LINE *line = object.tio.entity->tio.LINE;
                if (layer == line->parent->layer->obj->tio.object->tio.LAYER) {
                    BITCODE_3BD start = line->start;
                    BITCODE_3BD end = line->end;
                    calculateLimitsFor(start, &xMin, &xMax, &yMin, &yMax);
                    calculateLimitsFor(end, &xMin, &xMax, &yMin, &yMax);
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

void forceBoundingBoxForData(Dwg_Data *data, char *source_layer_name) {
    Dwg_Object_LAYER **layers = dwg_get_layers(data);
    unsigned int num_layers = dwg_get_layer_count(data);

    for (int i = 0; i < num_layers; i++) {
        Dwg_Object_LAYER *layer = layers[i];
        char *layerName = entityTextValue(data, layer->name);
        if (strcmp(layerName, source_layer_name) == 0) {
            limitsForLayer(data, layer);
            break;
        }
    }
}