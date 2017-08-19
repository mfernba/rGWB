// Test básicos...

#include "csmtest.h"

#include "csmedge.inl"
#include "csmedge.tli"
#include "csmhedge.inl"
#include "csmsolid.inl"
#include "csmeuler_mvfs.inl"
#include "csmeuler_kvfs.inl"
#include "csmeuler_lmef.inl"
#include "csmeuler_lmev.inl"
#include "csmeuler_lkef.inl"
#include "csmeuler_lkev.inl"
#include "csmeuler_lkemr.inl"
#include "csmeuler_lmekr.inl"

#include "csmnode.inl"
#include "csmopbas.inl"
#include "csmhashtb.inl"
#include "csmglue.h"
#include "csmface.inl"
#include "csmmath.inl"
#include "csmsetop.h"
#include "csmsolid.h"
#include "csmsweep.h"
#include "csmsplit.h"

#include "copiafor.h"
#include "cyassert.h"
#include "a_pto3d.h"
#include <geomcomp/gccontorno.h>
#include <geomcomp/gcelem2d.h>

#include "csmdebug.inl"
#include "csmviewer.inl"

// ------------------------------------------------------------------------------------------

static void i_test_crea_destruye_solido_vacio(void)
{
    struct csmsolid_t *solido;
    
    solido = csmeuler_mvfs(0., 0., 0., 0, NULL);
    
    csmeuler_kvfs(solido);
    
    csmsolid_free(&solido);
}

// ------------------------------------------------------------------------------------------

static void i_test_basico_solido_una_arista(void)
{
    struct csmsolid_t *solido;
    struct csmhedge_t *hedge;
    struct csmhedge_t *he1, *he2;
    
    solido = csmeuler_mvfs(0., 0., 0., 0, &hedge);
    
    csmeuler_lmev(hedge, hedge, 1., 0., 0., NULL, NULL, &he1, &he2);
    
    csmeuler_lkev(&he1, &he2, &he1, NULL, &he2, NULL);
    assert(he1 == he2);

    csmeuler_kvfs(solido);
    
    csmsolid_free(&solido);
}

// ------------------------------------------------------------------------------------------

static void i_test_crea_lamina(void)
{
    struct csmsolid_t *solido;
    struct csmhedge_t *initial_hedge, *hedge_from_vertex1, *hedge_from_vertex2, *hedge_from_vertex3;
    struct csmface_t *initial_face, *new_face;
    struct csmhedge_t *he_pos, *he_neg;
    struct csmhedge_t *he1, *he2;
    
    solido = csmeuler_mvfs(0., 0., 0., 0, &initial_hedge);
    initial_face = csmopbas_face_from_hedge(initial_hedge);
    
    csmeuler_lmev_strut_edge(initial_hedge, 1., 0., 1., &hedge_from_vertex1);
    csmeuler_lmev_strut_edge(hedge_from_vertex1, 1., 1., 1., &hedge_from_vertex2);
    csmeuler_lmev_strut_edge(hedge_from_vertex2, 0., 1., 0., &hedge_from_vertex3);
    csmeuler_lmef(initial_hedge, hedge_from_vertex3, &new_face, &he_pos, &he_neg);
    
    {
        csmsolid_redo_geometric_generated_data(solido);
        
        assert(initial_face != new_face);

        assert(csmface_contains_point(initial_face, 0.5, 0.5, 0.5, NULL, NULL, NULL) == CIERTO);
        assert(csmface_contains_point(initial_face, 1., 1., 1., NULL, NULL, NULL) == CIERTO);
        assert(csmface_contains_point(initial_face, 5., 5., 0., NULL, NULL, NULL) == FALSO);
        
        assert(csmface_contains_point(new_face, 0.5, 0.5, 0.5, NULL, NULL, NULL) == CIERTO);
        assert(csmface_contains_point(new_face, 1., 1., 1., NULL, NULL, NULL) == CIERTO);
        assert(csmface_contains_point(new_face, 5., 5., 0., NULL, NULL, NULL) == FALSO);
    }
    
    csmeuler_lkef(&he_pos, &he_neg);
    assert(he_pos != NULL);
    assert(he_neg != NULL);
    assert(he_pos == hedge_from_vertex3);
    
    he1 = he_pos;
    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2, NULL, &he1, &he2, NULL);
    assert(he1 == hedge_from_vertex2);

    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2, NULL, &he1, &he2, NULL);
    assert(he1 == hedge_from_vertex1);

    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2, NULL, &he1, &he2, NULL);
    assert(he1 == he2);
    assert(he1 == initial_hedge);
    
    csmeuler_kvfs(solido);
    
    csmsolid_free(&solido);
}

