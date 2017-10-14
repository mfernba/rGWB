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
#include "csmsolid_debug.inl"
#include "csmsweep.h"
#include "csmsplit.h"
#include "csmshape3d.h"
#include "csmassert.inl"
#include "csmstring.inl"

#include "a_pto2d.h"
#include <geomcomp/gccontorno.h>
#include <geomcomp/gcelem2d.h>

#include "csmdebug.inl"
#include "csmviewer.inl"
#include "csmmath.tli"
#include "csmmaterial.h"

#include "csmtest_array.inl"

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

        assert(csmface_contains_point(initial_face, 0.5, 0.5, 0.5, NULL, NULL, NULL, NULL) == CSMTRUE);
        assert(csmface_contains_point(initial_face, 1., 1., 1., NULL, NULL, NULL, NULL) == CSMTRUE);
        assert(csmface_contains_point(initial_face, 5., 5., 0., NULL, NULL, NULL, NULL) == CSMFALSE);
        
        assert(csmface_contains_point(new_face, 0.5, 0.5, 0.5, NULL, NULL, NULL, NULL) == CSMTRUE);
        assert(csmface_contains_point(new_face, 1., 1., 1., NULL, NULL, NULL, NULL) == CSMTRUE);
        assert(csmface_contains_point(new_face, 5., 5., 0., NULL, NULL, NULL, NULL) == CSMFALSE);
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
    csmsolid_debug_print_debug(solido, CSMTRUE);
    
    // Cara inferior...
    {
        csmeuler_lmev_strut_edge(hei, 10.,  0., 0., &he1);
        csmeuler_lmev_strut_edge(he1, 10., 10., 0., &he2);
        csmeuler_lmev_strut_edge(he2,  0., 10., 0., &he3);
        csmeuler_lmef(hei, he3, NULL, &he4, NULL);
        assert(csmopbas_mate(he1) == hei);
    }

    csmsolid_debug_print_debug(solido, CSMTRUE);
    
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

    csmsolid_debug_print_debug(solido, CSMTRUE);
    
    csmeuler_lmef(he4_top, he3_top, NULL, NULL, NULL);
    csmeuler_lmef(he3_top, he2_top, NULL, NULL, NULL);
    csmeuler_lmef(he2_top, he1_top, NULL, NULL, NULL);

    //csmeuler_lmef(he1_top, he4_top, &id_nuevo_elemento, NULL, NULL, NULL);
    
    he1_top_next = csmhedge_next(he1_top);
    assert(he1_top_next == he1);
    
    he1_top_next_next = csmhedge_next(he1_top_next);
    he1_top_next_next_next = csmhedge_next(he1_top_next_next);
    assert(csmhedge_vertex(he1_top_next_next_next) == csmhedge_vertex(he4_top));

    csmsolid_debug_print_debug(solido, CSMTRUE);
    
    csmeuler_lmef(he1_top, he1_top_next_next_next, NULL, NULL, NULL);
    
    csmsolid_debug_print_debug(solido, CSMTRUE);
    
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
    
    csmsolid_debug_print_debug(solido, CSMTRUE);
    
    copia_solido = csmsolid_duplicate(solido);
    csmsolid_debug_print_debug(copia_solido, CSMTRUE);
    
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
    assert(csmhashtb_contains_id(tabla_hash, i_item_tabla_t, 5, NULL) == CSMFALSE);

    csmhashtb_add_item(tabla_hash, 5, &item2, i_item_tabla_t);
    
    {
        struct csmhashtb_iterator(i_item_tabla_t) *it;
        
        it = csmhashtb_create_iterator(tabla_hash, i_item_tabla_t);
        
        while (csmhashtb_has_next(it, i_item_tabla_t) == CSMTRUE)
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
    
    csmsolid_debug_print_debug(solid, CSMTRUE);
    
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
    
    csmsolid_debug_print_debug(solid, CSMTRUE);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid);
}

// ------------------------------------------------------------------------------------------

static void i_test_union_solidos_por_loopglue(void)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1, *solid2;
    CSMBOOL could_merge_solids;
    
    shape2d = gcelem2d_contorno_rectangular(0.3, 0.3);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_debug_print_debug(solid1, CSMTRUE);

    solid2 = csmsweep_create_solid_from_shape(shape2d, 0.3, 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0.3, 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_debug_print_debug(solid2, CSMTRUE);

    csmglue_join_solid2_to_solid1_given_equal_faces(solid1, 46, &solid2, 36, &could_merge_solids);
    assert(could_merge_solids == CSMTRUE);

    csmsolid_debug_print_debug(solid1, CSMTRUE);
    
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
}

// ------------------------------------------------------------------------------------------

#include "csmstring.inl"

static void i_set_output_debug_file(const char *file_name)
{
    char *file_path;
    
    file_path = copiafor_codigo1("/Users/manueru/%s", file_name);
    csmdebug_set_ouput_file(file_path);
    
    csmstring_free(&file_path);
}

// ------------------------------------------------------------------------------------------

static void i_show_split_results(
                        double A, double B, double C, double D,
                        CSMBOOL show_plane,
                        struct csmsolid_t *solid_above, struct csmsolid_t *solid_below)
{
    double desp;
    
    if (solid_above != NULL)
    {
        csmsolid_set_name(solid_above, "Above");
        csmsolid_debug_print_debug(solid_above, CSMTRUE);
    }
    
    if (solid_below != NULL)
    {
        csmsolid_set_name(solid_below, "Below");
        csmsolid_debug_print_debug(solid_below, CSMTRUE);
    }
    
    csmdebug_close_output_file();
    
    desp = 0.1;
    
    if (solid_above != NULL)
        csmsolid_move(solid_above, desp * A, desp * D, desp * C);
    
    if (solid_below != NULL)
        csmsolid_move(solid_below, -desp * A, -desp * D, -desp * C);

