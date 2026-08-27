#ifndef IMGUI_C89_INTERNAL_H
#define IMGUI_C89_INTERNAL_H

#include "../include/imgui_c89.h"
#include "../include/imgui_c89_platform.h"
#include "../include/imgui_c89_render.h"
#include "../include/imgui_c89_font.h"
#include "../include/imgui_c89_trace.h"

#define IMGUI_INTERNAL_SCOPE_CAPACITY 128
#define IMGUI_INTERNAL_ID_CAPACITY 64
#define IMGUI_INTERNAL_TREE_CAPACITY 2048
#define IMGUI_INTERNAL_NAV_ITEM_CAPACITY 256
#define IMGUI_INTERNAL_TABLE_COLUMN_CAPACITY 256
#define IMGUI_INTERNAL_DRAW_CHANNEL_CAPACITY 32
#define IMGUI_INTERNAL_WINDOW_CAPACITY 32
#define IMGUI_INTERNAL_DOCK_CAPACITY 32
#define IMGUI_INTERNAL_VIEWPORT_CAPACITY 32
#define IMGUI_INTERNAL_CHILD_CAPACITY 512
#define IMGUI_INTERNAL_PATH_CAPACITY 256
#define IMGUI_INTERNAL_POPUP_CAPACITY 16

typedef enum imgui_internal_frame_state {
    IMGUI_INTERNAL_FRAME_IDLE = 0,
    IMGUI_INTERNAL_FRAME_BUILDING = 1,
    IMGUI_INTERNAL_FRAME_ENDED = 2,
    IMGUI_INTERNAL_FRAME_RENDERED = 3
} imgui_internal_frame_state;

typedef enum imgui_internal_scope_kind {
    IMGUI_INTERNAL_SCOPE_WINDOW = 1,
    IMGUI_INTERNAL_SCOPE_CHILD = 2,
    IMGUI_INTERNAL_SCOPE_GROUP = 3,
    IMGUI_INTERNAL_SCOPE_COMBO = 4,
    IMGUI_INTERNAL_SCOPE_LIST_BOX = 5,
    IMGUI_INTERNAL_SCOPE_MENU_BAR = 6,
    IMGUI_INTERNAL_SCOPE_MENU = 7,
    IMGUI_INTERNAL_SCOPE_POPUP = 8,
    IMGUI_INTERNAL_SCOPE_TOOLTIP = 9,
    IMGUI_INTERNAL_SCOPE_TREE = 10,
    IMGUI_INTERNAL_SCOPE_TAB_BAR = 11,
    IMGUI_INTERNAL_SCOPE_TAB_ITEM = 12,
    IMGUI_INTERNAL_SCOPE_TABLE = 13,
    IMGUI_INTERNAL_SCOPE_DRAG_SOURCE = 14,
    IMGUI_INTERNAL_SCOPE_DRAG_TARGET = 15
} imgui_internal_scope_kind;

typedef struct imgui_internal_input_state {
    float mouse_x;
    float mouse_y;
    float mouse_wheel_x;
    float mouse_wheel_y;
    imgui_bool mouse_down[IMGUI_MOUSE_BUTTON_COUNT];
    imgui_bool mouse_down_previous[IMGUI_MOUSE_BUTTON_COUNT];
    imgui_bool keys_down[IMGUI_KEY_COUNT];
    imgui_bool keys_down_previous[IMGUI_KEY_COUNT];
    imgui_bool keys_pressed[IMGUI_KEY_COUNT];
    imgui_bool keys_released[IMGUI_KEY_COUNT];
    imgui_bool keys_repeated[IMGUI_KEY_COUNT];
    float key_down_duration[IMGUI_KEY_COUNT];
    float key_analog[IMGUI_KEY_COUNT];
    imgui_bool focused;
} imgui_internal_input_state;

struct imgui_draw_list {
    imgui_context *owner;
};

struct imgui_texture {
    imgui_texture_id backend_id;
    imgui_u32 identity;
    imgui_texture_desc desc;
    imgui_bool external;
    imgui_bool alive;
    imgui_context *owner;
    struct imgui_texture *next;
};