// ------------------------------------------------------------------------------------------

static void i_test_crea_lamina_con_hueco(void)
{
    struct csmsolid_t *solido;
    struct csmhedge_t *initial_hedge, *hedge_from_vertex1, *hedge_from_vertex2, *hedge_from_vertex3;
    struct csmhedge_t *he_pos, *he_neg;
    struct csmhedge_t *he1, *he2;
    
    solido = csmeuler_mvfs(0., 0., 0., 0, &initial_hedge);
    
    csmeuler_lmev_strut_edge(initial_hedge, 10., 0., 0., &hedge_from_vertex1);
    csmeuler_lmev_strut_edge(hedge_from_vertex1, 10., 1.0, 0., &hedge_from_vertex2);
    csmeuler_lmev_strut_edge(hedge_from_vertex2, 0., 10., 0., &hedge_from_vertex3);
    csmeuler_lmef(initial_hedge, hedge_from_vertex3, NULL, &he_pos, &he_neg);
    
    // Hueco de un sólo vértice...
    {
        struct csmhedge_t *he_from_vertex, *he_to_vertex;
        struct csmhedge_t *he1_hole_pos_next, *he2_hole_pos_next;
        struct csmhedge_t *hedge_lado_neg, *hedge_lado_pos;
        
        csmeuler_lmev(he_pos, he_pos, 1., 1., 0., NULL, NULL, &he_from_vertex, &he_to_vertex);
        csmeuler_lkemr(&he_to_vertex, &he_from_vertex, &he1_hole_pos_next, &he2_hole_pos_next);
        
        csmeuler_lmekr(he1_hole_pos_next, he2_hole_pos_next, &hedge_lado_neg, &hedge_lado_pos);
        csmeuler_lkev(&hedge_lado_neg, &hedge_lado_pos, &hedge_lado_neg, NULL, NULL, &hedge_lado_pos);
        assert(hedge_lado_neg != NULL);
        assert(hedge_lado_pos != NULL);
        assert(hedge_lado_pos == he_pos);
        assert(hedge_lado_neg == hedge_from_vertex1);
    }
    
    csmeuler_lkef(&he_pos, &he_neg);
    assert(he_pos != NULL);
    assert(he_neg != NULL);
    assert(he_pos == hedge_from_vertex3);
    
    he1 = he_pos;
    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2, NULL, &he1, &he2, NULL);
    assert(he1 == hedge_from_vertex2);

    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2, NULL, &he1, &he2, NULL);
    assert(he1 == hedge_from_vertex1);

    he2 = csmopbas_mate(he1);
    csmeuler_lkev(&he1, &he2, NULL, &he1, &he2, NULL);
    assert(he1 == he2);
    assert(he1 == initial_hedge);
    
    csmeuler_kvfs(solido);
    
    csmsolid_free(&solido);
}

// ------------------------------------------------------------------------------------------

static void i_test_crea_hexaedro(void)
{
    struct csmsolid_t *solido;
    unsigned long id_nuevo_elemento;
    struct csmhedge_t *hei, *he1, *he2, *he3, *he4;
    struct csmhedge_t *he1_top, *he2_top, *he3_top, *he4_top;
    struct csmhedge_t *he1_top_next, *he1_top_next_next, *he1_top_next_next_next;
    
    id_nuevo_elemento = 0;
    
    solido = csmeuler_mvfs(0., 0., 0., 0, &hei);
    csmsolid_print_debug(solido, CIERTO);
    
    // Cara inferior...
    {
        csmeuler_lmev_strut_edge(hei, 10.,  0., 0., &he1);
        csmeuler_lmev_strut_edge(he1, 10., 10., 0., &he2);
        csmeuler_lmev_strut_edge(he2,  0., 10., 0., &he3);
        csmeuler_lmef(hei, he3, NULL, &he4, NULL);
        assert(csmopbas_mate(he1) == hei);
    }

    csmsolid_print_debug(solido, CIERTO);
    
    // Aristas verticales...
    {
        csmeuler_lmev_strut_edge(he1, 10.,  0., 10., &he1_top);
        csmeuler_lmev_strut_edge(he2, 10., 10., 10., &he2_top);
        csmeuler_lmev_strut_edge(he3,  0., 10., 10., &he3_top);
        csmeuler_lmev_strut_edge(he4,  0.,  0., 10., &he4_top);
        
        assert(csmhedge_loop(he1_top) == csmhedge_loop(he2_top));
        assert(csmhedge_loop(he1_top) == csmhedge_loop(he3_top));
        assert(csmhedge_loop(he1_top) == csmhedge_loop(he4_top));
    }

    csmsolid_print_debug(solido, CIERTO);
    
    csmeuler_lmef(he4_top, he3_top, NULL, NULL, NULL);
    csmeuler_lmef(he3_top, he2_top, NULL, NULL, NULL);
    csmeuler_lmef(he2_top, he1_top, NULL, NULL, NULL);

    //csmeuler_lmef(he1_top, he4_top, &id_nuevo_elemento, NULL, NULL, NULL);
    
    he1_top_next = csmhedge_next(he1_top);
    assert(he1_top_next == he1);
    
    he1_top_next_next = csmhedge_next(he1_top_next);
    he1_top_next_next_next = csmhedge_next(he1_top_next_next);
    assert(csmhedge_vertex(he1_top_next_next_next) == csmhedge_vertex(he4_top));

    csmsolid_print_debug(solido, CIERTO);
    
    csmeuler_lmef(he1_top, he1_top_next_next_next, NULL, NULL, NULL);
    
    csmsolid_print_debug(solido, CIERTO);
    
    csmsolid_free(&solido);
}