    if (solid_above != NULL || solid_below != NULL )
    {
        if (show_plane == CSMTRUE)
            csmdebug_set_plane(A, B, C, D);
        else
            csmdebug_clear_plane();
        
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
    CSMBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    i_set_output_debug_file("split1.txt");
    
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_set_name(solid1, "Solid to split");
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    
    csmmath_implicit_plane_equation(0., 0., .5, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CSMTRUE);
    
    i_show_split_results(A, B, C, D, CSMTRUE, solid_above, solid_below);

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
    CSMBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    i_set_output_debug_file("split_hueco_plano_medio.txt");

    shape2d = gcelem2d_contorno_rectangular_hueco(1., 1., 0.5, 0.5);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    
    csmmath_implicit_plane_equation(0., 0., .5, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CSMTRUE);
    
    i_show_split_results(A, B, C, D, CSMTRUE, solid_above, solid_below);

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
    CSMBOOL splitted;
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
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    
    csmmath_implicit_plane_equation(0., 0., 0., 1., 0., 0., 0., 1., 0.5, &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CSMTRUE);

    i_show_split_results(A, B, C, D, CSMTRUE, solid_above, solid_below);
    
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
    CSMBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    i_set_output_debug_file("split_rectangular_hueco_por_plano_superior.txt");

    shape2d = gcelem2d_contorno_rectangular_hueco(1., 1., 0.5, 0.5);
    shape2d = gcelem2d_contorno_circular_hueco(1., 0.367, 32);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    
    csmmath_implicit_plane_equation(0., 0., 1., 1., 0., 0., 0., 1., 1., &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CSMTRUE);
    
    csmsolid_free(&solid_above);

    csmmath_implicit_plane_equation(0., 0., 1., 1., 0., 0., 0., 1., -1., &A, &B, &C, &D);

    splitted = csmsplit_does_plane_split_solid(solid_below, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CSMTRUE);
    
    i_show_split_results(A, B, C, D, CSMTRUE, solid_above, solid_below);

    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
}

// ------------------------------------------------------------------------------------------

static void i_test_divide_solido_rectangular_hueco_por_plano_superior2(void)
{
    struct gccontorno_t *shape2d;
    struct csmsolid_t *solid1;
    double A, B, C, D;
    CSMBOOL splitted;
    struct csmsolid_t *solid_above, *solid_below;
    
    shape2d = gcelem2d_contorno_rectangular_hueco(1., 1., 0.5, 0.5);
    
    solid1 = csmsweep_create_solid_from_shape(shape2d, 0., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 0., 0., 0., 1., 0., 0., 0., 1., 0.);
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    
    csmmath_implicit_plane_equation(0., 0., 1. - 1.5e-3, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
    
    splitted = csmsplit_does_plane_split_solid(solid1, A, B, C, D, &solid_above, &solid_below);
    assert(splitted == CSMTRUE);

    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid_above, CSMTRUE);
    csmsolid_debug_print_debug(solid_below, CSMTRUE);
    
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
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
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    shape3d = gcelem2d_contorno_rectangular(0.75, 0.75);
    solid3 = csmsweep_create_solid_from_shape_debug(shape3d, 1., 0., 1, 1., 0., 0., 0., 1., 0., shape3d, 1., 0., 0., 1., 0., 0., 0., 1., 0., 2000);
    
    solid_res2 = csmsetop_union_A_and_B(solid3, solid_res);
    csmsolid_debug_print_debug(solid_res2, CSMTRUE);
    
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    csmdebug_print_debug_info("******* Solid 1 intersect solid 2 [end]");

    csmdebug_print_debug_info("******* Solid 2 intersect solid 1 [begin]");
    solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    solid_res = csmsetop_union_A_and_B(solid1, solid2);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    solid_res = csmsetop_union_A_and_B(solid2, solid1);
    csmsolid_debug_print_debug(solid_res, CSMTRUE);
    
    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
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
    csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
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
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    gccontorno_destruye(&shape2d2);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro2(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d, *shape2d2;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("test_cilindro1.txt");

    cshape2d = gcelem2d_contorno_circular(0.55, 4);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    shape2d2 = gcelem2d_contorno_rectangular(1.5, 0.75);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1.25, 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  2.,   1., -1., 0., 0., 0., 0., 1.,
                        cshape2d, 1., -2., 1., -1., 0., 0., 0., 0., 1.,
                        1000);
     /*
      solid2 = csmsweep_create_solid_from_shape_debug(
                        shape2d2, 1.,  2., 1.5, 0., 0., 1., 1., 0., 0.,
                        shape2d2, 1., -2., .5, 0., 0., 1., 1., 0., 0.,
                        1000);
      */
    
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
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    gccontorno_destruye(&shape2d2);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro5(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d, *shape2d2;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("test_cilindro5.txt");

    cshape2d = gcelem2d_contorno_circular(0.25, 4);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    shape2d2 = gcelem2d_contorno_rectangular(1.5, 0.75);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1.25, 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, .75,  0.25,   1., -1., 0., 0., 0., 0., 1.,
                        cshape2d, .75, -2.,     1., -1., 0., 0., 0., 0., 1.,
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
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    gccontorno_destruye(&shape2d2);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro6(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d, *shape2d2;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("test_cilindro6.txt");

    cshape2d = gcelem2d_contorno_circular(0.50, 4);
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    shape2d2 = gcelem2d_contorno_rectangular(1.5, 0.75);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1.25, 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  2., 1., 0., 0., 1., 1., 0., 0.,
                        cshape2d, 1., -2., 1., 0., 0., 1., 1., 0., 0.,
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
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    gccontorno_destruye(&shape2d2);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro7(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("test_cilindro7.txt");

    {
        double ax, ay;
        ArrPunto2D *points;
        
        ax = 0.5;
        ay = 0.5;
        
        points = arr_CreaPunto2D(0);
        arr_AppendPunto2D(points, -0.5 * ax, -0.5 * ay);
        arr_AppendPunto2D(points,  0.5 * ax, -0.5 * ay);
        arr_AppendPunto2D(points,  0., 0.5 * ay);
        
        cshape2d = gccontorno_crea_vacio();
        gccontorno_append_array_puntos(cshape2d, &points);
    }
    
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  2., 0.75, -1., 0., 0., 0., 0., 1.,
                        cshape2d, 1., -2., 0.75, -1., 0., 0., 0., 0., 1.,
                        1000);
    
        solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
        csmsolid_free(&solid_res);
    
        solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
        solid_res = csmsetop_union_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
     

        solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [begin]");
        solid_res = csmsetop_union_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [end]");

    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro3(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    unsigned long no_sides_circle;
    
    i_set_output_debug_file("inters_cilindro2.txt");

    no_sides_circle = 32;
    
    cshape2d = gcelem2d_contorno_circular(0.40, no_sides_circle);
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
                        0);
    
    solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
    {
        struct csmsolid_t *solid3;
        struct csmsolid_t *solid_res2;
        
        cshape2d = gcelem2d_contorno_circular(0.40, no_sides_circle);
        
        solid3 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,  2., 0., .25, 0., 1., 0., 0., 0., 1.,
                        cshape2d, -1., 0., .25, 0., 1., 0., 0., 0., 1.,
                        0);
        
        solid_res2 = csmsetop_difference_A_minus_B(solid_res, solid3);
        
        //csmsolid_move(solid3, 0., -.4, 0.);
        //solid_res2 = csmsetop_difference_A_minus_B(solid_res2, solid3);

        //csmsolid_move(solid3, 0., 1., 0.);
        //solid_res2 = csmsetop_difference_A_minus_B(solid_res2, solid3);
        
        {
            CSMBOOL does_split;
            struct csmsolid_t *solid_above, *solid_below;
            double desp;
            double A, B, C, D;
            
            A = 1.;
            B = C = 0.;
            D = -1.;
            
            does_split = csmsplit_does_plane_split_solid(solid_res2, A, B, C, D, &solid_above, &solid_below);
            assert(does_split == CSMTRUE);
            
            desp = 0.5;
            csmsolid_move(solid_above, A * desp, B * desp, C * desp);
            
            desp = -0.5;
            csmsolid_move(solid_below, A * desp, B * desp, C * desp);
            
            i_show_split_results(A, B, C, D, CSMTRUE, solid_above, solid_below);
            
            csmsolid_free(&solid_above);
            csmsolid_free(&solid_below);
        }
        
        csmsolid_free(&solid3);
        csmsolid_free(&solid_res2);
    }
    
    //solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    //solid_res = csmsetop_union_A_and_B(solid1, solid2);
    //csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro4(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid3, *solid_res;
    unsigned long no_points_circle;
    
    i_set_output_debug_file("inters_cilindro2.txt");
    csmdebug_set_enabled_by_code(CSMFALSE);

    no_points_circle = 32;
    
    //cshape2d = gcelem2d_contorno_rectangular(0.80, 0.80);
    cshape2d = gcelem2d_contorno_circular(0.40, no_points_circle);
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
        
        
        csmdebug_set_enabled_by_code(CSMTRUE);
        
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
         
        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,    1.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    -0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);
        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,     0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    -0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);

