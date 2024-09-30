#include <stdio.h>

#include <libcmore.h>

#include "../lib/liblain.h"
#include "test.h"


void print_map_area(ln_vm_area * area, char * prefix) {

    fprintf(stderr, "%spathname:   %s\n", prefix, area->pathname);
    fprintf(stderr, "%sbasename:   %s\n", prefix, area->basename);

    fprintf(stderr, "%sstart_addr: %lx\n", prefix, area->start_addr);
    fprintf(stderr, "%send_addr:   %lx\n", prefix, area->end_addr);

    int mask_iter = 0;
    int mask_start = 1;
    char perm[] = {'r', 'w', 'x', 's'};

    //print permissions
    fprintf(stderr, "%saccess:     ", prefix);
    do {

        if (area->access & mask_start) {
            fprintf(stderr, "%c", perm[mask_iter]);
        } else {
            fprintf(stderr, "%c", '-');
        }

        mask_start *= 2;
        ++mask_iter;
    } while (mask_iter < 4);
    fprintf(stderr, "%c", '\n');

    fprintf(stderr, "%sid: %d\n", prefix, area->id);
    fprintf(stderr, "%smapped: %s", prefix, area->mapped ? "true" : "false");
    fprintf(stderr, "\n");

    return;
}


void print_map_obj(ln_vm_obj * obj) {

    int ret;

    cm_list_node * node;
    ln_vm_area * area;

    fprintf(stderr, "\tpathname: %s\n", obj->pathname);
    fprintf(stderr, "\tbasename: %s\n", obj->basename);

    fprintf(stderr, "\tstart_addr: %lx\n", obj->start_addr);
    fprintf(stderr, "\tend_addr: %lx\n", obj->end_addr);

    fprintf(stderr, "\tid: %d\n", obj->id);
    fprintf(stderr, "\tmapped: %s", obj->mapped ? "true" : "false");

    fprintf(stderr, "\n\tnodes:\n");

    for (int i = 0; i < obj->vm_area_node_ptrs.len; ++i) {

        ret = cm_list_get_val(&obj->vm_area_node_ptrs, i, (cm_byte *) &node);
        area = LN_GET_NODE_AREA(node);

        fprintf(stderr, "\n\t\t[AREA %d]\n", i);
        print_map_area(area, "\t\t");

    } //end for

    fprintf(stderr, "\n");

    return;
}


//print out map areas in chronological order to stdout
void print_areas(ln_vm_map * vm_map) {

    cm_list_node * node;
    ln_vm_area * area;

    for (int i = 0; i < vm_map->vm_areas.len; ++i) {

        node = cm_list_get_node(&vm_map->vm_areas, i);
        area = LN_GET_NODE_AREA(node);

        fprintf(stderr, "\n[AREA %d]\n", i);
        print_map_area(area, "\t");

    } //end for

    return;
}


void print_objs(ln_vm_map * vm_map) {

    cm_list_node * node;
    ln_vm_obj * obj;

    for (int i = 0; i < vm_map->vm_objs.len; ++i) {

        node = cm_list_get_node(&vm_map->vm_objs, i);
        obj = LN_GET_NODE_OBJ(node);

        fprintf(stderr, "\n[OBJ %d]\n", i);
        print_map_obj(obj);
    }

    return;
}


//print unmapped areas
void print_unmapped_areas(ln_vm_map * vm_map) {

    int ret;
    cm_list_node * node;
    ln_vm_area * area;

    int mask_start = 1;
    int mask_iter = 0;
    char perm[] = {'r', 'w', 'x', 's'};

    for (int i = 0; i < vm_map->vm_areas_unmapped.len; ++i) {

        ret = cm_list_get_val(&vm_map->vm_areas_unmapped, i, (cm_byte *) &node);
        area = LN_GET_NODE_AREA(node);

        fprintf(stderr, "\n[AREA %d]\n", i);
        print_map_area(area, "\t");

    } //end for

    return;
}


//print unmapped objs
void print_unmapped_objs(ln_vm_map * vm_map) {

    int ret;
    cm_list_node * node;
    ln_vm_obj * obj;

    for (int i = 0; i < vm_map->vm_objs_unmapped.len; ++i) {

        ret = cm_list_get_val(&vm_map->vm_objs_unmapped, i, (cm_byte *) &node);
        obj = LN_GET_NODE_OBJ(node);

        fprintf(stderr, "\n[OBJ %d]\n", i);
        print_map_obj(obj);
    }

    return;
}


void test_map(ln_vm_map * vm_map) {
 
    printf("\n\n --- [MAP & MAP UTILS] --- \n\n");
    printf("note: use a debugger to test map utils.\n");

    int ret;
    off_t off;
    char * name;
    char * badname = "lain";

    cm_list_node * node;
    ln_vm_area * area;
    ln_vm_obj * obj;

    //test obj by name
    node = cm_list_get_node(&vm_map->vm_areas, 0);
    area = LN_GET_NODE_AREA(node);
    name = area->pathname;

    node = ln_get_vm_obj_by_pathname(vm_map, name);
    obj = LN_GET_NODE_OBJ(node);
    name = obj->basename;

    node = ln_get_vm_obj_by_basename(vm_map, name);
    obj = LN_GET_NODE_OBJ(node);

    //test area/obj by address
    node = ln_get_vm_area_by_addr(vm_map, obj->start_addr + 0x500, &off);
    area = LN_GET_NODE_AREA(node);

    node = ln_get_vm_obj_by_addr(vm_map, area->start_addr + 0x500, &off);
    obj = LN_GET_NODE_OBJ(node);

    //dump areas
    print_areas(vm_map);

    //dump objs
    print_objs(vm_map);

    //dump unmapped areas
    print_unmapped_areas(vm_map);

    //dump unmapped objs
    print_unmapped_objs(vm_map);

    //clean unmapped areas/objs
    ret = ln_map_clean_unmapped(vm_map);

    return;
}