// ------------------------------------------------------------------------------------------

static void i_test_crea_hexaedro_y_copia(void)
{
    struct csmsolid_t *solido;
    unsigned long id_nuevo_elemento;
    struct csmhedge_t *hei, *he1, *he2, *he3, *he4;
    struct csmhedge_t *he1_top, *he2_top, *he3_top, *he4_top;
    struct csmhedge_t *he1_top_next, *he1_top_next_next, *he1_top_next_next_next;
    struct csmsolid_t *copia_solido;
    
    id_nuevo_elemento = 0;
    
    solido = csmeuler_mvfs(0., 0., 0., 0, &hei);
    
    // Cara inferior...
    {
        csmeuler_lmev_strut_edge(hei, 10.,  0., 0., &he1);
        csmeuler_lmev_strut_edge(he1, 10., 10., 0., &he2);
        csmeuler_lmev_strut_edge(he2,  0., 10., 0., &he3);
        csmeuler_lmef(hei, he3, NULL, &he4, NULL);
        assert(csmopbas_mate(he1) == hei);
    }

    // Aristas verticales...
    {
        csmeuler_lmev_strut_edge(he1, 10.,  0., 10., &he1_top);
        csmeuler_lmev_strut_edge(he2, 10., 10., 10., &he2_top);
        csmeuler_lmev_strut_edge(he3,  0., 10., 10., &he3_top);
        csmeuler_lmev_strut_edge(he4,  0.,  0., 10., &he4_top);
        
        assert(csmhedge_loop(he1_top) == csmhedge_loop(he2_top));
        assert(csmhedge_loop(he1_top) == csmhedge_loop(he3_top));
        assert(csmhedge_loop(he1_top) == csmhedge_loop(he4_top));
    }

    csmeuler_lmef(he4_top, he3_top, NULL, NULL, NULL);
    csmeuler_lmef(he3_top, he2_top, NULL, NULL, NULL);
    csmeuler_lmef(he2_top, he1_top, NULL, NULL, NULL);

    //csmeuler_lmef(he1_top, he4_top, &id_nuevo_elemento, NULL, NULL, NULL);
    
    he1_top_next = csmhedge_next(he1_top);
    assert(he1_top_next == he1);
    
    he1_top_next_next = csmhedge_next(he1_top_next);
    he1_top_next_next_next = csmhedge_next(he1_top_next_next);
    assert(csmhedge_vertex(he1_top_next_next_next) == csmhedge_vertex(he4_top));

    csmeuler_lmef(he1_top, he1_top_next_next_next, NULL, NULL, NULL);
    
    csmsolid_print_debug(solido, CIERTO);
    
    copia_solido = csmsolid_duplicate(solido);
    csmsolid_print_debug(copia_solido, CIERTO);
    
    csmsolid_free(&solido);
    csmsolid_free(&copia_solido);
}

// ------------------------------------------------------------------------------------------

struct i_item_tabla_t
{
    unsigned long id;
    double a;
};