         */
        
        solid4 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,    1.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    -0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);
        
        solid_res = csmsetop_difference_A_minus_B(solid_res, solid4);
        
        {
            struct csmsolid_t *solid5;
            
            cshape2d = gcelem2d_contorno_circular(0.46, no_points_circle); // Revisar resta con num_puntos_circulo = 8
            
            solid5 = csmsweep_create_solid_from_shape_debug(
                        cshape2d,    1.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        cshape2d,    -0.75, 0., 0.5,  0., 1., 0., 0., 0., 1.,
                        20000);
            
            csmsolid_move(solid1, -0.5, 0., 0.);
            //solid_res = csmsetop_difference_A_minus_B(solid1, solid_res);
            solid_res = csmsetop_difference_A_minus_B(solid_res, solid5);
            //solid_res = csmsetop_intersection_A_and_B(solid_res, solid5);
            //solid_res = csmsetop_intersection_A_and_B(solid_res5, solid_res);
        }
        
        {
            CSMBOOL does_split;
            struct csmsolid_t *solid_above, *solid_below;
            double desp;
            double A, B, C, D;
            
            A = 0.;
            B = 0;
            C = 1.;
            D = -0.55; // Falla split a -0.75
            
            does_split = csmsplit_does_plane_split_solid(solid_res, A, B, C, D, &solid_above, &solid_below);
            assert(does_split == CSMTRUE);
            
            desp = 0.5;
            csmsolid_move(solid_above, A * desp, B * desp, C * desp);
            
            desp = -0.5;
            csmsolid_move(solid_below, A * desp, B * desp, C * desp);
            
            i_show_split_results(A, B, C, D, CSMFALSE, solid_above, solid_below);
            
            csmsolid_free(&solid_above);
            csmsolid_free(&solid_below);
        }
        
        csmsolid_free(&solid3);
    }
    
    //solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
    //solid_res = csmsetop_union_A_and_B(solid1, solid2);
    //csmsolid_debug_print_debug(solid_res, CSMTRUE);

    csmviewer_set_results(viewer, solid_res, NULL);
    //csmviewer_show(viewer);
    
    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
    csmsolid_free(&solid_res);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro8(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("test_cilindro7.txt");

    {
        double ax, ay;
        ArrPunto2D *points;
        
        ax = 0.5;
        ay = 0.5;
        
        points = arr_CreaPunto2D(0);
        arr_AppendPunto2D(points, -0.5 * ax, -0.5 * ay);
        arr_AppendPunto2D(points,  0.5 * ax, -0.5 * ay);
        arr_AppendPunto2D(points,  0.01 * ax, 0.5 * ay);
        arr_AppendPunto2D(points,  -0.01 * ax, 0.5 * ay);
        
        cshape2d = gccontorno_crea_vacio();
        gccontorno_append_array_puntos(cshape2d, &points);
    }
    
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  2., 0.75, -1., 0., 0., 0., 0., 1.,
                        cshape2d, 1., -2., 0.75, -1., 0., 0., 0., 0., 1.,
                        1000);
    
        solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
        csmsolid_free(&solid_res);
    
        solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
        solid_res = csmsetop_union_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
     

        solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [begin]");
        solid_res = csmsetop_union_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [end]");

    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

