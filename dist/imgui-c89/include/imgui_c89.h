/* Generated Dear ImGui C89 header. Do not edit. */
#ifndef IMGUI_C89_H
#define IMGUI_C89_H
#ifdef IMGUI_C89_USE_CPP_TYPES
/* Declaration-only mode for the generated C++ facade. The
 * upstream imgui.h/imgui_internal.h types must already exist. */
#ifdef __cplusplus
extern "C" {
#endif
const ImGuiPayload * imgui_accept_drag_drop_payload(ImGuiContext *imgui_c89_ctx, const char * type, ImGuiDragDropFlags flags);
void imgui_align_text_to_frame_padding(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_arrow_button(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiDir dir);
unsigned char imgui_begin(ImGuiContext *imgui_c89_ctx, const char * name, unsigned char * p_open, ImGuiWindowFlags flags);
unsigned char imgui_begin_child_id(ImGuiContext *imgui_c89_ctx, ImGuiID id, const ImVec2 * size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags);
unsigned char imgui_begin_child_string(ImGuiContext *imgui_c89_ctx, const char * str_id, const ImVec2 * size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags);
unsigned char imgui_begin_combo(ImGuiContext *imgui_c89_ctx, const char * label, const char * preview_value, ImGuiComboFlags flags);
void imgui_begin_disabled(ImGuiContext *imgui_c89_ctx, unsigned char disabled);
unsigned char imgui_begin_drag_drop_source(ImGuiContext *imgui_c89_ctx, ImGuiDragDropFlags flags);
unsigned char imgui_begin_drag_drop_target(ImGuiContext *imgui_c89_ctx);
void imgui_begin_group(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_begin_item_tooltip(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_begin_list_box(ImGuiContext *imgui_c89_ctx, const char * label, const ImVec2 * size_arg);
unsigned char imgui_begin_main_menu_bar(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_begin_menu(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char enabled);
unsigned char imgui_begin_menu_bar(ImGuiContext *imgui_c89_ctx);
ImGuiMultiSelectIO * imgui_begin_multi_select(ImGuiContext *imgui_c89_ctx, ImGuiMultiSelectFlags flags, int selection_size, int items_count);
unsigned char imgui_begin_popup(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiWindowFlags flags);
unsigned char imgui_begin_popup_context_item(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiPopupFlags popup_flags);
unsigned char imgui_begin_popup_context_void(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiPopupFlags popup_flags);
unsigned char imgui_begin_popup_context_window(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiPopupFlags popup_flags);
unsigned char imgui_begin_popup_modal(ImGuiContext *imgui_c89_ctx, const char * name, unsigned char * p_open, ImGuiWindowFlags flags);
unsigned char imgui_begin_tab_bar(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiTabBarFlags flags);
unsigned char imgui_begin_tab_item(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char * p_open, ImGuiTabItemFlags flags);
unsigned char imgui_begin_table(ImGuiContext *imgui_c89_ctx, const char * str_id, int columns_count, ImGuiTableFlags flags, const ImVec2 * outer_size, float inner_width);
unsigned char imgui_begin_tooltip(ImGuiContext *imgui_c89_ctx);
void imgui_bullet(ImGuiContext *imgui_c89_ctx);
void imgui_bullet_text_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
unsigned char imgui_button(ImGuiContext *imgui_c89_ctx, const char * label, const ImVec2 * size_arg);
float imgui_calc_item_width(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_calc_text_size(ImGuiContext *imgui_c89_ctx, const char * text, const char * text_end, unsigned char hide_text_after_double_hash, float wrap_width);
unsigned char imgui_checkbox(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char * v);
unsigned char imgui_checkbox_flags_int_pointer_int(ImGuiContext *imgui_c89_ctx, const char * label, int * flags, int flags_value);
unsigned char imgui_checkbox_flags_uint_pointer_uint(ImGuiContext *imgui_c89_ctx, const char * label, unsigned int * flags, unsigned int flags_value);
void imgui_close_current_popup(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_collapsing_header_bool_pointer_tree_node_flags(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char * p_visible, ImGuiTreeNodeFlags flags);
unsigned char imgui_collapsing_header_tree_node_flags_none(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiTreeNodeFlags flags);
unsigned char imgui_color_button(ImGuiContext *imgui_c89_ctx, const char * desc_id, const ImVec4 * col, ImGuiColorEditFlags flags, const ImVec2 * size_arg);
ImU32 imgui_color_convert_float4_to_u32(const ImVec4 * in);
void imgui_color_convert_hs_vto_rgb(float h, float s, float v, float * out_r, float * out_g, float * out_b);
void imgui_color_convert_rg_bto_hsv(float r, float g, float b, float * out_h, float * out_s, float * out_v);
ImVec4 imgui_color_convert_u32_to_float4(ImU32 in);
unsigned char imgui_color_edit3(ImGuiContext *imgui_c89_ctx, const char * label, float * col, ImGuiColorEditFlags flags);
unsigned char imgui_color_edit4(ImGuiContext *imgui_c89_ctx, const char * label, float * col, ImGuiColorEditFlags flags);
ImColor imgui_color_hsv(float h, float s, float v, float a);
unsigned char imgui_color_picker3(ImGuiContext *imgui_c89_ctx, const char * label, float * col, ImGuiColorEditFlags flags);
unsigned char imgui_color_picker4(ImGuiContext *imgui_c89_ctx, const char * label, float * col, ImGuiColorEditFlags flags, const float * ref_col);
void imgui_color_set_hsv(ImColor *self, float h, float s, float v, float a);
void imgui_columns(ImGuiContext *imgui_c89_ctx, int columns_count, const char * id, unsigned char borders);
unsigned char imgui_combo_string_int_none_none(ImGuiContext *imgui_c89_ctx, const char * label, int * current_item, const char * items_separated_by_zeros, int height_in_items);
unsigned char imgui_combo_string_pointer_pointer_int_pointer_int_int(ImGuiContext *imgui_c89_ctx, const char * label, int * current_item, const char *(*getter)(void *, int), void * user_data, int items_count, int popup_max_height_in_items);
unsigned char imgui_combo_stringconst_pointer_int_int_none(ImGuiContext *imgui_c89_ctx, const char * label, int * current_item, const char *const * items, int items_count, int height_in_items);
ImGuiContext * imgui_create_context(ImFontAtlas * shared_font_atlas);
unsigned char imgui_debug_check_version_and_data_layout(const char * version, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx);
void imgui_debug_flash_style_color(ImGuiContext *imgui_c89_ctx, ImGuiCol idx);
void imgui_debug_log_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_debug_start_item_picker(ImGuiContext *imgui_c89_ctx);
void imgui_debug_text_encoding(ImGuiContext *imgui_c89_ctx, const char * str);
void imgui_destroy_context(ImGuiContext * ctx);
unsigned char imgui_drag_float(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_speed, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_float2(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_speed, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_float3(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_speed, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_float4(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_speed, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_float_range2(ImGuiContext *imgui_c89_ctx, const char * label, float * v_current_min, float * v_current_max, float v_speed, float v_min, float v_max, const char * format, const char * format_max, ImGuiSliderFlags flags);
unsigned char imgui_drag_int(ImGuiContext *imgui_c89_ctx, const char * label, int * v, float v_speed, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_int2(ImGuiContext *imgui_c89_ctx, const char * label, int * v, float v_speed, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_int3(ImGuiContext *imgui_c89_ctx, const char * label, int * v, float v_speed, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_int4(ImGuiContext *imgui_c89_ctx, const char * label, int * v, float v_speed, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_int_range2(ImGuiContext *imgui_c89_ctx, const char * label, int * v_current_min, int * v_current_max, float v_speed, int v_min, int v_max, const char * format, const char * format_max, ImGuiSliderFlags flags);
unsigned char imgui_drag_scalar(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiDataType data_type, void * p_data, float v_speed, const void * p_min, const void * p_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_scalar_n(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiDataType data_type, void * p_data, int components, float v_speed, const void * p_min, const void * p_max, const char * format, ImGuiSliderFlags flags);
ImTextureID imgui_draw_cmd_get_tex_id(ImDrawCmd *self);
void imgui_draw_data_add_draw_list(ImGuiContext *imgui_c89_ctx, ImDrawData *self, ImDrawList * draw_list);
void imgui_draw_data_clear(ImGuiContext *imgui_c89_ctx, ImDrawData *self);
void imgui_draw_data_de_index_all_buffers(ImGuiContext *imgui_c89_ctx, ImDrawData *self);
void imgui_draw_data_scale_clip_rects(ImDrawData *self, const ImVec2 * fb_scale);
void imgui_draw_list_add_bezier_cubic(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, ImU32 col, float thickness, int num_segments);
void imgui_draw_list_add_bezier_quadratic(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, ImU32 col, float thickness, int num_segments);
void imgui_draw_list_add_callback(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImDrawCallback callback, void * userdata, size_t userdata_size);
void imgui_draw_list_add_circle(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, ImU32 col, int num_segments, float thickness);
void imgui_draw_list_add_circle_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, ImU32 col, int num_segments);
void imgui_draw_list_add_concave_poly_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * points, const int points_count, ImU32 col);
void imgui_draw_list_add_convex_poly_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * points, const int points_count, ImU32 col);
void imgui_draw_list_add_draw_cmd(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_add_ellipse(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, const ImVec2 * radius, ImU32 col, float rot, int num_segments, float thickness);
void imgui_draw_list_add_ellipse_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, const ImVec2 * radius, ImU32 col, float rot, int num_segments);
void imgui_draw_list_add_image(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImTextureRef tex_ref, const ImVec2 * p_min, const ImVec2 * p_max, const ImVec2 * uv_min, const ImVec2 * uv_max, ImU32 col);
void imgui_draw_list_add_image_quad(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImTextureRef tex_ref, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, const ImVec2 * uv1, const ImVec2 * uv2, const ImVec2 * uv3, const ImVec2 * uv4, ImU32 col);
void imgui_draw_list_add_image_rounded(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImTextureRef tex_ref, const ImVec2 * p_min, const ImVec2 * p_max, const ImVec2 * uv_min, const ImVec2 * uv_max, ImU32 col, float rounding, ImDrawFlags flags);
void imgui_draw_list_add_line(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, ImU32 col, float thickness);
void imgui_draw_list_add_line_h(ImGuiContext *imgui_c89_ctx, ImDrawList *self, float min_x, float max_x, float y, ImU32 col, float thickness);
void imgui_draw_list_add_line_v(ImGuiContext *imgui_c89_ctx, ImDrawList *self, float x, float min_y, float max_y, ImU32 col, float thickness);
void imgui_draw_list_add_ngon(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, ImU32 col, int num_segments, float thickness);
void imgui_draw_list_add_ngon_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, ImU32 col, int num_segments);
void imgui_draw_list_add_polyline_int_draw_flags_float(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * points, int num_points, ImU32 col, ImDrawFlags flags, float thickness);
void imgui_draw_list_add_polyline_int_float_draw_flags(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * points, const int points_count, ImU32 col, float thickness, ImDrawFlags flags);
void imgui_draw_list_add_quad(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, ImU32 col, float thickness);
void imgui_draw_list_add_quad_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, ImU32 col);
void imgui_draw_list_add_rect_draw_flags_float(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p_min, const ImVec2 * p_max, ImU32 col, float rounding, ImDrawFlags flags, float thickness);
void imgui_draw_list_add_rect_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p_min, const ImVec2 * p_max, ImU32 col, float rounding, ImDrawFlags flags);
void imgui_draw_list_add_rect_filled_multi_color(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p_min, const ImVec2 * p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left);
void imgui_draw_list_add_rect_float_draw_flags(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p_min, const ImVec2 * p_max, ImU32 col, float rounding, float thickness, ImDrawFlags flags);
void imgui_draw_list_add_text_font_pointer_float_vec2_u32_string_string_float_vec4_pointer(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImFont * font, float font_size, const ImVec2 * pos, ImU32 col, const char * text_begin, const char * text_end, float wrap_width, const ImVec4 * cpu_fine_clip_rect);
void imgui_draw_list_add_text_vec2_u32_string_string_none_none_none_none(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * pos, ImU32 col, const char * text_begin, const char * text_end);
void imgui_draw_list_add_triangle(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, ImU32 col, float thickness);
void imgui_draw_list_add_triangle_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, ImU32 col);
int imgui_draw_list_calc_circle_auto_segment_count(ImDrawList *self, float radius);
void imgui_draw_list_channels_merge(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_channels_set_current(ImGuiContext *imgui_c89_ctx, ImDrawList *self, int n);
void imgui_draw_list_channels_split(ImGuiContext *imgui_c89_ctx, ImDrawList *self, int count);
void imgui_draw_list_clear_free_memory(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
ImDrawList * imgui_draw_list_clone_output(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
ImVec2 imgui_draw_list_get_clip_rect_max(ImDrawList *self);
ImVec2 imgui_draw_list_get_clip_rect_min(ImDrawList *self);
void imgui_draw_list_on_changed_clip_rect(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_on_changed_texture(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_on_changed_vtx_offset(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_path_arc_to(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, float a_min, float a_max, int num_segments);
void imgui_draw_list_path_arc_to_fast(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, int a_min_of_12, int a_max_of_12);
void imgui_draw_list_path_arc_to_fast_ex(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, int a_min_sample, int a_max_sample, int a_step);
void imgui_draw_list_path_arc_to_n(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, float a_min, float a_max, int num_segments);
void imgui_draw_list_path_bezier_cubic_curve_to(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, int num_segments);
void imgui_draw_list_path_bezier_quadratic_curve_to(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p2, const ImVec2 * p3, int num_segments);
void imgui_draw_list_path_clear(ImDrawList *self);
void imgui_draw_list_path_elliptical_arc_to(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, const ImVec2 * radius, float rot, float a_min, float a_max, int num_segments);
void imgui_draw_list_path_fill_concave(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImU32 col);
void imgui_draw_list_path_fill_convex(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImU32 col);
void imgui_draw_list_path_line_to(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * pos);
void imgui_draw_list_path_line_to_merge_duplicate(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * pos);
void imgui_draw_list_path_rect(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * a, const ImVec2 * b, float rounding, ImDrawFlags flags);
void imgui_draw_list_path_stroke_draw_flags_float(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImU32 col, ImDrawFlags flags, float thickness);
void imgui_draw_list_path_stroke_float_draw_flags(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImU32 col, float thickness, ImDrawFlags flags);
void imgui_draw_list_pop_clip_rect(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_pop_texture(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_pop_texture_id(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_pop_unused_draw_cmd(ImDrawList *self);
void imgui_draw_list_prim_quad_uv(ImDrawList *self, const ImVec2 * a, const ImVec2 * b, const ImVec2 * c, const ImVec2 * d, const ImVec2 * uv_a, const ImVec2 * uv_b, const ImVec2 * uv_c, const ImVec2 * uv_d, ImU32 col);
void imgui_draw_list_prim_rect(ImDrawList *self, const ImVec2 * a, const ImVec2 * c, ImU32 col);
void imgui_draw_list_prim_rect_uv(ImDrawList *self, const ImVec2 * a, const ImVec2 * c, const ImVec2 * uv_a, const ImVec2 * uv_c, ImU32 col);
void imgui_draw_list_prim_reserve(ImGuiContext *imgui_c89_ctx, ImDrawList *self, int idx_count, int vtx_count);
void imgui_draw_list_prim_unreserve(ImDrawList *self, int idx_count, int vtx_count);
void imgui_draw_list_prim_vtx(ImDrawList *self, const ImVec2 * pos, const ImVec2 * uv, ImU32 col);
void imgui_draw_list_prim_write_idx(ImDrawList *self, ImDrawIdx idx);
void imgui_draw_list_prim_write_vtx(ImDrawList *self, const ImVec2 * pos, const ImVec2 * uv, ImU32 col);
void imgui_draw_list_push_clip_rect(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * cr_min, const ImVec2 * cr_max, unsigned char intersect_with_current_clip_rect);
void imgui_draw_list_push_clip_rect_full_screen(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_push_texture(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImTextureRef tex_ref);
void imgui_draw_list_push_texture_id(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImTextureRef tex_ref);
void imgui_draw_list_reset_for_new_frame(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_set_draw_list_shared_data(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImDrawListSharedData * data);
void imgui_draw_list_set_texture(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImTextureRef tex_ref);
void imgui_draw_list_splitter_clear(ImDrawListSplitter *self);
void imgui_draw_list_splitter_clear_free_memory(ImGuiContext *imgui_c89_ctx, ImDrawListSplitter *self);
void imgui_draw_list_splitter_merge(ImGuiContext *imgui_c89_ctx, ImDrawListSplitter *self, ImDrawList * draw_list);
void imgui_draw_list_splitter_set_current_channel(ImGuiContext *imgui_c89_ctx, ImDrawListSplitter *self, ImDrawList * draw_list, int idx);
void imgui_draw_list_splitter_split(ImGuiContext *imgui_c89_ctx, ImDrawListSplitter *self, ImDrawList * draw_list, int channels_count);
void imgui_draw_list_try_merge_draw_cmds(ImDrawList *self);
void imgui_dummy(ImGuiContext *imgui_c89_ctx, const ImVec2 * size);
void imgui_end(ImGuiContext *imgui_c89_ctx);
void imgui_end_child(ImGuiContext *imgui_c89_ctx);
void imgui_end_combo(ImGuiContext *imgui_c89_ctx);
void imgui_end_disabled(ImGuiContext *imgui_c89_ctx);
void imgui_end_drag_drop_source(ImGuiContext *imgui_c89_ctx);
void imgui_end_drag_drop_target(ImGuiContext *imgui_c89_ctx);
void imgui_end_frame(ImGuiContext *imgui_c89_ctx);
void imgui_end_group(ImGuiContext *imgui_c89_ctx);
void imgui_end_list_box(ImGuiContext *imgui_c89_ctx);
void imgui_end_main_menu_bar(ImGuiContext *imgui_c89_ctx);
void imgui_end_menu(ImGuiContext *imgui_c89_ctx);
void imgui_end_menu_bar(ImGuiContext *imgui_c89_ctx);
ImGuiMultiSelectIO * imgui_end_multi_select(ImGuiContext *imgui_c89_ctx);
void imgui_end_popup(ImGuiContext *imgui_c89_ctx);
void imgui_end_tab_bar(ImGuiContext *imgui_c89_ctx);
void imgui_end_tab_item(ImGuiContext *imgui_c89_ctx);
void imgui_end_table(ImGuiContext *imgui_c89_ctx);
void imgui_end_tooltip(ImGuiContext *imgui_c89_ctx);
void imgui_font_add_remap_char(ImGuiContext *imgui_c89_ctx, ImFont *self, ImWchar from_codepoint, ImWchar to_codepoint);
ImFontAtlasRectId imgui_font_atlas_add_custom_rect(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, int width, int height, ImFontAtlasRect * out_r);
ImFontAtlasRectId imgui_font_atlas_add_custom_rect_font_glyph(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, ImFont * font, ImWchar codepoint, int width, int height, float advance_x, const ImVec2 * offset);
ImFontAtlasRectId imgui_font_atlas_add_custom_rect_font_glyph_for_size(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, ImFont * font, float font_size, ImWchar codepoint, int width, int height, float advance_x, const ImVec2 * offset);
ImFontAtlasRectId imgui_font_atlas_add_custom_rect_regular(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, int w, int h);
ImFont * imgui_font_atlas_add_font(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const ImFontConfig * font_cfg_in);
ImFont * imgui_font_atlas_add_font_default(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const ImFontConfig * font_cfg);
ImFont * imgui_font_atlas_add_font_default_bitmap(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const ImFontConfig * font_cfg_template);
ImFont * imgui_font_atlas_add_font_default_vector(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const ImFontConfig * font_cfg_template);
ImFont * imgui_font_atlas_add_font_from_file_ttf(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const char * filename, float size_pixels, const ImFontConfig * font_cfg_template, const ImWchar * glyph_ranges);
ImFont * imgui_font_atlas_add_font_from_memory_compressed_base85_ttf(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const char * compressed_ttf_data_base85, float size_pixels, const ImFontConfig * font_cfg, const ImWchar * glyph_ranges);
ImFont * imgui_font_atlas_add_font_from_memory_compressed_ttf(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const void * compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig * font_cfg_template, const ImWchar * glyph_ranges);
ImFont * imgui_font_atlas_add_font_from_memory_ttf(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, void * font_data, int font_data_size, float size_pixels, const ImFontConfig * font_cfg_template, const ImWchar * glyph_ranges);
unsigned char imgui_font_atlas_build(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
void imgui_font_atlas_calc_custom_rect_uv(ImFontAtlas *self, const ImFontAtlasRect * r, ImVec2 * out_uv_min, ImVec2 * out_uv_max);
void imgui_font_atlas_clear(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
void imgui_font_atlas_clear_fonts(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
void imgui_font_atlas_clear_input_data(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
void imgui_font_atlas_clear_tex_data(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
void imgui_font_atlas_compact_cache(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
unsigned char imgui_font_atlas_get_custom_rect(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, ImFontAtlasRectId id, ImFontAtlasRect * out_r);
const ImFontAtlasRect * imgui_font_atlas_get_custom_rect_by_index(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, ImFontAtlasRectId id);
const ImWchar * imgui_font_atlas_get_glyph_ranges_chinese_full(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_chinese_simplified_common(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_cyrillic(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_default(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_greek(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_japanese(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_korean(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_thai(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_vietnamese(ImFontAtlas *self);
void imgui_font_atlas_get_tex_data_as_alpha8(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, unsigned char ** out_pixels, int * out_width, int * out_height, int * out_bytes_per_pixel);
void imgui_font_atlas_get_tex_data_as_rgba32(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, unsigned char ** out_pixels, int * out_width, int * out_height, int * out_bytes_per_pixel);
unsigned char imgui_font_atlas_is_built(ImFontAtlas *self);
void imgui_font_atlas_remove_custom_rect(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, ImFontAtlasRectId id);
void imgui_font_atlas_remove_font(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, ImFont * font);
void imgui_font_atlas_set_font_loader(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const ImFontLoader * font_loader);
void imgui_font_atlas_set_tex_id_texture_id(ImFontAtlas *self, ImTextureID id);
void imgui_font_atlas_set_tex_id_texture_ref(ImFontAtlas *self, ImTextureRef id);
void imgui_font_baked_clear_output_data(ImGuiContext *imgui_c89_ctx, ImFontBaked *self);
ImFontGlyph * imgui_font_baked_find_glyph(ImGuiContext *imgui_c89_ctx, ImFontBaked *self, ImWchar c);
ImFontGlyph * imgui_font_baked_find_glyph_no_fallback(ImGuiContext *imgui_c89_ctx, ImFontBaked *self, ImWchar c);
float imgui_font_baked_get_char_advance(ImGuiContext *imgui_c89_ctx, ImFontBaked *self, ImWchar c);
unsigned char imgui_font_baked_is_glyph_loaded(ImFontBaked *self, ImWchar c);
ImVec2 imgui_font_calc_text_size_a(ImGuiContext *imgui_c89_ctx, ImFont *self, float size, float max_width, float wrap_width, const char * text_begin, const char * text_end, const char ** out_remaining);
const char * imgui_font_calc_word_wrap_position(ImGuiContext *imgui_c89_ctx, ImFont *self, float size, const char * text, const char * text_end, float wrap_width);
const char * imgui_font_calc_word_wrap_position_a(ImGuiContext *imgui_c89_ctx, ImFont *self, float scale, const char * text, const char * text_end, float wrap_width);
void imgui_font_clear_output_data(ImGuiContext *imgui_c89_ctx, ImFont *self);
const char * imgui_font_get_debug_name(ImFont *self);
ImFontBaked * imgui_font_get_font_baked(ImGuiContext *imgui_c89_ctx, ImFont *self, float size, float density);
void imgui_font_glyph_ranges_builder_add_char(ImFontGlyphRangesBuilder *self, ImWchar c);
void imgui_font_glyph_ranges_builder_add_ranges(ImFontGlyphRangesBuilder *self, const ImWchar * ranges);
void imgui_font_glyph_ranges_builder_add_text(ImFontGlyphRangesBuilder *self, const char * text, const char * text_end);
void imgui_font_glyph_ranges_builder_build_ranges(ImGuiContext *imgui_c89_ctx, ImFontGlyphRangesBuilder *self, ImVector<unsigned short> * out_ranges);
void imgui_font_glyph_ranges_builder_clear(ImGuiContext *imgui_c89_ctx, ImFontGlyphRangesBuilder *self);
unsigned char imgui_font_glyph_ranges_builder_get_bit(ImFontGlyphRangesBuilder *self, size_t n);
void imgui_font_glyph_ranges_builder_set_bit(ImFontGlyphRangesBuilder *self, size_t n);
unsigned char imgui_font_is_glyph_in_font(ImFont *self, ImWchar c);
unsigned char imgui_font_is_glyph_range_unused(ImFont *self, unsigned int c_begin, unsigned int c_last);
unsigned char imgui_font_is_loaded(ImFont *self);
void imgui_font_render_char(ImGuiContext *imgui_c89_ctx, ImFont *self, ImDrawList * draw_list, float size, const ImVec2 * pos, ImU32 col, ImWchar c, const ImVec4 * cpu_fine_clip);
void imgui_font_render_text(ImGuiContext *imgui_c89_ctx, ImFont *self, ImDrawList * draw_list, float size, const ImVec2 * pos, ImU32 col, const ImVec4 * clip_rect, const char * text_begin, const char * text_end, float wrap_width, ImDrawTextFlags flags);
void imgui_get_allocator_functions(ImGuiMemAllocFunc * p_alloc_func, ImGuiMemFreeFunc * p_free_func, void ** p_user_data);
ImDrawList * imgui_get_background_draw_list(ImGuiContext *imgui_c89_ctx);
const char * imgui_get_clipboard_text(ImGuiContext *imgui_c89_ctx);
ImU32 imgui_get_color_u32_col_float(ImGuiContext *imgui_c89_ctx, ImGuiCol idx, float alpha_mul);
ImU32 imgui_get_color_u32_u32_float(ImGuiContext *imgui_c89_ctx, ImU32 col, float alpha_mul);
ImU32 imgui_get_color_u32_vec4_none(ImGuiContext *imgui_c89_ctx, const ImVec4 * col);
int imgui_get_column_index(ImGuiContext *imgui_c89_ctx);
float imgui_get_column_offset(ImGuiContext *imgui_c89_ctx, int column_index);
float imgui_get_column_width(ImGuiContext *imgui_c89_ctx, int column_index);
int imgui_get_columns_count(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_content_region_avail(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_content_region_max(ImGuiContext *imgui_c89_ctx);
ImGuiContext * imgui_get_current_context(void);
ImVec2 imgui_get_cursor_pos(ImGuiContext *imgui_c89_ctx);
float imgui_get_cursor_pos_x(ImGuiContext *imgui_c89_ctx);
float imgui_get_cursor_pos_y(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_cursor_screen_pos(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_cursor_start_pos(ImGuiContext *imgui_c89_ctx);
const ImGuiPayload * imgui_get_drag_drop_payload(ImGuiContext *imgui_c89_ctx);
ImDrawData * imgui_get_draw_data(ImGuiContext *imgui_c89_ctx);
ImDrawListSharedData * imgui_get_draw_list_shared_data(ImGuiContext *imgui_c89_ctx);
ImFont * imgui_get_font(ImGuiContext *imgui_c89_ctx);
ImFontBaked * imgui_get_font_baked(ImGuiContext *imgui_c89_ctx);
float imgui_get_font_size(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_font_tex_uv_white_pixel(ImGuiContext *imgui_c89_ctx);
ImDrawList * imgui_get_foreground_draw_list(ImGuiContext *imgui_c89_ctx);
int imgui_get_frame_count(ImGuiContext *imgui_c89_ctx);
float imgui_get_frame_height(ImGuiContext *imgui_c89_ctx);
float imgui_get_frame_height_with_spacing(ImGuiContext *imgui_c89_ctx);
ImGuiID imgui_get_id_const_pointer_none(ImGuiContext *imgui_c89_ctx, const void * ptr_id);
ImGuiID imgui_get_id_int_none(ImGuiContext *imgui_c89_ctx, int int_id);
ImGuiID imgui_get_id_string_none(ImGuiContext *imgui_c89_ctx, const char * str_id);
ImGuiID imgui_get_id_string_string(ImGuiContext *imgui_c89_ctx, const char * str_id_begin, const char * str_id_end);
ImGuiIO * imgui_get_io(ImGuiContext *imgui_c89_ctx);
int imgui_get_item_clicked_count_with_single_click_delay(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton mouse_button, float delay);
ImGuiItemFlags imgui_get_item_flags(ImGuiContext *imgui_c89_ctx);
ImGuiID imgui_get_item_id(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_item_rect_max(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_item_rect_min(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_item_rect_size(ImGuiContext *imgui_c89_ctx);
const char * imgui_get_key_name(ImGuiKey key);
int imgui_get_key_pressed_amount(ImGuiContext *imgui_c89_ctx, ImGuiKey key, float repeat_delay, float repeat_rate);
ImGuiViewport * imgui_get_main_viewport(ImGuiContext *imgui_c89_ctx);
int imgui_get_mouse_clicked_count(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button);
ImGuiMouseCursor imgui_get_mouse_cursor(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_mouse_drag_delta(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, float lock_threshold);
ImVec2 imgui_get_mouse_pos(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_mouse_pos_on_opening_current_popup(ImGuiContext *imgui_c89_ctx);
ImGuiPlatformIO * imgui_get_platform_io(ImGuiContext *imgui_c89_ctx);
float imgui_get_scroll_max_x(ImGuiContext *imgui_c89_ctx);
float imgui_get_scroll_max_y(ImGuiContext *imgui_c89_ctx);
float imgui_get_scroll_x(ImGuiContext *imgui_c89_ctx);
float imgui_get_scroll_y(ImGuiContext *imgui_c89_ctx);
ImGuiStorage * imgui_get_state_storage(ImGuiContext *imgui_c89_ctx);
ImGuiStyle * imgui_get_style(ImGuiContext *imgui_c89_ctx);
const char * imgui_get_style_color_name(ImGuiCol idx);
const ImVec4 * imgui_get_style_color_vec4(ImGuiContext *imgui_c89_ctx, ImGuiCol idx);
float imgui_get_text_line_height(ImGuiContext *imgui_c89_ctx);
float imgui_get_text_line_height_with_spacing(ImGuiContext *imgui_c89_ctx);
double imgui_get_time(ImGuiContext *imgui_c89_ctx);
float imgui_get_tree_node_to_label_spacing(ImGuiContext *imgui_c89_ctx);
const char * imgui_get_version(void);
ImVec2 imgui_get_window_content_region_max(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_window_content_region_min(ImGuiContext *imgui_c89_ctx);
ImDrawList * imgui_get_window_draw_list(ImGuiContext *imgui_c89_ctx);
float imgui_get_window_height(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_window_pos(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_window_size(ImGuiContext *imgui_c89_ctx);
float imgui_get_window_width(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_image_button(ImGuiContext *imgui_c89_ctx, const char * str_id, ImTextureRef tex_ref, const ImVec2 * image_size, const ImVec2 * uv0, const ImVec2 * uv1, const ImVec4 * bg_col, const ImVec4 * tint_col);
void imgui_image_none_none(ImGuiContext *imgui_c89_ctx, ImTextureRef tex_ref, const ImVec2 * image_size, const ImVec2 * uv0, const ImVec2 * uv1);
void imgui_image_vec4_vec4(ImGuiContext *imgui_c89_ctx, ImTextureRef tex_ref, const ImVec2 * image_size, const ImVec2 * uv0, const ImVec2 * uv1, const ImVec4 * tint_col, const ImVec4 * border_col);
void imgui_image_with_bg(ImGuiContext *imgui_c89_ctx, ImTextureRef tex_ref, const ImVec2 * image_size, const ImVec2 * uv0, const ImVec2 * uv1, const ImVec4 * bg_col, const ImVec4 * tint_col);
void imgui_indent(ImGuiContext *imgui_c89_ctx, float indent_w);
unsigned char imgui_input_double(ImGuiContext *imgui_c89_ctx, const char * label, double * v, double step, double step_fast, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_float(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float step, float step_fast, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_float2(ImGuiContext *imgui_c89_ctx, const char * label, float * v, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_float3(ImGuiContext *imgui_c89_ctx, const char * label, float * v, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_float4(ImGuiContext *imgui_c89_ctx, const char * label, float * v, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_int(ImGuiContext *imgui_c89_ctx, const char * label, int * v, int step, int step_fast, ImGuiInputTextFlags flags);
unsigned char imgui_input_int2(ImGuiContext *imgui_c89_ctx, const char * label, int * v, ImGuiInputTextFlags flags);
unsigned char imgui_input_int3(ImGuiContext *imgui_c89_ctx, const char * label, int * v, ImGuiInputTextFlags flags);
unsigned char imgui_input_int4(ImGuiContext *imgui_c89_ctx, const char * label, int * v, ImGuiInputTextFlags flags);
unsigned char imgui_input_scalar(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiDataType data_type, void * p_data, const void * p_step, const void * p_step_fast, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_scalar_n(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiDataType data_type, void * p_data, int components, const void * p_step, const void * p_step_fast, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_text(ImGuiContext *imgui_c89_ctx, const char * label, char * buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void * user_data);
void imgui_input_text_callback_data_clear_selection(ImGuiInputTextCallbackData *self);
void imgui_input_text_callback_data_delete_chars(ImGuiInputTextCallbackData *self, int pos, int bytes_count);
unsigned char imgui_input_text_callback_data_has_selection(ImGuiInputTextCallbackData *self);
void imgui_input_text_callback_data_insert_chars(ImGuiContext *imgui_c89_ctx, ImGuiInputTextCallbackData *self, int pos, const char * new_text, const char * new_text_end);
void imgui_input_text_callback_data_select_all(ImGuiInputTextCallbackData *self);
void imgui_input_text_callback_data_set_selection(ImGuiInputTextCallbackData *self, int s, int e);
unsigned char imgui_input_text_multiline(ImGuiContext *imgui_c89_ctx, const char * label, char * buf, size_t buf_size, const ImVec2 * size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void * user_data);
unsigned char imgui_input_text_with_hint(ImGuiContext *imgui_c89_ctx, const char * label, const char * hint, char * buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void * user_data);
unsigned char imgui_invisible_button(ImGuiContext *imgui_c89_ctx, const char * str_id, const ImVec2 * size_arg, ImGuiButtonFlags flags);
void imgui_io_add_focus_event(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, unsigned char focused);
void imgui_io_add_input_character(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, unsigned int c);
void imgui_io_add_input_character_utf16(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, ImWchar16 c);
void imgui_io_add_input_characters_utf8(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, const char * str);
void imgui_io_add_key_analog_event(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, ImGuiKey key, unsigned char down, float analog_value);
void imgui_io_add_key_event(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, ImGuiKey key, unsigned char down);
void imgui_io_add_mouse_button_event(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, int mouse_button, unsigned char down);
void imgui_io_add_mouse_pos_event(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, float x, float y);
void imgui_io_add_mouse_source_event(ImGuiIO *self, ImGuiMouseSource source);
void imgui_io_add_mouse_wheel_event(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, float wheel_x, float wheel_y);
void imgui_io_clear_events_queue(ImGuiContext *imgui_c89_ctx, ImGuiIO *self);
void imgui_io_clear_input_keys(ImGuiContext *imgui_c89_ctx, ImGuiIO *self);
void imgui_io_clear_input_mouse(ImGuiIO *self);
void imgui_io_set_app_accepting_events(ImGuiIO *self, unsigned char accepting_events);
void imgui_io_set_key_event_native_data(ImGuiIO *self, ImGuiKey key, int native_keycode, int native_scancode, int native_legacy_index);
unsigned char imgui_is_any_item_active(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_any_item_focused(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_any_item_hovered(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_any_mouse_down(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_activated(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_active(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_clicked(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton mouse_button);
unsigned char imgui_is_item_deactivated(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_deactivated_after_edit(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_edited(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_focused(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_hovered(ImGuiContext *imgui_c89_ctx, ImGuiHoveredFlags flags);
unsigned char imgui_is_item_toggled_open(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_toggled_selection(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_visible(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_key_chord_pressed(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord);
unsigned char imgui_is_key_down(ImGuiContext *imgui_c89_ctx, ImGuiKey key);
unsigned char imgui_is_key_pressed(ImGuiContext *imgui_c89_ctx, ImGuiKey key, unsigned char repeat);
unsigned char imgui_is_key_released(ImGuiContext *imgui_c89_ctx, ImGuiKey key);
unsigned char imgui_is_mouse_clicked(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, unsigned char repeat);
unsigned char imgui_is_mouse_double_clicked(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button);
unsigned char imgui_is_mouse_down(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button);
unsigned char imgui_is_mouse_dragging(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, float lock_threshold);
unsigned char imgui_is_mouse_hovering_rect(ImGuiContext *imgui_c89_ctx, const ImVec2 * r_min, const ImVec2 * r_max, unsigned char clip);
unsigned char imgui_is_mouse_pos_valid(ImGuiContext *imgui_c89_ctx, const ImVec2 * mouse_pos);
unsigned char imgui_is_mouse_released(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button);
unsigned char imgui_is_mouse_released_with_delay(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, float delay);
unsigned char imgui_is_popup_open(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiPopupFlags popup_flags);
unsigned char imgui_is_rect_visible_none(ImGuiContext *imgui_c89_ctx, const ImVec2 * size);
unsigned char imgui_is_rect_visible_vec2(ImGuiContext *imgui_c89_ctx, const ImVec2 * rect_min, const ImVec2 * rect_max);
unsigned char imgui_is_window_appearing(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_window_collapsed(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_window_focused(ImGuiContext *imgui_c89_ctx, ImGuiFocusedFlags flags);
unsigned char imgui_is_window_hovered(ImGuiContext *imgui_c89_ctx, ImGuiHoveredFlags flags);
void imgui_label_text_v(ImGuiContext *imgui_c89_ctx, const char * label, const char * fmt, va_list args);
unsigned char imgui_list_box_string_pointer_pointer_int_pointer_int(ImGuiContext *imgui_c89_ctx, const char * label, int * current_item, const char *(*getter)(void *, int), void * user_data, int items_count, int height_in_items);
unsigned char imgui_list_box_stringconst_pointer_int_none(ImGuiContext *imgui_c89_ctx, const char * label, int * current_item, const char *const * items, int items_count, int height_items);
void imgui_list_clipper_begin(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self, int items_count, float items_height);
void imgui_list_clipper_end(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self);
void imgui_list_clipper_include_item_by_index(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self, int item_index);
void imgui_list_clipper_include_items_by_index(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self, int item_begin, int item_end);
void imgui_list_clipper_seek_cursor_for_item(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self, int item_n);
unsigned char imgui_list_clipper_step(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self);
void imgui_load_ini_settings_from_disk(ImGuiContext *imgui_c89_ctx, const char * ini_filename);
void imgui_load_ini_settings_from_memory(ImGuiContext *imgui_c89_ctx, const char * ini_data, size_t ini_size);
void imgui_log_buttons(ImGuiContext *imgui_c89_ctx);
void imgui_log_finish(ImGuiContext *imgui_c89_ctx);
void imgui_log_text_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_log_to_clipboard(ImGuiContext *imgui_c89_ctx, int auto_open_depth);
void imgui_log_to_file(ImGuiContext *imgui_c89_ctx, int auto_open_depth, const char * filename);
void imgui_log_to_tty(ImGuiContext *imgui_c89_ctx, int auto_open_depth);
void * imgui_mem_alloc(ImGuiContext *imgui_c89_ctx, size_t size);
void imgui_mem_free(ImGuiContext *imgui_c89_ctx, void * ptr);
unsigned char imgui_menu_item_bool(ImGuiContext *imgui_c89_ctx, const char * label, const char * shortcut, unsigned char selected, unsigned char enabled);
unsigned char imgui_menu_item_bool_pointer(ImGuiContext *imgui_c89_ctx, const char * label, const char * shortcut, unsigned char * p_selected, unsigned char enabled);
void imgui_new_frame(ImGuiContext *imgui_c89_ctx);
void imgui_new_line(ImGuiContext *imgui_c89_ctx);
void imgui_next_column(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_open_popup_id(ImGuiContext *imgui_c89_ctx, ImGuiID id, ImGuiPopupFlags popup_flags);
unsigned char imgui_open_popup_on_item_click(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiPopupFlags popup_flags);
unsigned char imgui_open_popup_string(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiPopupFlags popup_flags);
void imgui_payload_clear(ImGuiPayload *self);
unsigned char imgui_payload_is_data_type(ImGuiPayload *self, const char * type);
unsigned char imgui_payload_is_delivery(ImGuiPayload *self);
unsigned char imgui_payload_is_preview(ImGuiPayload *self);
void imgui_platform_io_clear_platform_handlers(ImGuiPlatformIO *self);
void imgui_platform_io_clear_renderer_handlers(ImGuiPlatformIO *self);
void imgui_plot_histogram_float_pointer_int_string_float_vec2_int(ImGuiContext *imgui_c89_ctx, const char * label, const float * values, int values_count, int values_offset, const char * overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride);
void imgui_plot_histogram_float_pointer_pointer_int_pointer_int_string_float_vec2(ImGuiContext *imgui_c89_ctx, const char * label, float (*values_getter)(void *, int), void * data, int values_count, int values_offset, const char * overlay_text, float scale_min, float scale_max, ImVec2 graph_size);
void imgui_plot_lines_float_pointer_int_string_float_vec2_int(ImGuiContext *imgui_c89_ctx, const char * label, const float * values, int values_count, int values_offset, const char * overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride);
void imgui_plot_lines_float_pointer_pointer_int_pointer_int_string_float_vec2(ImGuiContext *imgui_c89_ctx, const char * label, float (*values_getter)(void *, int), void * data, int values_count, int values_offset, const char * overlay_text, float scale_min, float scale_max, ImVec2 graph_size);
void imgui_pop_button_repeat(ImGuiContext *imgui_c89_ctx);
void imgui_pop_clip_rect(ImGuiContext *imgui_c89_ctx);
void imgui_pop_font(ImGuiContext *imgui_c89_ctx);
void imgui_pop_id(ImGuiContext *imgui_c89_ctx);
void imgui_pop_item_flag(ImGuiContext *imgui_c89_ctx);
void imgui_pop_item_width(ImGuiContext *imgui_c89_ctx);
void imgui_pop_style_color(ImGuiContext *imgui_c89_ctx, int count);
void imgui_pop_style_var(ImGuiContext *imgui_c89_ctx, int count);
void imgui_pop_tab_stop(ImGuiContext *imgui_c89_ctx);
void imgui_pop_text_wrap_pos(ImGuiContext *imgui_c89_ctx);
void imgui_progress_bar(ImGuiContext *imgui_c89_ctx, float fraction, const ImVec2 * size_arg, const char * overlay);
void imgui_push_button_repeat(ImGuiContext *imgui_c89_ctx, unsigned char repeat);
void imgui_push_clip_rect(ImGuiContext *imgui_c89_ctx, const ImVec2 * clip_rect_min, const ImVec2 * clip_rect_max, unsigned char intersect_with_current_clip_rect);
void imgui_push_font_float(ImGuiContext *imgui_c89_ctx, ImFont * font, float font_size_base);
void imgui_push_font_none(ImGuiContext *imgui_c89_ctx, ImFont * font);
void imgui_push_id_const_pointer_none(ImGuiContext *imgui_c89_ctx, const void * ptr_id);
void imgui_push_id_int_none(ImGuiContext *imgui_c89_ctx, int int_id);
void imgui_push_id_string_none(ImGuiContext *imgui_c89_ctx, const char * str_id);
void imgui_push_id_string_string(ImGuiContext *imgui_c89_ctx, const char * str_id_begin, const char * str_id_end);
void imgui_push_item_flag(ImGuiContext *imgui_c89_ctx, ImGuiItemFlags option, unsigned char enabled);
void imgui_push_item_width(ImGuiContext *imgui_c89_ctx, float item_width);
void imgui_push_style_color_u32(ImGuiContext *imgui_c89_ctx, ImGuiCol idx, ImU32 col);
void imgui_push_style_color_vec4(ImGuiContext *imgui_c89_ctx, ImGuiCol idx, const ImVec4 * col);
void imgui_push_style_var_float(ImGuiContext *imgui_c89_ctx, ImGuiStyleVar idx, float val);
void imgui_push_style_var_vec2(ImGuiContext *imgui_c89_ctx, ImGuiStyleVar idx, const ImVec2 * val);
void imgui_push_style_var_x(ImGuiContext *imgui_c89_ctx, ImGuiStyleVar idx, float val_x);
void imgui_push_style_var_y(ImGuiContext *imgui_c89_ctx, ImGuiStyleVar idx, float val_y);
void imgui_push_tab_stop(ImGuiContext *imgui_c89_ctx, unsigned char tab_stop);
void imgui_push_text_wrap_pos(ImGuiContext *imgui_c89_ctx, float wrap_local_pos_x);
unsigned char imgui_radio_button_bool_none(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char active);
unsigned char imgui_radio_button_int_pointer_int(ImGuiContext *imgui_c89_ctx, const char * label, int * v, int v_button);
void imgui_render(ImGuiContext *imgui_c89_ctx);
void imgui_reset_mouse_drag_delta(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button);
void imgui_same_line(ImGuiContext *imgui_c89_ctx, float offset_from_start_x, float spacing_w);
void imgui_save_ini_settings_to_disk(ImGuiContext *imgui_c89_ctx, const char * ini_filename);
const char * imgui_save_ini_settings_to_memory(ImGuiContext *imgui_c89_ctx, size_t * out_size);
unsigned char imgui_selectable_bool(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char selected, ImGuiSelectableFlags flags, const ImVec2 * size_arg);
unsigned char imgui_selectable_bool_pointer(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char * p_selected, ImGuiSelectableFlags flags, const ImVec2 * size_arg);
void imgui_selection_basic_storage_apply_requests(ImGuiContext *imgui_c89_ctx, ImGuiSelectionBasicStorage *self, ImGuiMultiSelectIO * ms_io);
void imgui_selection_basic_storage_clear(ImGuiContext *imgui_c89_ctx, ImGuiSelectionBasicStorage *self);
unsigned char imgui_selection_basic_storage_contains(ImGuiSelectionBasicStorage *self, ImGuiID id);
unsigned char imgui_selection_basic_storage_get_next_selected_item(ImGuiSelectionBasicStorage *self, void ** opaque_it, ImGuiID * out_id);
ImGuiID imgui_selection_basic_storage_get_storage_id_from_index(ImGuiSelectionBasicStorage *self, int idx);
void imgui_selection_basic_storage_set_item_selected(ImGuiContext *imgui_c89_ctx, ImGuiSelectionBasicStorage *self, ImGuiID id, unsigned char selected);
void imgui_selection_basic_storage_swap(ImGuiSelectionBasicStorage *self, ImGuiSelectionBasicStorage * r);
void imgui_selection_external_storage_apply_requests(ImGuiSelectionExternalStorage *self, ImGuiMultiSelectIO * ms_io);
void imgui_separator(ImGuiContext *imgui_c89_ctx);
void imgui_separator_text(ImGuiContext *imgui_c89_ctx, const char * label);
void imgui_set_allocator_functions(ImGuiMemAllocFunc alloc_func, ImGuiMemFreeFunc free_func, void * user_data);
void imgui_set_clipboard_text(ImGuiContext *imgui_c89_ctx, const char * text);
void imgui_set_color_edit_options(ImGuiContext *imgui_c89_ctx, ImGuiColorEditFlags flags);
void imgui_set_column_offset(ImGuiContext *imgui_c89_ctx, int column_index, float offset);
void imgui_set_column_width(ImGuiContext *imgui_c89_ctx, int column_index, float width);
void imgui_set_current_context(ImGuiContext * ctx);
void imgui_set_cursor_pos(ImGuiContext *imgui_c89_ctx, const ImVec2 * local_pos);
void imgui_set_cursor_pos_x(ImGuiContext *imgui_c89_ctx, float x);
void imgui_set_cursor_pos_y(ImGuiContext *imgui_c89_ctx, float y);
void imgui_set_cursor_screen_pos(ImGuiContext *imgui_c89_ctx, const ImVec2 * pos);
unsigned char imgui_set_drag_drop_payload(ImGuiContext *imgui_c89_ctx, const char * type, const void * data, size_t data_size, ImGuiCond cond);
void imgui_set_item_default_focus(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_set_item_key_owner(ImGuiContext *imgui_c89_ctx, ImGuiKey key);
void imgui_set_item_tooltip_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_set_keyboard_focus_here(ImGuiContext *imgui_c89_ctx, int offset);
void imgui_set_mouse_cursor(ImGuiContext *imgui_c89_ctx, ImGuiMouseCursor cursor_type);
void imgui_set_nav_cursor_visible(ImGuiContext *imgui_c89_ctx, unsigned char visible);
void imgui_set_next_frame_want_capture_keyboard(ImGuiContext *imgui_c89_ctx, unsigned char want_capture_keyboard);
void imgui_set_next_frame_want_capture_mouse(ImGuiContext *imgui_c89_ctx, unsigned char want_capture_mouse);
void imgui_set_next_item_allow_overlap(ImGuiContext *imgui_c89_ctx);
void imgui_set_next_item_open(ImGuiContext *imgui_c89_ctx, unsigned char is_open, ImGuiCond cond);
void imgui_set_next_item_selection_user_data(ImGuiContext *imgui_c89_ctx, ImGuiSelectionUserData selection_user_data);
void imgui_set_next_item_shortcut(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord, ImGuiInputFlags flags);
void imgui_set_next_item_storage_id(ImGuiContext *imgui_c89_ctx, ImGuiID storage_id);
void imgui_set_next_item_width(ImGuiContext *imgui_c89_ctx, float item_width);
void imgui_set_next_window_bg_alpha(ImGuiContext *imgui_c89_ctx, float alpha);
void imgui_set_next_window_collapsed(ImGuiContext *imgui_c89_ctx, unsigned char collapsed, ImGuiCond cond);
void imgui_set_next_window_content_size(ImGuiContext *imgui_c89_ctx, const ImVec2 * size);
void imgui_set_next_window_focus(ImGuiContext *imgui_c89_ctx);
void imgui_set_next_window_pos(ImGuiContext *imgui_c89_ctx, const ImVec2 * pos, ImGuiCond cond, const ImVec2 * pivot);
void imgui_set_next_window_scroll(ImGuiContext *imgui_c89_ctx, const ImVec2 * scroll);
void imgui_set_next_window_size(ImGuiContext *imgui_c89_ctx, const ImVec2 * size, ImGuiCond cond);
void imgui_set_next_window_size_constraints(ImGuiContext *imgui_c89_ctx, const ImVec2 * size_min, const ImVec2 * size_max, ImGuiSizeCallback custom_callback, void * custom_callback_user_data);
void imgui_set_scroll_from_pos_x(ImGuiContext *imgui_c89_ctx, float local_x, float center_x_ratio);
void imgui_set_scroll_from_pos_y(ImGuiContext *imgui_c89_ctx, float local_y, float center_y_ratio);
void imgui_set_scroll_here_x(ImGuiContext *imgui_c89_ctx, float center_x_ratio);
void imgui_set_scroll_here_y(ImGuiContext *imgui_c89_ctx, float center_y_ratio);
void imgui_set_scroll_x(ImGuiContext *imgui_c89_ctx, float scroll_x);
void imgui_set_scroll_y(ImGuiContext *imgui_c89_ctx, float scroll_y);
void imgui_set_state_storage(ImGuiContext *imgui_c89_ctx, ImGuiStorage * tree);
void imgui_set_tab_item_closed(ImGuiContext *imgui_c89_ctx, const char * label);
void imgui_set_tooltip_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_set_window_collapsed_bool_cond_none(ImGuiContext *imgui_c89_ctx, unsigned char collapsed, ImGuiCond cond);
void imgui_set_window_collapsed_string_bool_cond(ImGuiContext *imgui_c89_ctx, const char * name, unsigned char collapsed, ImGuiCond cond);
void imgui_set_window_focus_none(ImGuiContext *imgui_c89_ctx);
void imgui_set_window_focus_string(ImGuiContext *imgui_c89_ctx, const char * name);
void imgui_set_window_font_scale(ImGuiContext *imgui_c89_ctx, float scale);
void imgui_set_window_pos_string_vec2_cond(ImGuiContext *imgui_c89_ctx, const char * name, const ImVec2 * pos, ImGuiCond cond);
void imgui_set_window_pos_vec2_cond_none(ImGuiContext *imgui_c89_ctx, const ImVec2 * pos, ImGuiCond cond);
void imgui_set_window_size_string_vec2_cond(ImGuiContext *imgui_c89_ctx, const char * name, const ImVec2 * size, ImGuiCond cond);
void imgui_set_window_size_vec2_cond_none(ImGuiContext *imgui_c89_ctx, const ImVec2 * size, ImGuiCond cond);
unsigned char imgui_shortcut(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord, ImGuiInputFlags flags);
void imgui_show_debug_log_window(ImGuiContext *imgui_c89_ctx, unsigned char * p_open);
void imgui_show_font_selector(ImGuiContext *imgui_c89_ctx, const char * label);
void imgui_show_id_stack_tool_window(ImGuiContext *imgui_c89_ctx, unsigned char * p_open);
void imgui_show_metrics_window(ImGuiContext *imgui_c89_ctx, unsigned char * p_open);
unsigned char imgui_slider_angle(ImGuiContext *imgui_c89_ctx, const char * label, float * v_rad, float v_degrees_min, float v_degrees_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_float(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_float2(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_float3(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_float4(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_int(ImGuiContext *imgui_c89_ctx, const char * label, int * v, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_int2(ImGuiContext *imgui_c89_ctx, const char * label, int * v, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_int3(ImGuiContext *imgui_c89_ctx, const char * label, int * v, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_int4(ImGuiContext *imgui_c89_ctx, const char * label, int * v, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_scalar(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiDataType data_type, void * p_data, const void * p_min, const void * p_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_scalar_n(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiDataType data_type, void * v, int components, const void * v_min, const void * v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_small_button(ImGuiContext *imgui_c89_ctx, const char * label);
void imgui_spacing(ImGuiContext *imgui_c89_ctx);
void imgui_storage_build_sort_by_key(ImGuiStorage *self);
void imgui_storage_clear(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self);
unsigned char imgui_storage_get_bool(ImGuiStorage *self, ImGuiID key, unsigned char default_val);
unsigned char * imgui_storage_get_bool_ref(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, unsigned char default_val);
float imgui_storage_get_float(ImGuiStorage *self, ImGuiID key, float default_val);
float * imgui_storage_get_float_ref(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, float default_val);
int imgui_storage_get_int(ImGuiStorage *self, ImGuiID key, int default_val);
int * imgui_storage_get_int_ref(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, int default_val);
void * imgui_storage_get_void_ptr(ImGuiStorage *self, ImGuiID key);
void ** imgui_storage_get_void_ptr_ref(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, void * default_val);
void imgui_storage_set_all_int(ImGuiStorage *self, int v);
void imgui_storage_set_bool(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, unsigned char val);
void imgui_storage_set_float(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, float val);
void imgui_storage_set_int(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, int val);
void imgui_storage_set_void_ptr(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, void * val);
void imgui_style_colors_classic(ImGuiContext *imgui_c89_ctx, ImGuiStyle * dst);
void imgui_style_colors_dark(ImGuiContext *imgui_c89_ctx, ImGuiStyle * dst);
void imgui_style_colors_light(ImGuiContext *imgui_c89_ctx, ImGuiStyle * dst);
void imgui_style_scale_all_sizes(ImGuiStyle *self, float scale_factor);
unsigned char imgui_tab_item_button(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiTabItemFlags flags);
void imgui_table_angled_headers_row(ImGuiContext *imgui_c89_ctx);
int imgui_table_get_column_count(ImGuiContext *imgui_c89_ctx);
ImGuiTableColumnFlags imgui_table_get_column_flags(ImGuiContext *imgui_c89_ctx, int column_n);
int imgui_table_get_column_index(ImGuiContext *imgui_c89_ctx);
const char * imgui_table_get_column_name(ImGuiContext *imgui_c89_ctx, int column_n);
int imgui_table_get_hovered_column(ImGuiContext *imgui_c89_ctx);
int imgui_table_get_row_index(ImGuiContext *imgui_c89_ctx);
ImGuiTableSortSpecs * imgui_table_get_sort_specs(ImGuiContext *imgui_c89_ctx);
void imgui_table_header(ImGuiContext *imgui_c89_ctx, const char * label);
void imgui_table_headers_row(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_table_next_column(ImGuiContext *imgui_c89_ctx);
void imgui_table_next_row(ImGuiContext *imgui_c89_ctx, ImGuiTableRowFlags row_flags, float row_min_height);
void imgui_table_set_bg_color(ImGuiContext *imgui_c89_ctx, ImGuiTableBgTarget target, ImU32 color, int column_n);
void imgui_table_set_column_enabled(ImGuiContext *imgui_c89_ctx, int column_n, unsigned char enabled);
unsigned char imgui_table_set_column_index(ImGuiContext *imgui_c89_ctx, int column_n);
void imgui_table_setup_column(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiTableColumnFlags flags, float init_width_or_weight, ImGuiID user_data);
void imgui_table_setup_scroll_freeze(ImGuiContext *imgui_c89_ctx, int columns, int rows);
void imgui_text_buffer_append(ImGuiContext *imgui_c89_ctx, ImGuiTextBuffer *self, const char * str, const char * str_end);
void imgui_text_buffer_appendfv(ImGuiContext *imgui_c89_ctx, ImGuiTextBuffer *self, const char * fmt, va_list args);
const char * imgui_text_buffer_begin(ImGuiTextBuffer *self);
const char * imgui_text_buffer_c_str(ImGuiTextBuffer *self);
void imgui_text_buffer_clear(ImGuiContext *imgui_c89_ctx, ImGuiTextBuffer *self);
unsigned char imgui_text_buffer_empty(ImGuiTextBuffer *self);
const char * imgui_text_buffer_end(ImGuiTextBuffer *self);
void imgui_text_buffer_reserve(ImGuiContext *imgui_c89_ctx, ImGuiTextBuffer *self, int capacity);
void imgui_text_buffer_resize(ImGuiContext *imgui_c89_ctx, ImGuiTextBuffer *self, int size);
int imgui_text_buffer_size(ImGuiTextBuffer *self);
void imgui_text_colored_v(ImGuiContext *imgui_c89_ctx, const ImVec4 * col, const char * fmt, va_list args);
void imgui_text_disabled_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_text_filter_build(ImGuiContext *imgui_c89_ctx, ImGuiTextFilter *self);
void imgui_text_filter_clear(ImGuiContext *imgui_c89_ctx, ImGuiTextFilter *self);
unsigned char imgui_text_filter_draw(ImGuiContext *imgui_c89_ctx, ImGuiTextFilter *self, const char * label, float width);
unsigned char imgui_text_filter_is_active(ImGuiTextFilter *self);
unsigned char imgui_text_filter_pass_filter(ImGuiTextFilter *self, const char * text, const char * text_end);
unsigned char imgui_text_link(ImGuiContext *imgui_c89_ctx, const char * label);
unsigned char imgui_text_link_open_url(ImGuiContext *imgui_c89_ctx, const char * label, const char * url);
unsigned char imgui_text_range_empty(ImGuiTextFilter::ImGuiTextRange *self);
void imgui_text_range_split(ImGuiContext *imgui_c89_ctx, ImGuiTextFilter::ImGuiTextRange *self, char separator, ImVector<ImGuiTextFilter::ImGuiTextRange> * out);
void imgui_text_unformatted(ImGuiContext *imgui_c89_ctx, const char * text, const char * text_end);
void imgui_text_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_text_wrapped_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_texture_data_create(ImGuiContext *imgui_c89_ctx, ImTextureData *self, ImTextureFormat format, int w, int h);
void imgui_texture_data_destroy_pixels(ImGuiContext *imgui_c89_ctx, ImTextureData *self);
int imgui_texture_data_get_pitch(ImTextureData *self);
void * imgui_texture_data_get_pixels(ImTextureData *self);
void * imgui_texture_data_get_pixels_at(ImTextureData *self, int x, int y);
int imgui_texture_data_get_size_in_bytes(ImTextureData *self);
ImTextureID imgui_texture_data_get_tex_id(ImTextureData *self);
ImTextureRef imgui_texture_data_get_tex_ref(ImTextureData *self);
void imgui_texture_data_set_status(ImTextureData *self, ImTextureStatus status);
void imgui_texture_data_set_tex_id(ImTextureData *self, ImTextureID tex_id);
ImTextureID imgui_texture_ref_get_tex_id(ImTextureRef *self);
unsigned char imgui_tree_node(ImGuiContext *imgui_c89_ctx, const char * label);
unsigned char imgui_tree_node_ex(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiTreeNodeFlags flags);
unsigned char imgui_tree_node_ex_v_const_pointer(ImGuiContext *imgui_c89_ctx, const void * ptr_id, ImGuiTreeNodeFlags flags, const char * fmt, va_list args);
unsigned char imgui_tree_node_ex_v_string(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiTreeNodeFlags flags, const char * fmt, va_list args);
unsigned char imgui_tree_node_get_open(ImGuiContext *imgui_c89_ctx, ImGuiID storage_id);
unsigned char imgui_tree_node_v_const_pointer(ImGuiContext *imgui_c89_ctx, const void * ptr_id, const char * fmt, va_list args);
unsigned char imgui_tree_node_v_string(ImGuiContext *imgui_c89_ctx, const char * str_id, const char * fmt, va_list args);
void imgui_tree_pop(ImGuiContext *imgui_c89_ctx);
void imgui_tree_push_const_pointer(ImGuiContext *imgui_c89_ctx, const void * ptr_id);
void imgui_tree_push_string(ImGuiContext *imgui_c89_ctx, const char * str_id);
void imgui_unindent(ImGuiContext *imgui_c89_ctx, float indent_w);
unsigned char imgui_v_slider_float(ImGuiContext *imgui_c89_ctx, const char * label, const ImVec2 * size, float * v, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_v_slider_int(ImGuiContext *imgui_c89_ctx, const char * label, const ImVec2 * size, int * v, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_v_slider_scalar(ImGuiContext *imgui_c89_ctx, const char * label, const ImVec2 * size, ImGuiDataType data_type, void * p_data, const void * p_min, const void * p_max, const char * format, ImGuiSliderFlags flags);
void imgui_value_bool_none(ImGuiContext *imgui_c89_ctx, const char * prefix, unsigned char b);
void imgui_value_float_string(ImGuiContext *imgui_c89_ctx, const char * prefix, float v, const char * float_format);
void imgui_value_int_none(ImGuiContext *imgui_c89_ctx, const char * prefix, int v);
void imgui_value_uint_none(ImGuiContext *imgui_c89_ctx, const char * prefix, unsigned int v);
ImVec2 imgui_viewport_get_center(ImGuiViewport *self);
ImVec2 imgui_viewport_get_work_center(ImGuiViewport *self);
#ifdef __cplusplus
}
#endif
#else
#include <stddef.h>
#include <limits.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

/* C89 has no standard 64-bit integer spelling. Prefer the
 * native long where it is wide enough and isolate the required
 * compiler extension on ILP32/LLP64 targets. */
#if ULONG_MAX > 0xffffffffUL
typedef long imgui_c89_i64;
typedef unsigned long imgui_c89_u64;
#elif defined(_MSC_VER)
typedef __int64 imgui_c89_i64;
typedef unsigned __int64 imgui_c89_u64;
#elif defined(__GNUC__)
__extension__ typedef long long imgui_c89_i64;
__extension__ typedef unsigned long long imgui_c89_u64;
#else
#error "A 64-bit integer extension is required on this C89 target"
#endif
#if defined(__GNUC__)
#define IMGUI_C89_EXTENSION __extension__
#else
#define IMGUI_C89_EXTENSION
#endif
#if defined(__clang__) || defined(__GNUC__)
#define IMGUI_C89_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define IMGUI_C89_NOINLINE __declspec(noinline)
#else
#define IMGUI_C89_NOINLINE
#endif
#define imgui_c89_expect(condition, expected) \
    ((void)(expected), (condition))

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ImColor ImColor;
typedef struct ImDrawChannel ImDrawChannel;
typedef struct ImDrawCmd ImDrawCmd;
typedef struct ImDrawCmdHeader ImDrawCmdHeader;
typedef struct ImDrawData ImDrawData;
typedef struct ImDrawList ImDrawList;
typedef struct ImDrawListSharedData ImDrawListSharedData;
typedef struct ImDrawListSplitter ImDrawListSplitter;
typedef struct ImDrawVert ImDrawVert;
typedef struct ImFont ImFont;
typedef struct ImFontAtlas ImFontAtlas;
typedef union imgui_c89_anon_imgui_3836_5 imgui_c89_anon_imgui_3836_5;
typedef struct ImFontAtlasBuilder ImFontAtlasBuilder;
typedef struct ImFontAtlasRect ImFontAtlasRect;
typedef struct ImFontBaked ImFontBaked;
typedef struct ImFontConfig ImFontConfig;
typedef struct ImFontGlyph ImFontGlyph;
typedef struct ImFontGlyphRangesBuilder ImFontGlyphRangesBuilder;
typedef struct ImFontLoader ImFontLoader;
typedef struct ImGuiContext ImGuiContext;
typedef struct ImGuiIO ImGuiIO;
typedef struct ImGuiInputTextCallbackData ImGuiInputTextCallbackData;
typedef struct ImGuiKeyData ImGuiKeyData;
typedef struct ImGuiListClipper ImGuiListClipper;
typedef struct ImGuiMultiSelectIO ImGuiMultiSelectIO;
typedef struct ImGuiOnceUponAFrame ImGuiOnceUponAFrame;
typedef struct ImGuiPayload ImGuiPayload;
typedef struct ImGuiPlatformIO ImGuiPlatformIO;
typedef struct ImGuiPlatformImeData ImGuiPlatformImeData;
typedef struct ImGuiSelectionBasicStorage ImGuiSelectionBasicStorage;
typedef struct ImGuiSelectionExternalStorage ImGuiSelectionExternalStorage;
typedef struct ImGuiSelectionRequest ImGuiSelectionRequest;
typedef struct ImGuiSizeCallbackData ImGuiSizeCallbackData;
typedef struct ImGuiStorage ImGuiStorage;
typedef struct ImGuiStoragePair ImGuiStoragePair;
typedef union imgui_c89_anon_imgui_2816_5 imgui_c89_anon_imgui_2816_5;
typedef struct ImGuiStyle ImGuiStyle;
typedef struct ImGuiTableColumnSortSpecs ImGuiTableColumnSortSpecs;
typedef struct ImGuiTableSortSpecs ImGuiTableSortSpecs;
typedef struct ImGuiTextBuffer ImGuiTextBuffer;
typedef struct ImGuiTextFilter ImGuiTextFilter;
typedef struct ImGuiTextFilter_ImGuiTextRange ImGuiTextFilter_ImGuiTextRange;
typedef struct ImGuiViewport ImGuiViewport;
typedef struct ImNewWrapper ImNewWrapper;
typedef struct ImTextureData ImTextureData;
typedef struct ImTextureRect ImTextureRect;
typedef struct ImTextureRef ImTextureRef;
typedef struct ImVec2 ImVec2;
typedef struct ImVec4 ImVec4;
typedef struct ImVector_ImDrawChannel ImVector_ImDrawChannel;
typedef struct ImVector_ImDrawCmd ImVector_ImDrawCmd;
typedef struct ImVector_ImDrawList_ptr ImVector_ImDrawList_ptr;
typedef struct ImVector_ImDrawListSharedData_ptr ImVector_ImDrawListSharedData_ptr;
typedef struct ImVector_ImDrawVert ImVector_ImDrawVert;
typedef struct ImVector_ImFont_ptr ImVector_ImFont_ptr;
typedef struct ImVector_ImFontConfig_ptr ImVector_ImFontConfig_ptr;
typedef struct ImVector_ImFontConfig ImVector_ImFontConfig;
typedef struct ImVector_ImFontGlyph ImVector_ImFontGlyph;
typedef struct ImVector_ImGuiSelectionRequest ImVector_ImGuiSelectionRequest;
typedef struct ImVector_ImGuiStoragePair ImVector_ImGuiStoragePair;
typedef struct ImVector_ImGuiTextFilter_ImGuiTextRange ImVector_ImGuiTextFilter_ImGuiTextRange;
typedef struct ImVector_ImTextureData_ptr ImVector_ImTextureData_ptr;
typedef struct ImVector_ImTextureRect ImVector_ImTextureRect;
typedef struct ImVector_ImTextureRef ImVector_ImTextureRef;
typedef struct ImVector_ImVec2 ImVector_ImVec2;
typedef struct ImVector_ImVec4 ImVector_ImVec4;
typedef struct ImVector_char ImVector_char;
typedef struct ImVector_float ImVector_float;
typedef struct ImVector_unsigned_char ImVector_unsigned_char;
typedef struct ImVector_unsigned_int ImVector_unsigned_int;
typedef struct ImVector_unsigned_short ImVector_unsigned_short;

typedef unsigned int ImGuiID;
typedef signed char ImS8;
typedef unsigned char ImU8;
typedef short ImS16;
typedef unsigned short ImU16;
typedef int ImS32;
typedef unsigned int ImU32;
typedef imgui_c89_i64 ImS64;
typedef imgui_c89_u64 ImU64;
typedef int ImGuiCol;
typedef int ImGuiCond;
typedef int ImGuiDataType;
typedef int ImGuiMouseButton;
typedef int ImGuiMouseCursor;
typedef int ImGuiStyleVar;
typedef int ImGuiTableBgTarget;
typedef int ImDrawFlags;
typedef int ImDrawListFlags;
typedef int ImDrawTextFlags;
typedef int ImFontFlags;
typedef int ImFontAtlasFlags;
typedef int ImGuiBackendFlags;
typedef int ImGuiButtonFlags;
typedef int ImGuiChildFlags;
typedef int ImGuiColorEditFlags;
typedef int ImGuiConfigFlags;
typedef int ImGuiComboFlags;
typedef int ImGuiDragDropFlags;
typedef int ImGuiFocusedFlags;
typedef int ImGuiHoveredFlags;
typedef int ImGuiInputFlags;
typedef int ImGuiInputTextFlags;
typedef int ImGuiItemFlags;
typedef int ImGuiKeyChord;
typedef int ImGuiListClipperFlags;
typedef int ImGuiPopupFlags;
typedef int ImGuiMultiSelectFlags;
typedef int ImGuiSelectableFlags;
typedef int ImGuiSliderFlags;
typedef int ImGuiTabBarFlags;
typedef int ImGuiTabItemFlags;
typedef int ImGuiTableFlags;
typedef int ImGuiTableColumnFlags;
typedef int ImGuiTableRowFlags;
typedef int ImGuiTreeNodeFlags;
typedef int ImGuiViewportFlags;
typedef int ImGuiWindowFlags;
typedef unsigned int ImWchar32;
typedef unsigned short ImWchar16;
typedef ImWchar16 ImWchar;
typedef ImS64 ImGuiSelectionUserData;
typedef int (*ImGuiInputTextCallback)(ImGuiInputTextCallbackData *);
typedef void (*ImGuiSizeCallback)(ImGuiSizeCallbackData *);
typedef void *(*ImGuiMemAllocFunc)(size_t, void *);
typedef void (*ImGuiMemFreeFunc)(void *, void *);
typedef ImU64 ImTextureID;
typedef unsigned short ImDrawIdx;
typedef void (*ImDrawCallback)(const ImDrawList *, const ImDrawCmd *);
typedef int ImFontAtlasRectId;
typedef ImFontAtlasRect ImFontAtlasCustomRect;

typedef int ImGuiDir;
typedef int ImGuiKey;
typedef int ImGuiMouseSource;
typedef int ImGuiSelectionRequestType;
typedef int ImGuiSortDirection;
typedef int ImTextureFormat;
typedef int ImTextureStatus;

/* ImDrawFlags_ */
enum {
    ImDrawFlags_Closed = 512,
    ImDrawFlags_InvalidMask_ = -2147483633,
    ImDrawFlags_None = 0,
    ImDrawFlags_RoundCornersAll = 240,
    ImDrawFlags_RoundCornersBottom = 192,
    ImDrawFlags_RoundCornersBottomLeft = 64,
    ImDrawFlags_RoundCornersBottomRight = 128,
    ImDrawFlags_RoundCornersDefault_ = 240,
    ImDrawFlags_RoundCornersLeft = 80,
    ImDrawFlags_RoundCornersMask_ = 496,
    ImDrawFlags_RoundCornersNone = 256,
    ImDrawFlags_RoundCornersRight = 160,
    ImDrawFlags_RoundCornersTop = 48,
    ImDrawFlags_RoundCornersTopLeft = 16,
    ImDrawFlags_RoundCornersTopRight = 32
};

/* ImDrawListFlags_ */
enum {
    ImDrawListFlags_AllowVtxOffset = 8,
    ImDrawListFlags_AntiAliasedFill = 4,
    ImDrawListFlags_AntiAliasedLines = 1,
    ImDrawListFlags_AntiAliasedLinesUseTex = 2,
    ImDrawListFlags_None = 0,
    ImDrawListFlags_TextNoPixelSnap = 16
};

/* ImFontAtlasFlags_ */
enum {
    ImFontAtlasFlags_NoBakedLines = 4,
    ImFontAtlasFlags_NoMouseCursors = 2,
    ImFontAtlasFlags_NoPowerOfTwoHeight = 1,
    ImFontAtlasFlags_None = 0
};

/* ImFontFlags_ */
enum {
    ImFontFlags_ImplicitRefSize = 16,
    ImFontFlags_LockBakedSizes = 8,
    ImFontFlags_NoLoadError = 2,
    ImFontFlags_NoLoadGlyphs = 4,
    ImFontFlags_None = 0
};

/* ImGuiBackendFlags_ */
enum {
    ImGuiBackendFlags_HasGamepad = 1,
    ImGuiBackendFlags_HasMouseCursors = 2,
    ImGuiBackendFlags_HasSetMousePos = 4,
    ImGuiBackendFlags_None = 0,
    ImGuiBackendFlags_RendererHasTextures = 16,
    ImGuiBackendFlags_RendererHasVtxOffset = 8
};

/* ImGuiButtonFlags_ */
enum {
    ImGuiButtonFlags_AllowOverlap = 4096,
    ImGuiButtonFlags_EnableNav = 8,
    ImGuiButtonFlags_MouseButtonLeft = 1,
    ImGuiButtonFlags_MouseButtonMask_ = 7,
    ImGuiButtonFlags_MouseButtonMiddle = 4,
    ImGuiButtonFlags_MouseButtonRight = 2,
    ImGuiButtonFlags_None = 0
};

/* ImGuiChildFlags_ */
enum {
    ImGuiChildFlags_AlwaysAutoResize = 64,
    ImGuiChildFlags_AlwaysUseWindowPadding = 2,
    ImGuiChildFlags_AutoResizeX = 16,
    ImGuiChildFlags_AutoResizeY = 32,
    ImGuiChildFlags_Borders = 1,
    ImGuiChildFlags_FrameStyle = 128,
    ImGuiChildFlags_NavFlattened = 256,
    ImGuiChildFlags_None = 0,
    ImGuiChildFlags_ResizeX = 4,
    ImGuiChildFlags_ResizeY = 8
};

/* ImGuiCol_ */
enum {
    ImGuiCol_Border = 5,
    ImGuiCol_BorderShadow = 6,
    ImGuiCol_Button = 22,
    ImGuiCol_ButtonActive = 24,
    ImGuiCol_ButtonHovered = 23,
    ImGuiCol_COUNT = 61,
    ImGuiCol_CheckMark = 18,
    ImGuiCol_CheckboxSelectedBg = 19,
    ImGuiCol_ChildBg = 3,
    ImGuiCol_DragDropTarget = 54,
    ImGuiCol_DragDropTargetBg = 55,
    ImGuiCol_FrameBg = 7,
    ImGuiCol_FrameBgActive = 9,
    ImGuiCol_FrameBgHovered = 8,
    ImGuiCol_Header = 25,
    ImGuiCol_HeaderActive = 27,
    ImGuiCol_HeaderHovered = 26,
    ImGuiCol_InputTextCursor = 34,
    ImGuiCol_MenuBarBg = 13,
    ImGuiCol_ModalWindowDimBg = 60,
    ImGuiCol_NavCursor = 57,
    ImGuiCol_NavHighlight = 57,
    ImGuiCol_NavWindowingDimBg = 59,
    ImGuiCol_NavWindowingHighlight = 58,
    ImGuiCol_PlotHistogram = 44,
    ImGuiCol_PlotHistogramHovered = 45,
    ImGuiCol_PlotLines = 42,
    ImGuiCol_PlotLinesHovered = 43,
    ImGuiCol_PopupBg = 4,
    ImGuiCol_ResizeGrip = 31,
    ImGuiCol_ResizeGripActive = 33,
    ImGuiCol_ResizeGripHovered = 32,
    ImGuiCol_ScrollbarBg = 14,
    ImGuiCol_ScrollbarGrab = 15,
    ImGuiCol_ScrollbarGrabActive = 17,
    ImGuiCol_ScrollbarGrabHovered = 16,
    ImGuiCol_Separator = 28,
    ImGuiCol_SeparatorActive = 30,
    ImGuiCol_SeparatorHovered = 29,
    ImGuiCol_SliderGrab = 20,
    ImGuiCol_SliderGrabActive = 21,
    ImGuiCol_Tab = 36,
    ImGuiCol_TabActive = 37,
    ImGuiCol_TabDimmed = 39,
    ImGuiCol_TabDimmedSelected = 40,
    ImGuiCol_TabDimmedSelectedOverline = 41,
    ImGuiCol_TabHovered = 35,
    ImGuiCol_TabSelected = 37,
    ImGuiCol_TabSelectedOverline = 38,
    ImGuiCol_TabUnfocused = 39,
    ImGuiCol_TabUnfocusedActive = 40,
    ImGuiCol_TableBorderLight = 48,
    ImGuiCol_TableBorderStrong = 47,
    ImGuiCol_TableHeaderBg = 46,
    ImGuiCol_TableRowBg = 49,
    ImGuiCol_TableRowBgAlt = 50,
    ImGuiCol_Text = 0,
    ImGuiCol_TextDisabled = 1,
    ImGuiCol_TextLink = 51,
    ImGuiCol_TextSelectedBg = 52,
    ImGuiCol_TitleBg = 10,
    ImGuiCol_TitleBgActive = 11,
    ImGuiCol_TitleBgCollapsed = 12,
    ImGuiCol_TreeLines = 53,
    ImGuiCol_UnsavedMarker = 56,
    ImGuiCol_WindowBg = 2
};

/* ImGuiColorEditFlags_ */
enum {
    ImGuiColorEditFlags_AlphaBar = 262144,
    ImGuiColorEditFlags_AlphaMask_ = 28674,
    ImGuiColorEditFlags_AlphaNoBg = 8192,
    ImGuiColorEditFlags_AlphaOpaque = 4096,
    ImGuiColorEditFlags_AlphaPreview = 0,
    ImGuiColorEditFlags_AlphaPreviewHalf = 16384,
    ImGuiColorEditFlags_DataTypeMask_ = 25165824,
    ImGuiColorEditFlags_DefaultOptions_ = 311427072,
    ImGuiColorEditFlags_DisplayHSV = 2097152,
    ImGuiColorEditFlags_DisplayHex = 4194304,
    ImGuiColorEditFlags_DisplayMask_ = 7340032,
    ImGuiColorEditFlags_DisplayRGB = 1048576,
    ImGuiColorEditFlags_Float = 16777216,
    ImGuiColorEditFlags_HDR = 524288,
    ImGuiColorEditFlags_InputHSV = 536870912,
    ImGuiColorEditFlags_InputMask_ = 805306368,
    ImGuiColorEditFlags_InputRGB = 268435456,
    ImGuiColorEditFlags_NoAlpha = 2,
    ImGuiColorEditFlags_NoBorder = 1024,
    ImGuiColorEditFlags_NoColorMarkers = 2048,
    ImGuiColorEditFlags_NoDragDrop = 512,
    ImGuiColorEditFlags_NoInputs = 32,
    ImGuiColorEditFlags_NoLabel = 128,
    ImGuiColorEditFlags_NoOptions = 8,
    ImGuiColorEditFlags_NoPicker = 4,
    ImGuiColorEditFlags_NoSidePreview = 256,
    ImGuiColorEditFlags_NoSmallPreview = 16,
    ImGuiColorEditFlags_NoTooltip = 64,
    ImGuiColorEditFlags_None = 0,
    ImGuiColorEditFlags_PickerHueBar = 33554432,
    ImGuiColorEditFlags_PickerHueWheel = 67108864,
    ImGuiColorEditFlags_PickerMask_ = 100663296,
    ImGuiColorEditFlags_PickerNoRotate = 134217728,
    ImGuiColorEditFlags_Uint8 = 8388608
};

/* ImGuiComboFlags_ */
enum {
    ImGuiComboFlags_HeightLarge = 8,
    ImGuiComboFlags_HeightLargest = 16,
    ImGuiComboFlags_HeightMask_ = 30,
    ImGuiComboFlags_HeightRegular = 4,
    ImGuiComboFlags_HeightSmall = 2,
    ImGuiComboFlags_NoArrowButton = 32,
    ImGuiComboFlags_NoPreview = 64,
    ImGuiComboFlags_None = 0,
    ImGuiComboFlags_PopupAlignLeft = 1,
    ImGuiComboFlags_WidthFitPreview = 128
};

/* ImGuiCond_ */
enum {
    ImGuiCond_Always = 1,
    ImGuiCond_Appearing = 8,
    ImGuiCond_FirstUseEver = 4,
    ImGuiCond_None = 0,
    ImGuiCond_Once = 2
};

/* ImGuiConfigFlags_ */
enum {
    ImGuiConfigFlags_IsSRGB = 1048576,
    ImGuiConfigFlags_IsTouchScreen = 2097152,
    ImGuiConfigFlags_NavEnableGamepad = 2,
    ImGuiConfigFlags_NavEnableKeyboard = 1,
    ImGuiConfigFlags_NavEnableSetMousePos = 4,
    ImGuiConfigFlags_NavNoCaptureKeyboard = 8,
    ImGuiConfigFlags_NoKeyboard = 64,
    ImGuiConfigFlags_NoMouse = 16,
    ImGuiConfigFlags_NoMouseCursorChange = 32,
    ImGuiConfigFlags_None = 0
};

/* ImGuiDataType_ */
enum {
    ImGuiDataType_Bool = 10,
    ImGuiDataType_COUNT = 12,
    ImGuiDataType_Double = 9,
    ImGuiDataType_Float = 8,
    ImGuiDataType_S16 = 2,
    ImGuiDataType_S32 = 4,
    ImGuiDataType_S64 = 6,
    ImGuiDataType_S8 = 0,
    ImGuiDataType_String = 11,
    ImGuiDataType_U16 = 3,
    ImGuiDataType_U32 = 5,
    ImGuiDataType_U64 = 7,
    ImGuiDataType_U8 = 1
};

/* ImGuiDir */
enum {
    ImGuiDir_COUNT = 4,
    ImGuiDir_Down = 3,
    ImGuiDir_Left = 0,
    ImGuiDir_None = -1,
    ImGuiDir_Right = 1,
    ImGuiDir_Up = 2
};

/* ImGuiDragDropFlags_ */
enum {
    ImGuiDragDropFlags_AcceptBeforeDelivery = 1024,
    ImGuiDragDropFlags_AcceptDrawAsHovered = 8192,
    ImGuiDragDropFlags_AcceptNoDrawDefaultRect = 2048,
    ImGuiDragDropFlags_AcceptNoPreviewTooltip = 4096,
    ImGuiDragDropFlags_AcceptPeekOnly = 3072,
    ImGuiDragDropFlags_None = 0,
    ImGuiDragDropFlags_PayloadAutoExpire = 32,
    ImGuiDragDropFlags_PayloadNoCrossContext = 64,
    ImGuiDragDropFlags_PayloadNoCrossProcess = 128,
    ImGuiDragDropFlags_SourceAllowNullID = 8,
    ImGuiDragDropFlags_SourceExtern = 16,
    ImGuiDragDropFlags_SourceNoDisableHover = 2,
    ImGuiDragDropFlags_SourceNoHoldToOpenOthers = 4,
    ImGuiDragDropFlags_SourceNoPreviewTooltip = 1
};

/* ImGuiFocusedFlags_ */
enum {
    ImGuiFocusedFlags_AnyWindow = 4,
    ImGuiFocusedFlags_ChildWindows = 1,
    ImGuiFocusedFlags_NoPopupHierarchy = 8,
    ImGuiFocusedFlags_None = 0,
    ImGuiFocusedFlags_RootAndChildWindows = 3,
    ImGuiFocusedFlags_RootWindow = 2
};

/* ImGuiHoveredFlags_ */
enum {
    ImGuiHoveredFlags_AllowWhenBlockedByActiveItem = 128,
    ImGuiHoveredFlags_AllowWhenBlockedByPopup = 32,
    ImGuiHoveredFlags_AllowWhenDisabled = 1024,
    ImGuiHoveredFlags_AllowWhenOverlapped = 768,
    ImGuiHoveredFlags_AllowWhenOverlappedByItem = 256,
    ImGuiHoveredFlags_AllowWhenOverlappedByWindow = 512,
    ImGuiHoveredFlags_AnyWindow = 4,
    ImGuiHoveredFlags_ChildWindows = 1,
    ImGuiHoveredFlags_DelayNone = 16384,
    ImGuiHoveredFlags_DelayNormal = 65536,
    ImGuiHoveredFlags_DelayShort = 32768,
    ImGuiHoveredFlags_ForTooltip = 4096,
    ImGuiHoveredFlags_NoNavOverride = 2048,
    ImGuiHoveredFlags_NoPopupHierarchy = 8,
    ImGuiHoveredFlags_NoSharedDelay = 131072,
    ImGuiHoveredFlags_None = 0,
    ImGuiHoveredFlags_RectOnly = 928,
    ImGuiHoveredFlags_RootAndChildWindows = 3,
    ImGuiHoveredFlags_RootWindow = 2,
    ImGuiHoveredFlags_Stationary = 8192
};

/* ImGuiInputFlags_ */
enum {
    ImGuiInputFlags_None = 0,
    ImGuiInputFlags_Repeat = 1,
    ImGuiInputFlags_RouteActive = 1024,
    ImGuiInputFlags_RouteAlways = 8192,
    ImGuiInputFlags_RouteFocused = 2048,
    ImGuiInputFlags_RouteFromRootWindow = 131072,
    ImGuiInputFlags_RouteGlobal = 4096,
    ImGuiInputFlags_RouteOverActive = 32768,
    ImGuiInputFlags_RouteOverFocused = 16384,
    ImGuiInputFlags_RouteUnlessBgFocused = 65536,
    ImGuiInputFlags_Tooltip = 262144
};

/* ImGuiInputTextFlags_ */
enum {
    ImGuiInputTextFlags_AllowTabInput = 32,
    ImGuiInputTextFlags_AlwaysOverwrite = 2048,
    ImGuiInputTextFlags_AutoSelectAll = 4096,
    ImGuiInputTextFlags_CallbackAlways = 1048576,
    ImGuiInputTextFlags_CallbackCharFilter = 2097152,
    ImGuiInputTextFlags_CallbackCompletion = 262144,
    ImGuiInputTextFlags_CallbackEdit = 8388608,
    ImGuiInputTextFlags_CallbackHistory = 524288,
    ImGuiInputTextFlags_CallbackResize = 4194304,
    ImGuiInputTextFlags_CharsDecimal = 1,
    ImGuiInputTextFlags_CharsHexadecimal = 2,
    ImGuiInputTextFlags_CharsNoBlank = 16,
    ImGuiInputTextFlags_CharsScientific = 4,
    ImGuiInputTextFlags_CharsUppercase = 8,
    ImGuiInputTextFlags_CtrlEnterForNewLine = 256,
    ImGuiInputTextFlags_DisplayEmptyRefVal = 16384,
    ImGuiInputTextFlags_ElideLeft = 131072,
    ImGuiInputTextFlags_EnterReturnsTrue = 64,
    ImGuiInputTextFlags_EscapeClearsAll = 128,
    ImGuiInputTextFlags_NoHorizontalScroll = 32768,
    ImGuiInputTextFlags_NoUndoRedo = 65536,
    ImGuiInputTextFlags_None = 0,
    ImGuiInputTextFlags_ParseEmptyRefVal = 8192,
    ImGuiInputTextFlags_Password = 1024,
    ImGuiInputTextFlags_ReadOnly = 512,
    ImGuiInputTextFlags_WordWrap = 16777216
};

/* ImGuiItemFlags_ */
enum {
    ImGuiItemFlags_AllowDuplicateId = 32,
    ImGuiItemFlags_AutoClosePopups = 16,
    ImGuiItemFlags_ButtonRepeat = 8,
    ImGuiItemFlags_Disabled = 64,
    ImGuiItemFlags_LiveEditOnInput = 384,
    ImGuiItemFlags_LiveEditOnInputScalar = 256,
    ImGuiItemFlags_LiveEditOnInputText = 128,
    ImGuiItemFlags_NoNav = 2,
    ImGuiItemFlags_NoNavDefaultFocus = 4,
    ImGuiItemFlags_NoTabStop = 1,
    ImGuiItemFlags_None = 0
};

/* ImGuiKey */
enum {
    ImGuiKey_0 = 536,
    ImGuiKey_1 = 537,
    ImGuiKey_2 = 538,
    ImGuiKey_3 = 539,
    ImGuiKey_4 = 540,
    ImGuiKey_5 = 541,
    ImGuiKey_6 = 542,
    ImGuiKey_7 = 543,
    ImGuiKey_8 = 544,
    ImGuiKey_9 = 545,
    ImGuiKey_A = 546,
    ImGuiKey_Apostrophe = 596,
    ImGuiKey_AppBack = 629,
    ImGuiKey_AppForward = 630,
    ImGuiKey_B = 547,
    ImGuiKey_Backslash = 604,
    ImGuiKey_Backspace = 523,
    ImGuiKey_C = 548,
    ImGuiKey_COUNT = 667,
    ImGuiKey_CapsLock = 607,
    ImGuiKey_Comma = 597,
    ImGuiKey_D = 549,
    ImGuiKey_Delete = 522,
    ImGuiKey_DownArrow = 516,
    ImGuiKey_E = 550,
    ImGuiKey_End = 520,
    ImGuiKey_Enter = 525,
    ImGuiKey_Equal = 602,
    ImGuiKey_Escape = 526,
    ImGuiKey_F = 551,
    ImGuiKey_F1 = 572,
    ImGuiKey_F10 = 581,
    ImGuiKey_F11 = 582,
    ImGuiKey_F12 = 583,
    ImGuiKey_F13 = 584,
    ImGuiKey_F14 = 585,
    ImGuiKey_F15 = 586,
    ImGuiKey_F16 = 587,
    ImGuiKey_F17 = 588,
    ImGuiKey_F18 = 589,
    ImGuiKey_F19 = 590,
    ImGuiKey_F2 = 573,
    ImGuiKey_F20 = 591,
    ImGuiKey_F21 = 592,
    ImGuiKey_F22 = 593,
    ImGuiKey_F23 = 594,
    ImGuiKey_F24 = 595,
    ImGuiKey_F3 = 574,
    ImGuiKey_F4 = 575,
    ImGuiKey_F5 = 576,
    ImGuiKey_F6 = 577,
    ImGuiKey_F7 = 578,
    ImGuiKey_F8 = 579,
    ImGuiKey_F9 = 580,
    ImGuiKey_G = 552,
    ImGuiKey_GamepadBack = 633,
    ImGuiKey_GamepadDpadDown = 641,
    ImGuiKey_GamepadDpadLeft = 638,
    ImGuiKey_GamepadDpadRight = 639,
    ImGuiKey_GamepadDpadUp = 640,
    ImGuiKey_GamepadFaceDown = 637,
    ImGuiKey_GamepadFaceLeft = 634,
    ImGuiKey_GamepadFaceRight = 635,
    ImGuiKey_GamepadFaceUp = 636,
    ImGuiKey_GamepadL1 = 642,
    ImGuiKey_GamepadL2 = 644,
    ImGuiKey_GamepadL3 = 646,
    ImGuiKey_GamepadLStickDown = 651,
    ImGuiKey_GamepadLStickLeft = 648,
    ImGuiKey_GamepadLStickRight = 649,
    ImGuiKey_GamepadLStickUp = 650,
    ImGuiKey_GamepadR1 = 643,
    ImGuiKey_GamepadR2 = 645,
    ImGuiKey_GamepadR3 = 647,
    ImGuiKey_GamepadRStickDown = 655,
    ImGuiKey_GamepadRStickLeft = 652,
    ImGuiKey_GamepadRStickRight = 653,
    ImGuiKey_GamepadRStickUp = 654,
    ImGuiKey_GamepadStart = 632,
    ImGuiKey_GraveAccent = 606,
    ImGuiKey_H = 553,
    ImGuiKey_Home = 519,
    ImGuiKey_I = 554,
    ImGuiKey_Insert = 521,
    ImGuiKey_J = 555,
    ImGuiKey_K = 556,
    ImGuiKey_Keypad0 = 612,
    ImGuiKey_Keypad1 = 613,
    ImGuiKey_Keypad2 = 614,
    ImGuiKey_Keypad3 = 615,
    ImGuiKey_Keypad4 = 616,
    ImGuiKey_Keypad5 = 617,
    ImGuiKey_Keypad6 = 618,
    ImGuiKey_Keypad7 = 619,
    ImGuiKey_Keypad8 = 620,
    ImGuiKey_Keypad9 = 621,
    ImGuiKey_KeypadAdd = 626,
    ImGuiKey_KeypadDecimal = 622,
    ImGuiKey_KeypadDivide = 623,
    ImGuiKey_KeypadEnter = 627,
    ImGuiKey_KeypadEqual = 628,
    ImGuiKey_KeypadMultiply = 624,
    ImGuiKey_KeypadSubtract = 625,
    ImGuiKey_L = 557,
    ImGuiKey_LeftAlt = 529,
    ImGuiKey_LeftArrow = 513,
    ImGuiKey_LeftBracket = 603,
    ImGuiKey_LeftCtrl = 527,
    ImGuiKey_LeftShift = 528,
    ImGuiKey_LeftSuper = 530,
    ImGuiKey_M = 558,
    ImGuiKey_Menu = 535,
    ImGuiKey_Minus = 598,
    ImGuiKey_MouseLeft = 656,
    ImGuiKey_MouseMiddle = 658,
    ImGuiKey_MouseRight = 657,
    ImGuiKey_MouseWheelX = 661,
    ImGuiKey_MouseWheelY = 662,
    ImGuiKey_MouseX1 = 659,
    ImGuiKey_MouseX2 = 660,
    ImGuiKey_N = 559,
    ImGuiKey_NamedKey_BEGIN = 512,
    ImGuiKey_NamedKey_COUNT = 155,
    ImGuiKey_NamedKey_END = 667,
    ImGuiKey_None = 0,
    ImGuiKey_NumLock = 609,
    ImGuiKey_O = 560,
    ImGuiKey_Oem102 = 631,
    ImGuiKey_P = 561,
    ImGuiKey_PageDown = 518,
    ImGuiKey_PageUp = 517,
    ImGuiKey_Pause = 611,
    ImGuiKey_Period = 599,
    ImGuiKey_PrintScreen = 610,
    ImGuiKey_Q = 562,
    ImGuiKey_R = 563,
    ImGuiKey_ReservedForModAlt = 665,
    ImGuiKey_ReservedForModCtrl = 663,
    ImGuiKey_ReservedForModShift = 664,
    ImGuiKey_ReservedForModSuper = 666,
    ImGuiKey_RightAlt = 533,
    ImGuiKey_RightArrow = 514,
    ImGuiKey_RightBracket = 605,
    ImGuiKey_RightCtrl = 531,
    ImGuiKey_RightShift = 532,
    ImGuiKey_RightSuper = 534,
    ImGuiKey_S = 564,
    ImGuiKey_ScrollLock = 608,
    ImGuiKey_Semicolon = 601,
    ImGuiKey_Slash = 600,
    ImGuiKey_Space = 524,
    ImGuiKey_T = 565,
    ImGuiKey_Tab = 512,
    ImGuiKey_U = 566,
    ImGuiKey_UpArrow = 515,
    ImGuiKey_V = 567,
    ImGuiKey_W = 568,
    ImGuiKey_X = 569,
    ImGuiKey_Y = 570,
    ImGuiKey_Z = 571,
    ImGuiMod_Alt = 16384,
    ImGuiMod_Ctrl = 4096,
    ImGuiMod_Mask_ = 61440,
    ImGuiMod_None = 0,
    ImGuiMod_Shift = 8192,
    ImGuiMod_Shortcut = 4096,
    ImGuiMod_Super = 32768
};

/* ImGuiListClipperFlags_ */
enum {
    ImGuiListClipperFlags_NoSetTableRowCounters = 1,
    ImGuiListClipperFlags_None = 0
};

/* ImGuiMouseButton_ */
enum {
    ImGuiMouseButton_COUNT = 5,
    ImGuiMouseButton_Left = 0,
    ImGuiMouseButton_Middle = 2,
    ImGuiMouseButton_Right = 1
};

/* ImGuiMouseCursor_ */
enum {
    ImGuiMouseCursor_Arrow = 0,
    ImGuiMouseCursor_COUNT = 11,
    ImGuiMouseCursor_Hand = 7,
    ImGuiMouseCursor_None = -1,
    ImGuiMouseCursor_NotAllowed = 10,
    ImGuiMouseCursor_Progress = 9,
    ImGuiMouseCursor_ResizeAll = 2,
    ImGuiMouseCursor_ResizeEW = 4,
    ImGuiMouseCursor_ResizeNESW = 5,
    ImGuiMouseCursor_ResizeNS = 3,
    ImGuiMouseCursor_ResizeNWSE = 6,
    ImGuiMouseCursor_TextInput = 1,
    ImGuiMouseCursor_Wait = 8
};

/* ImGuiMouseSource */
enum {
    ImGuiMouseSource_COUNT = 3,
    ImGuiMouseSource_Mouse = 0,
    ImGuiMouseSource_Pen = 2,
    ImGuiMouseSource_TouchScreen = 1
};

/* ImGuiMultiSelectFlags_ */
enum {
    ImGuiMultiSelectFlags_BoxSelect1d = 64,
    ImGuiMultiSelectFlags_BoxSelect2d = 128,
    ImGuiMultiSelectFlags_BoxSelectNoScroll = 256,
    ImGuiMultiSelectFlags_CheckboxMode_ = 1048576,
    ImGuiMultiSelectFlags_ClearOnClickVoid = 1024,
    ImGuiMultiSelectFlags_ClearOnEscape = 512,
    ImGuiMultiSelectFlags_NavWrapX = 65536,
    ImGuiMultiSelectFlags_NoAutoClear = 16,
    ImGuiMultiSelectFlags_NoAutoClearOnReselect = 32,
    ImGuiMultiSelectFlags_NoAutoSelect = 8,
    ImGuiMultiSelectFlags_NoRangeSelect = 4,
    ImGuiMultiSelectFlags_NoSelectAll = 2,
    ImGuiMultiSelectFlags_NoSelectOnRightClick = 131072,
    ImGuiMultiSelectFlags_None = 0,
    ImGuiMultiSelectFlags_ScopeRect = 4096,
    ImGuiMultiSelectFlags_ScopeWindow = 2048,
    ImGuiMultiSelectFlags_SelectOnAuto = 8192,
    ImGuiMultiSelectFlags_SelectOnClick = 8192,
    ImGuiMultiSelectFlags_SelectOnClickAlways = 16384,
    ImGuiMultiSelectFlags_SelectOnClickRelease = 32768,
    ImGuiMultiSelectFlags_SelectOnMask_ = 57344,
    ImGuiMultiSelectFlags_SingleSelect = 1
};

/* ImGuiPopupFlags_ */
enum {
    ImGuiPopupFlags_AnyPopup = 3072,
    ImGuiPopupFlags_AnyPopupId = 1024,
    ImGuiPopupFlags_AnyPopupLevel = 2048,
    ImGuiPopupFlags_InvalidMask_ = 3,
    ImGuiPopupFlags_MouseButtonLeft = 4,
    ImGuiPopupFlags_MouseButtonMask_ = 12,
    ImGuiPopupFlags_MouseButtonMiddle = 12,
    ImGuiPopupFlags_MouseButtonRight = 8,
    ImGuiPopupFlags_MouseButtonShift_ = 2,
    ImGuiPopupFlags_NoOpenOverExistingPopup = 128,
    ImGuiPopupFlags_NoOpenOverItems = 256,
    ImGuiPopupFlags_NoReopen = 32,
    ImGuiPopupFlags_None = 0
};

/* ImGuiSelectableFlags_ */
enum {
    ImGuiSelectableFlags_AllowDoubleClick = 4,
    ImGuiSelectableFlags_AllowOverlap = 16,
    ImGuiSelectableFlags_Disabled = 8,
    ImGuiSelectableFlags_DontClosePopups = 1,
    ImGuiSelectableFlags_Highlight = 32,
    ImGuiSelectableFlags_NoAutoClosePopups = 1,
    ImGuiSelectableFlags_None = 0,
    ImGuiSelectableFlags_SelectOnNav = 64,
    ImGuiSelectableFlags_SpanAllColumns = 2
};

/* ImGuiSelectionRequestType */
enum {
    ImGuiSelectionRequestType_None = 0,
    ImGuiSelectionRequestType_SetAll = 1,
    ImGuiSelectionRequestType_SetRange = 2
};

/* ImGuiSliderFlags_ */
enum {
    ImGuiSliderFlags_AlwaysClamp = 1536,
    ImGuiSliderFlags_ClampOnInput = 512,
    ImGuiSliderFlags_ClampZeroRange = 1024,
    ImGuiSliderFlags_ColorMarkers = 4096,
    ImGuiSliderFlags_InvalidMask_ = 1879048207,
    ImGuiSliderFlags_Logarithmic = 32,
    ImGuiSliderFlags_NoInput = 128,
    ImGuiSliderFlags_NoRoundToFormat = 64,
    ImGuiSliderFlags_NoSpeedTweaks = 2048,
    ImGuiSliderFlags_None = 0,
    ImGuiSliderFlags_WrapAround = 256
};

/* ImGuiSortDirection */
enum {
    ImGuiSortDirection_Ascending = 1,
    ImGuiSortDirection_Descending = 2,
    ImGuiSortDirection_None = 0
};

/* ImGuiStyleVar_ */
enum {
    ImGuiStyleVar_Alpha = 0,
    ImGuiStyleVar_ButtonTextAlign = 38,
    ImGuiStyleVar_COUNT = 44,
    ImGuiStyleVar_CellPadding = 17,
    ImGuiStyleVar_ChildBorderSize = 8,
    ImGuiStyleVar_ChildRounding = 7,
    ImGuiStyleVar_DisabledAlpha = 1,
    ImGuiStyleVar_DragDropTargetRounding = 37,
    ImGuiStyleVar_FrameBorderSize = 13,
    ImGuiStyleVar_FramePadding = 11,
    ImGuiStyleVar_FrameRounding = 12,
    ImGuiStyleVar_GrabMinSize = 21,
    ImGuiStyleVar_GrabRounding = 22,
    ImGuiStyleVar_ImageBorderSize = 24,
    ImGuiStyleVar_ImageRounding = 23,
    ImGuiStyleVar_IndentSpacing = 16,
    ImGuiStyleVar_ItemInnerSpacing = 15,
    ImGuiStyleVar_ItemSpacing = 14,
    ImGuiStyleVar_MenuItemRounding = 35,
    ImGuiStyleVar_PopupBorderSize = 10,
    ImGuiStyleVar_PopupRounding = 9,
    ImGuiStyleVar_ScrollbarPadding = 20,
    ImGuiStyleVar_ScrollbarRounding = 19,
    ImGuiStyleVar_ScrollbarSize = 18,
    ImGuiStyleVar_SelectableRounding = 36,
    ImGuiStyleVar_SelectableTextAlign = 39,
    ImGuiStyleVar_SeparatorSize = 40,
    ImGuiStyleVar_SeparatorTextAlign = 42,
    ImGuiStyleVar_SeparatorTextBorderSize = 41,
    ImGuiStyleVar_SeparatorTextPadding = 43,
    ImGuiStyleVar_TabBarBorderSize = 29,
    ImGuiStyleVar_TabBarOverlineSize = 30,
    ImGuiStyleVar_TabBorderSize = 26,
    ImGuiStyleVar_TabMinWidthBase = 27,
    ImGuiStyleVar_TabMinWidthShrink = 28,
    ImGuiStyleVar_TabRounding = 25,
    ImGuiStyleVar_TableAngledHeadersAngle = 31,
    ImGuiStyleVar_TableAngledHeadersTextAlign = 32,
    ImGuiStyleVar_TreeLinesRounding = 34,
    ImGuiStyleVar_TreeLinesSize = 33,
    ImGuiStyleVar_WindowBorderSize = 4,
    ImGuiStyleVar_WindowMinSize = 5,
    ImGuiStyleVar_WindowPadding = 2,
    ImGuiStyleVar_WindowRounding = 3,
    ImGuiStyleVar_WindowTitleAlign = 6
};

/* ImGuiTabBarFlags_ */
enum {
    ImGuiTabBarFlags_AutoSelectNewTabs = 2,
    ImGuiTabBarFlags_DrawSelectedOverline = 64,
    ImGuiTabBarFlags_FittingPolicyDefault_ = 128,
    ImGuiTabBarFlags_FittingPolicyMask_ = 896,
    ImGuiTabBarFlags_FittingPolicyMixed = 128,
    ImGuiTabBarFlags_FittingPolicyResizeDown = 256,
    ImGuiTabBarFlags_FittingPolicyScroll = 512,
    ImGuiTabBarFlags_FittingPolicyShrink = 256,
    ImGuiTabBarFlags_NoCloseWithMiddleMouseButton = 8,
    ImGuiTabBarFlags_NoTabListScrollingButtons = 16,
    ImGuiTabBarFlags_NoTooltip = 32,
    ImGuiTabBarFlags_None = 0,
    ImGuiTabBarFlags_Reorderable = 1,
    ImGuiTabBarFlags_TabListPopupButton = 4
};

/* ImGuiTabItemFlags_ */
enum {
    ImGuiTabItemFlags_Leading = 64,
    ImGuiTabItemFlags_NoAssumedClosure = 256,
    ImGuiTabItemFlags_NoCloseWithMiddleMouseButton = 4,
    ImGuiTabItemFlags_NoPushId = 8,
    ImGuiTabItemFlags_NoReorder = 32,
    ImGuiTabItemFlags_NoTooltip = 16,
    ImGuiTabItemFlags_None = 0,
    ImGuiTabItemFlags_SetSelected = 2,
    ImGuiTabItemFlags_Trailing = 128,
    ImGuiTabItemFlags_UnsavedDocument = 1
};

/* ImGuiTableBgTarget_ */
enum {
    ImGuiTableBgTarget_CellBg = 3,
    ImGuiTableBgTarget_None = 0,
    ImGuiTableBgTarget_RowBg0 = 1,
    ImGuiTableBgTarget_RowBg1 = 2
};

/* ImGuiTableColumnFlags_ */
enum {
    ImGuiTableColumnFlags_AngledHeader = 262144,
    ImGuiTableColumnFlags_DefaultHide = 2,
    ImGuiTableColumnFlags_DefaultSort = 4,
    ImGuiTableColumnFlags_Disabled = 1,
    ImGuiTableColumnFlags_IndentDisable = 131072,
    ImGuiTableColumnFlags_IndentEnable = 65536,
    ImGuiTableColumnFlags_IndentMask_ = 196608,
    ImGuiTableColumnFlags_IsEnabled = 16777216,
    ImGuiTableColumnFlags_IsHovered = 134217728,
    ImGuiTableColumnFlags_IsSorted = 67108864,
    ImGuiTableColumnFlags_IsVisible = 33554432,
    ImGuiTableColumnFlags_NoClip = 256,
    ImGuiTableColumnFlags_NoDirectResize_ = 1073741824,
    ImGuiTableColumnFlags_NoHeaderLabel = 4096,
    ImGuiTableColumnFlags_NoHeaderWidth = 8192,
    ImGuiTableColumnFlags_NoHide = 128,
    ImGuiTableColumnFlags_NoReorder = 64,
    ImGuiTableColumnFlags_NoResize = 32,
    ImGuiTableColumnFlags_NoSort = 512,
    ImGuiTableColumnFlags_NoSortAscending = 1024,
    ImGuiTableColumnFlags_NoSortDescending = 2048,
    ImGuiTableColumnFlags_None = 0,
    ImGuiTableColumnFlags_PreferSortAscending = 16384,
    ImGuiTableColumnFlags_PreferSortDescending = 32768,
    ImGuiTableColumnFlags_StatusMask_ = 251658240,
    ImGuiTableColumnFlags_WidthFixed = 16,
    ImGuiTableColumnFlags_WidthMask_ = 24,
    ImGuiTableColumnFlags_WidthStretch = 8
};

/* ImGuiTableFlags_ */
enum {
    ImGuiTableFlags_Borders = 1920,
    ImGuiTableFlags_BordersH = 384,
    ImGuiTableFlags_BordersInner = 640,
    ImGuiTableFlags_BordersInnerH = 128,
    ImGuiTableFlags_BordersInnerV = 512,
    ImGuiTableFlags_BordersOuter = 1280,
    ImGuiTableFlags_BordersOuterH = 256,
    ImGuiTableFlags_BordersOuterV = 1024,
    ImGuiTableFlags_BordersV = 1536,
    ImGuiTableFlags_ContextMenuInBody = 32,
    ImGuiTableFlags_Hideable = 4,
    ImGuiTableFlags_HighlightHoveredColumn = 268435456,
    ImGuiTableFlags_NoBordersInBody = 2048,
    ImGuiTableFlags_NoBordersInBodyUntilResize = 4096,
    ImGuiTableFlags_NoClip = 1048576,
    ImGuiTableFlags_NoHostExtendX = 65536,
    ImGuiTableFlags_NoHostExtendY = 131072,
    ImGuiTableFlags_NoKeepColumnsVisible = 262144,
    ImGuiTableFlags_NoPadInnerX = 8388608,
    ImGuiTableFlags_NoPadOuterX = 4194304,
    ImGuiTableFlags_NoSavedSettings = 16,
    ImGuiTableFlags_None = 0,
    ImGuiTableFlags_PadOuterX = 2097152,
    ImGuiTableFlags_PreciseWidths = 524288,
    ImGuiTableFlags_Reorderable = 2,
    ImGuiTableFlags_Resizable = 1,
    ImGuiTableFlags_RowBg = 64,
    ImGuiTableFlags_ScrollX = 16777216,
    ImGuiTableFlags_ScrollY = 33554432,
    ImGuiTableFlags_SizingFixedFit = 8192,
    ImGuiTableFlags_SizingFixedSame = 16384,
    ImGuiTableFlags_SizingMask_ = 57344,
    ImGuiTableFlags_SizingStretchProp = 24576,
    ImGuiTableFlags_SizingStretchSame = 32768,
    ImGuiTableFlags_SortMulti = 67108864,
    ImGuiTableFlags_SortTristate = 134217728,
    ImGuiTableFlags_Sortable = 8
};

/* ImGuiTableRowFlags_ */
enum {
    ImGuiTableRowFlags_Headers = 1,
    ImGuiTableRowFlags_None = 0
};

/* ImGuiTreeNodeFlags_ */
enum {
    ImGuiTreeNodeFlags_AllowOverlap = 4,
    ImGuiTreeNodeFlags_Bullet = 512,
    ImGuiTreeNodeFlags_CollapsingHeader = 26,
    ImGuiTreeNodeFlags_DefaultOpen = 32,
    ImGuiTreeNodeFlags_DrawLinesFull = 524288,
    ImGuiTreeNodeFlags_DrawLinesNone = 262144,
    ImGuiTreeNodeFlags_DrawLinesToNodes = 1048576,
    ImGuiTreeNodeFlags_FramePadding = 1024,
    ImGuiTreeNodeFlags_Framed = 2,
    ImGuiTreeNodeFlags_LabelSpanAllColumns = 32768,
    ImGuiTreeNodeFlags_Leaf = 256,
    ImGuiTreeNodeFlags_NavLeftJumpsBackHere = 131072,
    ImGuiTreeNodeFlags_NavLeftJumpsToParent = 131072,
    ImGuiTreeNodeFlags_NoAutoOpenOnLog = 16,
    ImGuiTreeNodeFlags_NoTreePushOnOpen = 8,
    ImGuiTreeNodeFlags_None = 0,
    ImGuiTreeNodeFlags_OpenOnArrow = 128,
    ImGuiTreeNodeFlags_OpenOnDoubleClick = 64,
    ImGuiTreeNodeFlags_Selected = 1,
    ImGuiTreeNodeFlags_SpanAllColumns = 16384,
    ImGuiTreeNodeFlags_SpanAvailWidth = 2048,
    ImGuiTreeNodeFlags_SpanFullWidth = 4096,
    ImGuiTreeNodeFlags_SpanLabelWidth = 8192
};

/* ImGuiViewportFlags_ */
enum {
    ImGuiViewportFlags_IsPlatformMonitor = 2,
    ImGuiViewportFlags_IsPlatformWindow = 1,
    ImGuiViewportFlags_None = 0,
    ImGuiViewportFlags_OwnedByApp = 4
};

/* ImGuiWindowFlags_ */
enum {
    ImGuiWindowFlags_AlwaysAutoResize = 64,
    ImGuiWindowFlags_AlwaysHorizontalScrollbar = 32768,
    ImGuiWindowFlags_AlwaysVerticalScrollbar = 16384,
    ImGuiWindowFlags_ChildMenu = 268435456,
    ImGuiWindowFlags_ChildWindow = 16777216,
    ImGuiWindowFlags_HorizontalScrollbar = 2048,
    ImGuiWindowFlags_MenuBar = 1024,
    ImGuiWindowFlags_Modal = 134217728,
    ImGuiWindowFlags_NoBackground = 128,
    ImGuiWindowFlags_NoBringToFrontOnFocus = 8192,
    ImGuiWindowFlags_NoCollapse = 32,
    ImGuiWindowFlags_NoDecoration = 43,
    ImGuiWindowFlags_NoFocusOnAppearing = 4096,
    ImGuiWindowFlags_NoInputs = 197120,
    ImGuiWindowFlags_NoMouseInputs = 512,
    ImGuiWindowFlags_NoMove = 4,
    ImGuiWindowFlags_NoNav = 196608,
    ImGuiWindowFlags_NoNavFocus = 131072,
    ImGuiWindowFlags_NoNavInputs = 65536,
    ImGuiWindowFlags_NoResize = 2,
    ImGuiWindowFlags_NoSavedSettings = 256,
    ImGuiWindowFlags_NoScrollWithMouse = 16,
    ImGuiWindowFlags_NoScrollbar = 8,
    ImGuiWindowFlags_NoTitleBar = 1,
    ImGuiWindowFlags_None = 0,
    ImGuiWindowFlags_Popup = 67108864,
    ImGuiWindowFlags_Tooltip = 33554432,
    ImGuiWindowFlags_UnsavedDocument = 262144
};

/* ImTextureFormat */
enum {
    ImTextureFormat_Alpha8 = 1,
    ImTextureFormat_RGBA32 = 0
};

/* ImTextureStatus */
enum {
    ImTextureStatus_Destroyed = 1,
    ImTextureStatus_OK = 0,
    ImTextureStatus_WantCreate = 2,
    ImTextureStatus_WantDestroy = 4,
    ImTextureStatus_WantUpdates = 3
};

struct ImFontGlyph {
    IMGUI_C89_EXTENSION unsigned int Colored : 1;
    IMGUI_C89_EXTENSION unsigned int Visible : 1;
    IMGUI_C89_EXTENSION unsigned int SourceIdx : 4;
    IMGUI_C89_EXTENSION unsigned int Codepoint : 26;
    float AdvanceX;
    float X0;
    float Y0;
    float X1;
    float Y1;
    float U0;
    float V0;
    float U1;
    float V1;
    int PackId;
};

struct ImGuiInputTextCallbackData {
    ImGuiContext * Ctx;
    ImGuiInputTextFlags EventFlag;
    ImGuiInputTextFlags Flags;
    void * UserData;
    ImGuiID ID;
    ImGuiKey EventKey;
    ImWchar EventChar;
    unsigned char EventActivated;
    unsigned char BufDirty;
    char * Buf;
    int BufTextLen;
    int BufSize;
    int CursorPos;
    int SelectionStart;
    int SelectionEnd;
};

struct ImGuiKeyData {
    unsigned char Down;
    float DownDuration;
    float DownDurationPrev;
    float AnalogValue;
};

struct ImGuiListClipper {
    int DisplayStart;
    int DisplayEnd;
    int UserIndex;
    int ItemsCount;
    float ItemsHeight;
    ImGuiListClipperFlags Flags;
    double StartPosY;
    double StartSeekOffsetY;
    ImGuiContext * Ctx;
    void * TempData;
};

struct ImGuiOnceUponAFrame {
    int RefFrame;
};

struct ImGuiPayload {
    void * Data;
    int DataSize;
    ImGuiID SourceId;
    ImGuiID SourceParentId;
    int DataFrameCount;
    char DataType[33];
    unsigned char Preview;
    unsigned char Delivery;
};

struct ImGuiSelectionExternalStorage {
    void * UserData;
    void (*AdapterSetItemSelected)(ImGuiSelectionExternalStorage *, int, unsigned char);
};

struct ImGuiSelectionRequest {
    ImGuiSelectionRequestType Type;
    unsigned char Selected;
    ImS8 RangeDirection;
    ImGuiSelectionUserData RangeFirstItem;
    ImGuiSelectionUserData RangeLastItem;
};

union imgui_c89_anon_imgui_2816_5 {
    int val_i;
    float val_f;
    void * val_p;
};

struct ImGuiStoragePair {
    ImGuiID key;
    imgui_c89_anon_imgui_2816_5 imgui_c89_unnamed_ae97675b;
};

struct ImGuiTableColumnSortSpecs {
    ImGuiID ColumnUserID;
    ImS16 ColumnIndex;
    ImS16 SortOrder;
    ImGuiSortDirection SortDirection;
};

struct ImGuiTableSortSpecs {
    const ImGuiTableColumnSortSpecs * Specs;
    int SpecsCount;
    unsigned char SpecsDirty;
};

struct ImGuiTextFilter_ImGuiTextRange {
    const char * b;
    const char * e;
};

struct ImNewWrapper {
    unsigned char imgui_c89_empty;
};

struct ImTextureRect {
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
};

struct ImTextureRef {
    ImTextureData * _TexData;
    ImTextureID _TexID;
};

union imgui_c89_anon_imgui_3836_5 {
    ImTextureRef TexRef;
    ImTextureRef TexID;
};

struct ImVec2 {
    float x;
    float y;
};

struct ImDrawVert {
    ImVec2 pos;
    ImVec2 uv;
    ImU32 col;
};

struct ImFontAtlasRect {
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
    ImVec2 uv0;
    ImVec2 uv1;
};

struct ImFontConfig {
    char Name[40];
    void * FontData;
    int FontDataSize;
    unsigned char FontDataOwnedByAtlas;
    unsigned char MergeMode;
    unsigned char PixelSnapH;
    ImS8 OversampleH;
    ImS8 OversampleV;
    ImWchar EllipsisChar;
    float SizePixels;
    const ImWchar * GlyphRanges;
    const ImWchar * GlyphExcludeRanges;
    ImVec2 GlyphOffset;
    float GlyphMinAdvanceX;
    float GlyphMaxAdvanceX;
    float GlyphExtraAdvanceX;
    ImU32 FontNo;
    unsigned int FontLoaderFlags;
    float RasterizerMultiply;
    float RasterizerDensity;
    float ExtraSizeScale;
    ImFontFlags Flags;
    ImFont * DstFont;
    const ImFontLoader * FontLoader;
    void * FontLoaderData;
    unsigned char PixelSnapV;
};

struct ImGuiPlatformImeData {
    unsigned char WantVisible;
    unsigned char WantTextInput;
    ImVec2 InputPos;
    float InputLineHeight;
    ImGuiID ViewportId;
};

struct ImGuiSizeCallbackData {
    void * UserData;
    ImVec2 Pos;
    ImVec2 CurrentSize;
    ImVec2 DesiredSize;
};

struct ImGuiViewport {
    ImGuiID ID;
    ImGuiViewportFlags Flags;
    ImVec2 Pos;
    ImVec2 Size;
    ImVec2 FramebufferScale;
    ImVec2 WorkPos;
    ImVec2 WorkSize;
    void * PlatformHandle;
    void * PlatformHandleRaw;
};

struct ImVec4 {
    float x;
    float y;
    float z;
    float w;
};

struct ImColor {
    ImVec4 Value;
};

struct ImDrawCmd {
    ImVec4 ClipRect;
    ImTextureRef TexRef;
    unsigned int VtxOffset;
    unsigned int IdxOffset;
    unsigned int ElemCount;
    ImDrawCallback UserCallback;
    void * UserCallbackData;
    int UserCallbackDataSize;
    int UserCallbackDataOffset;
};

struct ImDrawCmdHeader {
    ImVec4 ClipRect;
    ImTextureRef TexRef;
    unsigned int VtxOffset;
};

struct ImGuiStyle {
    float FontSizeBase;
    float FontScaleMain;
    float FontScaleDpi;
    float Alpha;
    float DisabledAlpha;
    ImVec2 WindowPadding;
    float WindowRounding;
    float WindowBorderSize;
    float WindowBorderHoverPadding;
    ImVec2 WindowMinSize;
    ImVec2 WindowTitleAlign;
    ImGuiDir WindowMenuButtonPosition;
    float ChildRounding;
    float ChildBorderSize;
    float PopupRounding;
    float PopupBorderSize;
    ImVec2 FramePadding;
    float FrameRounding;
    float FrameBorderSize;
    ImVec2 ItemSpacing;
    ImVec2 ItemInnerSpacing;
    ImVec2 CellPadding;
    ImVec2 TouchExtraPadding;
    float IndentSpacing;
    float ColumnsMinSpacing;
    float ScrollbarSize;
    float ScrollbarRounding;
    float ScrollbarPadding;
    float GrabMinSize;
    float GrabRounding;
    float LogSliderDeadzone;
    float ImageRounding;
    float ImageBorderSize;
    float TabRounding;
    float TabBorderSize;
    float TabMinWidthBase;
    float TabMinWidthShrink;
    float TabCloseButtonMinWidthSelected;
    float TabCloseButtonMinWidthUnselected;
    float TabBarBorderSize;
    float TabBarOverlineSize;
    float TableAngledHeadersAngle;
    ImVec2 TableAngledHeadersTextAlign;
    ImGuiTreeNodeFlags TreeLinesFlags;
    float TreeLinesSize;
    float TreeLinesRounding;
    float MenuItemRounding;
    float SelectableRounding;
    float DragDropTargetRounding;
    float DragDropTargetBorderSize;
    float DragDropTargetPadding;
    float ColorMarkerSize;
    ImGuiDir ColorButtonPosition;
    ImVec2 ButtonTextAlign;
    ImVec2 SelectableTextAlign;
    float InputTextCursorSize;
    float SeparatorSize;
    float SeparatorTextBorderSize;
    ImVec2 SeparatorTextAlign;
    ImVec2 SeparatorTextPadding;
    ImVec2 DisplayWindowPadding;
    ImVec2 DisplaySafeAreaPadding;
    float MouseCursorScale;
    unsigned char AntiAliasedLines;
    unsigned char AntiAliasedLinesUseTex;
    unsigned char AntiAliasedFill;
    float CurveTessellationTol;
    float CircleTessellationMaxError;
    ImVec4 Colors[61];
    float HoverStationaryDelay;
    float HoverDelayShort;
    float HoverDelayNormal;
    ImGuiHoveredFlags HoverFlagsForTooltipMouse;
    ImGuiHoveredFlags HoverFlagsForTooltipNav;
    float _MainScale;
    float _NextFrameFontSizeBase;
};

struct ImVector_ImDrawChannel {
    int Size;
    int Capacity;
    ImDrawChannel * Data;
};

struct ImDrawListSplitter {
    int _Current;
    int _Count;
    ImVector_ImDrawChannel _Channels;
};

struct ImVector_ImDrawCmd {
    int Size;
    int Capacity;
    ImDrawCmd * Data;
};

struct ImVector_ImDrawList_ptr {
    int Size;
    int Capacity;
    ImDrawList ** Data;
};

struct ImDrawData {
    unsigned char Valid;
    int FrameCount;
    int TotalIdxCount;
    int TotalVtxCount;
    ImVector_ImDrawList_ptr CmdLists;
    ImVec2 DisplayPos;
    ImVec2 DisplaySize;
    ImVec2 FramebufferScale;
    ImGuiViewport * OwnerViewport;
    ImVector_ImTextureData_ptr * Textures;
    int CmdListsCount;
};

struct ImVector_ImDrawListSharedData_ptr {
    int Size;
    int Capacity;
    ImDrawListSharedData ** Data;
};

struct ImVector_ImDrawVert {
    int Size;
    int Capacity;
    ImDrawVert * Data;
};

struct ImVector_ImFont_ptr {
    int Size;
    int Capacity;
    ImFont ** Data;
};

struct ImVector_ImFontConfig_ptr {
    int Size;
    int Capacity;
    ImFontConfig ** Data;
};

struct ImVector_ImFontConfig {
    int Size;
    int Capacity;
    ImFontConfig * Data;
};

struct ImVector_ImFontGlyph {
    int Size;
    int Capacity;
    ImFontGlyph * Data;
};

struct ImVector_ImGuiSelectionRequest {
    int Size;
    int Capacity;
    ImGuiSelectionRequest * Data;
};

struct ImGuiMultiSelectIO {
    ImVector_ImGuiSelectionRequest Requests;
    ImGuiSelectionUserData RangeSrcItem;
    ImGuiSelectionUserData NavIdItem;
    unsigned char NavIdSelected;
    unsigned char RangeSrcReset;
    int ItemsCount;
};

struct ImVector_ImGuiStoragePair {
    int Size;
    int Capacity;
    ImGuiStoragePair * Data;
};

struct ImGuiStorage {
    ImVector_ImGuiStoragePair Data;
};

struct ImFont {
    ImFontBaked * LastBaked;
    ImFontAtlas * OwnerAtlas;
    ImFontFlags Flags;
    float CurrentRasterizerDensity;
    ImGuiID FontId;
    float LegacySize;
    ImVector_ImFontConfig_ptr Sources;
    ImWchar EllipsisChar;
    ImWchar FallbackChar;
    ImU8 Used8kPagesMap[1];
    unsigned char EllipsisAutoBake;
    ImGuiStorage RemapPairs;
    float Scale;
};

struct ImGuiSelectionBasicStorage {
    int Size;
    unsigned char PreserveOrder;
    void * UserData;
    ImGuiID (*AdapterIndexToStorageId)(ImGuiSelectionBasicStorage *, int);
    int _SelectionOrder;
    ImGuiStorage _Storage;
};

struct ImVector_ImGuiTextFilter_ImGuiTextRange {
    int Size;
    int Capacity;
    ImGuiTextFilter_ImGuiTextRange * Data;
};

struct ImGuiTextFilter {
    char InputBuf[256];
    ImVector_ImGuiTextFilter_ImGuiTextRange Filters;
    int CountGrep;
};

struct ImVector_ImTextureData_ptr {
    int Size;
    int Capacity;
    ImTextureData ** Data;
};

struct ImFontAtlas {
    ImFontAtlasFlags Flags;
    ImTextureFormat TexDesiredFormat;
    int TexGlyphPadding;
    int TexMinWidth;
    int TexMinHeight;
    int TexMaxWidth;
    int TexMaxHeight;
    void * UserData;
    imgui_c89_anon_imgui_3836_5 imgui_c89_unnamed_50ed8dbe;
    ImTextureData * TexData;
    ImVector_ImTextureData_ptr TexList;
    unsigned char Locked;
    unsigned char RendererHasTextures;
    unsigned char TexIsBuilt;
    unsigned char TexPixelsUseColors;
    ImVec2 TexUvScale;
    ImVec2 TexUvWhitePixel;
    ImVector_ImFont_ptr Fonts;
    ImVector_ImFontConfig Sources;
    ImVec4 TexUvLines[33];
    int TexNextUniqueID;
    int FontNextUniqueID;
    ImVector_ImDrawListSharedData_ptr DrawListSharedDatas;
    ImFontAtlasBuilder * Builder;
    const ImFontLoader * FontLoader;
    const char * FontLoaderName;
    void * FontLoaderData;
    unsigned int FontLoaderFlags;
    int RefCount;
    ImGuiContext * OwnerContext;
    ImFontAtlasRect TempRect;
};

struct ImGuiPlatformIO {
    const char *(*Platform_GetClipboardTextFn)(ImGuiContext *);
    void (*Platform_SetClipboardTextFn)(ImGuiContext *, const char *);
    void * Platform_ClipboardUserData;
    unsigned char (*Platform_OpenInShellFn)(ImGuiContext *, const char *);
    void * Platform_OpenInShellUserData;
    void (*Platform_SetImeDataFn)(ImGuiContext *, ImGuiViewport *, ImGuiPlatformImeData *);
    void * Platform_ImeUserData;
    ImWchar Platform_LocaleDecimalPoint;
    int Platform_SessionDate;
    int Renderer_TextureMaxWidth;
    int Renderer_TextureMaxHeight;
    void * Renderer_RenderState;
    ImDrawCallback DrawCallback_ResetRenderState;
    ImDrawCallback DrawCallback_SetSamplerLinear;
    ImDrawCallback DrawCallback_SetSamplerNearest;
    ImVector_ImTextureData_ptr Textures;
};

struct ImVector_ImTextureRect {
    int Size;
    int Capacity;
    ImTextureRect * Data;
};

struct ImTextureData {
    int UniqueID;
    ImTextureStatus Status;
    void * BackendUserData;
    void * QueueUserData;
    ImTextureID TexID;
    ImTextureFormat Format;
    int Width;
    int Height;
    int BytesPerPixel;
    unsigned char * Pixels;
    ImTextureRect UsedRect;
    ImTextureRect UpdateRect;
    ImVector_ImTextureRect Updates;
    int UnusedFrames;
    unsigned short RefCount;
    unsigned char UseColors;
    unsigned char WantDestroyNextFrame;
};

struct ImVector_ImTextureRef {
    int Size;
    int Capacity;
    ImTextureRef * Data;
};

struct ImVector_ImVec2 {
    int Size;
    int Capacity;
    ImVec2 * Data;
};

struct ImVector_ImVec4 {
    int Size;
    int Capacity;
    ImVec4 * Data;
};

struct ImVector_char {
    int Size;
    int Capacity;
    char * Data;
};

struct ImGuiTextBuffer {
    ImVector_char Buf;
};

struct ImVector_float {
    int Size;
    int Capacity;
    float * Data;
};

struct ImVector_unsigned_char {
    int Size;
    int Capacity;
    unsigned char * Data;
};

struct ImVector_unsigned_int {
    int Size;
    int Capacity;
    unsigned int * Data;
};

struct ImFontGlyphRangesBuilder {
    ImVector_unsigned_int UsedChars;
};

struct ImVector_unsigned_short {
    int Size;
    int Capacity;
    unsigned short * Data;
};

struct ImDrawChannel {
    ImVector_ImDrawCmd _CmdBuffer;
    ImVector_unsigned_short _IdxBuffer;
};

struct ImDrawList {
    ImVector_ImDrawCmd CmdBuffer;
    ImVector_unsigned_short IdxBuffer;
    ImVector_ImDrawVert VtxBuffer;
    ImDrawListFlags Flags;
    unsigned int _VtxCurrentIdx;
    ImDrawListSharedData * _Data;
    ImDrawVert * _VtxWritePtr;
    ImDrawIdx * _IdxWritePtr;
    ImVector_ImVec2 _Path;
    ImDrawCmdHeader _CmdHeader;
    ImDrawListSplitter _Splitter;
    ImVector_ImVec4 _ClipRectStack;
    ImVector_ImTextureRef _TextureStack;
    ImVector_unsigned_char _CallbacksDataBuf;
    float _FringeScale;
    const char * _OwnerName;
};

struct ImFontBaked {
    ImVector_float IndexAdvanceX;
    float FallbackAdvanceX;
    float Size;
    float RasterizerDensity;
    ImVector_unsigned_short IndexLookup;
    ImVector_ImFontGlyph Glyphs;
    int FallbackGlyphIndex;
    float Ascent;
    float Descent;
    IMGUI_C89_EXTENSION unsigned int MetricsTotalSurface : 26;
    IMGUI_C89_EXTENSION unsigned int WantDestroy : 1;
    IMGUI_C89_EXTENSION unsigned int LoadNoFallback : 1;
    IMGUI_C89_EXTENSION unsigned int LoadNoRenderOnLayout : 1;
    int LastUsedFrame;
    ImGuiID BakedId;
    ImFont * OwnerFont;
    void * FontLoaderDatas;
};

struct ImGuiIO {
    ImGuiConfigFlags ConfigFlags;
    ImGuiBackendFlags BackendFlags;
    ImVec2 DisplaySize;
    ImVec2 DisplayFramebufferScale;
    float DeltaTime;
    float IniSavingRate;
    const char * IniFilename;
    const char * LogFilename;
    void * UserData;
    ImFontAtlas * Fonts;
    ImFont * FontDefault;
    unsigned char FontAllowUserScaling;
    unsigned char ConfigNavSwapGamepadButtons;
    unsigned char ConfigNavMoveSetMousePos;
    unsigned char ConfigNavCaptureKeyboard;
    unsigned char ConfigNavEscapeClearFocusItem;
    unsigned char ConfigNavEscapeClearFocusWindow;
    unsigned char ConfigNavCursorVisibleAuto;
    unsigned char ConfigNavCursorVisibleAlways;
    unsigned char ConfigMacOSXBehaviors;
    unsigned char ConfigInputTrickleEventQueue;
    unsigned char ConfigInputTextCursorBlink;
    unsigned char ConfigInputTextEnterKeepActive;
    ImGuiColorEditFlags ConfigColorEditFlags;
    unsigned char ConfigDragClickToInputText;
    unsigned char ConfigWindowsResizeFromEdges;
    unsigned char ConfigWindowsMoveFromTitleBarOnly;
    unsigned char ConfigWindowsCopyContentsWithCtrlC;
    unsigned char ConfigScrollbarScrollByPage;
    unsigned char ConfigIniSettingsSaveLastUsedDate;
    int ConfigIniSettingsAutoDiscardMonths;
    unsigned char ConfigDebugIniSettings;
    unsigned char MouseDrawCursor;
    float ConfigMemoryCompactTimer;
    float MouseDoubleClickTime;
    float MouseDoubleClickMaxDist;
    float MouseSingleClickDelay;
    float MouseDragThreshold;
    float KeyRepeatDelay;
    float KeyRepeatRate;
    unsigned char ConfigErrorRecovery;
    unsigned char ConfigErrorRecoveryEnableAssert;
    unsigned char ConfigErrorRecoveryEnableDebugLog;
    unsigned char ConfigErrorRecoveryEnableTooltip;
    unsigned char ConfigDebugIsDebuggerPresent;
    unsigned char ConfigDebugHighlightIdConflicts;
    unsigned char ConfigDebugHighlightIdConflictsShowItemPicker;
    unsigned char ConfigDebugBeginReturnValueOnce;
    unsigned char ConfigDebugBeginReturnValueLoop;
    unsigned char ConfigDebugIgnoreFocusLoss;
    const char * BackendPlatformName;
    const char * BackendRendererName;
    void * BackendPlatformUserData;
    void * BackendRendererUserData;
    void * BackendLanguageUserData;
    unsigned char WantCaptureMouse;
    unsigned char WantCaptureKeyboard;
    unsigned char WantTextInput;
    unsigned char WantSetMousePos;
    unsigned char WantSaveIniSettings;
    unsigned char NavActive;
    unsigned char NavVisible;
    float Framerate;
    int MetricsRenderVertices;
    int MetricsRenderIndices;
    int MetricsRenderWindows;
    int MetricsActiveWindows;
    ImVec2 MouseDelta;
    ImGuiContext * Ctx;
    ImVec2 MousePos;
    unsigned char MouseDown[5];
    float MouseWheel;
    float MouseWheelH;
    ImGuiMouseSource MouseSource;
    unsigned char KeyCtrl;
    unsigned char KeyShift;
    unsigned char KeyAlt;
    unsigned char KeySuper;
    ImGuiKeyChord KeyMods;
    ImGuiKeyData KeysData[155];
    unsigned char WantCaptureMouseUnlessPopupClose;
    ImVec2 MousePosPrev;
    ImVec2 MouseClickedPos[5];
    double MouseClickedTime[5];
    unsigned char MouseClicked[5];
    unsigned char MouseDoubleClicked[5];
    ImU16 MouseClickedCount[5];
    ImU16 MouseClickedLastCount[5];
    unsigned char MouseReleased[5];
    double MouseReleasedTime[5];
    unsigned char MouseDownOwned[5];
    unsigned char MouseDownOwnedUnlessPopupClose[5];
    unsigned char MouseWheelRequestAxisSwap;
    unsigned char MouseCtrlLeftAsRightClick;
    float MouseDownDuration[5];
    float MouseDownDurationPrev[5];
    float MouseDragMaxDistanceSqr[5];
    float PenPressure;
    unsigned char AppFocusLost;
    unsigned char AppAcceptingEvents;
    ImWchar16 InputQueueSurrogate;
    ImVector_unsigned_short InputQueueCharacters;
    float FontGlobalScale;
    const char *(*GetClipboardTextFn)(void *);
    void (*SetClipboardTextFn)(void *, const char *);
    void * ClipboardUserData;
};

const ImGuiPayload * imgui_accept_drag_drop_payload(ImGuiContext *imgui_c89_ctx, const char * type, ImGuiDragDropFlags flags);
void imgui_align_text_to_frame_padding(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_arrow_button(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiDir dir);
unsigned char imgui_begin(ImGuiContext *imgui_c89_ctx, const char * name, unsigned char * p_open, ImGuiWindowFlags flags);
unsigned char imgui_begin_child_id(ImGuiContext *imgui_c89_ctx, ImGuiID id, const ImVec2 * size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags);
unsigned char imgui_begin_child_string(ImGuiContext *imgui_c89_ctx, const char * str_id, const ImVec2 * size_arg, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags);
unsigned char imgui_begin_combo(ImGuiContext *imgui_c89_ctx, const char * label, const char * preview_value, ImGuiComboFlags flags);
void imgui_begin_disabled(ImGuiContext *imgui_c89_ctx, unsigned char disabled);
unsigned char imgui_begin_drag_drop_source(ImGuiContext *imgui_c89_ctx, ImGuiDragDropFlags flags);
unsigned char imgui_begin_drag_drop_target(ImGuiContext *imgui_c89_ctx);
void imgui_begin_group(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_begin_item_tooltip(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_begin_list_box(ImGuiContext *imgui_c89_ctx, const char * label, const ImVec2 * size_arg);
unsigned char imgui_begin_main_menu_bar(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_begin_menu(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char enabled);
unsigned char imgui_begin_menu_bar(ImGuiContext *imgui_c89_ctx);
ImGuiMultiSelectIO * imgui_begin_multi_select(ImGuiContext *imgui_c89_ctx, ImGuiMultiSelectFlags flags, int selection_size, int items_count);
unsigned char imgui_begin_popup(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiWindowFlags flags);
unsigned char imgui_begin_popup_context_item(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiPopupFlags popup_flags);
unsigned char imgui_begin_popup_context_void(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiPopupFlags popup_flags);
unsigned char imgui_begin_popup_context_window(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiPopupFlags popup_flags);
unsigned char imgui_begin_popup_modal(ImGuiContext *imgui_c89_ctx, const char * name, unsigned char * p_open, ImGuiWindowFlags flags);
unsigned char imgui_begin_tab_bar(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiTabBarFlags flags);
unsigned char imgui_begin_tab_item(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char * p_open, ImGuiTabItemFlags flags);
unsigned char imgui_begin_table(ImGuiContext *imgui_c89_ctx, const char * str_id, int columns_count, ImGuiTableFlags flags, const ImVec2 * outer_size, float inner_width);
unsigned char imgui_begin_tooltip(ImGuiContext *imgui_c89_ctx);
void imgui_bullet(ImGuiContext *imgui_c89_ctx);
void imgui_bullet_text_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
unsigned char imgui_button(ImGuiContext *imgui_c89_ctx, const char * label, const ImVec2 * size_arg);
float imgui_calc_item_width(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_calc_text_size(ImGuiContext *imgui_c89_ctx, const char * text, const char * text_end, unsigned char hide_text_after_double_hash, float wrap_width);
unsigned char imgui_checkbox(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char * v);
unsigned char imgui_checkbox_flags_int_pointer_int(ImGuiContext *imgui_c89_ctx, const char * label, int * flags, int flags_value);
unsigned char imgui_checkbox_flags_uint_pointer_uint(ImGuiContext *imgui_c89_ctx, const char * label, unsigned int * flags, unsigned int flags_value);
void imgui_close_current_popup(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_collapsing_header_bool_pointer_tree_node_flags(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char * p_visible, ImGuiTreeNodeFlags flags);
unsigned char imgui_collapsing_header_tree_node_flags_none(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiTreeNodeFlags flags);
unsigned char imgui_color_button(ImGuiContext *imgui_c89_ctx, const char * desc_id, const ImVec4 * col, ImGuiColorEditFlags flags, const ImVec2 * size_arg);
ImU32 imgui_color_convert_float4_to_u32(const ImVec4 * in);
void imgui_color_convert_hs_vto_rgb(float h, float s, float v, float * out_r, float * out_g, float * out_b);
void imgui_color_convert_rg_bto_hsv(float r, float g, float b, float * out_h, float * out_s, float * out_v);
ImVec4 imgui_color_convert_u32_to_float4(ImU32 in);
unsigned char imgui_color_edit3(ImGuiContext *imgui_c89_ctx, const char * label, float * col, ImGuiColorEditFlags flags);
unsigned char imgui_color_edit4(ImGuiContext *imgui_c89_ctx, const char * label, float * col, ImGuiColorEditFlags flags);
ImColor imgui_color_hsv(float h, float s, float v, float a);
unsigned char imgui_color_picker3(ImGuiContext *imgui_c89_ctx, const char * label, float * col, ImGuiColorEditFlags flags);
unsigned char imgui_color_picker4(ImGuiContext *imgui_c89_ctx, const char * label, float * col, ImGuiColorEditFlags flags, const float * ref_col);
void imgui_color_set_hsv(ImColor *self, float h, float s, float v, float a);
void imgui_columns(ImGuiContext *imgui_c89_ctx, int columns_count, const char * id, unsigned char borders);
unsigned char imgui_combo_string_int_none_none(ImGuiContext *imgui_c89_ctx, const char * label, int * current_item, const char * items_separated_by_zeros, int height_in_items);
unsigned char imgui_combo_string_pointer_pointer_int_pointer_int_int(ImGuiContext *imgui_c89_ctx, const char * label, int * current_item, const char *(*getter)(void *, int), void * user_data, int items_count, int popup_max_height_in_items);
unsigned char imgui_combo_stringconst_pointer_int_int_none(ImGuiContext *imgui_c89_ctx, const char * label, int * current_item, const char *const * items, int items_count, int height_in_items);
ImGuiContext * imgui_create_context(ImFontAtlas * shared_font_atlas);
unsigned char imgui_debug_check_version_and_data_layout(const char * version, size_t sz_io, size_t sz_style, size_t sz_vec2, size_t sz_vec4, size_t sz_vert, size_t sz_idx);
void imgui_debug_flash_style_color(ImGuiContext *imgui_c89_ctx, ImGuiCol idx);
void imgui_debug_log_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_debug_start_item_picker(ImGuiContext *imgui_c89_ctx);
void imgui_debug_text_encoding(ImGuiContext *imgui_c89_ctx, const char * str);
void imgui_destroy_context(ImGuiContext * ctx);
unsigned char imgui_drag_float(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_speed, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_float2(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_speed, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_float3(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_speed, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_float4(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_speed, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_float_range2(ImGuiContext *imgui_c89_ctx, const char * label, float * v_current_min, float * v_current_max, float v_speed, float v_min, float v_max, const char * format, const char * format_max, ImGuiSliderFlags flags);
unsigned char imgui_drag_int(ImGuiContext *imgui_c89_ctx, const char * label, int * v, float v_speed, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_int2(ImGuiContext *imgui_c89_ctx, const char * label, int * v, float v_speed, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_int3(ImGuiContext *imgui_c89_ctx, const char * label, int * v, float v_speed, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_int4(ImGuiContext *imgui_c89_ctx, const char * label, int * v, float v_speed, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_int_range2(ImGuiContext *imgui_c89_ctx, const char * label, int * v_current_min, int * v_current_max, float v_speed, int v_min, int v_max, const char * format, const char * format_max, ImGuiSliderFlags flags);
unsigned char imgui_drag_scalar(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiDataType data_type, void * p_data, float v_speed, const void * p_min, const void * p_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_drag_scalar_n(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiDataType data_type, void * p_data, int components, float v_speed, const void * p_min, const void * p_max, const char * format, ImGuiSliderFlags flags);
ImTextureID imgui_draw_cmd_get_tex_id(ImDrawCmd *self);
void imgui_draw_data_add_draw_list(ImGuiContext *imgui_c89_ctx, ImDrawData *self, ImDrawList * draw_list);
void imgui_draw_data_clear(ImGuiContext *imgui_c89_ctx, ImDrawData *self);
void imgui_draw_data_de_index_all_buffers(ImGuiContext *imgui_c89_ctx, ImDrawData *self);
void imgui_draw_data_scale_clip_rects(ImDrawData *self, const ImVec2 * fb_scale);
void imgui_draw_list_add_bezier_cubic(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, ImU32 col, float thickness, int num_segments);
void imgui_draw_list_add_bezier_quadratic(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, ImU32 col, float thickness, int num_segments);
void imgui_draw_list_add_callback(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImDrawCallback callback, void * userdata, size_t userdata_size);
void imgui_draw_list_add_circle(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, ImU32 col, int num_segments, float thickness);
void imgui_draw_list_add_circle_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, ImU32 col, int num_segments);
void imgui_draw_list_add_concave_poly_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * points, const int points_count, ImU32 col);
void imgui_draw_list_add_convex_poly_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * points, const int points_count, ImU32 col);
void imgui_draw_list_add_draw_cmd(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_add_ellipse(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, const ImVec2 * radius, ImU32 col, float rot, int num_segments, float thickness);
void imgui_draw_list_add_ellipse_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, const ImVec2 * radius, ImU32 col, float rot, int num_segments);
void imgui_draw_list_add_image(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImTextureRef tex_ref, const ImVec2 * p_min, const ImVec2 * p_max, const ImVec2 * uv_min, const ImVec2 * uv_max, ImU32 col);
void imgui_draw_list_add_image_quad(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImTextureRef tex_ref, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, const ImVec2 * uv1, const ImVec2 * uv2, const ImVec2 * uv3, const ImVec2 * uv4, ImU32 col);
void imgui_draw_list_add_image_rounded(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImTextureRef tex_ref, const ImVec2 * p_min, const ImVec2 * p_max, const ImVec2 * uv_min, const ImVec2 * uv_max, ImU32 col, float rounding, ImDrawFlags flags);
void imgui_draw_list_add_line(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, ImU32 col, float thickness);
void imgui_draw_list_add_line_h(ImGuiContext *imgui_c89_ctx, ImDrawList *self, float min_x, float max_x, float y, ImU32 col, float thickness);
void imgui_draw_list_add_line_v(ImGuiContext *imgui_c89_ctx, ImDrawList *self, float x, float min_y, float max_y, ImU32 col, float thickness);
void imgui_draw_list_add_ngon(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, ImU32 col, int num_segments, float thickness);
void imgui_draw_list_add_ngon_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, ImU32 col, int num_segments);
void imgui_draw_list_add_polyline_int_draw_flags_float(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * points, int num_points, ImU32 col, ImDrawFlags flags, float thickness);
void imgui_draw_list_add_polyline_int_float_draw_flags(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * points, const int points_count, ImU32 col, float thickness, ImDrawFlags flags);
void imgui_draw_list_add_quad(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, ImU32 col, float thickness);
void imgui_draw_list_add_quad_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, ImU32 col);
void imgui_draw_list_add_rect_draw_flags_float(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p_min, const ImVec2 * p_max, ImU32 col, float rounding, ImDrawFlags flags, float thickness);
void imgui_draw_list_add_rect_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p_min, const ImVec2 * p_max, ImU32 col, float rounding, ImDrawFlags flags);
void imgui_draw_list_add_rect_filled_multi_color(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p_min, const ImVec2 * p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left);
void imgui_draw_list_add_rect_float_draw_flags(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p_min, const ImVec2 * p_max, ImU32 col, float rounding, float thickness, ImDrawFlags flags);
void imgui_draw_list_add_text_font_pointer_float_vec2_u32_string_string_float_vec4_pointer(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImFont * font, float font_size, const ImVec2 * pos, ImU32 col, const char * text_begin, const char * text_end, float wrap_width, const ImVec4 * cpu_fine_clip_rect);
void imgui_draw_list_add_text_vec2_u32_string_string_none_none_none_none(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * pos, ImU32 col, const char * text_begin, const char * text_end);
void imgui_draw_list_add_triangle(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, ImU32 col, float thickness);
void imgui_draw_list_add_triangle_filled(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p1, const ImVec2 * p2, const ImVec2 * p3, ImU32 col);
int imgui_draw_list_calc_circle_auto_segment_count(ImDrawList *self, float radius);
void imgui_draw_list_channels_merge(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_channels_set_current(ImGuiContext *imgui_c89_ctx, ImDrawList *self, int n);
void imgui_draw_list_channels_split(ImGuiContext *imgui_c89_ctx, ImDrawList *self, int count);
void imgui_draw_list_clear_free_memory(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
ImDrawList * imgui_draw_list_clone_output(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
ImVec2 imgui_draw_list_get_clip_rect_max(ImDrawList *self);
ImVec2 imgui_draw_list_get_clip_rect_min(ImDrawList *self);
void imgui_draw_list_on_changed_clip_rect(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_on_changed_texture(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_on_changed_vtx_offset(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_path_arc_to(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, float a_min, float a_max, int num_segments);
void imgui_draw_list_path_arc_to_fast(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, int a_min_of_12, int a_max_of_12);
void imgui_draw_list_path_arc_to_fast_ex(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, int a_min_sample, int a_max_sample, int a_step);
void imgui_draw_list_path_arc_to_n(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, float radius, float a_min, float a_max, int num_segments);
void imgui_draw_list_path_bezier_cubic_curve_to(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p2, const ImVec2 * p3, const ImVec2 * p4, int num_segments);
void imgui_draw_list_path_bezier_quadratic_curve_to(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * p2, const ImVec2 * p3, int num_segments);
void imgui_draw_list_path_clear(ImDrawList *self);
void imgui_draw_list_path_elliptical_arc_to(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * center, const ImVec2 * radius, float rot, float a_min, float a_max, int num_segments);
void imgui_draw_list_path_fill_concave(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImU32 col);
void imgui_draw_list_path_fill_convex(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImU32 col);
void imgui_draw_list_path_line_to(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * pos);
void imgui_draw_list_path_line_to_merge_duplicate(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * pos);
void imgui_draw_list_path_rect(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * a, const ImVec2 * b, float rounding, ImDrawFlags flags);
void imgui_draw_list_path_stroke_draw_flags_float(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImU32 col, ImDrawFlags flags, float thickness);
void imgui_draw_list_path_stroke_float_draw_flags(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImU32 col, float thickness, ImDrawFlags flags);
void imgui_draw_list_pop_clip_rect(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_pop_texture(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_pop_texture_id(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_pop_unused_draw_cmd(ImDrawList *self);
void imgui_draw_list_prim_quad_uv(ImDrawList *self, const ImVec2 * a, const ImVec2 * b, const ImVec2 * c, const ImVec2 * d, const ImVec2 * uv_a, const ImVec2 * uv_b, const ImVec2 * uv_c, const ImVec2 * uv_d, ImU32 col);
void imgui_draw_list_prim_rect(ImDrawList *self, const ImVec2 * a, const ImVec2 * c, ImU32 col);
void imgui_draw_list_prim_rect_uv(ImDrawList *self, const ImVec2 * a, const ImVec2 * c, const ImVec2 * uv_a, const ImVec2 * uv_c, ImU32 col);
void imgui_draw_list_prim_reserve(ImGuiContext *imgui_c89_ctx, ImDrawList *self, int idx_count, int vtx_count);
void imgui_draw_list_prim_unreserve(ImDrawList *self, int idx_count, int vtx_count);
void imgui_draw_list_prim_vtx(ImDrawList *self, const ImVec2 * pos, const ImVec2 * uv, ImU32 col);
void imgui_draw_list_prim_write_idx(ImDrawList *self, ImDrawIdx idx);
void imgui_draw_list_prim_write_vtx(ImDrawList *self, const ImVec2 * pos, const ImVec2 * uv, ImU32 col);
void imgui_draw_list_push_clip_rect(ImGuiContext *imgui_c89_ctx, ImDrawList *self, const ImVec2 * cr_min, const ImVec2 * cr_max, unsigned char intersect_with_current_clip_rect);
void imgui_draw_list_push_clip_rect_full_screen(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_push_texture(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImTextureRef tex_ref);
void imgui_draw_list_push_texture_id(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImTextureRef tex_ref);
void imgui_draw_list_reset_for_new_frame(ImGuiContext *imgui_c89_ctx, ImDrawList *self);
void imgui_draw_list_set_draw_list_shared_data(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImDrawListSharedData * data);
void imgui_draw_list_set_texture(ImGuiContext *imgui_c89_ctx, ImDrawList *self, ImTextureRef tex_ref);
void imgui_draw_list_splitter_clear(ImDrawListSplitter *self);
void imgui_draw_list_splitter_clear_free_memory(ImGuiContext *imgui_c89_ctx, ImDrawListSplitter *self);
void imgui_draw_list_splitter_merge(ImGuiContext *imgui_c89_ctx, ImDrawListSplitter *self, ImDrawList * draw_list);
void imgui_draw_list_splitter_set_current_channel(ImGuiContext *imgui_c89_ctx, ImDrawListSplitter *self, ImDrawList * draw_list, int idx);
void imgui_draw_list_splitter_split(ImGuiContext *imgui_c89_ctx, ImDrawListSplitter *self, ImDrawList * draw_list, int channels_count);
void imgui_draw_list_try_merge_draw_cmds(ImDrawList *self);
void imgui_dummy(ImGuiContext *imgui_c89_ctx, const ImVec2 * size);
void imgui_end(ImGuiContext *imgui_c89_ctx);
void imgui_end_child(ImGuiContext *imgui_c89_ctx);
void imgui_end_combo(ImGuiContext *imgui_c89_ctx);
void imgui_end_disabled(ImGuiContext *imgui_c89_ctx);
void imgui_end_drag_drop_source(ImGuiContext *imgui_c89_ctx);
void imgui_end_drag_drop_target(ImGuiContext *imgui_c89_ctx);
void imgui_end_frame(ImGuiContext *imgui_c89_ctx);
void imgui_end_group(ImGuiContext *imgui_c89_ctx);
void imgui_end_list_box(ImGuiContext *imgui_c89_ctx);
void imgui_end_main_menu_bar(ImGuiContext *imgui_c89_ctx);
void imgui_end_menu(ImGuiContext *imgui_c89_ctx);
void imgui_end_menu_bar(ImGuiContext *imgui_c89_ctx);
ImGuiMultiSelectIO * imgui_end_multi_select(ImGuiContext *imgui_c89_ctx);
void imgui_end_popup(ImGuiContext *imgui_c89_ctx);
void imgui_end_tab_bar(ImGuiContext *imgui_c89_ctx);
void imgui_end_tab_item(ImGuiContext *imgui_c89_ctx);
void imgui_end_table(ImGuiContext *imgui_c89_ctx);
void imgui_end_tooltip(ImGuiContext *imgui_c89_ctx);
void imgui_font_add_remap_char(ImGuiContext *imgui_c89_ctx, ImFont *self, ImWchar from_codepoint, ImWchar to_codepoint);
ImFontAtlasRectId imgui_font_atlas_add_custom_rect(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, int width, int height, ImFontAtlasRect * out_r);
ImFontAtlasRectId imgui_font_atlas_add_custom_rect_font_glyph(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, ImFont * font, ImWchar codepoint, int width, int height, float advance_x, const ImVec2 * offset);
ImFontAtlasRectId imgui_font_atlas_add_custom_rect_font_glyph_for_size(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, ImFont * font, float font_size, ImWchar codepoint, int width, int height, float advance_x, const ImVec2 * offset);
ImFontAtlasRectId imgui_font_atlas_add_custom_rect_regular(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, int w, int h);
ImFont * imgui_font_atlas_add_font(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const ImFontConfig * font_cfg_in);
ImFont * imgui_font_atlas_add_font_default(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const ImFontConfig * font_cfg);
ImFont * imgui_font_atlas_add_font_default_bitmap(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const ImFontConfig * font_cfg_template);
ImFont * imgui_font_atlas_add_font_default_vector(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const ImFontConfig * font_cfg_template);
ImFont * imgui_font_atlas_add_font_from_file_ttf(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const char * filename, float size_pixels, const ImFontConfig * font_cfg_template, const ImWchar * glyph_ranges);
ImFont * imgui_font_atlas_add_font_from_memory_compressed_base85_ttf(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const char * compressed_ttf_data_base85, float size_pixels, const ImFontConfig * font_cfg, const ImWchar * glyph_ranges);
ImFont * imgui_font_atlas_add_font_from_memory_compressed_ttf(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const void * compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig * font_cfg_template, const ImWchar * glyph_ranges);
ImFont * imgui_font_atlas_add_font_from_memory_ttf(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, void * font_data, int font_data_size, float size_pixels, const ImFontConfig * font_cfg_template, const ImWchar * glyph_ranges);
unsigned char imgui_font_atlas_build(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
void imgui_font_atlas_calc_custom_rect_uv(ImFontAtlas *self, const ImFontAtlasRect * r, ImVec2 * out_uv_min, ImVec2 * out_uv_max);
void imgui_font_atlas_clear(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
void imgui_font_atlas_clear_fonts(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
void imgui_font_atlas_clear_input_data(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
void imgui_font_atlas_clear_tex_data(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
void imgui_font_atlas_compact_cache(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self);
unsigned char imgui_font_atlas_get_custom_rect(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, ImFontAtlasRectId id, ImFontAtlasRect * out_r);
const ImFontAtlasRect * imgui_font_atlas_get_custom_rect_by_index(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, ImFontAtlasRectId id);
const ImWchar * imgui_font_atlas_get_glyph_ranges_chinese_full(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_chinese_simplified_common(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_cyrillic(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_default(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_greek(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_japanese(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_korean(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_thai(ImFontAtlas *self);
const ImWchar * imgui_font_atlas_get_glyph_ranges_vietnamese(ImFontAtlas *self);
void imgui_font_atlas_get_tex_data_as_alpha8(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, unsigned char ** out_pixels, int * out_width, int * out_height, int * out_bytes_per_pixel);
void imgui_font_atlas_get_tex_data_as_rgba32(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, unsigned char ** out_pixels, int * out_width, int * out_height, int * out_bytes_per_pixel);
unsigned char imgui_font_atlas_is_built(ImFontAtlas *self);
void imgui_font_atlas_remove_custom_rect(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, ImFontAtlasRectId id);
void imgui_font_atlas_remove_font(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, ImFont * font);
void imgui_font_atlas_set_font_loader(ImGuiContext *imgui_c89_ctx, ImFontAtlas *self, const ImFontLoader * font_loader);
void imgui_font_atlas_set_tex_id_texture_id(ImFontAtlas *self, ImTextureID id);
void imgui_font_atlas_set_tex_id_texture_ref(ImFontAtlas *self, ImTextureRef id);
void imgui_font_baked_clear_output_data(ImGuiContext *imgui_c89_ctx, ImFontBaked *self);
ImFontGlyph * imgui_font_baked_find_glyph(ImGuiContext *imgui_c89_ctx, ImFontBaked *self, ImWchar c);
ImFontGlyph * imgui_font_baked_find_glyph_no_fallback(ImGuiContext *imgui_c89_ctx, ImFontBaked *self, ImWchar c);
float imgui_font_baked_get_char_advance(ImGuiContext *imgui_c89_ctx, ImFontBaked *self, ImWchar c);
unsigned char imgui_font_baked_is_glyph_loaded(ImFontBaked *self, ImWchar c);
ImVec2 imgui_font_calc_text_size_a(ImGuiContext *imgui_c89_ctx, ImFont *self, float size, float max_width, float wrap_width, const char * text_begin, const char * text_end, const char ** out_remaining);
const char * imgui_font_calc_word_wrap_position(ImGuiContext *imgui_c89_ctx, ImFont *self, float size, const char * text, const char * text_end, float wrap_width);
const char * imgui_font_calc_word_wrap_position_a(ImGuiContext *imgui_c89_ctx, ImFont *self, float scale, const char * text, const char * text_end, float wrap_width);
void imgui_font_clear_output_data(ImGuiContext *imgui_c89_ctx, ImFont *self);
const char * imgui_font_get_debug_name(ImFont *self);
ImFontBaked * imgui_font_get_font_baked(ImGuiContext *imgui_c89_ctx, ImFont *self, float size, float density);
void imgui_font_glyph_ranges_builder_add_char(ImFontGlyphRangesBuilder *self, ImWchar c);
void imgui_font_glyph_ranges_builder_add_ranges(ImFontGlyphRangesBuilder *self, const ImWchar * ranges);
void imgui_font_glyph_ranges_builder_add_text(ImFontGlyphRangesBuilder *self, const char * text, const char * text_end);
void imgui_font_glyph_ranges_builder_build_ranges(ImGuiContext *imgui_c89_ctx, ImFontGlyphRangesBuilder *self, ImVector_unsigned_short * out_ranges);
void imgui_font_glyph_ranges_builder_clear(ImGuiContext *imgui_c89_ctx, ImFontGlyphRangesBuilder *self);
unsigned char imgui_font_glyph_ranges_builder_get_bit(ImFontGlyphRangesBuilder *self, size_t n);
void imgui_font_glyph_ranges_builder_set_bit(ImFontGlyphRangesBuilder *self, size_t n);
unsigned char imgui_font_is_glyph_in_font(ImFont *self, ImWchar c);
unsigned char imgui_font_is_glyph_range_unused(ImFont *self, unsigned int c_begin, unsigned int c_last);
unsigned char imgui_font_is_loaded(ImFont *self);
void imgui_font_render_char(ImGuiContext *imgui_c89_ctx, ImFont *self, ImDrawList * draw_list, float size, const ImVec2 * pos, ImU32 col, ImWchar c, const ImVec4 * cpu_fine_clip);
void imgui_font_render_text(ImGuiContext *imgui_c89_ctx, ImFont *self, ImDrawList * draw_list, float size, const ImVec2 * pos, ImU32 col, const ImVec4 * clip_rect, const char * text_begin, const char * text_end, float wrap_width, ImDrawTextFlags flags);
void imgui_get_allocator_functions(ImGuiMemAllocFunc * p_alloc_func, ImGuiMemFreeFunc * p_free_func, void ** p_user_data);
ImDrawList * imgui_get_background_draw_list(ImGuiContext *imgui_c89_ctx);
const char * imgui_get_clipboard_text(ImGuiContext *imgui_c89_ctx);
ImU32 imgui_get_color_u32_col_float(ImGuiContext *imgui_c89_ctx, ImGuiCol idx, float alpha_mul);
ImU32 imgui_get_color_u32_u32_float(ImGuiContext *imgui_c89_ctx, ImU32 col, float alpha_mul);
ImU32 imgui_get_color_u32_vec4_none(ImGuiContext *imgui_c89_ctx, const ImVec4 * col);
int imgui_get_column_index(ImGuiContext *imgui_c89_ctx);
float imgui_get_column_offset(ImGuiContext *imgui_c89_ctx, int column_index);
float imgui_get_column_width(ImGuiContext *imgui_c89_ctx, int column_index);
int imgui_get_columns_count(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_content_region_avail(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_content_region_max(ImGuiContext *imgui_c89_ctx);
ImGuiContext * imgui_get_current_context(void);
ImVec2 imgui_get_cursor_pos(ImGuiContext *imgui_c89_ctx);
float imgui_get_cursor_pos_x(ImGuiContext *imgui_c89_ctx);
float imgui_get_cursor_pos_y(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_cursor_screen_pos(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_cursor_start_pos(ImGuiContext *imgui_c89_ctx);
const ImGuiPayload * imgui_get_drag_drop_payload(ImGuiContext *imgui_c89_ctx);
ImDrawData * imgui_get_draw_data(ImGuiContext *imgui_c89_ctx);
ImDrawListSharedData * imgui_get_draw_list_shared_data(ImGuiContext *imgui_c89_ctx);
ImFont * imgui_get_font(ImGuiContext *imgui_c89_ctx);
ImFontBaked * imgui_get_font_baked(ImGuiContext *imgui_c89_ctx);
float imgui_get_font_size(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_font_tex_uv_white_pixel(ImGuiContext *imgui_c89_ctx);
ImDrawList * imgui_get_foreground_draw_list(ImGuiContext *imgui_c89_ctx);
int imgui_get_frame_count(ImGuiContext *imgui_c89_ctx);
float imgui_get_frame_height(ImGuiContext *imgui_c89_ctx);
float imgui_get_frame_height_with_spacing(ImGuiContext *imgui_c89_ctx);
ImGuiID imgui_get_id_const_pointer_none(ImGuiContext *imgui_c89_ctx, const void * ptr_id);
ImGuiID imgui_get_id_int_none(ImGuiContext *imgui_c89_ctx, int int_id);
ImGuiID imgui_get_id_string_none(ImGuiContext *imgui_c89_ctx, const char * str_id);
ImGuiID imgui_get_id_string_string(ImGuiContext *imgui_c89_ctx, const char * str_id_begin, const char * str_id_end);
ImGuiIO * imgui_get_io(ImGuiContext *imgui_c89_ctx);
int imgui_get_item_clicked_count_with_single_click_delay(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton mouse_button, float delay);
ImGuiItemFlags imgui_get_item_flags(ImGuiContext *imgui_c89_ctx);
ImGuiID imgui_get_item_id(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_item_rect_max(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_item_rect_min(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_item_rect_size(ImGuiContext *imgui_c89_ctx);
const char * imgui_get_key_name(ImGuiKey key);
int imgui_get_key_pressed_amount(ImGuiContext *imgui_c89_ctx, ImGuiKey key, float repeat_delay, float repeat_rate);
ImGuiViewport * imgui_get_main_viewport(ImGuiContext *imgui_c89_ctx);
int imgui_get_mouse_clicked_count(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button);
ImGuiMouseCursor imgui_get_mouse_cursor(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_mouse_drag_delta(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, float lock_threshold);
ImVec2 imgui_get_mouse_pos(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_mouse_pos_on_opening_current_popup(ImGuiContext *imgui_c89_ctx);
ImGuiPlatformIO * imgui_get_platform_io(ImGuiContext *imgui_c89_ctx);
float imgui_get_scroll_max_x(ImGuiContext *imgui_c89_ctx);
float imgui_get_scroll_max_y(ImGuiContext *imgui_c89_ctx);
float imgui_get_scroll_x(ImGuiContext *imgui_c89_ctx);
float imgui_get_scroll_y(ImGuiContext *imgui_c89_ctx);
ImGuiStorage * imgui_get_state_storage(ImGuiContext *imgui_c89_ctx);
ImGuiStyle * imgui_get_style(ImGuiContext *imgui_c89_ctx);
const char * imgui_get_style_color_name(ImGuiCol idx);
const ImVec4 * imgui_get_style_color_vec4(ImGuiContext *imgui_c89_ctx, ImGuiCol idx);
float imgui_get_text_line_height(ImGuiContext *imgui_c89_ctx);
float imgui_get_text_line_height_with_spacing(ImGuiContext *imgui_c89_ctx);
double imgui_get_time(ImGuiContext *imgui_c89_ctx);
float imgui_get_tree_node_to_label_spacing(ImGuiContext *imgui_c89_ctx);
const char * imgui_get_version(void);
ImVec2 imgui_get_window_content_region_max(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_window_content_region_min(ImGuiContext *imgui_c89_ctx);
ImDrawList * imgui_get_window_draw_list(ImGuiContext *imgui_c89_ctx);
float imgui_get_window_height(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_window_pos(ImGuiContext *imgui_c89_ctx);
ImVec2 imgui_get_window_size(ImGuiContext *imgui_c89_ctx);
float imgui_get_window_width(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_image_button(ImGuiContext *imgui_c89_ctx, const char * str_id, ImTextureRef tex_ref, const ImVec2 * image_size, const ImVec2 * uv0, const ImVec2 * uv1, const ImVec4 * bg_col, const ImVec4 * tint_col);
void imgui_image_none_none(ImGuiContext *imgui_c89_ctx, ImTextureRef tex_ref, const ImVec2 * image_size, const ImVec2 * uv0, const ImVec2 * uv1);
void imgui_image_vec4_vec4(ImGuiContext *imgui_c89_ctx, ImTextureRef tex_ref, const ImVec2 * image_size, const ImVec2 * uv0, const ImVec2 * uv1, const ImVec4 * tint_col, const ImVec4 * border_col);
void imgui_image_with_bg(ImGuiContext *imgui_c89_ctx, ImTextureRef tex_ref, const ImVec2 * image_size, const ImVec2 * uv0, const ImVec2 * uv1, const ImVec4 * bg_col, const ImVec4 * tint_col);
void imgui_indent(ImGuiContext *imgui_c89_ctx, float indent_w);
unsigned char imgui_input_double(ImGuiContext *imgui_c89_ctx, const char * label, double * v, double step, double step_fast, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_float(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float step, float step_fast, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_float2(ImGuiContext *imgui_c89_ctx, const char * label, float * v, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_float3(ImGuiContext *imgui_c89_ctx, const char * label, float * v, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_float4(ImGuiContext *imgui_c89_ctx, const char * label, float * v, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_int(ImGuiContext *imgui_c89_ctx, const char * label, int * v, int step, int step_fast, ImGuiInputTextFlags flags);
unsigned char imgui_input_int2(ImGuiContext *imgui_c89_ctx, const char * label, int * v, ImGuiInputTextFlags flags);
unsigned char imgui_input_int3(ImGuiContext *imgui_c89_ctx, const char * label, int * v, ImGuiInputTextFlags flags);
unsigned char imgui_input_int4(ImGuiContext *imgui_c89_ctx, const char * label, int * v, ImGuiInputTextFlags flags);
unsigned char imgui_input_scalar(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiDataType data_type, void * p_data, const void * p_step, const void * p_step_fast, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_scalar_n(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiDataType data_type, void * p_data, int components, const void * p_step, const void * p_step_fast, const char * format, ImGuiInputTextFlags flags);
unsigned char imgui_input_text(ImGuiContext *imgui_c89_ctx, const char * label, char * buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void * user_data);
void imgui_input_text_callback_data_clear_selection(ImGuiInputTextCallbackData *self);
void imgui_input_text_callback_data_delete_chars(ImGuiInputTextCallbackData *self, int pos, int bytes_count);
unsigned char imgui_input_text_callback_data_has_selection(ImGuiInputTextCallbackData *self);
void imgui_input_text_callback_data_insert_chars(ImGuiContext *imgui_c89_ctx, ImGuiInputTextCallbackData *self, int pos, const char * new_text, const char * new_text_end);
void imgui_input_text_callback_data_select_all(ImGuiInputTextCallbackData *self);
void imgui_input_text_callback_data_set_selection(ImGuiInputTextCallbackData *self, int s, int e);
unsigned char imgui_input_text_multiline(ImGuiContext *imgui_c89_ctx, const char * label, char * buf, size_t buf_size, const ImVec2 * size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void * user_data);
unsigned char imgui_input_text_with_hint(ImGuiContext *imgui_c89_ctx, const char * label, const char * hint, char * buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void * user_data);
unsigned char imgui_invisible_button(ImGuiContext *imgui_c89_ctx, const char * str_id, const ImVec2 * size_arg, ImGuiButtonFlags flags);
void imgui_io_add_focus_event(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, unsigned char focused);
void imgui_io_add_input_character(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, unsigned int c);
void imgui_io_add_input_character_utf16(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, ImWchar16 c);
void imgui_io_add_input_characters_utf8(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, const char * str);
void imgui_io_add_key_analog_event(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, ImGuiKey key, unsigned char down, float analog_value);
void imgui_io_add_key_event(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, ImGuiKey key, unsigned char down);
void imgui_io_add_mouse_button_event(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, int mouse_button, unsigned char down);
void imgui_io_add_mouse_pos_event(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, float x, float y);
void imgui_io_add_mouse_source_event(ImGuiIO *self, ImGuiMouseSource source);
void imgui_io_add_mouse_wheel_event(ImGuiContext *imgui_c89_ctx, ImGuiIO *self, float wheel_x, float wheel_y);
void imgui_io_clear_events_queue(ImGuiContext *imgui_c89_ctx, ImGuiIO *self);
void imgui_io_clear_input_keys(ImGuiContext *imgui_c89_ctx, ImGuiIO *self);
void imgui_io_clear_input_mouse(ImGuiIO *self);
void imgui_io_set_app_accepting_events(ImGuiIO *self, unsigned char accepting_events);
void imgui_io_set_key_event_native_data(ImGuiIO *self, ImGuiKey key, int native_keycode, int native_scancode, int native_legacy_index);
unsigned char imgui_is_any_item_active(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_any_item_focused(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_any_item_hovered(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_any_mouse_down(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_activated(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_active(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_clicked(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton mouse_button);
unsigned char imgui_is_item_deactivated(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_deactivated_after_edit(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_edited(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_focused(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_hovered(ImGuiContext *imgui_c89_ctx, ImGuiHoveredFlags flags);
unsigned char imgui_is_item_toggled_open(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_toggled_selection(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_item_visible(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_key_chord_pressed(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord);
unsigned char imgui_is_key_down(ImGuiContext *imgui_c89_ctx, ImGuiKey key);
unsigned char imgui_is_key_pressed(ImGuiContext *imgui_c89_ctx, ImGuiKey key, unsigned char repeat);
unsigned char imgui_is_key_released(ImGuiContext *imgui_c89_ctx, ImGuiKey key);
unsigned char imgui_is_mouse_clicked(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, unsigned char repeat);
unsigned char imgui_is_mouse_double_clicked(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button);
unsigned char imgui_is_mouse_down(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button);
unsigned char imgui_is_mouse_dragging(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, float lock_threshold);
unsigned char imgui_is_mouse_hovering_rect(ImGuiContext *imgui_c89_ctx, const ImVec2 * r_min, const ImVec2 * r_max, unsigned char clip);
unsigned char imgui_is_mouse_pos_valid(ImGuiContext *imgui_c89_ctx, const ImVec2 * mouse_pos);
unsigned char imgui_is_mouse_released(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button);
unsigned char imgui_is_mouse_released_with_delay(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button, float delay);
unsigned char imgui_is_popup_open(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiPopupFlags popup_flags);
unsigned char imgui_is_rect_visible_none(ImGuiContext *imgui_c89_ctx, const ImVec2 * size);
unsigned char imgui_is_rect_visible_vec2(ImGuiContext *imgui_c89_ctx, const ImVec2 * rect_min, const ImVec2 * rect_max);
unsigned char imgui_is_window_appearing(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_window_collapsed(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_is_window_focused(ImGuiContext *imgui_c89_ctx, ImGuiFocusedFlags flags);
unsigned char imgui_is_window_hovered(ImGuiContext *imgui_c89_ctx, ImGuiHoveredFlags flags);
void imgui_label_text_v(ImGuiContext *imgui_c89_ctx, const char * label, const char * fmt, va_list args);
unsigned char imgui_list_box_string_pointer_pointer_int_pointer_int(ImGuiContext *imgui_c89_ctx, const char * label, int * current_item, const char *(*getter)(void *, int), void * user_data, int items_count, int height_in_items);
unsigned char imgui_list_box_stringconst_pointer_int_none(ImGuiContext *imgui_c89_ctx, const char * label, int * current_item, const char *const * items, int items_count, int height_items);
void imgui_list_clipper_begin(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self, int items_count, float items_height);
void imgui_list_clipper_end(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self);
void imgui_list_clipper_include_item_by_index(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self, int item_index);
void imgui_list_clipper_include_items_by_index(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self, int item_begin, int item_end);
void imgui_list_clipper_seek_cursor_for_item(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self, int item_n);
unsigned char imgui_list_clipper_step(ImGuiContext *imgui_c89_ctx, ImGuiListClipper *self);
void imgui_load_ini_settings_from_disk(ImGuiContext *imgui_c89_ctx, const char * ini_filename);
void imgui_load_ini_settings_from_memory(ImGuiContext *imgui_c89_ctx, const char * ini_data, size_t ini_size);
void imgui_log_buttons(ImGuiContext *imgui_c89_ctx);
void imgui_log_finish(ImGuiContext *imgui_c89_ctx);
void imgui_log_text_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_log_to_clipboard(ImGuiContext *imgui_c89_ctx, int auto_open_depth);
void imgui_log_to_file(ImGuiContext *imgui_c89_ctx, int auto_open_depth, const char * filename);
void imgui_log_to_tty(ImGuiContext *imgui_c89_ctx, int auto_open_depth);
void * imgui_mem_alloc(ImGuiContext *imgui_c89_ctx, size_t size);
void imgui_mem_free(ImGuiContext *imgui_c89_ctx, void * ptr);
unsigned char imgui_menu_item_bool(ImGuiContext *imgui_c89_ctx, const char * label, const char * shortcut, unsigned char selected, unsigned char enabled);
unsigned char imgui_menu_item_bool_pointer(ImGuiContext *imgui_c89_ctx, const char * label, const char * shortcut, unsigned char * p_selected, unsigned char enabled);
void imgui_new_frame(ImGuiContext *imgui_c89_ctx);
void imgui_new_line(ImGuiContext *imgui_c89_ctx);
void imgui_next_column(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_open_popup_id(ImGuiContext *imgui_c89_ctx, ImGuiID id, ImGuiPopupFlags popup_flags);
unsigned char imgui_open_popup_on_item_click(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiPopupFlags popup_flags);
unsigned char imgui_open_popup_string(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiPopupFlags popup_flags);
void imgui_payload_clear(ImGuiPayload *self);
unsigned char imgui_payload_is_data_type(ImGuiPayload *self, const char * type);
unsigned char imgui_payload_is_delivery(ImGuiPayload *self);
unsigned char imgui_payload_is_preview(ImGuiPayload *self);
void imgui_platform_io_clear_platform_handlers(ImGuiPlatformIO *self);
void imgui_platform_io_clear_renderer_handlers(ImGuiPlatformIO *self);
void imgui_plot_histogram_float_pointer_int_string_float_vec2_int(ImGuiContext *imgui_c89_ctx, const char * label, const float * values, int values_count, int values_offset, const char * overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride);
void imgui_plot_histogram_float_pointer_pointer_int_pointer_int_string_float_vec2(ImGuiContext *imgui_c89_ctx, const char * label, float (*values_getter)(void *, int), void * data, int values_count, int values_offset, const char * overlay_text, float scale_min, float scale_max, ImVec2 graph_size);
void imgui_plot_lines_float_pointer_int_string_float_vec2_int(ImGuiContext *imgui_c89_ctx, const char * label, const float * values, int values_count, int values_offset, const char * overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride);
void imgui_plot_lines_float_pointer_pointer_int_pointer_int_string_float_vec2(ImGuiContext *imgui_c89_ctx, const char * label, float (*values_getter)(void *, int), void * data, int values_count, int values_offset, const char * overlay_text, float scale_min, float scale_max, ImVec2 graph_size);
void imgui_pop_button_repeat(ImGuiContext *imgui_c89_ctx);
void imgui_pop_clip_rect(ImGuiContext *imgui_c89_ctx);
void imgui_pop_font(ImGuiContext *imgui_c89_ctx);
void imgui_pop_id(ImGuiContext *imgui_c89_ctx);
void imgui_pop_item_flag(ImGuiContext *imgui_c89_ctx);
void imgui_pop_item_width(ImGuiContext *imgui_c89_ctx);
void imgui_pop_style_color(ImGuiContext *imgui_c89_ctx, int count);
void imgui_pop_style_var(ImGuiContext *imgui_c89_ctx, int count);
void imgui_pop_tab_stop(ImGuiContext *imgui_c89_ctx);
void imgui_pop_text_wrap_pos(ImGuiContext *imgui_c89_ctx);
void imgui_progress_bar(ImGuiContext *imgui_c89_ctx, float fraction, const ImVec2 * size_arg, const char * overlay);
void imgui_push_button_repeat(ImGuiContext *imgui_c89_ctx, unsigned char repeat);
void imgui_push_clip_rect(ImGuiContext *imgui_c89_ctx, const ImVec2 * clip_rect_min, const ImVec2 * clip_rect_max, unsigned char intersect_with_current_clip_rect);
void imgui_push_font_float(ImGuiContext *imgui_c89_ctx, ImFont * font, float font_size_base);
void imgui_push_font_none(ImGuiContext *imgui_c89_ctx, ImFont * font);
void imgui_push_id_const_pointer_none(ImGuiContext *imgui_c89_ctx, const void * ptr_id);
void imgui_push_id_int_none(ImGuiContext *imgui_c89_ctx, int int_id);
void imgui_push_id_string_none(ImGuiContext *imgui_c89_ctx, const char * str_id);
void imgui_push_id_string_string(ImGuiContext *imgui_c89_ctx, const char * str_id_begin, const char * str_id_end);
void imgui_push_item_flag(ImGuiContext *imgui_c89_ctx, ImGuiItemFlags option, unsigned char enabled);
void imgui_push_item_width(ImGuiContext *imgui_c89_ctx, float item_width);
void imgui_push_style_color_u32(ImGuiContext *imgui_c89_ctx, ImGuiCol idx, ImU32 col);
void imgui_push_style_color_vec4(ImGuiContext *imgui_c89_ctx, ImGuiCol idx, const ImVec4 * col);
void imgui_push_style_var_float(ImGuiContext *imgui_c89_ctx, ImGuiStyleVar idx, float val);
void imgui_push_style_var_vec2(ImGuiContext *imgui_c89_ctx, ImGuiStyleVar idx, const ImVec2 * val);
void imgui_push_style_var_x(ImGuiContext *imgui_c89_ctx, ImGuiStyleVar idx, float val_x);
void imgui_push_style_var_y(ImGuiContext *imgui_c89_ctx, ImGuiStyleVar idx, float val_y);
void imgui_push_tab_stop(ImGuiContext *imgui_c89_ctx, unsigned char tab_stop);
void imgui_push_text_wrap_pos(ImGuiContext *imgui_c89_ctx, float wrap_local_pos_x);
unsigned char imgui_radio_button_bool_none(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char active);
unsigned char imgui_radio_button_int_pointer_int(ImGuiContext *imgui_c89_ctx, const char * label, int * v, int v_button);
void imgui_render(ImGuiContext *imgui_c89_ctx);
void imgui_reset_mouse_drag_delta(ImGuiContext *imgui_c89_ctx, ImGuiMouseButton button);
void imgui_same_line(ImGuiContext *imgui_c89_ctx, float offset_from_start_x, float spacing_w);
void imgui_save_ini_settings_to_disk(ImGuiContext *imgui_c89_ctx, const char * ini_filename);
const char * imgui_save_ini_settings_to_memory(ImGuiContext *imgui_c89_ctx, size_t * out_size);
unsigned char imgui_selectable_bool(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char selected, ImGuiSelectableFlags flags, const ImVec2 * size_arg);
unsigned char imgui_selectable_bool_pointer(ImGuiContext *imgui_c89_ctx, const char * label, unsigned char * p_selected, ImGuiSelectableFlags flags, const ImVec2 * size_arg);
void imgui_selection_basic_storage_apply_requests(ImGuiContext *imgui_c89_ctx, ImGuiSelectionBasicStorage *self, ImGuiMultiSelectIO * ms_io);
void imgui_selection_basic_storage_clear(ImGuiContext *imgui_c89_ctx, ImGuiSelectionBasicStorage *self);
unsigned char imgui_selection_basic_storage_contains(ImGuiSelectionBasicStorage *self, ImGuiID id);
unsigned char imgui_selection_basic_storage_get_next_selected_item(ImGuiSelectionBasicStorage *self, void ** opaque_it, ImGuiID * out_id);
ImGuiID imgui_selection_basic_storage_get_storage_id_from_index(ImGuiSelectionBasicStorage *self, int idx);
void imgui_selection_basic_storage_set_item_selected(ImGuiContext *imgui_c89_ctx, ImGuiSelectionBasicStorage *self, ImGuiID id, unsigned char selected);
void imgui_selection_basic_storage_swap(ImGuiSelectionBasicStorage *self, ImGuiSelectionBasicStorage * r);
void imgui_selection_external_storage_apply_requests(ImGuiSelectionExternalStorage *self, ImGuiMultiSelectIO * ms_io);
void imgui_separator(ImGuiContext *imgui_c89_ctx);
void imgui_separator_text(ImGuiContext *imgui_c89_ctx, const char * label);
void imgui_set_allocator_functions(ImGuiMemAllocFunc alloc_func, ImGuiMemFreeFunc free_func, void * user_data);
void imgui_set_clipboard_text(ImGuiContext *imgui_c89_ctx, const char * text);
void imgui_set_color_edit_options(ImGuiContext *imgui_c89_ctx, ImGuiColorEditFlags flags);
void imgui_set_column_offset(ImGuiContext *imgui_c89_ctx, int column_index, float offset);
void imgui_set_column_width(ImGuiContext *imgui_c89_ctx, int column_index, float width);
void imgui_set_current_context(ImGuiContext * ctx);
void imgui_set_cursor_pos(ImGuiContext *imgui_c89_ctx, const ImVec2 * local_pos);
void imgui_set_cursor_pos_x(ImGuiContext *imgui_c89_ctx, float x);
void imgui_set_cursor_pos_y(ImGuiContext *imgui_c89_ctx, float y);
void imgui_set_cursor_screen_pos(ImGuiContext *imgui_c89_ctx, const ImVec2 * pos);
unsigned char imgui_set_drag_drop_payload(ImGuiContext *imgui_c89_ctx, const char * type, const void * data, size_t data_size, ImGuiCond cond);
void imgui_set_item_default_focus(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_set_item_key_owner(ImGuiContext *imgui_c89_ctx, ImGuiKey key);
void imgui_set_item_tooltip_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_set_keyboard_focus_here(ImGuiContext *imgui_c89_ctx, int offset);
void imgui_set_mouse_cursor(ImGuiContext *imgui_c89_ctx, ImGuiMouseCursor cursor_type);
void imgui_set_nav_cursor_visible(ImGuiContext *imgui_c89_ctx, unsigned char visible);
void imgui_set_next_frame_want_capture_keyboard(ImGuiContext *imgui_c89_ctx, unsigned char want_capture_keyboard);
void imgui_set_next_frame_want_capture_mouse(ImGuiContext *imgui_c89_ctx, unsigned char want_capture_mouse);
void imgui_set_next_item_allow_overlap(ImGuiContext *imgui_c89_ctx);
void imgui_set_next_item_open(ImGuiContext *imgui_c89_ctx, unsigned char is_open, ImGuiCond cond);
void imgui_set_next_item_selection_user_data(ImGuiContext *imgui_c89_ctx, ImGuiSelectionUserData selection_user_data);
void imgui_set_next_item_shortcut(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord, ImGuiInputFlags flags);
void imgui_set_next_item_storage_id(ImGuiContext *imgui_c89_ctx, ImGuiID storage_id);
void imgui_set_next_item_width(ImGuiContext *imgui_c89_ctx, float item_width);
void imgui_set_next_window_bg_alpha(ImGuiContext *imgui_c89_ctx, float alpha);
void imgui_set_next_window_collapsed(ImGuiContext *imgui_c89_ctx, unsigned char collapsed, ImGuiCond cond);
void imgui_set_next_window_content_size(ImGuiContext *imgui_c89_ctx, const ImVec2 * size);
void imgui_set_next_window_focus(ImGuiContext *imgui_c89_ctx);
void imgui_set_next_window_pos(ImGuiContext *imgui_c89_ctx, const ImVec2 * pos, ImGuiCond cond, const ImVec2 * pivot);
void imgui_set_next_window_scroll(ImGuiContext *imgui_c89_ctx, const ImVec2 * scroll);
void imgui_set_next_window_size(ImGuiContext *imgui_c89_ctx, const ImVec2 * size, ImGuiCond cond);
void imgui_set_next_window_size_constraints(ImGuiContext *imgui_c89_ctx, const ImVec2 * size_min, const ImVec2 * size_max, ImGuiSizeCallback custom_callback, void * custom_callback_user_data);
void imgui_set_scroll_from_pos_x(ImGuiContext *imgui_c89_ctx, float local_x, float center_x_ratio);
void imgui_set_scroll_from_pos_y(ImGuiContext *imgui_c89_ctx, float local_y, float center_y_ratio);
void imgui_set_scroll_here_x(ImGuiContext *imgui_c89_ctx, float center_x_ratio);
void imgui_set_scroll_here_y(ImGuiContext *imgui_c89_ctx, float center_y_ratio);
void imgui_set_scroll_x(ImGuiContext *imgui_c89_ctx, float scroll_x);
void imgui_set_scroll_y(ImGuiContext *imgui_c89_ctx, float scroll_y);
void imgui_set_state_storage(ImGuiContext *imgui_c89_ctx, ImGuiStorage * tree);
void imgui_set_tab_item_closed(ImGuiContext *imgui_c89_ctx, const char * label);
void imgui_set_tooltip_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_set_window_collapsed_bool_cond_none(ImGuiContext *imgui_c89_ctx, unsigned char collapsed, ImGuiCond cond);
void imgui_set_window_collapsed_string_bool_cond(ImGuiContext *imgui_c89_ctx, const char * name, unsigned char collapsed, ImGuiCond cond);
void imgui_set_window_focus_none(ImGuiContext *imgui_c89_ctx);
void imgui_set_window_focus_string(ImGuiContext *imgui_c89_ctx, const char * name);
void imgui_set_window_font_scale(ImGuiContext *imgui_c89_ctx, float scale);
void imgui_set_window_pos_string_vec2_cond(ImGuiContext *imgui_c89_ctx, const char * name, const ImVec2 * pos, ImGuiCond cond);
void imgui_set_window_pos_vec2_cond_none(ImGuiContext *imgui_c89_ctx, const ImVec2 * pos, ImGuiCond cond);
void imgui_set_window_size_string_vec2_cond(ImGuiContext *imgui_c89_ctx, const char * name, const ImVec2 * size, ImGuiCond cond);
void imgui_set_window_size_vec2_cond_none(ImGuiContext *imgui_c89_ctx, const ImVec2 * size, ImGuiCond cond);
unsigned char imgui_shortcut(ImGuiContext *imgui_c89_ctx, ImGuiKeyChord key_chord, ImGuiInputFlags flags);
void imgui_show_debug_log_window(ImGuiContext *imgui_c89_ctx, unsigned char * p_open);
void imgui_show_font_selector(ImGuiContext *imgui_c89_ctx, const char * label);
void imgui_show_id_stack_tool_window(ImGuiContext *imgui_c89_ctx, unsigned char * p_open);
void imgui_show_metrics_window(ImGuiContext *imgui_c89_ctx, unsigned char * p_open);
unsigned char imgui_slider_angle(ImGuiContext *imgui_c89_ctx, const char * label, float * v_rad, float v_degrees_min, float v_degrees_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_float(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_float2(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_float3(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_float4(ImGuiContext *imgui_c89_ctx, const char * label, float * v, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_int(ImGuiContext *imgui_c89_ctx, const char * label, int * v, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_int2(ImGuiContext *imgui_c89_ctx, const char * label, int * v, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_int3(ImGuiContext *imgui_c89_ctx, const char * label, int * v, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_int4(ImGuiContext *imgui_c89_ctx, const char * label, int * v, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_scalar(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiDataType data_type, void * p_data, const void * p_min, const void * p_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_slider_scalar_n(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiDataType data_type, void * v, int components, const void * v_min, const void * v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_small_button(ImGuiContext *imgui_c89_ctx, const char * label);
void imgui_spacing(ImGuiContext *imgui_c89_ctx);
void imgui_storage_build_sort_by_key(ImGuiStorage *self);
void imgui_storage_clear(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self);
unsigned char imgui_storage_get_bool(ImGuiStorage *self, ImGuiID key, unsigned char default_val);
unsigned char * imgui_storage_get_bool_ref(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, unsigned char default_val);
float imgui_storage_get_float(ImGuiStorage *self, ImGuiID key, float default_val);
float * imgui_storage_get_float_ref(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, float default_val);
int imgui_storage_get_int(ImGuiStorage *self, ImGuiID key, int default_val);
int * imgui_storage_get_int_ref(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, int default_val);
void * imgui_storage_get_void_ptr(ImGuiStorage *self, ImGuiID key);
void ** imgui_storage_get_void_ptr_ref(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, void * default_val);
void imgui_storage_set_all_int(ImGuiStorage *self, int v);
void imgui_storage_set_bool(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, unsigned char val);
void imgui_storage_set_float(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, float val);
void imgui_storage_set_int(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, int val);
void imgui_storage_set_void_ptr(ImGuiContext *imgui_c89_ctx, ImGuiStorage *self, ImGuiID key, void * val);
void imgui_style_colors_classic(ImGuiContext *imgui_c89_ctx, ImGuiStyle * dst);
void imgui_style_colors_dark(ImGuiContext *imgui_c89_ctx, ImGuiStyle * dst);
void imgui_style_colors_light(ImGuiContext *imgui_c89_ctx, ImGuiStyle * dst);
void imgui_style_scale_all_sizes(ImGuiStyle *self, float scale_factor);
unsigned char imgui_tab_item_button(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiTabItemFlags flags);
void imgui_table_angled_headers_row(ImGuiContext *imgui_c89_ctx);
int imgui_table_get_column_count(ImGuiContext *imgui_c89_ctx);
ImGuiTableColumnFlags imgui_table_get_column_flags(ImGuiContext *imgui_c89_ctx, int column_n);
int imgui_table_get_column_index(ImGuiContext *imgui_c89_ctx);
const char * imgui_table_get_column_name(ImGuiContext *imgui_c89_ctx, int column_n);
int imgui_table_get_hovered_column(ImGuiContext *imgui_c89_ctx);
int imgui_table_get_row_index(ImGuiContext *imgui_c89_ctx);
ImGuiTableSortSpecs * imgui_table_get_sort_specs(ImGuiContext *imgui_c89_ctx);
void imgui_table_header(ImGuiContext *imgui_c89_ctx, const char * label);
void imgui_table_headers_row(ImGuiContext *imgui_c89_ctx);
unsigned char imgui_table_next_column(ImGuiContext *imgui_c89_ctx);
void imgui_table_next_row(ImGuiContext *imgui_c89_ctx, ImGuiTableRowFlags row_flags, float row_min_height);
void imgui_table_set_bg_color(ImGuiContext *imgui_c89_ctx, ImGuiTableBgTarget target, ImU32 color, int column_n);
void imgui_table_set_column_enabled(ImGuiContext *imgui_c89_ctx, int column_n, unsigned char enabled);
unsigned char imgui_table_set_column_index(ImGuiContext *imgui_c89_ctx, int column_n);
void imgui_table_setup_column(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiTableColumnFlags flags, float init_width_or_weight, ImGuiID user_data);
void imgui_table_setup_scroll_freeze(ImGuiContext *imgui_c89_ctx, int columns, int rows);
void imgui_text_buffer_append(ImGuiContext *imgui_c89_ctx, ImGuiTextBuffer *self, const char * str, const char * str_end);
void imgui_text_buffer_appendfv(ImGuiContext *imgui_c89_ctx, ImGuiTextBuffer *self, const char * fmt, va_list args);
const char * imgui_text_buffer_begin(ImGuiTextBuffer *self);
const char * imgui_text_buffer_c_str(ImGuiTextBuffer *self);
void imgui_text_buffer_clear(ImGuiContext *imgui_c89_ctx, ImGuiTextBuffer *self);
unsigned char imgui_text_buffer_empty(ImGuiTextBuffer *self);
const char * imgui_text_buffer_end(ImGuiTextBuffer *self);
void imgui_text_buffer_reserve(ImGuiContext *imgui_c89_ctx, ImGuiTextBuffer *self, int capacity);
void imgui_text_buffer_resize(ImGuiContext *imgui_c89_ctx, ImGuiTextBuffer *self, int size);
int imgui_text_buffer_size(ImGuiTextBuffer *self);
void imgui_text_colored_v(ImGuiContext *imgui_c89_ctx, const ImVec4 * col, const char * fmt, va_list args);
void imgui_text_disabled_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_text_filter_build(ImGuiContext *imgui_c89_ctx, ImGuiTextFilter *self);
void imgui_text_filter_clear(ImGuiContext *imgui_c89_ctx, ImGuiTextFilter *self);
unsigned char imgui_text_filter_draw(ImGuiContext *imgui_c89_ctx, ImGuiTextFilter *self, const char * label, float width);
unsigned char imgui_text_filter_is_active(ImGuiTextFilter *self);
unsigned char imgui_text_filter_pass_filter(ImGuiTextFilter *self, const char * text, const char * text_end);
unsigned char imgui_text_link(ImGuiContext *imgui_c89_ctx, const char * label);
unsigned char imgui_text_link_open_url(ImGuiContext *imgui_c89_ctx, const char * label, const char * url);
unsigned char imgui_text_range_empty(ImGuiTextFilter_ImGuiTextRange *self);
void imgui_text_range_split(ImGuiContext *imgui_c89_ctx, ImGuiTextFilter_ImGuiTextRange *self, char separator, ImVector_ImGuiTextFilter_ImGuiTextRange * out);
void imgui_text_unformatted(ImGuiContext *imgui_c89_ctx, const char * text, const char * text_end);
void imgui_text_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_text_wrapped_v(ImGuiContext *imgui_c89_ctx, const char * fmt, va_list args);
void imgui_texture_data_create(ImGuiContext *imgui_c89_ctx, ImTextureData *self, ImTextureFormat format, int w, int h);
void imgui_texture_data_destroy_pixels(ImGuiContext *imgui_c89_ctx, ImTextureData *self);
int imgui_texture_data_get_pitch(ImTextureData *self);
void * imgui_texture_data_get_pixels(ImTextureData *self);
void * imgui_texture_data_get_pixels_at(ImTextureData *self, int x, int y);
int imgui_texture_data_get_size_in_bytes(ImTextureData *self);
ImTextureID imgui_texture_data_get_tex_id(ImTextureData *self);
ImTextureRef imgui_texture_data_get_tex_ref(ImTextureData *self);
void imgui_texture_data_set_status(ImTextureData *self, ImTextureStatus status);
void imgui_texture_data_set_tex_id(ImTextureData *self, ImTextureID tex_id);
ImTextureID imgui_texture_ref_get_tex_id(ImTextureRef *self);
unsigned char imgui_tree_node(ImGuiContext *imgui_c89_ctx, const char * label);
unsigned char imgui_tree_node_ex(ImGuiContext *imgui_c89_ctx, const char * label, ImGuiTreeNodeFlags flags);
unsigned char imgui_tree_node_ex_v_const_pointer(ImGuiContext *imgui_c89_ctx, const void * ptr_id, ImGuiTreeNodeFlags flags, const char * fmt, va_list args);
unsigned char imgui_tree_node_ex_v_string(ImGuiContext *imgui_c89_ctx, const char * str_id, ImGuiTreeNodeFlags flags, const char * fmt, va_list args);
unsigned char imgui_tree_node_get_open(ImGuiContext *imgui_c89_ctx, ImGuiID storage_id);
unsigned char imgui_tree_node_v_const_pointer(ImGuiContext *imgui_c89_ctx, const void * ptr_id, const char * fmt, va_list args);
unsigned char imgui_tree_node_v_string(ImGuiContext *imgui_c89_ctx, const char * str_id, const char * fmt, va_list args);
void imgui_tree_pop(ImGuiContext *imgui_c89_ctx);
void imgui_tree_push_const_pointer(ImGuiContext *imgui_c89_ctx, const void * ptr_id);
void imgui_tree_push_string(ImGuiContext *imgui_c89_ctx, const char * str_id);
void imgui_unindent(ImGuiContext *imgui_c89_ctx, float indent_w);
unsigned char imgui_v_slider_float(ImGuiContext *imgui_c89_ctx, const char * label, const ImVec2 * size, float * v, float v_min, float v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_v_slider_int(ImGuiContext *imgui_c89_ctx, const char * label, const ImVec2 * size, int * v, int v_min, int v_max, const char * format, ImGuiSliderFlags flags);
unsigned char imgui_v_slider_scalar(ImGuiContext *imgui_c89_ctx, const char * label, const ImVec2 * size, ImGuiDataType data_type, void * p_data, const void * p_min, const void * p_max, const char * format, ImGuiSliderFlags flags);
void imgui_value_bool_none(ImGuiContext *imgui_c89_ctx, const char * prefix, unsigned char b);
void imgui_value_float_string(ImGuiContext *imgui_c89_ctx, const char * prefix, float v, const char * float_format);
void imgui_value_int_none(ImGuiContext *imgui_c89_ctx, const char * prefix, int v);
void imgui_value_uint_none(ImGuiContext *imgui_c89_ctx, const char * prefix, unsigned int v);
ImVec2 imgui_viewport_get_center(ImGuiViewport *self);
ImVec2 imgui_viewport_get_work_center(ImGuiViewport *self);

#ifdef __cplusplus
}
#endif

#endif /* IMGUI_C89_USE_CPP_TYPES */
#endif