static void i_test_tabla_hash(void)
{
    struct csmhashtb(i_item_tabla_t) *tabla_hash;
    struct i_item_tabla_t item1, item2, item3;
    struct i_item_tabla_t *item;
    
    item1.id = 3;
    item1.a = 9.;

    item2.id = 5;
    item2.a = 19.;

    item3.id = 8;
    item3.a = 25.;
    
    tabla_hash = csmhashtb_create_empty(i_item_tabla_t);
    
    csmhashtb_add_item(tabla_hash, 3, &item1, i_item_tabla_t);
    csmhashtb_add_item(tabla_hash, 5, &item2, i_item_tabla_t);
    csmhashtb_add_item(tabla_hash, 1000, &item3, i_item_tabla_t);
    
    item = csmhashtb_ptr_for_id(tabla_hash, 5, i_item_tabla_t);
    assert_no_null(item);
    assert(item->id == 5);
    
    csmhashtb_remove_item(tabla_hash, 5, i_item_tabla_t);
    assert(csmhashtb_contains_id(tabla_hash, i_item_tabla_t, 5, NULL) == FALSO);

    csmhashtb_add_item(tabla_hash, 5, &item2, i_item_tabla_t);
    
    {
        struct csmhashtb_iterator(i_item_tabla_t) *it;
        
        it = csmhashtb_create_iterator(tabla_hash, i_item_tabla_t);
        
        while (csmhashtb_has_next(it, i_item_tabla_t) == CIERTO)
        {
            unsigned long id;
            struct i_item_tabla_t *item_tabla;
            
            csmhashtb_next_pair(it, &id, &item_tabla, i_item_tabla_t);
            fprintf(stdout, "%lu %lu %lf\n", id, item_tabla->id, item_tabla->a);
        }
        
        csmhashtb_free_iterator(&it, i_item_tabla_t);
    }
    
    csmhashtb_free(&tabla_hash, i_item_tabla_t, NULL);
}

// ------------------------------------------------------------------------------------------

static void i_test_solid_from_shape2D(void)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid;
    
    shape2d = gcelem2d_contorno_rectangular(0.3, 0.3);
    
    solid = csmsweep_create_solid_from_shape(
                                shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0.,
                                shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    
    csmsolid_print_debug(solid, CIERTO);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid);
}

// ------------------------------------------------------------------------------------------

static void i_test_solid_from_shape2D_with_hole(void)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid;
    
    shape2d = gcelem2d_contorno_rectangular_hueco(0.6, 0.6, 0.3, 0.3);
    
    solid = csmsweep_create_solid_from_shape(
                                shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0.,
                                shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    
    csmsolid_print_debug(solid, CIERTO);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid);
}

// ------------------------------------------------------------------------------------------

static void i_test_union_solidos_por_loopglue(void)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1, *solid2;
    CYBOOL could_merge_solids;
    
    shape2d = gcelem2d_contorno_rectangular(0.3, 0.3);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_print_debug(solid1, CIERTO);

    solid2 = csmsweep_create_solid_from_shape(shape2d, 0.3, 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0.3, 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_print_debug(solid2, CIERTO);

    csmglue_join_solid2_to_solid1_given_equal_faces(solid1, 46, &solid2, 36, &could_merge_solids);
    assert(could_merge_solids == CIERTO);

    csmsolid_print_debug(solid1, CIERTO);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
}

// ------------------------------------------------------------------------------------------

#include "copiafor.h"
#include "cypestr.h"

static void i_set_output_debug_file(const char *file_name)
{
    char *file_path;
    
    file_path = copiafor_codigo1("/Users/manueru/%s", file_name);
    csmdebug_set_ouput_file(file_path);
    
    cypestr_destruye(&file_path);
}

// ------------------------------------------------------------------------------------------

static void i_show_split_results(
                        double A, double B, double C, double D,
                        struct csmsolid_t *solid_above, struct csmsolid_t *solid_below)
{
    double desp;
    
    if (solid_above != NULL)
    {
        csmsolid_set_name(solid_above, "Above");
        csmsolid_print_debug(solid_above, CIERTO);
    }
    
    if (solid_below != NULL)
    {
        csmsolid_set_name(solid_below, "Below");
        csmsolid_print_debug(solid_below, CIERTO);
    }
    
    csmdebug_close_output_file();
    
    desp = 0.1;
    
    if (solid_above != NULL)
        csmsolid_move(solid_above, desp * A, desp * D, desp * C);
    
    if (solid_below != NULL)
        csmsolid_move(solid_below, -desp * A, -desp * D, -desp * C);

    if (solid_above != NULL || solid_below != NULL )
    {
        csmdebug_set_plane(A, B, C, D);
        csmdebug_set_viewer_results(solid_above, solid_below);
        csmdebug_show_viewer();
    }
}

// ------------------------------------------------------------------------------------------

static void i_test_divide_solido_rectangular_por_plano_medio(void)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1;
    double A, B, C, D;
    CYBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    i_set_output_debug_file("split1.txt");
    
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_set_name(solid1, "Solid to split");
    csmsolid_print_debug(solid1, CIERTO);
    
    csmmath_implicit_plane_equation(0., 0., .5, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CIERTO);
    
    i_show_split_results(A, B, C, D, solid_above, solid_below);

    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid_above);
    csmsolid_free(&solid_below);
}