static void i_test_cilindro9(struct csmviewer_t *viewer)
{
    struct gccontorno_t *shape2d, *cshape2d;
    struct csmsolid_t *solid1, *solid2, *solid_res;
    
    i_set_output_debug_file("test_cilindro7.txt");

    {
        double ax, ay;
        ArrPunto2D *points;
        
        ax = 0.5;
        ay = 0.5;
        
        points = arr_CreaPunto2D(0);
        arr_AppendPunto2D(points, -0.5 * ax, -0.5 * ay);
        arr_AppendPunto2D(points,  0.5 * ax, -0.5 * ay);
        arr_AppendPunto2D(points,  0., 0.5 * ay);
        
        cshape2d = gccontorno_crea_vacio();
        gccontorno_append_array_puntos(cshape2d, &points);
    }
    
    shape2d = gcelem2d_contorno_rectangular(1., 1.);
    
    // Adjacent solids to face at 0.5, 0.5, NON equal vertex coordinates...
    solid1 = csmsweep_create_solid_from_shape_debug(shape2d, 1., 0., 1., 1., 0., 0., 0., 1., 0., shape2d, 1., 0., 0.05, 1., 0., 0., 0., 1., 0., 0);
    
    solid2 = csmsweep_create_solid_from_shape_debug(
                        cshape2d, 1.,  2., 1.25, 1., 0., 0., 0., 0., -1.,
                        cshape2d, 1., -2., 1.25, 1., 0., 0., 0., 0., -1.,
                        1000);
    
        solid_res = csmsetop_difference_A_minus_B(solid1, solid2);
        csmsolid_free(&solid_res);
    
        solid_res = csmsetop_intersection_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
        solid_res = csmsetop_union_A_and_B(solid1, solid2);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 1 union solid 2 [begin]");
     

        solid_res = csmsetop_difference_A_minus_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        solid_res = csmsetop_intersection_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [begin]");
        solid_res = csmsetop_union_A_and_B(solid2, solid1);
        csmsolid_free(&solid_res);
        csmdebug_print_debug_info("******* Solid 2 union solid 1 [end]");

    csmsolid_debug_print_debug(solid1, CSMTRUE);
    csmsolid_debug_print_debug(solid2, CSMTRUE);
    
    gccontorno_destruye(&cshape2d);
    gccontorno_destruye(&shape2d);
    csmsolid_free(&solid1);
    csmsolid_free(&solid2);
}

// ------------------------------------------------------------------------------------------

CONSTRUCTOR(static struct csmsolid_t *, i_make_ring_part, (
                        double Zo,
                        double height,
                        double outer_radius, double inner_radius, unsigned long no_points_circle))
{
    struct csmsolid_t *ring_part;
    struct gccontorno_t *ring_shape;
    
    ring_shape = gcelem2d_contorno_circular_hueco(outer_radius, inner_radius, no_points_circle);
    
    ring_part = csmsweep_create_solid_from_shape_debug(
                        ring_shape,
                        0., 0., Zo + height, 1., 0., 0., 0., 1., 0.,
                        ring_shape,
                        0., 0., Zo, 1., 0., 0., 0., 1., 0.,
                        1000);
    
    gccontorno_destruye(&ring_shape);
    
    return ring_part;
}

// ------------------------------------------------------------------------------------------