typedef struct imgui_internal_window {
    imgui_id id;
    imgui_vec2 position;
    imgui_vec2 size;
    imgui_window_flags flags;
    imgui_bool initialized;
    imgui_bool appearing;
    imgui_bool hidden_this_frame;
    imgui_bool auto_fit_pending;
    imgui_bool open;
    imgui_bool collapsed;
    imgui_vec2 expanded_size;
    float scroll_y;
    float scroll_max_y;
    float scroll_x;
    float scroll_max_x;
    char *title;
    size_t title_capacity;
    imgui_u32 z_order;
    imgui_u32 command_start;
    imgui_u32 command_end;
    imgui_id dock_id;
    imgui_id viewport_id;
    unsigned long settings_load_generation;
} imgui_internal_window;

typedef struct imgui_internal_viewport {
    imgui_viewport_desc desc;
    imgui_bool configured;
    imgui_bool platform_created;
} imgui_internal_viewport;

typedef struct imgui_internal_dock_node {
    imgui_id id;
    imgui_vec2 position;
    imgui_vec2 size;
    imgui_dock_flags flags;
    imgui_bool initialized;
    imgui_dock_split_direction split_direction;
    float split_ratio;
    imgui_id child_a;
    imgui_id child_b;
    imgui_id active_window_id;
} imgui_internal_dock_node;

typedef struct imgui_internal_table_width_state {
    imgui_id id;
    int columns;
    float *widths;
    int sort_column;
    int sort_direction;
    imgui_table_sort_spec *sort_specs;
    int sort_spec_count;
} imgui_internal_table_width_state;

typedef struct imgui_internal_window_scope_state {
    int window_index;
    imgui_vec2 origin;
    imgui_vec2 size;
    imgui_vec2 content_max;
    imgui_vec2 cursor;
    imgui_rect clip;
    imgui_window_flags flags;
    imgui_bool active;
    imgui_bool focused;
    float scroll_y;
    float scroll_x;
    float indent_width;
    float item_spacing;
    float line_spacing_override;
    imgui_bool line_spacing_override_valid;
    int child_current_index;
    imgui_flags child_flags;
    imgui_vec2 window_size_min;
    imgui_vec2 window_size_max;
    imgui_bool window_size_constraints_valid;
} imgui_internal_window_scope_state;