// ------------------------------------------------------------------------------------------

static void i_test_divide_solido_rectangular_hueco_por_plano_medio(void)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1;
    double A, B, C, D;
    CYBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    i_set_output_debug_file("split_hueco_plano_medio.txt");

    shape2d = gcelem2d_contorno_rectangular_hueco(1., 1., 0.5, 0.5);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_print_debug(solid1, CIERTO);
    
    csmmath_implicit_plane_equation(0., 0., .5, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CIERTO);
    
    i_show_split_results(A, B, C, D, solid_above, solid_below);

    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid_above);
    csmsolid_free(&solid_below);
}

// ------------------------------------------------------------------------------------------

static void i_test_divide_solido_rectangular_hueco_por_plano_medio2(void)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1;
    double A, B, C, D;
    CYBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    i_set_output_debug_file("split_hueco_plano_medio2.txt");
    
    shape2d = gcelem2d_contorno_rectangular_hueco(1., 1., 0.5, 0.5);
    
    solid1 = csmsweep_create_solid_from_shape(
                        shape2d,
                        0., 0., 0.,
                        1., 0., 0., 0., 0., 1.,
                        shape2d,
                        0., 1., 0.,
                        1., 0., 0., 0., 0., 1.);
    csmsolid_print_debug(solid1, CIERTO);
    
    csmmath_implicit_plane_equation(0., 0., 0., 1., 0., 0., 0., 1., 0.5, &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CIERTO);

    i_show_split_results(A, B, C, D, solid_above, solid_below);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid_above);
    csmsolid_free(&solid_below);
}

// ------------------------------------------------------------------------------------------

static void i_test_divide_solido_rectangular_hueco_por_plano_superior(void)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1;
    double A, B, C, D;
    CYBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    i_set_output_debug_file("split_rectangular_hueco_por_plano_superior.txt");

    shape2d = gcelem2d_contorno_rectangular_hueco(1., 1., 0.5, 0.5);
    shape2d = gcelem2d_contorno_circular_hueco(1., 0.367, 32);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_print_debug(solid1, CIERTO);
    
    csmmath_implicit_plane_equation(0., 0., 1., 1., 0., 0., 0., 1., 1., &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CIERTO);
    
    csmsolid_free(&solid_above);

    csmmath_implicit_plane_equation(0., 0., 1., 1., 0., 0., 0., 1., -1., &A, &B, &C, &D);

    splitted = csmsplit_does_plane_split_solid(solid_below, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CIERTO);
    
    i_show_split_results(A, B, C, D, solid_above, solid_below);

    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
}

// ------------------------------------------------------------------------------------------

static void i_test_divide_solido_rectangular_hueco_por_plano_superior2(void)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1;
    double A, B, C, D;
    CYBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    shape2d = gcelem2d_contorno_rectangular_hueco(1., 1., 0.5, 0.5);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_print_debug(solid1, CIERTO);
    
    csmmath_implicit_plane_equation(0., 0., 1. - 1.5e-3, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CIERTO);

    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid_above, CIERTO);
    csmsolid_print_debug(solid_below, CIERTO);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid_above);
    csmsolid_free(&solid_below);
}

// ------------------------------------------------------------------------------------------