static void i_test_mechanical_part1(void)
{
    struct csmsolid_t *basic_part;
    struct csmsolid_t *main_part;
    double ax, ay, main_part_length;
    struct gccontorno_t *main_part_shape;
    
    i_set_output_debug_file("mechanical1.txt");
    
    ax = 0.1;
    ay = 0.1;
    main_part_length = 1.;
    
    csmdebug_set_enabled_by_code(CSMFALSE);
    
    //main_part_shape = gcelem2d_contorno_rectangular(ax, ay);
    main_part_shape = gcelem2d_contorno_circular(ax, 32);
    
    basic_part = csmsweep_create_solid_from_shape_debug(
                        main_part_shape,
                        0., 0., main_part_length, 1., 0., 0., 0., 1., 0.,
                        main_part_shape,
                        0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);
    
    main_part = csmsolid_duplicate(basic_part);
    
    {
        unsigned long i, no_divisions;
        double ring_part_height;
        struct csmsolid_t *ring_part, *ring_part2;
        double incr_z;
        unsigned long no_operations;
        
        no_divisions = 20;
        ring_part_height = main_part_length / no_divisions;
        ring_part = i_make_ring_part(0., ring_part_height, /*1.2*/1.5 * ax, 0.4 * ax, 16);
        ring_part2 = i_make_ring_part(0., ring_part_height, 1.2 * ax, 0.4 * ax, 4);
        
        incr_z = ring_part_height;
        no_operations = 0;
        
        for (i = 0; i < no_divisions - 1; i++)
        {
            if (i % 2 == 1)
            {
                struct csmsolid_t *main_part_loc;
            
                if (no_operations % 2 == 0)
                    main_part_loc = csmsetop_difference_A_minus_B(main_part, ring_part);
                else
                    main_part_loc = csmsetop_difference_A_minus_B(main_part, ring_part2);
                
                csmsolid_free(&main_part);
                main_part = main_part_loc;
                
                no_operations++;
                //break;
            }
            
            csmsolid_move(ring_part, 0., 0., incr_z);
            csmsolid_move(ring_part2, 0., 0., incr_z);
        }
     
        csmsolid_free(&ring_part);
        csmsolid_free(&ring_part2);
     }
    
    csmdebug_set_enabled_by_code(CSMTRUE);
    csmdebug_set_viewer_results(main_part, NULL);
    csmdebug_show_viewer();
    //csmdebug_set_enabled_by_code(CSMFALSE);
   
    
    {
        struct gccontorno_t *diff_shape;
        struct csmsolid_t *basic_part2, *main_part_loc;
        
        //diff_shape = gcelem2d_contorno_rectangular(0.6 * ax, 0.6 * ax);
        diff_shape = gcelem2d_contorno_circular(0.8 * ax, 16);
        
        basic_part2 = csmsweep_create_solid_from_shape_debug(
                        diff_shape,
                        0., 0., /*0.55*/ 0.5 * main_part_length, 1., 0., 0., 0., 1., 0.,
                        diff_shape,
                        0., 0., 0.05, 1., 0., 0., 0., 1., 0.,
                        1000);

        csmsolid_move(basic_part2, 0.25 * ax, 0., 0./*0.05 * main_part_length*/);
        
        main_part_loc = csmsetop_difference_A_minus_B(main_part, basic_part2);
        csmsolid_free(&main_part);
        main_part = main_part_loc;
        
        gccontorno_destruye(&diff_shape);
        csmsolid_free(&basic_part2);
    }

    csmdebug_set_enabled_by_code(CSMTRUE);

    csmdebug_set_viewer_results(main_part, NULL);
    csmdebug_show_viewer();
    
    {
        double A, B, C, D;
        struct csmsolid_t *solid_above, *solid_below;
        CSMBOOL splitted;

        csmdebug_set_enabled_by_code(CSMFALSE);
        
        csmmath_implicit_plane_equation(0., 0., 0., 0., 1., 0., 0., 0., 1., &A, &B, &C, &D);
    
        splitted = csmsplit_does_plane_split_solid(main_part, A, B, C, D, &solid_above, &solid_below);
        assert(splitted == CSMTRUE);

        csmdebug_set_enabled_by_code(CSMTRUE);
        
        i_show_split_results(A, B, C, D, CSMFALSE, solid_above, solid_below);
    }
    
    gccontorno_destruye(&main_part_shape);
    csmsolid_free(&main_part);
    csmsolid_free(&basic_part);
}

// ------------------------------------------------------------------------------------------