struct imgui_context {
    imgui_allocator allocator;
    imgui_error_fn error_callback;
    void *error_user_data;
    float key_repeat_delay;
    float key_repeat_rate;
    float double_click_time;
    float double_click_max_distance;
    float mouse_drag_threshold;
    imgui_internal_frame_state frame_state;
    imgui_u32 frame_index;
    imgui_frame_desc frame_desc;
    imgui_frame_output frame_output;
    imgui_style style;
    imgui_style *style_stack;
    int style_stack_count;
    int style_stack_capacity;
    imgui_internal_input_state input;
    imgui_bool mouse_clicked[IMGUI_MOUSE_BUTTON_COUNT];
    int mouse_clicked_count[IMGUI_MOUSE_BUTTON_COUNT];
    imgui_bool mouse_released[IMGUI_MOUSE_BUTTON_COUNT];
    imgui_bool mouse_double_clicked[IMGUI_MOUSE_BUTTON_COUNT];
    float mouse_down_duration[IMGUI_MOUSE_BUTTON_COUNT];
    imgui_vec2 mouse_down_start[IMGUI_MOUSE_BUTTON_COUNT];
    imgui_bool item_clicked_this_frame;
    double mouse_last_click_time[IMGUI_MOUSE_BUTTON_COUNT];
    imgui_vec2 mouse_last_click_position[IMGUI_MOUSE_BUTTON_COUNT];
    imgui_bool mouse_last_click_valid[IMGUI_MOUSE_BUTTON_COUNT];
    imgui_vec2 cursor;
    imgui_vec2 next_window_position;
    imgui_vec2 next_window_position_pivot;
    imgui_id next_window_viewport_id;
    imgui_vec2 next_window_size;
    imgui_vec2 next_window_content_size;
    imgui_vec2 next_window_size_min;
    imgui_vec2 next_window_size_max;
    imgui_vec2 window_size_min;
    imgui_vec2 window_size_max;
    float next_window_background_alpha;
    imgui_bool next_window_position_valid;
    imgui_bool next_window_position_pivot_valid;
    imgui_bool next_window_viewport_valid;
    imgui_bool next_window_size_valid;
    imgui_bool next_window_content_size_valid;
    imgui_bool next_window_size_constraints_valid;
    imgui_bool window_size_constraints_valid;
    imgui_bool next_window_background_alpha_valid;
    imgui_vec2 next_window_scroll;
    imgui_bool next_window_scroll_valid;
    imgui_bool next_window_collapsed_valid;
    imgui_bool next_window_collapsed;
    imgui_bool next_window_focus_valid;
    imgui_vec2 window_origin;
    imgui_vec2 window_size;
    float content_max_x;
    float content_max_y;
    float next_item_width;
    imgui_bool next_item_width_valid;
    imgui_key next_item_shortcut_key;
    imgui_key_modifiers next_item_shortcut_modifiers;
    imgui_bool next_item_shortcut_valid;
    imgui_bool next_item_open;
    imgui_bool next_item_open_valid;
    imgui_internal_window *windows;
    int window_count;
    int window_capacity;
    imgui_u32 next_window_z_order;
    imgui_bool window_z_order_dirty;
    int current_window_index;
    int last_window_index;
    imgui_id focused_window_id;
    imgui_bool focused_window_valid;
    imgui_internal_dock_node *dock_nodes;
    int dock_node_count;
    int dock_node_capacity;
    imgui_rect clip_rect;
    imgui_bool window_active;
    imgui_bool window_focused;
    imgui_bool frame_any_window_hovered;
    imgui_bool frame_any_window_focused;
    imgui_id moving_window_id;
    imgui_bool moving_window_valid;
    imgui_bool resizing_window;
    imgui_vec2 window_drag_mouse;
    imgui_vec2 window_drag_position;
    imgui_vec2 window_drag_size;
    imgui_bool window_drag_moved;
    imgui_id scrollbar_drag_window_id;
    imgui_bool scrollbar_drag_active;
    imgui_bool scrollbar_drag_child;
    imgui_bool scrollbar_drag_horizontal;
    float scrollbar_drag_mouse_start;
    float scrollbar_drag_scroll_start;
    imgui_window_flags window_flags;
    float item_spacing;
    float line_spacing_override;
    imgui_bool line_spacing_override_valid;
    float indent_width;
    float child_scroll_y;
    float child_scroll_x;
    imgui_id *child_ids;
    float *child_scrolls;
    float *child_scroll_maxs;
    float *child_scroll_xs;
    float *child_scroll_max_xs;
    float *child_auto_widths;
    float *child_auto_heights;
    int child_count;
    int child_capacity;
    int child_current_index;
    imgui_bool table_active;
    int table_columns;
    int table_current_column;
    imgui_id table_active_id;
    float table_start_x;
    float table_start_y;
    float table_content_start_x;
    float table_content_start_y;
    float table_freeze_origin_x;
    float table_freeze_origin_y;
    float table_frozen_height;
    int table_freeze_columns;
    int table_freeze_rows;
    float table_row_start_y;
    float table_row_height;
    imgui_u32 table_row_background_vertex_offset;
    imgui_bool table_row_background_valid;
    float table_column_width;
    imgui_table_flags table_flags;
    int table_row_index;
    float *table_column_widths;
    const char **table_column_labels;
    imgui_table_column_flags *table_column_flags;
    int table_column_capacity;
    int table_sort_column;
    int table_sort_direction;
    imgui_table_sort_spec *table_sort_specs;
    int table_sort_spec_count;
    int table_sort_spec_capacity;
    imgui_table_sort_specs table_sort_specs_view;
    imgui_internal_table_width_state *table_width_states;
    int table_width_state_count;
    int table_width_state_capacity;
    imgui_platform_desc platform;
    imgui_platform_output platform_output;
    imgui_renderer_desc renderer;
    imgui_log_desc log_desc;
    imgui_bool log_active;
    imgui_localization_desc localization;
    const imgui_font *font;
    imgui_texture *font_texture;
    imgui_font_atlas *font_atlases;
    imgui_internal_scope_kind *scopes;
    imgui_bool *scope_active;
    imgui_vec2 *scope_saved_cursor;
    imgui_vec2 *scope_saved_size;
    imgui_rect *scope_saved_clip;
    imgui_internal_window_scope_state *scope_saved_window;
    int *scope_saved_child_index;
    float *scope_saved_child_scroll;
    float *scope_saved_child_scroll_x;
    imgui_flags *scope_saved_child_flags;
    imgui_bool *scope_tree_pushed;
    imgui_flags child_flags;
    int scope_capacity;
    int scope_depth;
    int id_depth;
    imgui_id *id_stack;
    int id_capacity;
    imgui_id last_item_id;
    imgui_rect last_item_rect;
    imgui_rect last_item_clip_rect;
    imgui_bool last_item_hovered;
    imgui_bool last_item_disabled;
    imgui_bool last_item_active;
    imgui_bool last_item_activated;
    imgui_bool last_item_deactivated;
    imgui_bool last_item_deactivated_after_edit;
    imgui_bool last_item_edited;
    imgui_bool last_item_toggled_open;
    imgui_bool last_item_toggled_selection;
    imgui_bool last_item_clicked;
    imgui_bool any_item_hovered;
    imgui_bool any_item_active;
    imgui_bool any_item_focused;
    imgui_id active_item_id;
    imgui_bool active_item_valid;
    imgui_bool active_item_was_edited;
    imgui_id drag_value_id;
    imgui_bool drag_value_active;
    float drag_value_start;
    float drag_value_mouse_start;
    imgui_id previous_active_item_id;
    imgui_bool previous_active_item_valid;
    imgui_bool previous_active_item_was_edited;
    imgui_id focused_item_id;
    imgui_bool focused_item_valid;
    imgui_id navigation_window_id;
    imgui_bool navigation_focused_item_seen;
    imgui_bool navigation_tab_pending;
    imgui_bool navigation_reverse_pending;
    int navigation_spatial_direction;
    imgui_bool navigation_tab_seen_focus;
    imgui_id navigation_first_id;
    imgui_bool navigation_first_valid;
    imgui_id navigation_candidate_id;
    imgui_bool navigation_candidate_valid;
    imgui_id navigation_last_id;
    imgui_bool navigation_last_valid;
    imgui_id *navigation_item_ids;
    imgui_rect *navigation_item_rects;
    int navigation_item_capacity;
    int navigation_item_count;
    imgui_rect navigation_focused_rect;
    imgui_bool navigation_focused_rect_valid;
    imgui_id active_text_id;
    imgui_bool text_input_active;
    imgui_id text_scroll_id;
    float text_scroll_x;
    imgui_id text_edit_id;
    size_t text_cursor_byte;
    size_t text_selection_anchor_byte;
    size_t text_selection_start_byte;
    size_t text_selection_end_byte;
    imgui_id text_undo_id;
    size_t text_undo_length;
    imgui_bool text_undo_valid;
    char *text_undo_buffer;
    size_t text_undo_capacity;
    imgui_id text_redo_id;
    size_t text_redo_length;
    imgui_bool text_redo_valid;
    char *text_redo_buffer;
    size_t text_redo_capacity;
    imgui_id popup_id;
    imgui_bool popup_open;
    int popup_mouse_button;
    imgui_id popup_stack_ids[IMGUI_INTERNAL_POPUP_CAPACITY];
    imgui_rect popup_stack_rects[IMGUI_INTERNAL_POPUP_CAPACITY];
    imgui_bool popup_stack_rect_valid[IMGUI_INTERNAL_POPUP_CAPACITY];
    imgui_bool popup_stack_modal[IMGUI_INTERNAL_POPUP_CAPACITY];
    int popup_stack_count;
    imgui_u32 popup_render_start;
    imgui_u32 popup_render_end;
    imgui_bool popup_render_valid;
    imgui_u32 combo_render_start;
    imgui_u32 combo_render_end;
    imgui_bool combo_render_valid;
    imgui_bool combo_render_tracking;
    imgui_u32 tooltip_render_start;
    imgui_u32 tooltip_render_end;
    imgui_bool tooltip_render_valid;
    imgui_bool tooltip_render_tracking;
    imgui_flags tooltip_saved_window_flags;
    imgui_bool tooltip_saved_window_active;
    int list_box_child_index;
    imgui_rect list_box_rect;
    float list_box_scroll_y;
    float list_box_scroll_max_y;
    float list_box_saved_content_max_x;
    float list_box_saved_content_max_y;
    imgui_bool popup_modal;
    imgui_id current_popup_id;
    imgui_rect popup_rect;
    imgui_vec2 popup_opening_mouse_position;
    imgui_bool popup_rect_valid;
    imgui_u32 popup_background_vertex_offset;
    imgui_bool popup_background_active;
    imgui_rect tooltip_rect;
    imgui_u32 tooltip_background_vertex_offset;
    imgui_bool tooltip_background_active;
    imgui_id tab_bar_id;
    imgui_id tab_active_id;
    int tab_bar_item_count;
    imgui_vec2 tab_bar_row_start;
    float tab_bar_next_x;
    float tab_bar_row_y;
    imgui_vec2 tab_bar_content_cursor;
    imgui_bool tab_bar_content_valid;
    imgui_multi_select_storage *multi_select_storage;
    imgui_multi_select_flags multi_select_flags;
    imgui_bool multi_select_active;
    imgui_id multi_select_anchor_id;
    imgui_multi_select_storage *multi_select_anchor_storage;
    imgui_id *multi_select_frame_ids;
    int multi_select_frame_count;
    int multi_select_frame_capacity;
    imgui_bool drag_payload_active;
    imgui_bool drag_source_scope_active;
    imgui_drag_drop_source_flags drag_source_flags;
    imgui_bool drag_target_scope_active;
    imgui_bool drag_target_rect_drawn;
    imgui_id drag_payload_source_id;
    imgui_u32 drag_payload_source_frame;
    imgui_drag_payload drag_payload;
    char *drag_payload_type;
    size_t drag_payload_type_capacity;
    void *drag_payload_data;
    size_t drag_payload_capacity;
    imgui_id *tree_ids;
    imgui_bool *tree_open;
    imgui_bool *tree_initialized;
    int tree_count;
    int tree_capacity;
    char *numeric_buffer;
    size_t numeric_buffer_capacity;
    imgui_id numeric_buffer_id;
    imgui_bool numeric_buffer_valid;
    char *pending_text;
    size_t pending_text_length;
    size_t pending_text_capacity;
    imgui_bool demo_enabled;
    int demo_integer;
    float demo_float;
    float demo_color[4];
    imgui_bool path_active;
    imgui_vec2 *path_points;
    int path_count;
    int path_capacity;
    int disabled_depth;
    int focus_request;
    imgui_draw_list default_draw_list;
    imgui_render_packet packet;
    imgui_render_list render_list;
    imgui_viewport_packet viewport;
    imgui_internal_viewport viewport_configs[IMGUI_INTERNAL_VIEWPORT_CAPACITY];
    imgui_viewport_packet viewport_packets[IMGUI_INTERNAL_VIEWPORT_CAPACITY];
    imgui_render_list *viewport_lists[IMGUI_INTERNAL_VIEWPORT_CAPACITY];
    int viewport_list_capacity;
    int viewport_count;
    imgui_render_command *commands;
    imgui_u32 command_count;
    imgui_u32 command_capacity;
    void **command_payloads;
    imgui_bool draw_channels_active;
    int draw_channel_count;
    int draw_channel_current;
    imgui_render_command *draw_channel_commands[
        IMGUI_INTERNAL_DRAW_CHANNEL_CAPACITY];
    imgui_u32 draw_channel_command_counts[
        IMGUI_INTERNAL_DRAW_CHANNEL_CAPACITY];
    imgui_u32 draw_channel_command_capacities[
        IMGUI_INTERNAL_DRAW_CHANNEL_CAPACITY];
    void **draw_channel_payloads[IMGUI_INTERNAL_DRAW_CHANNEL_CAPACITY];
    imgui_render_vertex *vertices;
    imgui_render_index *indices;
    imgui_u32 vertex_count;
    imgui_u32 vertex_capacity;
    imgui_u32 index_count;
    imgui_u32 index_capacity;
    imgui_resource_operation *resource_operations;
    void **resource_payloads;
    imgui_u32 resource_operation_count;
    imgui_u32 resource_operation_capacity;
    imgui_u32 resource_operation_consumed_count;
    imgui_texture *textures;
    imgui_u32 next_texture_identity;
    unsigned long settings_load_generation;
};

void imgui_internal_report(imgui_context *ctx,
                           imgui_error_code code,
                           const char *message);
imgui_bool imgui_internal_require_building(imgui_context *ctx,
                                           const char *operation);
imgui_scope imgui_internal_scope_begin(imgui_context *ctx,
                                       imgui_internal_scope_kind kind,
                                       imgui_bool active);
void imgui_internal_scope_end(imgui_context *ctx,
                              imgui_internal_scope_kind expected);
void *imgui_internal_allocate(const imgui_allocator *allocator, size_t size);
void imgui_internal_release(const imgui_allocator *allocator, void *memory);
void imgui_font_atlas_detach_context(imgui_context *ctx);

#endif