static void i_test_union_solidos1(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("union_solidos1.txt");
    
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);

    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    //solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., -0.25, 0.75, 1., 0., 0., 0., 1., 0., shape2d, 0., -0.25, 0., 1., 0., 0., 0., 1., 0., 0);
    //solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_union_solidos2(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("union_solidos2.txt");
    
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, equal vertex coordinates...
    //solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    //solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);

    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., -0.25, 0.75, 1., 0., 0., 0., 1., 0., shape2d, 0., -0.25, 0., 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid_res, CIERTO);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_interseccion_solidos1(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0.5, -0.25, 0.75, 1., 0., 0., 0., 1., 0., shape2d, 0., -0.25, 0., 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_interseccion_solidos2(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *circular_shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    circular_shape2d = gcelem2d_contorno_circular(0.25, 4);
    //circular_shape2d = gcelem2d_contorno_rectangular_con_pto_inflexion(0.25, 0.25);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    solid1 = csmsweep_create_solid_from_shape_debug(circular_shape2d, 0.5, -0.15, 0.75, 1., 0., 0., 0., 1., 0., circular_shape2d, 0.5, -0.15, -0.01, 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);

    solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
    csmsolid_print_debug(solid_res, CIERTO);
    
    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_interseccion_solidos5(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *circular_shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    //circular_shape2d = gcelem2d_contorno_circular(0.25, 4);
    circular_shape2d = gcelem2d_contorno_rectangular_con_pto_inflexion(0.5, 0.5);
    shape2d = gcelem2d_contorno_rectangular(2., 2.);
    
    solid1 = csmsweep_create_solid_from_shape_debug(circular_shape2d, 1.0, -0.15, 0.75, 1., 0., 0., 0., 1., 0., circular_shape2d, 1.0, -0.15, 0.01, 1., 0., 0., 0., 1., 0., 0);
    //solid1 = csmsweep_create_solid_from_shape_debug(circular_shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., circular_shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 2., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 2., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_interseccion_solidos7(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *c_shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    c_shape2d = gcelem2d_contorno_c(1., 0.9);
    //c_shape2d = gcelem2d_contorno_L(1., 1.);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    solid1 = csmsweep_create_solid_from_shape_debug(c_shape2d, 1. / 3., 0., 0.75, 1., 0., 0., 0., 1., 0., c_shape2d, 1. / 3., 0., 0.01, 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    //solid_res = csmsetop_union_A_and_B(solid1, solid2);
    //solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&shape2d);
    gccontorno_destruye(&c_shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_union_solidos6(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *circular_shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    struct gccontorno_t *shape3d;
    struct csmsolid_t *solid3, *solid_res2;
    
    circular_shape2d = gcelem2d_contorno_circular(0.25, 4);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    solid1 = csmsweep_create_solid_from_shape_debug(circular_shape2d, 0.5, -0.15, 1., 1., 0., 0., 0., 1., 0., circular_shape2d, 0.5, -0.15, 0.5, 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 0.5, 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    shape3d = gcelem2d_contorno_rectangular(0.75, 0.75);
    solid3 = csmsweep_create_solid_from_shape_debug(shape3d, 1., 0., 1, 1., 0., 0., 0., 1., 0., shape3d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 2000);
    
    solid_res2 = csmsetop_union_A_and_B(solid3, solid_res);
    csmsolid_print_debug(solid_res2, CIERTO);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
    csmsolid_free(&solid3);
    csmsolid_free(&solid_res2);
}

// ------------------------------------------------------------------------------------------

static void i_test_interseccion_solidos3(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *circular_shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("inters_solidos3.txt");

    circular_shape2d = gcelem2d_contorno_circular(0.25, 4);
    //circular_shape2d = gcelem2d_contorno_rectangular_con_pto_inflexion(0.25, 0.25);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    //solid1 = csmsweep_create_solid_from_shape_debug(circular_shape2d, 0.5, -0.16, 0.75, 1., 0., 0., 0., 1., 0., circular_shape2d, 0.5, -0.16, 0.0, 1., 0., 1., 0., 1., 0., 0);
    solid1 = csmsweep_create_solid_from_shape_debug(circular_shape2d, 0.5, -0.16, 0.75, 1., 0., 1., 0., 1., 0., circular_shape2d, 0.5, -0.16, 0.01, 1., 0., 1., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    csmdebug_print_debug_info("******* Solid 1 intersect solid 2 [begin]");
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);
    csmdebug_print_debug_info("******* Solid 1 intersect solid 2 [end]");

    csmdebug_print_debug_info("******* Solid 2 intersect solid 1 [begin]");
    solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
    csmsolid_print_debug(solid_res, CIERTO);
    csmdebug_print_debug_info("******* Solid 2 intersect solid 2 [end]");
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_interseccion_solidos4(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, equal vertex coordinates...
    //solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    //solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);

    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0.25, -0.25, 0.75, 1., 0., 0., 0., 1., 0., shape2d, 0.25, -0.25, 0., 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_resta_solidos1(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, equal vertex coordinates...
    //solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    //solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);

    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0.25, -0.25, 0.75, 1., 0., 0., 0., 1., 0., shape2d, 0.25, -0.25, 0., 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_resta_solidos2(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *rshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    rshape2d = gcelem2d_contorno_rectangular(0.25, 0.25);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, equal vertex coordinates...
    //solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    //solid2 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 1000);

    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    solid2 = csmsweep_create_solid_from_shape_debug(rshape2d, 0.5, -0.25, 0.75, 1., 0., 0., 0., 1., 0., rshape2d, 0.5, -0.25, 0., 1., 0., 0., 0., 1., 0., 1000);
    
    solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
    csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&rshape2d);
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_multiple_solidos1(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    cshape2d = gcelem2d_contorno_circular(0.5, 5);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    /*
     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., -1., 0., 1., 1., 1., 0.,
                        cshape2d, 0.5, -0.25,   0., -1., 0., 1., 1., 1., 0.,
                        1000);
     */

     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., 0., 0., 1., 1., 0., 0.,
                        cshape2d, 0.5, -0.25,   0., 0., 0., 1., 1., 0., 0.,
                        1000);
    
    solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
    solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
    solid_res = csmsetop_union_A_and_B(solid2, solid1);
    csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_multiple_solidos2(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    cshape2d = gcelem2d_contorno_circular(0.5, 16);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    /*
     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., -1., 0., 1., 1., 1., 0.,
                        cshape2d, 0.5, -0.25,   0., -1., 0., 1., 1., 1., 0.,
                        1000);
     */

     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., 0., 0., 1., 1., 0., 0.,
                        cshape2d, 0.5, -0.25,   0., 0., 0., 1., 1., 0., 0.,
                        1000);
    
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);
    
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);

    solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
    csmsolid_print_debug(solid_res, CIERTO);
    
    solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
    csmsolid_print_debug(solid_res, CIERTO);
    
    solid_res = csmsetop_union_A_and_B(solid2, solid1);
    csmsolid_print_debug(solid_res, CIERTO);
    
    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_multiple_solidos3(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    cshape2d = gcelem2d_contorno_circular(0.5, 16);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    /*
     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., -1., 0., 1., 1., 1., 0.,
                        cshape2d, 0.5, -0.25,   0., -1., 0., 1., 1., 1., 0.,
                        1000);
     */

     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., 0., 0., 1., 1., 0., 0.,
                        cshape2d, 0.5, -0.25,   0., 0., 0., 1., 1., 0., 0.,
                        1000);
    
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro1(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d, *shape2d2;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("union_cilindro1.txt");

    cshape2d = gcelem2d_contorno_circular(0.55, 4);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    shape2d2 = gcelem2d_contorno_rectangular(1.5, 0.75);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    /*
     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., -1., 0., 1., 1., 1., 0.,
                        cshape2d, 0.5, -0.25,   0., -1., 0., 1., 1., 1., 0.,
                        1000);
     */

     solid2 = csmsweep_create_solid_from_shape_debug(
                        shape2d2, 1.,  2., 1.5, 0., 0., 1., 1., 0., 0.,
                        shape2d2, 1., -2., .5, 0., 0., 1., 1., 0., 0.,
                        1000);
    
    {
        solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
        solid_res = csmsetop_union_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
    }

    {
        solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [begin]");
        solid_res = csmsetop_union_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [end]");
    }
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    gccontorno_destruye(&shape2d2);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro3(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("inters_cilindro2.txt");

    cshape2d = gcelem2d_contorno_circular(0.40, 16);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    /*
     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 0.5,  2.,     0., -1., 0., 1., 1., 1., 0.,
                        cshape2d, 0.5, -0.25,   0., -1., 0., 1., 1., 1., 0.,
                        1000);
     */

     solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  1., 1., 0., 0., 1., 1., 0., 0.,
                        cshape2d, 1., -1., 1., 0., 0., 1., 1., 0., 0.,
                        1000);
    
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    {
        struct csmsolid_t *solid3;
        struct csmsolid_t *solid_res2;
        
        cshape2d = gcelem2d_contorno_circular(0.10, 16);
        
        solid3 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, -1., 0., .25, 0., 1., 0., 0., 0., 1.,
                        cshape2d,  2., 0., .25, 0., 1., 0., 0., 0., 1.,
                        2000);
        
        solid_res2 = csmsetop_difference_A_minus_B(solid_res, solid3);
        
        //csmsolid_move(solid3, 0., -.4, 0.);
        //solid_res2 = csmsetop_difference_A_minus_B(solid_res2, solid3);

        //csmsolid_move(solid3, 0., 1., 0.);
        //solid_res2 = csmsetop_difference_A_minus_B(solid_res2, solid3);
        
        {
            CYBOOL does_split;
            struct csmsolid_t *solid_above, *solid_below;
            double desp;
            double A, B, C, D;
            
            A = 1.;
            B = C = 0.;
            D = -1.;
            
            does_split = csmsplit_does_plane_split_solid(solid_res2, A, B, C, D, &solid_above, &solid_below);
            assert(does_split == CIERTO);
            
            desp = 0.5;
            csmsolid_move(solid_above, A * desp, B * desp, C * desp);
            
            desp = -0.5;
            csmsolid_move(solid_below, A * desp, B * desp, C * desp);
            
            i_show_split_results(A, B, C, D, solid_above, solid_below);
            
            csmsolid_free(&solid_above);
            csmsolid_free(&solid_below);
        }
        
        csmsolid_free(&solid3);
        csmsolid_free(&solid_res2);
    }
    
    //solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    //solid_res = csmsetop_union_A_and_B(solid1, solid2);
    //csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro2(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid3, *solid_res;
    
    i_set_output_debug_file("inters_cilindro2.txt");
    csmdebug_set_enabled_by_code(FALSO);

    //cshape2d = gcelem2d_contorno_rectangular(0.80, 0.80);
    cshape2d = gcelem2d_contorno_circular(0.40, 8);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  2.,     0.5, -1., 0., 0., 0., 0., 1.,
                        cshape2d, 1., -2.,   0.5, -1., 0., 0., 0., 0., 1.,
                        //cshape2d, 1., -0.25,   0.5, -1., 0., 0., 0., 0., 1.,
                        5000);
    
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    
    {
        struct csmsolid_t *solid3, *solid4;
        
        solid3 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,  1., 0., 1.05, 1., 0., 0., 0., 1., 0.,
                        cshape2d,  1., 0., 0.0,  1., 0., 0., 0., 1., 0.,
                        10000);
        
        solid_res = csmsetop_difference_A_minus_B(solid_res, solid3);
        
        
        csmdebug_set_enabled_by_code(CIERTO);
        
        //cshape2d = gcelem2d_contorno_rectangular(0.75, 0.75);

        /*
        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,   2., 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,  -2., 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);
         
        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,     0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    -0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);

        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,    1.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);
         
         */
        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,    1.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    -0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);
        
        solid_res = csmsetop_difference_A_minus_B(solid_res, solid4);
        
        
        {
            CYBOOL does_split;
            struct csmsolid_t *solid_above, *solid_below;
            double desp;
            double A, B, C, D;
            
            A = 1.;
            B = C = 0.;
            D = -1.05;
            
            does_split = csmsplit_does_plane_split_solid(solid_res, A, B, C, D, &solid_above, &solid_below);
            assert(does_split == CIERTO);
            
            desp = 0.5;
            csmsolid_move(solid_above, A * desp, B * desp, C * desp);
            
            desp = -0.5;
            csmsolid_move(solid_below, A * desp, B * desp, C * desp);
            
            i_show_split_results(A, B, C, D, solid_above, solid_below);
            
            csmsolid_free(&solid_above);
            csmsolid_free(&solid_below);
        }
        
        csmsolid_free(&solid3);
    }
    
    //solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    //solid_res = csmsetop_union_A_and_B(solid1, solid2);
    //csmsolid_print_debug(solid_res, CIERTO);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_print_debug(solid1, CIERTO);
    csmsolid_print_debug(solid2, CIERTO);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

void csmtest_test(void)
{
    struct csmviewer_t *viewer;

    viewer = csmviewer_new();
    csmdebug_set_viewer(viewer, csmviewer_show, csmviewer_set_parameters, csmviewer_set_results);
    
    /*
    i_test_crea_destruye_solido_vacio();
    i_test_basico_solido_una_arista();
    i_test_crea_lamina();
    i_test_crea_lamina_con_hueco();
    i_test_crea_hexaedro();
    i_test_crea_hexaedro_y_copia();
    i_test_tabla_hash();
    i_test_solid_from_shape2D();
    i_test_solid_from_shape2D_with_hole();
    i_test_union_solidos_por_loopglue();
    */
    
    //i_test_divide_solido_rectangular_por_plano_medio();
    
    //i_test_divide_solido_rectangular_hueco_por_plano_medio();
    //i_test_divide_solido_rectangular_hueco_por_plano_medio2();
    //i_test_divide_solido_rectangular_hueco_por_plano_superior();
    //i_test_divide_solido_rectangular_hueco_por_plano_superior2();

    /*
    i_test_union_solidos1(viewer);
    i_test_union_solidos2(viewer);
    i_test_union_solidos6(viewer);  // --> Pendiente eliminar caras dentro de caras
    i_test_interseccion_solidos1(viewer);
    
    i_test_interseccion_solidos2(viewer);
    i_test_interseccion_solidos3(viewer);
    i_test_interseccion_solidos4(viewer);
    i_test_interseccion_solidos5(viewer);
    i_test_interseccion_solidos7(viewer);
    i_test_resta_solidos1(viewer);
    i_test_resta_solidos2(viewer);
    i_test_multiple_solidos1(viewer);
    i_test_multiple_solidos2(viewer);
    i_test_multiple_solidos3(viewer);

    i_test_cilindro1(viewer);
    */
     
    i_test_cilindro2(viewer);
    
    csmviewer_free(&viewer);
}