static void i_test_mechanical_part1_redux(void)
{
    struct csmsolid_t *basic_part;
    struct csmsolid_t *main_part;
    double ax, ay, main_part_length;
    struct gccontorno_t *main_part_shape;
    
    i_set_output_debug_file("mechanical1_redux.txt");
    
    ax = 0.1;
    ay = 0.1;
    main_part_length = 1.;
    
    csmdebug_set_enabled_by_code(CSMFALSE);
    
    main_part_shape = gcelem2d_contorno_rectangular(ax, ay);
    //main_part_shape = gcelem2d_contorno_circular(ax, 32);
    
    basic_part = csmsweep_create_solid_from_shape_debug(
                        main_part_shape,
                        0., 0., main_part_length, 1., 0., 0., 0., 1., 0.,
                        main_part_shape,
                        0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);
    
    main_part = csmsolid_duplicate(basic_part);
    
    {
        unsigned long i, no_divisions;
        double ring_part_height;
        struct csmsolid_t *ring_part, *ring_part2;
        double incr_z;
        unsigned long no_operations;
        
        no_divisions = 20;
        ring_part_height = main_part_length / no_divisions;
        ring_part = i_make_ring_part(0., ring_part_height, /*1.2*/1.5 * ax, 0.4 * ax, 16);
        ring_part2 = i_make_ring_part(0., ring_part_height, 1.2 * ax, 0.4 * ax, 4);
        
        incr_z = ring_part_height;
        no_operations = 0;
        
        for (i = 0; i < no_divisions - 1; i++)
        {
            if (i % 2 == 1)
            {
                struct csmsolid_t *main_part_loc;
            
                if (no_operations % 2 == 0)
                    main_part_loc = csmsetop_difference_A_minus_B(main_part, ring_part);
                else
                    main_part_loc = csmsetop_difference_A_minus_B(main_part, ring_part2);
                
                csmsolid_free(&main_part);
                main_part = main_part_loc;
                
                no_operations++;
                //break;
            }
            
            csmsolid_move(ring_part, 0., 0., incr_z);
            csmsolid_move(ring_part2, 0., 0., incr_z);
        }
     
        csmsolid_free(&ring_part);
        csmsolid_free(&ring_part2);
     }
    
    csmdebug_set_enabled_by_code(CSMTRUE);
    csmdebug_set_viewer_results(main_part, NULL);
    csmdebug_show_viewer();
    //csmdebug_set_enabled_by_code(CSMFALSE);
   
    
    {
        double A, B, C, D;
        struct csmsolid_t *solid_above, *solid_below;
        CSMBOOL splitted;

        csmdebug_set_enabled_by_code(CSMFALSE);
        
        csmmath_implicit_plane_equation(0., 0., 0.5, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
    
        splitted = csmsplit_does_plane_split_solid(main_part, A, B, C, D, &solid_above, &solid_below);
        assert(splitted == CSMTRUE);

        csmdebug_set_enabled_by_code(CSMTRUE);
        
        main_part = solid_above;
        
        csmmath_implicit_plane_equation(0., 0., 0.56, 1., 0., 0., 0., 1., 0., &A, &B, &C, &D);
        splitted = csmsplit_does_plane_split_solid(main_part, A, B, C, D, &solid_above, &solid_below);
        assert(splitted == CSMTRUE);
        
        main_part = solid_below;
        
        csmdebug_clear_plane();
    }
    
    {
        struct gccontorno_t *diff_shape;
        struct csmsolid_t *basic_part2, *main_part_loc;
        
        //diff_shape = gcelem2d_contorno_rectangular(0.6 * ax, 0.6 * ax);
        diff_shape = gcelem2d_contorno_circular(0.6 * ax, 6);
        
        basic_part2 = csmsweep_create_solid_from_shape_debug(
                        diff_shape,
                        0., 0., 0.55 /*0.5 * main_part_length*/, 1., 0., 0., 0., 1., 0.,
                        diff_shape,
                        0., 0., 0.5, 1., 0., 0., 0., 1., 0.,
                        1000);

        csmsolid_move(basic_part2, 0.25 * ax, 0., 0./*0.05 * main_part_length*/);
        
        main_part_loc = csmsetop_difference_A_minus_B(main_part, basic_part2);
        csmsolid_free(&main_part);
        main_part = main_part_loc;
        
        gccontorno_destruye(&diff_shape);
        csmsolid_free(&basic_part2);
    }

    csmdebug_set_enabled_by_code(CSMTRUE);

    csmdebug_set_viewer_results(main_part, NULL);
    csmdebug_show_viewer();
    
    {
        double A, B, C, D;
        struct csmsolid_t *solid_above, *solid_below;
        CSMBOOL splitted;

        csmdebug_set_enabled_by_code(CSMFALSE);
        
        csmmath_implicit_plane_equation(0., 0., 0., 0., 1., 0., 0., 0., 1., &A, &B, &C, &D);
    
        splitted = csmsplit_does_plane_split_solid(main_part, A, B, C, D, &solid_above, &solid_below);
        assert(splitted == CSMTRUE);

        csmdebug_set_enabled_by_code(CSMTRUE);
        
        i_show_split_results(A, B, C, D, CSMFALSE, solid_above, solid_below);
    }
    
    gccontorno_destruye(&main_part_shape);
    csmsolid_free(&main_part);
    csmsolid_free(&basic_part);
}

// ------------------------------------------------------------------------------------------

static void i_test_mechanical_part2(void)
{
    double ax, ay, head_length, bolt_length, thread_length;
    struct gccontorno_t *head_shape, *body_shape, *thread_shape;
    struct csmsolid_t *head_solid, *body_solid, *thread_solid, *bolt_solid;
    
    i_set_output_debug_file("mechanical2.txt");
    
    ax = 0.025;
    ay = 0.1;
    head_length = 0.01;
    bolt_length = 0.08;
    thread_length = 0.0025;
    
    //main_part_shape = gcelem2d_contorno_rectangular(ax, ay);
    head_shape = gcelem2d_contorno_circular(ax, 6);
    body_shape = gcelem2d_contorno_circular(0.5 * ax, 32);
    thread_shape = gcelem2d_contorno_circular(0.6 * ax, 32);
    
    head_solid = csmsweep_create_solid_from_shape_debug(
                        head_shape,
                        0., 0., head_length, 1., 0., 0., 0., 1., 0.,
                        head_shape,
                        0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);

    body_solid = csmsweep_create_solid_from_shape_debug(
                        body_shape,
                        0., 0., bolt_length, 1., 0., 0., 0., 1., 0.,
                        body_shape,
                        0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);
    
    thread_solid = csmsweep_create_solid_from_shape_debug(
                        thread_shape,
                        0., 0., thread_length, 1., 0., 0., 0., 1., 0.,
                        thread_shape,
                        0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);
    
    csmsolid_move(head_solid, 0., 0., bolt_length);
    
    bolt_solid = csmsetop_union_A_and_B(head_solid, body_solid);
    
    csmdebug_set_enabled_by_code(CSMFALSE);
    {
        unsigned long i, num_divisions;
        
        num_divisions = bolt_length / thread_length;
        
        for (i = 0; i < num_divisions; i++)
        {
            if (i % 2 == 0)
                bolt_solid = csmsetop_union_A_and_B(bolt_solid, thread_solid);
            
            csmsolid_move(thread_solid, 0., 0., thread_length);
        }
    }
    csmdebug_set_enabled_by_code(CSMTRUE);
    
    csmdebug_set_viewer_results(bolt_solid, NULL);
    csmdebug_show_viewer();
    
    {
        struct csmsolid_t *bolt_solid2, *bolt_solid3;
        struct gccontorno_t *block_shape;
        struct csmsolid_t *block_solid;
        
        bolt_solid2 = csmsolid_duplicate(bolt_solid);
        csmsolid_rotate(bolt_solid2, 0.6 * CSMMATH_PI, 0., 0., 0.5 * (bolt_length + head_length), 0., 1., 0.);
        csmsolid_move(bolt_solid2, 0.1 - 0.2 * (bolt_length + head_length) + head_length, 0., 0.);
        
        bolt_solid3 = csmsolid_duplicate(bolt_solid);
        csmsolid_rotate(bolt_solid3, -0.5 * CSMMATH_PI, 0., 0., 0.5 * (bolt_length + head_length), 0., 1., 0.);
        csmsolid_move(bolt_solid3, -0.1 + 0.5 * (bolt_length + head_length) - head_length, 0., 0.);

        csmdebug_set_viewer_results(bolt_solid2, bolt_solid3);
        csmdebug_show_viewer();
        
        csmdebug_set_enabled_by_code(CSMFALSE);
        {
            block_shape = gcelem2d_contorno_rectangular(0.2, 0.2);
        
            block_solid = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        0., 0., bolt_length + head_length, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        0., 0., 0., 1., 0., 0., 0., 1., 0.,
                        0);

            csmdebug_set_viewer_results(bolt_solid2, block_solid);
            csmdebug_show_viewer();
            
            block_solid = csmsetop_difference_A_minus_B(block_solid, bolt_solid);
            block_solid = csmsetop_difference_A_minus_B(block_solid, bolt_solid2);
            block_solid = csmsetop_difference_A_minus_B(block_solid, bolt_solid3);
        }
        csmdebug_set_enabled_by_code(CSMTRUE);
        
        csmdebug_set_viewer_results(block_solid, NULL);
        csmdebug_show_viewer();
        
        {
            double A, B, C, D;
            struct csmsolid_t *solid_above, *solid_below;
            CSMBOOL splitted;

            csmdebug_set_enabled_by_code(CSMFALSE);
            
            csmmath_implicit_plane_equation(0., 0., 0., 0., 0., 1., 1., 0., 0., &A, &B, &C, &D);
        
            splitted = csmsplit_does_plane_split_solid(block_solid, A, B, C, D, &solid_above, &solid_below);
            assert(splitted == CSMTRUE);

            csmdebug_set_enabled_by_code(CSMTRUE);
            
            i_show_split_results(A, B, C, D, CSMFALSE, solid_above, NULL);
        }
    }
}

// ------------------------------------------------------------------------------------------

static void i_test_toroide(void)
{
    struct csmsolid_t *toroide;
    struct csmsolid_t *block;
    struct csmsolid_t *res;
    
    //toroide = csmshape3d_create_torus(3., .75, 3, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    toroide = csmshape3d_create_torus(
                        3.,  16,
                        .75, 8,
                        0., 0., 0.,
                        0., 1., 0.,
                        0., 0., 1.,
                        1);
    
    {
        struct gccontorno_t *block_shape;
        
        block_shape = gcelem2d_contorno_rectangular(6., 6.);
        
        block = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        0., 0.,  0.5, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        0., 0., -0.5, 1., 0., 0., 0., 1., 0.,
                        10000);
    }
    
    res = csmsetop_difference_A_minus_B(block, toroide);
    
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

static void i_test_cono(void)
{
    struct csmsolid_t *cono;
    struct csmsolid_t *block;
    struct csmsolid_t *res;
    
    //toroide = csmshape3d_create_torus(3., .75, 3, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    cono = csmshape3d_create_cone(
                        5., 1., 32,
                        0., 0., 0.,
                        1., 0., 0.,
                        0., 0., 1.,
                        1);

    csmdebug_set_viewer_results(cono, NULL);
    csmdebug_show_viewer();
    
    {
        struct gccontorno_t *block_shape;
        
        block_shape = gcelem2d_contorno_rectangular(6., 6.);
        
        block = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        0., 0.,  0.5, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        0., 0., -0.5, 1., 0., 0., 0., 1., 0.,
                        10000);
    }
    
    res = csmsetop_difference_A_minus_B(block, cono);
    
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

static void i_test_sphere(void)
{
    struct csmsolid_t *cono;
    struct csmsolid_t *block;
    struct csmsolid_t *res;
    
    //toroide = csmshape3d_create_torus(3., .75, 3, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    cono = csmshape3d_create_sphere(
                        2.,
                        8,
                        16,
                        1.2, 1., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);

    csmdebug_set_viewer_results(cono, NULL);
    //csmdebug_show_viewer();
    
    {
        struct gccontorno_t *block_shape;
        
        block_shape = gcelem2d_contorno_rectangular(6., 6.);
        
        block = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        0., 0.,  0.5, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        0., 0., -0.5, 1., 0., 0., 0., 1., 0.,
                        10000);
    }
    
    res = csmsetop_difference_A_minus_B(block, cono);
    
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

static void i_test_sphere2(void)
{
    struct csmsolid_t *sphere, *torus, *torus2, *cone;
    struct csmsolid_t *block;
    struct csmsolid_t *res;
    
    //toroide = csmshape3d_create_torus(3., .75, 3, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    sphere = csmshape3d_create_sphere(
                        2.8,
                        8,
                        16,
                        0., 0., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);

    torus = csmshape3d_create_torus(
                        3.,  16,
                        .75, 16,
                        0., 0., 0.,
                        0., 1., 0.,
                        0., 0., 1.,
                        1);

    torus2 = csmshape3d_create_torus(
                        3.,  16,
                        .75, 16,
                        0., 0., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);
    
    cone = csmshape3d_create_cone(
                        5., 2., 32,
                        0., 0., 0.,
                        -1., 0., 0.,
                        0., 0., 1.,
                        1);
    
    csmdebug_set_viewer_results(sphere, torus);
    //csmdebug_show_viewer();
    
    {
        struct gccontorno_t *block_shape;
        
        block_shape = gcelem2d_contorno_rectangular(6., 6.);
        
        block = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        0., 0.,  0.5, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        0., 0., -0.5, 1., 0., 0., 0., 1., 0.,
                        10000);
    }
    
    res = csmsetop_difference_A_minus_B(sphere, torus);
    res = csmsetop_difference_A_minus_B(res, torus2);
    res = csmsetop_difference_A_minus_B(res, cone);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

static void i_test_sphere3(void)
{
    struct csmsolid_t *sphere, *torus, *torus2;
    struct csmsolid_t *block;
    struct csmsolid_t *res;
    
    //toroide = csmshape3d_create_torus(3., .75, 3, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    sphere = csmshape3d_create_sphere(
                        2.8,
                        8,
                        16,
                        0., 0., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);

    torus = csmshape3d_create_torus(
                        3.,  16,
                        .75, 16,
                        0., 0., 0.,
                        0., 1., 0.,
                        0., 0., 1.,
                        1);

    torus2 = csmshape3d_create_torus(
                        3.,  16,
                        .75, 16,
                        0., 0., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);
    
    csmdebug_set_viewer_results(sphere, torus);
    //csmdebug_show_viewer();
    
    {
        struct gccontorno_t *block_shape;
        
        block_shape = gcelem2d_contorno_rectangular(6., 6.);
        
        block = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        0., 0.,  0.5, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        0., 0., -0.5, 1., 0., 0., 0., 1., 0.,
                        10000);
    }
    
    res = csmsetop_union_A_and_B(sphere, torus);
    res = csmsetop_difference_A_minus_B(res, torus2);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

static void i_assign_flat_material_to_solid(float r, float g, float b, struct csmsolid_t *solid)
{
    struct csmmaterial_t *material;
    
    material = csmmaterial_new_flat_material(r, g, b, 100);
    csmsolid_assign_visualization_material(solid, &material);
}

// ------------------------------------------------------------------------------------------

static void i_test_sphere4(void)
{
    struct csmsolid_t *sphere1, *sphere2, *sphere3, *sphere4;
    struct csmsolid_t *res;
    struct csmsolid_t *block;
    struct csmsolid_t *torus;
    struct csmsolid_t *cylinder;
    
    //toroide = csmshape3d_create_torus(3., .75, 3, 0., 0., 0., 1., 0., 0., 0., 1., 0., 0);
    sphere1 = csmshape3d_create_sphere(
                        2.8,
                        8,
                        16,
                        0., 0., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);

    sphere2 = csmshape3d_create_sphere(
                        2.8,
                        8,
                        16,
                        2.8, 0., 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);

    sphere3 = csmshape3d_create_sphere(
                        2.8,
                        8,
                        16,
                        2.8, 2.8, 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);

    sphere4 = csmshape3d_create_sphere(
                        2.8,
                        8,
                        16,
                        0.0, 2.8, 0.,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);
    
    i_assign_flat_material_to_solid(1., 0., 0., sphere1);
    i_assign_flat_material_to_solid(1., 0., 0., sphere2);
    i_assign_flat_material_to_solid(1., 0., 0., sphere3);
    i_assign_flat_material_to_solid(1., 0., 0., sphere4);
    
    res = csmsetop_union_A_and_B(sphere1, sphere2);
    res = csmsetop_union_A_and_B(res, sphere3);
    res = csmsetop_union_A_and_B(res, sphere4);

    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
    
    {
        struct gccontorno_t *block_shape;
        
        block_shape = gcelem2d_contorno_rectangular(2.9, 2.9);
        
        block = csmsweep_create_solid_from_shape_debug(
                        block_shape,
                        1.4, 1.4,  0.5, 1., 0., 0., 0., 1., 0.,
                        block_shape,
                        1.4, 1.4, -2.8, 1., 0., 0., 0., 1., 0.,
                        10000);
        
        i_assign_flat_material_to_solid(0., 1., 0., block);
    }
    
    res = csmsetop_difference_A_minus_B(block, res);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
    
    i_assign_flat_material_to_solid(1., 0., 0., res);
    
    res = csmsetop_difference_A_minus_B(block, res);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
    
    torus = csmshape3d_create_torus(
                        1.5,  16,
                        1., 16,
                        1.4, 1.4, -.6,
                        1., 0., 0.,
                        0., 1., 0.,
                        1);
    
    i_assign_flat_material_to_solid(1., 1., 1., torus);
    
    res = csmsetop_difference_A_minus_B(res, torus);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
    
    {
        struct gccontorno_t *cylinder_shape;
        
        cylinder_shape = gcelem2d_contorno_circular(0.3, 32);
        
        cylinder = csmsweep_create_solid_from_shape_debug(
                        cylinder_shape,
                        1.5, 1.4, 0.0, 0., 1., 0., 0., 0., 1.,
                        cylinder_shape,
                        -1.5, 1.4, 0.0, 0., 1., 0., 0., 0., 1.,
                        10000);
        
        i_assign_flat_material_to_solid(1., 0., 1., cylinder);
    }
    
    res = csmsetop_difference_A_minus_B(res, cylinder);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
    
    {
        struct gccontorno_t *cylinder_shape;
        
        cylinder_shape = gcelem2d_contorno_circular(0.6, 32);
        
        cylinder = csmsweep_create_solid_from_shape_debug(
                        cylinder_shape,
                        2.5, 1.4, -1.8, 0., 1., 0., 0., 0., 1.,
                        cylinder_shape,
                        -2.5, 1.4, -1.8, 0., 1., 0., 0., 0., 1.,
                        10000);
        
        i_assign_flat_material_to_solid(1., 0., 1., cylinder);
    }
    
    res = csmsetop_difference_A_minus_B(res, cylinder);
    csmsolid_set_draw_only_border_edges(res, CSMFALSE);
    csmdebug_set_viewer_results(res, NULL);
    csmdebug_show_viewer();
}

// ------------------------------------------------------------------------------------------

void csmtest_test(void)
{
    struct csmviewer_t *viewer;

    viewer = csmviewer_new();
    csmdebug_set_viewer(viewer, csmviewer_show, csmviewer_set_parameters, csmviewer_set_results);
    
    //csmtest_array_test1();
    //csmtest_array_test2();
    
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
    
    i_test_divide_solido_rectangular_hueco_por_plano_medio();
    i_test_divide_solido_rectangular_hueco_por_plano_medio2();
    i_test_divide_solido_rectangular_hueco_por_plano_superior();
    i_test_divide_solido_rectangular_hueco_por_plano_superior2();

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
    i_test_cilindro2(viewer);
    i_test_cilindro3(viewer);
    i_test_cilindro4(viewer); // --> Revisar la orientación de las caras del hueco, falla split a 0,75. Assert de puntos repetidos al realizar la diferencia, arista nula no borrada?
    i_test_cilindro5(viewer); // -- Intersecciones non-manifold.
    i_test_cilindro6(viewer); // --> Intersecciones non-manifold.
    i_test_cilindro7(viewer); // --> Intersecciones non-manifold.
    i_test_cilindro8(viewer); // --> Intersecciones non-manifold.
    
    //i_test_cilindro9(viewer); // --> Intersecciones non-manifold.
                              // --> Detectar situación de error y gestionarla correctamente, la unión no tiene sentido porque no se puede realizar a través de una cara
                              // --> No manipular las intersecciones non-manifold, parece que el caso out-on-out se gestiona correctamente.

    
    i_test_mechanical_part1_redux();
    i_test_mechanical_part1();
    i_test_mechanical_part2();
    
    i_test_toroide();
    i_test_cono();
    i_test_sphere();

    i_test_sphere2();
    i_test_sphere3();
    
    //i_test_mechanical_part2();

    i_test_sphere4();
    
    csmviewer_free(&viewer);
}













