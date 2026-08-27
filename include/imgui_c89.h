#ifndef IMGUI_C89_H
#define IMGUI_C89_H

/*
 * First-class C89 API for a behavioral port of Dear ImGui.
 *
 * This header intentionally does not preserve the Dear ImGui C++ ABI or the
 * generated cimgui API. See docs/DESIGN.md for the governing contract.
 */

#include <limits.h>
#include <stddef.h>

#if defined(_WIN32) && defined(IMGUI_C89_SHARED)
# if defined(IMGUI_C89_BUILD)
#  define IMGUI_API __declspec(dllexport)
# else
#  define IMGUI_API __declspec(dllimport)
# endif
#elif defined(__GNUC__) && defined(IMGUI_C89_SHARED)
# define IMGUI_API __attribute__((visibility("default")))
#else
# define IMGUI_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define IMGUI_VERSION_STRING "0.1.0-c89-port"
#define IMGUI_VERSION_NUMBER 100UL

#define IMGUI_FALSE 0
#define IMGUI_TRUE 1

typedef int imgui_bool;

#if UINT_MAX == 0xffffffffUL
typedef unsigned int imgui_u32;
typedef signed int imgui_s32;
#elif ULONG_MAX == 0xffffffffUL
typedef unsigned long imgui_u32;
typedef signed long imgui_s32;
#else
# error "imgui_c89 requires a native 32-bit integer type"
#endif

typedef unsigned short imgui_u16;
typedef signed short imgui_s16;
typedef unsigned char imgui_u8;
typedef signed char imgui_s8;

typedef char imgui_check_u32_size[(sizeof(imgui_u32) == 4) ? 1 : -1];
typedef char imgui_check_u16_size[(sizeof(imgui_u16) == 2) ? 1 : -1];

typedef imgui_u32 imgui_id;
typedef imgui_u32 imgui_flags;
typedef void *imgui_texture_id;

typedef struct imgui_context imgui_context;
typedef struct imgui_texture imgui_texture;
typedef struct imgui_draw_list imgui_draw_list;
typedef struct imgui_render_packet imgui_render_packet;

typedef struct imgui_vec2 {
    float x;
    float y;
} imgui_vec2;

typedef struct imgui_vec4 {
    float x;
    float y;
    float z;
    float w;
} imgui_vec4;

typedef struct imgui_rect {
    float x1;
    float y1;
    float x2;
    float y2;
} imgui_rect;

typedef enum imgui_result {
    IMGUI_RESULT_OK = 0,
    IMGUI_RESULT_INVALID_ARGUMENT = 1,
    IMGUI_RESULT_INVALID_STATE = 2,
    IMGUI_RESULT_OUT_OF_MEMORY = 3,
    IMGUI_RESULT_UNSUPPORTED = 4,
    IMGUI_RESULT_NOT_IMPLEMENTED = 5,
    IMGUI_RESULT_CORRUPT_DATA = 6
} imgui_result;

typedef enum imgui_error_code {
    IMGUI_ERROR_NONE = 0,
    IMGUI_ERROR_INVALID_ARGUMENT = 1,
    IMGUI_ERROR_INVALID_STATE = 2,
    IMGUI_ERROR_SCOPE_MISMATCH = 3,
    IMGUI_ERROR_UNCLOSED_SCOPE = 4,
    IMGUI_ERROR_OUT_OF_MEMORY = 5,
    IMGUI_ERROR_UNSUPPORTED = 6,
    IMGUI_ERROR_NOT_IMPLEMENTED = 7,
    IMGUI_ERROR_CORRUPT_DATA = 8
} imgui_error_code;

typedef enum imgui_scope {
    IMGUI_SCOPE_ERROR = -1,
    IMGUI_SCOPE_INACTIVE = 0,
    IMGUI_SCOPE_ACTIVE = 1
} imgui_scope;

typedef void *(*imgui_allocate_fn)(size_t size, void *user_data);
typedef void (*imgui_release_fn)(void *memory, void *user_data);
typedef void (*imgui_error_fn)(imgui_error_code code,
                               const char *message,
                               void *user_data);

typedef void (*imgui_log_callback)(const char *text,
                                   size_t length,
                                   void *user_data);

typedef struct imgui_log_desc {
    size_t struct_size;
    imgui_log_callback callback;
    void *user_data;
} imgui_log_desc;

IMGUI_API void imgui_log_desc_init(imgui_log_desc *desc);

typedef const char *(*imgui_localize_callback)(const char *key,
                                                size_t length,
                                                void *user_data);

typedef struct imgui_localization_desc {
    size_t struct_size;
    imgui_localize_callback callback;
    void *user_data;
} imgui_localization_desc;

IMGUI_API void imgui_localization_desc_init(
    imgui_localization_desc *desc);

typedef struct imgui_allocator {
    imgui_allocate_fn allocate;
    imgui_release_fn release;
    void *user_data;
} imgui_allocator;

typedef struct imgui_config {
    size_t struct_size;
    imgui_allocator allocator;
    imgui_error_fn error_callback;
    void *error_user_data;
    const char *settings_filename;
    float double_click_time;
    float double_click_max_distance;
    float mouse_drag_threshold;
    float key_repeat_delay;
    float key_repeat_rate;
    imgui_flags flags;
} imgui_config;

typedef enum imgui_mouse_button {
    IMGUI_MOUSE_BUTTON_LEFT = 0,
    IMGUI_MOUSE_BUTTON_RIGHT = 1,
    IMGUI_MOUSE_BUTTON_MIDDLE = 2,
    IMGUI_MOUSE_BUTTON_EXTRA_1 = 3,
    IMGUI_MOUSE_BUTTON_EXTRA_2 = 4,
    IMGUI_MOUSE_BUTTON_COUNT = 5
} imgui_mouse_button;

typedef enum imgui_key {
    IMGUI_KEY_NONE = 0,
    IMGUI_KEY_TAB,
    IMGUI_KEY_LEFT_ARROW,
    IMGUI_KEY_RIGHT_ARROW,
    IMGUI_KEY_UP_ARROW,
    IMGUI_KEY_DOWN_ARROW,
    IMGUI_KEY_PAGE_UP,
    IMGUI_KEY_PAGE_DOWN,
    IMGUI_KEY_HOME,
    IMGUI_KEY_END,
    IMGUI_KEY_INSERT,
    IMGUI_KEY_DELETE,
    IMGUI_KEY_BACKSPACE,
    IMGUI_KEY_SPACE,
    IMGUI_KEY_ENTER,
    IMGUI_KEY_ESCAPE,
    IMGUI_KEY_APOSTROPHE,
    IMGUI_KEY_COMMA,
    IMGUI_KEY_MINUS,
    IMGUI_KEY_PERIOD,
    IMGUI_KEY_SLASH,
    IMGUI_KEY_SEMICOLON,
    IMGUI_KEY_EQUAL,
    IMGUI_KEY_LEFT_BRACKET,
    IMGUI_KEY_BACKSLASH,
    IMGUI_KEY_RIGHT_BRACKET,
    IMGUI_KEY_GRAVE_ACCENT,
    IMGUI_KEY_CAPS_LOCK,
    IMGUI_KEY_SCROLL_LOCK,
    IMGUI_KEY_NUM_LOCK,
    IMGUI_KEY_PRINT_SCREEN,
    IMGUI_KEY_PAUSE,
    IMGUI_KEY_0,
    IMGUI_KEY_1,
    IMGUI_KEY_2,
    IMGUI_KEY_3,
    IMGUI_KEY_4,
    IMGUI_KEY_5,
    IMGUI_KEY_6,
    IMGUI_KEY_7,
    IMGUI_KEY_8,
    IMGUI_KEY_9,
    IMGUI_KEY_A,
    IMGUI_KEY_B,
    IMGUI_KEY_C,
    IMGUI_KEY_D,
    IMGUI_KEY_E,
    IMGUI_KEY_F,
    IMGUI_KEY_G,
    IMGUI_KEY_H,
    IMGUI_KEY_I,
    IMGUI_KEY_J,
    IMGUI_KEY_K,
    IMGUI_KEY_L,
    IMGUI_KEY_M,
    IMGUI_KEY_N,
    IMGUI_KEY_O,
    IMGUI_KEY_P,
    IMGUI_KEY_Q,
    IMGUI_KEY_R,
    IMGUI_KEY_S,
    IMGUI_KEY_T,
    IMGUI_KEY_U,
    IMGUI_KEY_V,
    IMGUI_KEY_W,
    IMGUI_KEY_X,
    IMGUI_KEY_Y,
    IMGUI_KEY_Z,
    IMGUI_KEY_F1,
    IMGUI_KEY_F2,
    IMGUI_KEY_F3,
    IMGUI_KEY_F4,
    IMGUI_KEY_F5,
    IMGUI_KEY_F6,
    IMGUI_KEY_F7,
    IMGUI_KEY_F8,
    IMGUI_KEY_F9,
    IMGUI_KEY_F10,
    IMGUI_KEY_F11,
    IMGUI_KEY_F12,
    IMGUI_KEY_LEFT_CTRL,
    IMGUI_KEY_LEFT_SHIFT,
    IMGUI_KEY_LEFT_ALT,
    IMGUI_KEY_LEFT_SUPER,
    IMGUI_KEY_RIGHT_CTRL,
    IMGUI_KEY_RIGHT_SHIFT,
    IMGUI_KEY_RIGHT_ALT,
    IMGUI_KEY_RIGHT_SUPER,
    IMGUI_KEY_MENU,
    IMGUI_KEY_GAMEPAD_START,
    IMGUI_KEY_GAMEPAD_BACK,
    IMGUI_KEY_GAMEPAD_FACE_LEFT,
    IMGUI_KEY_GAMEPAD_FACE_RIGHT,
    IMGUI_KEY_GAMEPAD_FACE_UP,
    IMGUI_KEY_GAMEPAD_FACE_DOWN,
    IMGUI_KEY_GAMEPAD_DPAD_LEFT,
    IMGUI_KEY_GAMEPAD_DPAD_RIGHT,
    IMGUI_KEY_GAMEPAD_DPAD_UP,
    IMGUI_KEY_GAMEPAD_DPAD_DOWN,
    IMGUI_KEY_GAMEPAD_L1,
    IMGUI_KEY_GAMEPAD_R1,
    IMGUI_KEY_GAMEPAD_L2,
    IMGUI_KEY_GAMEPAD_R2,
    IMGUI_KEY_GAMEPAD_L3,
    IMGUI_KEY_GAMEPAD_R3,
    IMGUI_KEY_GAMEPAD_LSTICK_LEFT,
    IMGUI_KEY_GAMEPAD_LSTICK_RIGHT,
    IMGUI_KEY_GAMEPAD_LSTICK_UP,
    IMGUI_KEY_GAMEPAD_LSTICK_DOWN,
    IMGUI_KEY_GAMEPAD_RSTICK_LEFT,
    IMGUI_KEY_GAMEPAD_RSTICK_RIGHT,
    IMGUI_KEY_GAMEPAD_RSTICK_UP,
    IMGUI_KEY_GAMEPAD_RSTICK_DOWN,
    IMGUI_KEY_COUNT
} imgui_key;

typedef imgui_flags imgui_key_modifiers;

enum {
    IMGUI_KEY_MOD_NONE = 0,
    IMGUI_KEY_MOD_CTRL = 1UL << 0,
    IMGUI_KEY_MOD_SHIFT = 1UL << 1,
    IMGUI_KEY_MOD_ALT = 1UL << 2,
    IMGUI_KEY_MOD_SUPER = 1UL << 3
};

typedef struct imgui_frame_desc {
    size_t struct_size;
    imgui_vec2 display_size;
    imgui_vec2 framebuffer_scale;
    float delta_time;
    double time;
} imgui_frame_desc;

typedef struct imgui_viewport_desc {
    size_t struct_size;
    imgui_id viewport_id;
    imgui_vec2 position;
    imgui_vec2 size;
    imgui_vec2 framebuffer_scale;
} imgui_viewport_desc;

typedef struct imgui_frame_output {
    imgui_bool want_capture_mouse;
    imgui_bool want_capture_keyboard;
    imgui_bool want_text_input;
    imgui_bool want_save_settings;
    imgui_bool navigation_active;
    imgui_bool navigation_visible;
    imgui_u32 frame_index;
    double next_wake_time;
} imgui_frame_output;

typedef struct imgui_style {
    size_t struct_size;
    imgui_vec2 window_padding;
    imgui_vec2 frame_padding;
    float item_spacing;
    float indent_spacing;
    float window_rounding;
    float frame_rounding;
    imgui_u32 color_text;
    imgui_u32 color_text_disabled;
    imgui_u32 color_window_background;
    imgui_u32 color_header;
    imgui_u32 color_header_hovered;
    imgui_u32 color_header_active;
    imgui_u32 color_separator;
    imgui_u32 color_check_mark;
    imgui_u32 color_button;
    imgui_u32 color_button_hovered;
    imgui_u32 color_button_active;
    imgui_u32 color_frame;
    imgui_u32 color_frame_hovered;
    imgui_u32 color_frame_active;
    /* Window decoration colors. Appended for struct-size compatibility. */
    imgui_u32 color_window_title_background;
    imgui_u32 color_window_title_text;
    imgui_u32 color_window_border;
    /* Scrollbar colors. Appended for struct-size compatibility. */
    imgui_u32 color_scrollbar_background;
    imgui_u32 color_scrollbar_grab;
    imgui_u32 color_scrollbar_grab_hovered;
    imgui_u32 color_scrollbar_grab_active;
    /* Table colors. Appended for struct-size compatibility. */
    imgui_u32 color_table_row_even;
    imgui_u32 color_table_row_odd;
    imgui_u32 color_table_border;
    imgui_u32 color_text_selection;
    /* Plot palette colors. Appended for legacy struct-size compatibility. */
    imgui_u32 color_plot_lines;
    imgui_u32 color_plot_histogram;
    /* Modal backdrop color. Appended for struct-size compatibility. */
    imgui_u32 color_modal_dim;
    /* Child-scope background color. Appended for struct-size compatibility. */
    imgui_u32 color_child_background;
    /* Title state colors. Appended for struct-size compatibility. */
    imgui_u32 color_window_title_background_active;
    imgui_u32 color_window_title_background_collapsed;
    /* Popup/overlay background color. Appended for struct-size compatibility. */
    imgui_u32 color_popup_background;
    /* Resize-grip colors. Appended for struct-size compatibility. */
    imgui_u32 color_resize_grip;
    imgui_u32 color_resize_grip_hovered;
    imgui_u32 color_resize_grip_active;
    /* Tab colors. Appended for struct-size compatibility. */
    imgui_u32 color_tab;
    imgui_u32 color_tab_hovered;
    imgui_u32 color_tab_active;
    imgui_u32 color_tab_active_hovered;
    imgui_u32 color_drag_drop_target;
    imgui_u32 color_nav_highlight;
    imgui_u32 color_menu_bar_background;
    imgui_u32 color_menu_item;
    imgui_u32 color_menu_item_hovered;
    imgui_u32 color_menu_item_active;
    imgui_u32 color_popup_border;
    /* Text-link palette colors. Appended for struct-size compatibility. */
    imgui_u32 color_text_link;
    imgui_u32 color_text_link_hovered;
    imgui_u32 color_text_link_active;
    /* Geometry controls appended for ABI-safe style extension. */
    float scrollbar_size;
    float scrollbar_grab_min_size;
    float child_rounding;
    float window_border_size;
    float child_border_size;
    float frame_border_size;
} imgui_style;

typedef enum imgui_style_color {
    IMGUI_STYLE_COLOR_TEXT = 0,
    IMGUI_STYLE_COLOR_TEXT_DISABLED,
    IMGUI_STYLE_COLOR_WINDOW_BACKGROUND,
    IMGUI_STYLE_COLOR_HEADER,
    IMGUI_STYLE_COLOR_HEADER_HOVERED,
    IMGUI_STYLE_COLOR_HEADER_ACTIVE,
    IMGUI_STYLE_COLOR_SEPARATOR,
    IMGUI_STYLE_COLOR_CHECK_MARK,
    IMGUI_STYLE_COLOR_BUTTON,
    IMGUI_STYLE_COLOR_BUTTON_HOVERED,
    IMGUI_STYLE_COLOR_BUTTON_ACTIVE,
    IMGUI_STYLE_COLOR_FRAME,
    IMGUI_STYLE_COLOR_FRAME_HOVERED,
    IMGUI_STYLE_COLOR_FRAME_ACTIVE,
    IMGUI_STYLE_COLOR_WINDOW_TITLE_BACKGROUND,
    IMGUI_STYLE_COLOR_WINDOW_TITLE_BACKGROUND_ACTIVE,
    IMGUI_STYLE_COLOR_WINDOW_TITLE_BACKGROUND_COLLAPSED,
    IMGUI_STYLE_COLOR_WINDOW_TITLE_TEXT,
    IMGUI_STYLE_COLOR_WINDOW_BORDER,
    IMGUI_STYLE_COLOR_SCROLLBAR_BACKGROUND,
    IMGUI_STYLE_COLOR_SCROLLBAR_GRAB,
    IMGUI_STYLE_COLOR_SCROLLBAR_GRAB_HOVERED,
    IMGUI_STYLE_COLOR_SCROLLBAR_GRAB_ACTIVE,
    IMGUI_STYLE_COLOR_TABLE_ROW_EVEN,
    IMGUI_STYLE_COLOR_TABLE_ROW_ODD,
    IMGUI_STYLE_COLOR_TABLE_BORDER,
    IMGUI_STYLE_COLOR_TEXT_SELECTION,
    IMGUI_STYLE_COLOR_PLOT_LINES,
    IMGUI_STYLE_COLOR_PLOT_HISTOGRAM,
    IMGUI_STYLE_COLOR_MODAL_DIM,
    IMGUI_STYLE_COLOR_CHILD_BACKGROUND,
    IMGUI_STYLE_COLOR_POPUP_BACKGROUND,
    IMGUI_STYLE_COLOR_RESIZE_GRIP,
    IMGUI_STYLE_COLOR_RESIZE_GRIP_HOVERED,
    IMGUI_STYLE_COLOR_RESIZE_GRIP_ACTIVE,
    IMGUI_STYLE_COLOR_TAB,
    IMGUI_STYLE_COLOR_TAB_HOVERED,
    IMGUI_STYLE_COLOR_TAB_ACTIVE,
    IMGUI_STYLE_COLOR_TAB_ACTIVE_HOVERED,
    IMGUI_STYLE_COLOR_DRAG_DROP_TARGET,
    IMGUI_STYLE_COLOR_NAV_HIGHLIGHT,
    IMGUI_STYLE_COLOR_MENU_BAR_BACKGROUND,
    IMGUI_STYLE_COLOR_MENU_ITEM,
    IMGUI_STYLE_COLOR_MENU_ITEM_HOVERED,
    IMGUI_STYLE_COLOR_MENU_ITEM_ACTIVE,
    IMGUI_STYLE_COLOR_POPUP_BORDER,
    IMGUI_STYLE_COLOR_TEXT_LINK,
    IMGUI_STYLE_COLOR_TEXT_LINK_HOVERED,
    IMGUI_STYLE_COLOR_TEXT_LINK_ACTIVE
} imgui_style_color;

typedef enum imgui_style_var {
    IMGUI_STYLE_VAR_ITEM_SPACING = 0,
    IMGUI_STYLE_VAR_INDENT_SPACING,
    IMGUI_STYLE_VAR_WINDOW_ROUNDING,
    IMGUI_STYLE_VAR_FRAME_ROUNDING,
    IMGUI_STYLE_VAR_WINDOW_PADDING,
    IMGUI_STYLE_VAR_FRAME_PADDING,
    IMGUI_STYLE_VAR_CHILD_ROUNDING,
    IMGUI_STYLE_VAR_WINDOW_BORDER_SIZE,
    IMGUI_STYLE_VAR_CHILD_BORDER_SIZE,
    IMGUI_STYLE_VAR_SCROLLBAR_SIZE,
    IMGUI_STYLE_VAR_SCROLLBAR_GRAB_MIN_SIZE,
    IMGUI_STYLE_VAR_FRAME_BORDER_SIZE
} imgui_style_var;

typedef struct imgui_metrics {
    size_t struct_size;
    imgui_u32 frame_index;
    imgui_u32 window_count;
    imgui_u32 open_window_count;
    imgui_u32 texture_count;
    imgui_u32 vertex_count;
    imgui_u32 index_count;
    imgui_u32 command_count;
    imgui_u32 resource_operation_count;
    int navigation_item_count;
    imgui_id active_item_id;
    imgui_id focused_item_id;
} imgui_metrics;

typedef imgui_flags imgui_window_flags;

enum {
    IMGUI_WINDOW_NONE = 0,
    IMGUI_WINDOW_NO_TITLE_BAR = 1UL << 0,
    IMGUI_WINDOW_NO_RESIZE = 1UL << 1,
    IMGUI_WINDOW_NO_MOVE = 1UL << 2,
    IMGUI_WINDOW_NO_SCROLLBAR = 1UL << 3,
    IMGUI_WINDOW_NO_COLLAPSE = 1UL << 4,
    IMGUI_WINDOW_ALWAYS_AUTO_RESIZE = 1UL << 5,
    IMGUI_WINDOW_NO_BACKGROUND = 1UL << 6,
    IMGUI_WINDOW_NO_SAVED_SETTINGS = 1UL << 7,
    IMGUI_WINDOW_NO_MOUSE_INPUTS = 1UL << 8,
    IMGUI_WINDOW_MENU_BAR = 1UL << 9,
    IMGUI_WINDOW_HORIZONTAL_SCROLLBAR = 1UL << 10,
    IMGUI_WINDOW_NO_FOCUS_ON_APPEARING = 1UL << 11,
    IMGUI_WINDOW_UNSAVED_DOCUMENT = 1UL << 12,
    IMGUI_WINDOW_MODAL = 1UL << 13,
    IMGUI_WINDOW_NO_SCROLL_WITH_MOUSE = 1UL << 14,
    IMGUI_WINDOW_NO_BRING_TO_FRONT_ON_FOCUS = 1UL << 15,
    IMGUI_WINDOW_ALWAYS_VERTICAL_SCROLLBAR = 1UL << 16,
    IMGUI_WINDOW_ALWAYS_HORIZONTAL_SCROLLBAR = 1UL << 17,
    IMGUI_WINDOW_NO_NAV_INPUTS = 1UL << 18,
    IMGUI_WINDOW_NO_NAV_FOCUS = 1UL << 19,
    IMGUI_WINDOW_NO_NAV = IMGUI_WINDOW_NO_NAV_INPUTS |
                          IMGUI_WINDOW_NO_NAV_FOCUS,
    IMGUI_WINDOW_NO_DECORATION = IMGUI_WINDOW_NO_TITLE_BAR |
                                 IMGUI_WINDOW_NO_RESIZE |
                                 IMGUI_WINDOW_NO_SCROLLBAR |
                                 IMGUI_WINDOW_NO_COLLAPSE,
    IMGUI_WINDOW_NO_INPUTS = IMGUI_WINDOW_NO_MOUSE_INPUTS |
                             IMGUI_WINDOW_NO_NAV
};

/* Child-scope options use a separate high-bit range so callers may continue
   passing legacy window-scroll flags without collisions. */
enum {
    IMGUI_CHILD_NONE = 0,
    IMGUI_CHILD_BORDER = 1UL << 16,
    IMGUI_CHILD_NO_SCROLLBAR = 1UL << 17,
    IMGUI_CHILD_HORIZONTAL_SCROLLBAR = 1UL << 18,
    IMGUI_CHILD_FRAME_STYLE = 1UL << 19,
    IMGUI_CHILD_ALWAYS_USE_WINDOW_PADDING = 1UL << 20,
    IMGUI_CHILD_RESIZE_X = 1UL << 21,
    IMGUI_CHILD_RESIZE_Y = 1UL << 22,
    IMGUI_CHILD_AUTO_RESIZE_X = 1UL << 23,
    IMGUI_CHILD_AUTO_RESIZE_Y = 1UL << 24,
    IMGUI_CHILD_ALWAYS_AUTO_RESIZE = 1UL << 25,
    IMGUI_CHILD_NAV_FLATTENED = 1UL << 26
};

typedef struct imgui_window_desc {
    size_t struct_size;
    const char *title;
    imgui_id id;
    imgui_bool use_explicit_id;
    imgui_bool *open;
    imgui_window_flags flags;
    imgui_bool has_position;
    imgui_vec2 position;
    imgui_bool has_size;
    imgui_vec2 size;
} imgui_window_desc;

typedef imgui_flags imgui_input_text_flags;

enum {
    IMGUI_INPUT_TEXT_NONE = 0,
    IMGUI_INPUT_TEXT_DECIMAL = 1UL << 0,
    IMGUI_INPUT_TEXT_HEXADECIMAL = 1UL << 1,
    IMGUI_INPUT_TEXT_UPPERCASE = 1UL << 2,
    IMGUI_INPUT_TEXT_NO_BLANK = 1UL << 3,
    IMGUI_INPUT_TEXT_AUTO_SELECT_ALL = 1UL << 4,
    IMGUI_INPUT_TEXT_ENTER_RETURNS_TRUE = 1UL << 5,
    IMGUI_INPUT_TEXT_READ_ONLY = 1UL << 6,
    IMGUI_INPUT_TEXT_PASSWORD = 1UL << 7,
    IMGUI_INPUT_TEXT_ALLOW_TAB = 1UL << 8,
    IMGUI_INPUT_TEXT_NO_UNDO_REDO = 1UL << 9,
    IMGUI_INPUT_TEXT_ESCAPE_CLEARS = 1UL << 10,
    IMGUI_INPUT_TEXT_WORD_WRAP = 1UL << 11,
    /* Appended so older descriptors retain their original flag meanings. */
    IMGUI_INPUT_TEXT_SCIENTIFIC = 1UL << 12,
    IMGUI_INPUT_TEXT_CTRL_ENTER_FOR_NEW_LINE = 1UL << 13,
    IMGUI_INPUT_TEXT_ALWAYS_OVERWRITE = 1UL << 14,
    IMGUI_INPUT_TEXT_NO_HORIZONTAL_SCROLL = 1UL << 15
};

typedef enum imgui_text_event_type {
    IMGUI_TEXT_EVENT_EDIT = 0,
    IMGUI_TEXT_EVENT_COMPLETION = 1,
    IMGUI_TEXT_EVENT_HISTORY = 2,
    IMGUI_TEXT_EVENT_FILTER_CHARACTER = 3,
    IMGUI_TEXT_EVENT_ALWAYS = 4
} imgui_text_event_type;

typedef struct imgui_text_event {
    imgui_text_event_type type;
    char *data;
    size_t length;
    size_t capacity;
    int cursor_byte;
    int selection_start_byte;
    int selection_end_byte;
    unsigned long input_codepoint;
    imgui_bool data_changed;
    void *user_data;
} imgui_text_event;

typedef int (*imgui_text_event_fn)(imgui_text_event *event);
typedef imgui_result (*imgui_text_reserve_fn)(void *user_data,
                                              size_t required_capacity,
                                              char **data,
                                              size_t *capacity);

typedef struct imgui_text_buffer {
    size_t struct_size;
    char *data;
    size_t length;
    size_t capacity;
    imgui_text_reserve_fn reserve;
    void *user_data;
} imgui_text_buffer;

typedef struct imgui_input_text_desc {
    size_t struct_size;
    const char *label;
    const char *hint;
    imgui_id id;
    imgui_bool use_explicit_id;
    imgui_input_text_flags flags;
    imgui_vec2 size;
    imgui_bool multiline;
    imgui_text_event_fn event_callback;
    void *event_user_data;
} imgui_input_text_desc;

typedef imgui_flags imgui_combo_flags;
enum {
    IMGUI_COMBO_NONE = 0,
    IMGUI_COMBO_NO_ARROW_BUTTON = 1UL << 0,
    IMGUI_COMBO_NO_PREVIEW = 1UL << 1
};
typedef imgui_flags imgui_selectable_flags;
typedef imgui_flags imgui_slider_flags;
enum {
    IMGUI_SLIDER_NONE = 0,
    IMGUI_SLIDER_ALWAYS_CLAMP = 1UL << 0,
    IMGUI_SLIDER_LOGARITHMIC = 1UL << 1,
    IMGUI_SLIDER_NO_ROUND_TO_FORMAT = 1UL << 2
};
typedef imgui_flags imgui_color_edit_flags;
typedef enum imgui_arrow_direction {
    IMGUI_ARROW_NONE = 0,
    IMGUI_ARROW_LEFT,
    IMGUI_ARROW_RIGHT,
    IMGUI_ARROW_UP,
    IMGUI_ARROW_DOWN
} imgui_arrow_direction;
typedef imgui_flags imgui_tree_flags;
enum {
    IMGUI_TREE_NONE = 0,
    IMGUI_TREE_DEFAULT_OPEN = 1UL << 0,
    IMGUI_TREE_LEAF = 1UL << 1,
    IMGUI_TREE_NO_TREE_PUSH_ON_OPEN = 1UL << 2
};
enum {
    IMGUI_SELECTABLE_NONE = 0,
    IMGUI_SELECTABLE_DISABLED = 1UL << 0,
    IMGUI_SELECTABLE_ALLOW_DOUBLE_CLICK = 1UL << 1
};
typedef imgui_flags imgui_popup_flags;
enum {
    IMGUI_POPUP_NONE = 0,
    IMGUI_POPUP_MOUSE_BUTTON_RIGHT = 1UL << 0,
    IMGUI_POPUP_MOUSE_BUTTON_MIDDLE = 1UL << 1,
    IMGUI_POPUP_NO_OPEN_OVER_EXISTING = 1UL << 2,
    IMGUI_POPUP_NO_OPEN_OVER_ITEMS = 1UL << 3
};
typedef imgui_flags imgui_tab_flags;
enum {
    IMGUI_TAB_NONE = 0,
    IMGUI_TAB_ITEM_DISABLED = 1UL << 0,
    IMGUI_TAB_ITEM_UNSAVED_DOCUMENT = 1UL << 1
};
typedef imgui_flags imgui_table_flags;
typedef imgui_flags imgui_table_column_flags;
typedef struct imgui_table_sort_spec {
    int column_index;
    int sort_order;
    int direction;
    imgui_id column_user_id;
} imgui_table_sort_spec;
typedef struct imgui_table_sort_specs {
    size_t struct_size;
    const imgui_table_sort_spec *specs;
    int count;
    imgui_bool dirty;
} imgui_table_sort_specs;
enum {
    IMGUI_TABLE_COLUMN_NONE = 0,
    IMGUI_TABLE_COLUMN_NO_SORT = 1UL << 0,
    IMGUI_TABLE_COLUMN_NO_SORT_ASCENDING = 1UL << 1,
    IMGUI_TABLE_COLUMN_NO_SORT_DESCENDING = 1UL << 2,
    IMGUI_TABLE_COLUMN_PREFER_SORT_ASCENDING = 1UL << 3,
    IMGUI_TABLE_COLUMN_PREFER_SORT_DESCENDING = 1UL << 4
};
typedef imgui_flags imgui_dock_flags;

typedef enum imgui_dock_split_direction {
    IMGUI_DOCK_SPLIT_NONE = 0,
    IMGUI_DOCK_SPLIT_LEFT = 1,
    IMGUI_DOCK_SPLIT_RIGHT = 2,
    IMGUI_DOCK_SPLIT_UP = 3,
    IMGUI_DOCK_SPLIT_DOWN = 4
} imgui_dock_split_direction;
typedef imgui_flags imgui_multi_select_flags;
typedef imgui_flags imgui_drag_drop_source_flags;
typedef imgui_flags imgui_drag_drop_target_flags;
/* Hover-query flags.  These retain the Dear ImGui query concepts while
   keeping the C89 API's single unsigned-flags argument. */
typedef imgui_flags imgui_hovered_flags;
enum {
    IMGUI_HOVERED_NONE = 0,
    IMGUI_HOVERED_ALLOW_WHEN_BLOCKED_BY_POPUP = 1UL << 0,
    IMGUI_HOVERED_ALLOW_WHEN_BLOCKED_BY_ACTIVE_ITEM = 1UL << 1,
    IMGUI_HOVERED_ALLOW_WHEN_OVERLAPPED_BY_ITEM = 1UL << 2,
    IMGUI_HOVERED_RECT_ONLY = 1UL << 3,
    IMGUI_HOVERED_ALLOW_WHEN_DISABLED = 1UL << 4,
    IMGUI_HOVERED_ANY_WINDOW = 1UL << 5
};
enum {
    IMGUI_FOCUSED_NONE = 0,
    IMGUI_FOCUSED_ANY_WINDOW = 1UL << 0
};

enum {
    IMGUI_DOCK_NONE = 0,
    IMGUI_DOCK_PASSTHRU_CENTRAL = 1UL << 0
};

enum {
    IMGUI_COLOR_EDIT_NONE = 0,
    IMGUI_COLOR_EDIT_NO_ALPHA = 1UL << 0,
    IMGUI_COLOR_EDIT_NO_INPUTS = 1UL << 1,
    IMGUI_COLOR_EDIT_PICKER = 1UL << 2
};

enum {
    IMGUI_TABLE_NONE = 0,
    IMGUI_TABLE_BORDERS = 1UL << 0,
    IMGUI_TABLE_ROW_BACKGROUND = 1UL << 1
};

enum {
    IMGUI_MULTI_SELECT_NONE = 0,
    IMGUI_MULTI_SELECT_TOGGLE_ON_CLICK = 1UL << 0,
    IMGUI_MULTI_SELECT_ALLOW_SHIFT_RANGE = 1UL << 1
};

enum {
    IMGUI_DRAG_DROP_SOURCE_NONE = 0,
    IMGUI_DRAG_DROP_SOURCE_NO_PREVIEW = 1UL << 0,
    IMGUI_DRAG_DROP_SOURCE_NO_DISABLE_HOVER = 1UL << 1,
    IMGUI_DRAG_DROP_SOURCE_NO_HOLD_TO_OPEN_OTHERS = 1UL << 2,
    IMGUI_DRAG_DROP_SOURCE_ALLOW_NULL_ID = 1UL << 3,
    IMGUI_DRAG_DROP_SOURCE_EXTERN = 1UL << 4,
    IMGUI_DRAG_DROP_SOURCE_PAYLOAD_AUTO_EXPIRE = 1UL << 5,
    IMGUI_DRAG_DROP_SOURCE_PAYLOAD_NO_CROSS_CONTEXT = 1UL << 6,
    IMGUI_DRAG_DROP_SOURCE_PAYLOAD_NO_CROSS_PROCESS = 1UL << 7
};

enum {
    IMGUI_DRAG_DROP_TARGET_NONE = 0,
    IMGUI_DRAG_DROP_TARGET_ACCEPT_BEFORE_DELIVERY = 1UL << 10,
    IMGUI_DRAG_DROP_TARGET_NO_DRAW_DEFAULT_RECT = 1UL << 11,
    IMGUI_DRAG_DROP_TARGET_NO_PREVIEW_TOOLTIP = 1UL << 12,
    IMGUI_DRAG_DROP_TARGET_DRAW_AS_HOVERED = 1UL << 13,
    IMGUI_DRAG_DROP_TARGET_ACCEPT_PEEK_ONLY =
        IMGUI_DRAG_DROP_TARGET_ACCEPT_BEFORE_DELIVERY |
        IMGUI_DRAG_DROP_TARGET_NO_DRAW_DEFAULT_RECT
};

typedef struct imgui_drag_payload {
    size_t struct_size;
    const char *type;
    const void *data;
    size_t data_size;
    imgui_id source_id;
    imgui_bool preview;
    imgui_bool delivery;
} imgui_drag_payload;

typedef struct imgui_multi_select_storage {
    size_t struct_size;
    imgui_id *ids;
    size_t count;
    size_t capacity;
} imgui_multi_select_storage;

typedef enum imgui_texture_format {
    IMGUI_TEXTURE_FORMAT_RGBA8 = 0,
    IMGUI_TEXTURE_FORMAT_ALPHA8 = 1
} imgui_texture_format;

typedef struct imgui_texture_desc {
    size_t struct_size;
    int width;
    int height;
    imgui_texture_format format;
    imgui_flags flags;
    const char *debug_name;
} imgui_texture_desc;

/* Initialization and scalar helpers. */
IMGUI_API void imgui_config_init(imgui_config *config);
IMGUI_API void imgui_frame_desc_init(imgui_frame_desc *desc);
IMGUI_API void imgui_viewport_desc_init(imgui_viewport_desc *desc,
                                         imgui_id viewport_id);
IMGUI_API void imgui_metrics_init(imgui_metrics *metrics);
IMGUI_API void imgui_style_init(imgui_style *style);
IMGUI_API void imgui_window_desc_init(imgui_window_desc *desc,
                                      const char *title);
IMGUI_API void imgui_text_buffer_init(imgui_text_buffer *buffer);
IMGUI_API void imgui_input_text_desc_init(imgui_input_text_desc *desc,
                                          const char *label);
IMGUI_API void imgui_multi_select_storage_init(
    imgui_multi_select_storage *storage,
    imgui_id *ids,
    size_t capacity);
IMGUI_API void imgui_texture_desc_init(imgui_texture_desc *desc);
IMGUI_API imgui_vec2 imgui_make_vec2(float x, float y);
IMGUI_API imgui_vec4 imgui_make_vec4(float x, float y, float z, float w);

/* Context and frame lifecycle. */
IMGUI_API const char *imgui_get_version(void);
IMGUI_API imgui_context *imgui_context_create(const imgui_config *config);
IMGUI_API void imgui_context_destroy(imgui_context *ctx);
IMGUI_API imgui_result imgui_frame_begin(imgui_context *ctx,
                                         const imgui_frame_desc *desc);
IMGUI_API imgui_result imgui_frame_end(imgui_context *ctx);
IMGUI_API const imgui_render_packet *imgui_render(imgui_context *ctx);
IMGUI_API const imgui_frame_output *imgui_get_frame_output(
    const imgui_context *ctx);
IMGUI_API imgui_result imgui_get_metrics(const imgui_context *ctx,
                                        imgui_metrics *metrics);
/* Optional built-in diagnostics window; call while building a frame. */
IMGUI_API void imgui_show_metrics_window(imgui_context *ctx,
                                         imgui_bool *open);
/* Optional feature/demo window; call while building a frame. */
IMGUI_API void imgui_show_demo_window(imgui_context *ctx,
                                      imgui_bool *open);
IMGUI_API const imgui_style *imgui_style_get(const imgui_context *ctx);
IMGUI_API imgui_result imgui_style_set(imgui_context *ctx,
                                       const imgui_style *style);
IMGUI_API imgui_result imgui_style_push(imgui_context *ctx,
                                        const imgui_style *style);
IMGUI_API imgui_result imgui_style_pop(imgui_context *ctx);
IMGUI_API imgui_result imgui_style_push_color(imgui_context *ctx,
                                              imgui_style_color color,
                                              imgui_u32 value);
IMGUI_API imgui_result imgui_style_push_var_float(imgui_context *ctx,
                                                  imgui_style_var var,
                                                  float value);
IMGUI_API imgui_result imgui_style_push_var_vec2(imgui_context *ctx,
                                                 imgui_style_var var,
                                                 imgui_vec2 value);
IMGUI_API imgui_result imgui_style_pop_color(imgui_context *ctx);
IMGUI_API imgui_result imgui_log_begin(imgui_context *ctx,
                                       const imgui_log_desc *desc);
IMGUI_API imgui_result imgui_log_text(imgui_context *ctx,
                                      const char *begin,
                                      const char *end);
IMGUI_API imgui_result imgui_log_end(imgui_context *ctx);
IMGUI_API imgui_bool imgui_log_is_active(const imgui_context *ctx);
IMGUI_API imgui_result imgui_localization_configure(
    imgui_context *ctx, const imgui_localization_desc *desc);
IMGUI_API const char *imgui_localize(const imgui_context *ctx,
                                     const char *key);

/* Deterministic settings text for the persistent state currently supported by
   the port.  Save reports the required byte count including the NUL byte. */
IMGUI_API imgui_result imgui_settings_save(imgui_context *ctx,
                                            char *buffer,
                                            size_t capacity,
                                            size_t *required);
IMGUI_API imgui_result imgui_settings_load(imgui_context *ctx,
                                            const char *data,
                                            size_t length);

/* Input event submission. Events are consumed by the next frame begin. */
IMGUI_API imgui_result imgui_input_add_mouse_position(imgui_context *ctx,
                                                      float x,
                                                      float y);
IMGUI_API imgui_result imgui_input_add_mouse_button(imgui_context *ctx,
                                                    imgui_mouse_button button,
                                                    imgui_bool down);
IMGUI_API imgui_result imgui_input_add_mouse_wheel(imgui_context *ctx,
                                                   float horizontal,
                                                   float vertical);
IMGUI_API imgui_result imgui_input_add_key(imgui_context *ctx,
                                           imgui_key key,
                                           imgui_bool down);
IMGUI_API imgui_result imgui_input_add_key_analog(imgui_context *ctx,
                                                  imgui_key key,
                                                  imgui_bool down,
                                                  float value);
IMGUI_API imgui_result imgui_input_add_text_utf8(imgui_context *ctx,
                                                 const char *text);
IMGUI_API imgui_result imgui_input_add_codepoint(imgui_context *ctx,
                                                 unsigned long codepoint);
IMGUI_API imgui_result imgui_input_add_focus(imgui_context *ctx,
                                             imgui_bool focused);
/* Frame/input state queries. Mouse coordinates are in display space, and
   timing is taken from the descriptor supplied to imgui_frame_begin(). */
IMGUI_API imgui_vec2 imgui_get_mouse_position(const imgui_context *ctx);
IMGUI_API imgui_u32 imgui_get_frame_count(const imgui_context *ctx);
IMGUI_API float imgui_get_delta_time(const imgui_context *ctx);
IMGUI_API double imgui_get_time(const imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_key_down(const imgui_context *ctx,
                                       imgui_key key);
IMGUI_API float imgui_get_key_analog(const imgui_context *ctx,
                                     imgui_key key);
IMGUI_API imgui_bool imgui_is_key_pressed(const imgui_context *ctx,
                                          imgui_key key,
                                          imgui_bool repeat);
IMGUI_API int imgui_get_key_pressed_amount(const imgui_context *ctx,
                                           imgui_key key,
                                           float repeat_delay,
                                           float repeat_rate);
IMGUI_API imgui_bool imgui_is_key_released(const imgui_context *ctx,
                                           imgui_key key);
IMGUI_API imgui_bool imgui_is_shortcut_pressed(const imgui_context *ctx,
                                               imgui_key key,
                                               imgui_key_modifiers modifiers,
                                               imgui_bool repeat);
IMGUI_API imgui_bool imgui_is_mouse_down(const imgui_context *ctx,
                                         imgui_mouse_button button);
IMGUI_API imgui_bool imgui_is_mouse_clicked(const imgui_context *ctx,
                                            imgui_mouse_button button);
IMGUI_API int imgui_get_mouse_clicked_count(const imgui_context *ctx,
                                            imgui_mouse_button button);
IMGUI_API imgui_bool imgui_is_mouse_released(const imgui_context *ctx,
                                             imgui_mouse_button button);
IMGUI_API imgui_bool imgui_is_mouse_double_clicked(
    const imgui_context *ctx, imgui_mouse_button button);
IMGUI_API imgui_bool imgui_is_any_mouse_down(const imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_mouse_dragging(const imgui_context *ctx,
                                             imgui_mouse_button button,
                                             float lock_threshold);
IMGUI_API imgui_vec2 imgui_get_mouse_drag_delta(const imgui_context *ctx,
                                                imgui_mouse_button button,
                                                float lock_threshold);
IMGUI_API void imgui_reset_mouse_drag_delta(imgui_context *ctx,
                                            imgui_mouse_button button);

/* Identity. */
IMGUI_API void imgui_push_id_string(imgui_context *ctx, const char *id);
IMGUI_API void imgui_push_id_range(imgui_context *ctx,
                                   const char *begin,
                                   const char *end);
IMGUI_API void imgui_push_id_pointer(imgui_context *ctx, const void *id);
IMGUI_API void imgui_push_id_integer(imgui_context *ctx, int id);
IMGUI_API void imgui_push_id_value(imgui_context *ctx, imgui_id id);
IMGUI_API void imgui_pop_id(imgui_context *ctx);
IMGUI_API imgui_id imgui_get_id_string(imgui_context *ctx, const char *id);
IMGUI_API imgui_id imgui_get_id_range(imgui_context *ctx,
                                      const char *begin,
                                      const char *end);
IMGUI_API imgui_id imgui_get_id_pointer(imgui_context *ctx, const void *id);
IMGUI_API imgui_id imgui_get_id_integer(imgui_context *ctx, int id);

/* Windows and layout. Every begin call always has a matching end call. */
IMGUI_API imgui_scope imgui_window_begin(imgui_context *ctx,
                                         const char *title);
IMGUI_API imgui_scope imgui_window_begin_ex(imgui_context *ctx,
                                            const imgui_window_desc *desc);
IMGUI_API void imgui_window_end(imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_window_collapsed(const imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_window_appearing(const imgui_context *ctx);
IMGUI_API imgui_result imgui_set_window_collapsed(imgui_context *ctx,
                                                   imgui_bool collapsed);
IMGUI_API imgui_result imgui_set_window_focus(imgui_context *ctx);
IMGUI_API imgui_result imgui_viewport_configure(
    imgui_context *ctx, const imgui_viewport_desc *desc);
IMGUI_API imgui_result imgui_viewport_destroy(imgui_context *ctx,
                                              imgui_id viewport_id);
IMGUI_API imgui_result imgui_window_set_viewport(imgui_context *ctx,
                                                 imgui_id window_id,
                                                 imgui_id viewport_id);
IMGUI_API imgui_id imgui_window_get_viewport(const imgui_context *ctx,
                                             imgui_id window_id);
IMGUI_API imgui_scope imgui_child_begin(imgui_context *ctx,
                                        imgui_id id,
                                        imgui_vec2 size,
                                        imgui_flags flags);
IMGUI_API void imgui_child_end(imgui_context *ctx);
IMGUI_API void imgui_same_line(imgui_context *ctx);
IMGUI_API void imgui_same_line_at(imgui_context *ctx,
                                  float offset_from_start,
                                  float spacing);
IMGUI_API void imgui_set_next_item_width(imgui_context *ctx, float width);
IMGUI_API void imgui_set_next_item_open(imgui_context *ctx, imgui_bool open);
IMGUI_API void imgui_set_next_item_shortcut(
    imgui_context *ctx, imgui_key key, imgui_key_modifiers modifiers);
IMGUI_API void imgui_new_line(imgui_context *ctx);
IMGUI_API void imgui_spacing(imgui_context *ctx);
IMGUI_API void imgui_separator(imgui_context *ctx);
IMGUI_API void imgui_separator_text(imgui_context *ctx, const char *label);
IMGUI_API void imgui_indent(imgui_context *ctx, float width);
IMGUI_API void imgui_unindent(imgui_context *ctx, float width);
IMGUI_API void imgui_group_begin(imgui_context *ctx);
IMGUI_API void imgui_group_end(imgui_context *ctx);
IMGUI_API void imgui_begin_disabled(imgui_context *ctx);
IMGUI_API void imgui_end_disabled(imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_disabled(const imgui_context *ctx);
IMGUI_API void imgui_set_keyboard_focus_here(imgui_context *ctx,
                                             int offset);
IMGUI_API void imgui_set_item_default_focus(imgui_context *ctx);
IMGUI_API imgui_vec2 imgui_get_cursor_screen_position(imgui_context *ctx);
IMGUI_API void imgui_set_cursor_screen_position(imgui_context *ctx,
                                                imgui_vec2 position);
IMGUI_API imgui_vec2 imgui_get_cursor_position(const imgui_context *ctx);
IMGUI_API void imgui_set_cursor_position(imgui_context *ctx,
                                         imgui_vec2 position);
IMGUI_API imgui_vec2 imgui_get_cursor_start_position(
    const imgui_context *ctx);
IMGUI_API float imgui_get_cursor_position_x(const imgui_context *ctx);
IMGUI_API float imgui_get_cursor_position_y(const imgui_context *ctx);
IMGUI_API void imgui_set_cursor_position_x(imgui_context *ctx, float x);
IMGUI_API void imgui_set_cursor_position_y(imgui_context *ctx, float y);
IMGUI_API imgui_vec2 imgui_get_window_position(const imgui_context *ctx);
IMGUI_API imgui_vec2 imgui_get_window_size(const imgui_context *ctx);
IMGUI_API imgui_vec2 imgui_get_window_content_region_min(
    const imgui_context *ctx);
IMGUI_API imgui_vec2 imgui_get_window_content_region_max(
    const imgui_context *ctx);
IMGUI_API imgui_vec2 imgui_get_window_content_region_size(
    const imgui_context *ctx);
IMGUI_API imgui_result imgui_set_next_window_position(imgui_context *ctx,
                                                       imgui_vec2 position);
IMGUI_API imgui_result imgui_set_next_window_position_ex(
    imgui_context *ctx, imgui_vec2 position, imgui_vec2 pivot);
IMGUI_API imgui_result imgui_set_next_window_viewport(
    imgui_context *ctx, imgui_id viewport_id);
IMGUI_API imgui_result imgui_set_next_window_size(imgui_context *ctx,
                                                  imgui_vec2 size);
IMGUI_API imgui_result imgui_set_next_window_content_size(
    imgui_context *ctx, imgui_vec2 size);
IMGUI_API imgui_result imgui_set_next_window_background_alpha(
    imgui_context *ctx, float alpha);
IMGUI_API imgui_result imgui_set_next_window_size_constraints(
    imgui_context *ctx, imgui_vec2 minimum, imgui_vec2 maximum);
IMGUI_API imgui_result imgui_set_next_window_scroll(imgui_context *ctx,
                                                    imgui_vec2 scroll);
IMGUI_API imgui_result imgui_set_next_window_collapsed(imgui_context *ctx,
                                                       imgui_bool collapsed);
IMGUI_API imgui_result imgui_set_next_window_focus(imgui_context *ctx);
IMGUI_API void imgui_set_window_position(imgui_context *ctx,
                                         imgui_vec2 position);
IMGUI_API void imgui_set_window_size(imgui_context *ctx, imgui_vec2 size);
IMGUI_API imgui_vec2 imgui_get_content_region_available(imgui_context *ctx);
IMGUI_API float imgui_calc_item_width(const imgui_context *ctx);
IMGUI_API float imgui_get_text_line_height(const imgui_context *ctx);
IMGUI_API float imgui_get_frame_height(const imgui_context *ctx);
IMGUI_API float imgui_get_frame_height_with_spacing(const imgui_context *ctx);
IMGUI_API float imgui_get_scroll_y(const imgui_context *ctx);
IMGUI_API float imgui_get_scroll_max_y(const imgui_context *ctx);
IMGUI_API imgui_result imgui_set_scroll_y(imgui_context *ctx, float scroll_y);
IMGUI_API float imgui_get_scroll_x(const imgui_context *ctx);
IMGUI_API float imgui_get_scroll_max_x(const imgui_context *ctx);
IMGUI_API imgui_result imgui_set_scroll_x(imgui_context *ctx, float scroll_x);
IMGUI_API imgui_result imgui_dock_space(imgui_context *ctx,
                                        imgui_id dock_id,
                                        imgui_vec2 size,
                                        imgui_dock_flags flags);
IMGUI_API imgui_result imgui_dock_space_split(
    imgui_context *ctx, imgui_id parent_id,
    imgui_dock_split_direction direction, float ratio,
    imgui_id child_id);
IMGUI_API imgui_result imgui_window_set_dock(imgui_context *ctx,
                                             imgui_id window_id,
                                             imgui_id dock_id);
IMGUI_API imgui_id imgui_window_get_dock(const imgui_context *ctx,
                                         imgui_id window_id);
IMGUI_API imgui_result imgui_dock_activate(imgui_context *ctx,
                                           imgui_id dock_id,
                                           imgui_id window_id);
IMGUI_API imgui_id imgui_dock_get_active_window(const imgui_context *ctx,
                                                imgui_id dock_id);
IMGUI_API imgui_bool imgui_dock_tab_button(imgui_context *ctx,
                                            imgui_id dock_id,
                                            imgui_id window_id,
                                            const char *label);
IMGUI_API imgui_result imgui_dock_tab_bar(imgui_context *ctx,
                                          imgui_id dock_id);

/* Text and basic widgets. */
IMGUI_API void imgui_text(imgui_context *ctx, const char *format, ...);
IMGUI_API void imgui_text_colored(imgui_context *ctx, imgui_u32 color,
                                  const char *format, ...);
IMGUI_API void imgui_text_disabled(imgui_context *ctx, const char *format,
                                   ...);
IMGUI_API imgui_bool imgui_text_link(imgui_context *ctx, const char *label);
IMGUI_API void imgui_text_unformatted(imgui_context *ctx,
                                      const char *begin,
                                      const char *end);
IMGUI_API void imgui_text_wrapped(imgui_context *ctx, const char *text,
                                  float wrap_width);
IMGUI_API void imgui_text_localized(imgui_context *ctx, const char *key);
IMGUI_API void imgui_dummy(imgui_context *ctx, imgui_vec2 size);
IMGUI_API void imgui_bullet(imgui_context *ctx);
IMGUI_API void imgui_bullet_text(imgui_context *ctx, const char *format, ...);
IMGUI_API imgui_vec2 imgui_calc_text_size(const char *begin,
                                          const char *end,
                                          float wrap_width);
IMGUI_API void imgui_label_text(imgui_context *ctx,
                                const char *label,
                                const char *format,
                                ...);
IMGUI_API imgui_bool imgui_button(imgui_context *ctx, const char *label);
IMGUI_API imgui_bool imgui_arrow_button(imgui_context *ctx, imgui_id id,
                                        imgui_arrow_direction direction);
IMGUI_API imgui_bool imgui_button_sized(imgui_context *ctx,
                                        const char *label,
                                        imgui_vec2 size);
IMGUI_API imgui_bool imgui_button_with_id(imgui_context *ctx,
                                          imgui_id id,
                                          const char *label);
IMGUI_API imgui_bool imgui_button_localized(imgui_context *ctx,
                                            const char *key);
IMGUI_API imgui_bool imgui_small_button(imgui_context *ctx,
                                        const char *label);
IMGUI_API imgui_bool imgui_invisible_button(imgui_context *ctx,
                                            imgui_id id,
                                            imgui_vec2 size,
                                            imgui_flags flags);
IMGUI_API imgui_bool imgui_checkbox(imgui_context *ctx,
                                    const char *label,
                                    imgui_bool *value);
IMGUI_API imgui_bool imgui_checkbox_flags(imgui_context *ctx,
                                          const char *label,
                                          imgui_flags *flags,
                                          imgui_flags value);
IMGUI_API imgui_bool imgui_radio_button(imgui_context *ctx,
                                        const char *label,
                                        imgui_bool active);
IMGUI_API imgui_bool imgui_radio_button_value(imgui_context *ctx,
                                              const char *label,
                                              int *value,
                                              int button_value);
IMGUI_API void imgui_progress_bar(imgui_context *ctx, float fraction);
IMGUI_API void imgui_progress_bar_ex(imgui_context *ctx,
                                     float fraction,
                                     imgui_vec2 size,
                                     const char *overlay);
IMGUI_API void imgui_plot_lines(imgui_context *ctx,
                                const char *label,
                                const float *values,
                                size_t count,
                                imgui_vec2 size,
                                float minimum,
                                float maximum);
IMGUI_API void imgui_plot_histogram(imgui_context *ctx,
                                    const char *label,
                                    const float *values,
                                    size_t count,
                                    imgui_vec2 size,
                                    float minimum,
                                    float maximum);
IMGUI_API imgui_bool imgui_color_edit_rgba(imgui_context *ctx,
                                           const char *label,
                                           float rgba[4],
                                           imgui_color_edit_flags flags);
IMGUI_API imgui_bool imgui_color_edit_rgb(imgui_context *ctx,
                                          const char *label,
                                          float rgb[3],
                                          imgui_color_edit_flags flags);
IMGUI_API imgui_bool imgui_color_picker_rgba(
    imgui_context *ctx, const char *label, float rgba[4],
    imgui_color_edit_flags flags);
IMGUI_API imgui_bool imgui_color_picker_rgb(
    imgui_context *ctx, const char *label, float rgb[3],
    imgui_color_edit_flags flags);
/* Color conversion helpers use the same IM_COL32 byte order as render data. */
IMGUI_API imgui_u32 imgui_color_rgba_to_u32(const float rgba[4]);
IMGUI_API void imgui_color_u32_to_rgba(imgui_u32 color, float rgba[4]);
IMGUI_API void imgui_color_rgb_to_hsv(const float rgb[3], float *hue,
                                      float *saturation, float *value);
IMGUI_API void imgui_color_hsv_to_rgb(float hue, float saturation,
                                      float value, float rgb[3]);
IMGUI_API imgui_bool imgui_color_button(imgui_context *ctx,
                                        imgui_id id,
                                        const float rgba[4],
                                        imgui_vec2 size);

/* Images and textures. */
IMGUI_API imgui_result imgui_texture_register_external(
    imgui_context *ctx,
    imgui_texture_id backend_id,
    const imgui_texture_desc *desc,
    imgui_texture **out_texture);
IMGUI_API imgui_result imgui_texture_create(imgui_context *ctx,
                                            const imgui_texture_desc *desc,
                                            const void *pixels,
                                            size_t pitch,
                                            imgui_texture **out_texture);
IMGUI_API imgui_result imgui_texture_update(imgui_context *ctx,
                                            imgui_texture *texture,
                                            int x,
                                            int y,
                                            int width,
                                            int height,
                                            const void *pixels,
                                            size_t pitch);
IMGUI_API imgui_result imgui_texture_destroy(imgui_context *ctx,
                                             imgui_texture *texture);
IMGUI_API imgui_bool imgui_texture_equivalent(const imgui_texture *left,
                                              const imgui_texture *right);
IMGUI_API void imgui_image(imgui_context *ctx,
                           imgui_texture *texture,
                           imgui_vec2 size);
IMGUI_API void imgui_image_uv(imgui_context *ctx,
                              imgui_texture *texture,
                              imgui_vec2 size,
                              imgui_vec2 uv_min,
                              imgui_vec2 uv_max);
IMGUI_API imgui_bool imgui_image_button(imgui_context *ctx,
                                        imgui_id id,
                                        imgui_texture *texture,
                                        imgui_vec2 size);

/* Selection, combo boxes, menus, popups, trees, and tabs. */
IMGUI_API imgui_scope imgui_combo_begin(imgui_context *ctx,
                                        const char *label,
                                        const char *preview);
IMGUI_API imgui_scope imgui_combo_begin_ex(imgui_context *ctx,
                                           const char *label,
                                           const char *preview,
                                           imgui_combo_flags flags);
IMGUI_API void imgui_combo_end(imgui_context *ctx);
IMGUI_API imgui_bool imgui_selectable(imgui_context *ctx,
                                      const char *label,
                                      imgui_bool selected);
IMGUI_API imgui_bool imgui_selectable_ex(imgui_context *ctx,
                                         const char *label,
                                         imgui_bool *selected,
                                         imgui_selectable_flags flags,
                                         imgui_vec2 size);
IMGUI_API imgui_result imgui_multi_select_begin(
    imgui_context *ctx,
    imgui_multi_select_storage *storage,
    imgui_multi_select_flags flags);
IMGUI_API imgui_result imgui_multi_select_end(imgui_context *ctx);
IMGUI_API imgui_bool imgui_multi_select_contains(
    const imgui_context *ctx,
    imgui_id id);
IMGUI_API imgui_bool imgui_multi_select_item(
    imgui_context *ctx,
    imgui_id id);
IMGUI_API imgui_result imgui_multi_select_clear(imgui_context *ctx);
IMGUI_API imgui_scope imgui_drag_drop_source_begin(
    imgui_context *ctx,
    imgui_id source_id,
    imgui_drag_drop_source_flags flags);
IMGUI_API imgui_result imgui_drag_drop_set_payload(
    imgui_context *ctx,
    const char *type,
    const void *data,
    size_t data_size);
IMGUI_API void imgui_drag_drop_source_end(imgui_context *ctx);
IMGUI_API imgui_scope imgui_drag_drop_target_begin(imgui_context *ctx);
IMGUI_API const imgui_drag_payload *imgui_drag_drop_target_accept(
    imgui_context *ctx,
    const char *type,
    imgui_drag_drop_target_flags flags);
IMGUI_API void imgui_drag_drop_target_end(imgui_context *ctx);
IMGUI_API imgui_scope imgui_list_box_begin(imgui_context *ctx,
                                           const char *label,
                                           imgui_vec2 size);
IMGUI_API void imgui_list_box_end(imgui_context *ctx);
IMGUI_API imgui_scope imgui_menu_bar_begin(imgui_context *ctx);
IMGUI_API void imgui_menu_bar_end(imgui_context *ctx);
IMGUI_API imgui_scope imgui_menu_begin(imgui_context *ctx,
                                       const char *label,
                                       imgui_bool enabled);
IMGUI_API void imgui_menu_end(imgui_context *ctx);
IMGUI_API imgui_bool imgui_menu_item(imgui_context *ctx,
                                     const char *label,
                                     const char *shortcut,
                                     imgui_bool enabled);
IMGUI_API imgui_bool imgui_menu_item_shortcut(
    imgui_context *ctx,
    const char *label,
    const char *shortcut_text,
    imgui_key key,
    imgui_key_modifiers modifiers,
    imgui_bool enabled);
IMGUI_API imgui_bool imgui_menu_item_toggle(imgui_context *ctx,
                                            const char *label,
                                            const char *shortcut,
                                            imgui_bool *selected,
                                            imgui_bool enabled);
IMGUI_API void imgui_popup_open(imgui_context *ctx,
                                const char *id,
                                imgui_popup_flags flags);
IMGUI_API imgui_result imgui_popup_open_on_item_click(
    imgui_context *ctx, const char *id, imgui_popup_flags flags);
IMGUI_API imgui_bool imgui_popup_is_open(imgui_context *ctx,
                                         const char *id);
IMGUI_API imgui_vec2 imgui_get_mouse_position_on_opening_current_popup(
    const imgui_context *ctx);
IMGUI_API imgui_scope imgui_popup_begin(imgui_context *ctx,
                                        const char *id,
                                        imgui_window_flags flags);
IMGUI_API void imgui_popup_end(imgui_context *ctx);
IMGUI_API void imgui_popup_close_current(imgui_context *ctx);
IMGUI_API imgui_scope imgui_tooltip_begin(imgui_context *ctx);
IMGUI_API imgui_scope imgui_item_tooltip_begin(imgui_context *ctx);
IMGUI_API void imgui_tooltip_end(imgui_context *ctx);
IMGUI_API void imgui_set_tooltip(imgui_context *ctx, const char *format, ...);
IMGUI_API void imgui_set_item_tooltip(imgui_context *ctx,
                                      const char *format, ...);
IMGUI_API imgui_scope imgui_tree_node_begin(imgui_context *ctx,
                                            const char *label,
                                            imgui_tree_flags flags);
IMGUI_API imgui_scope imgui_tree_node_begin_with_id(imgui_context *ctx,
                                                    imgui_id id,
                                                    const char *label,
                                                    imgui_tree_flags flags);
IMGUI_API void imgui_tree_node_end(imgui_context *ctx);
IMGUI_API imgui_bool imgui_collapsing_header(imgui_context *ctx,
                                             const char *label,
                                             imgui_tree_flags flags);
IMGUI_API imgui_scope imgui_tab_bar_begin(imgui_context *ctx,
                                          const char *id,
                                          imgui_tab_flags flags);
IMGUI_API void imgui_tab_bar_end(imgui_context *ctx);
IMGUI_API imgui_scope imgui_tab_item_begin(imgui_context *ctx,
                                           const char *label,
                                           imgui_bool *open,
                                           imgui_tab_flags flags);
IMGUI_API void imgui_tab_item_end(imgui_context *ctx);
IMGUI_API imgui_scope imgui_table_begin(imgui_context *ctx,
                                        const char *id,
                                        int columns,
                                        imgui_table_flags flags);
IMGUI_API void imgui_table_next_row(imgui_context *ctx);
IMGUI_API void imgui_table_next_column(imgui_context *ctx);
IMGUI_API void imgui_table_set_column_index(imgui_context *ctx, int column);
IMGUI_API void imgui_table_set_column_width(imgui_context *ctx,
                                            float width);
IMGUI_API imgui_result imgui_table_setup_column(imgui_context *ctx,
                                                int column,
                                                const char *label,
                                                imgui_table_column_flags flags);
IMGUI_API imgui_result imgui_table_setup_scroll_freeze(
    imgui_context *ctx, int columns, int rows);
IMGUI_API imgui_bool imgui_table_header(imgui_context *ctx,
                                        const char *label);
IMGUI_API void imgui_table_headers_row(imgui_context *ctx);
IMGUI_API imgui_bool imgui_table_get_sort(const imgui_context *ctx,
                                          int *column,
                                          int *direction);
IMGUI_API const imgui_table_sort_specs *imgui_table_get_sort_specs(
    const imgui_context *ctx);
IMGUI_API int imgui_table_get_column_count(const imgui_context *ctx);
IMGUI_API int imgui_table_get_column_index(const imgui_context *ctx);
IMGUI_API int imgui_table_get_row_index(const imgui_context *ctx);
IMGUI_API float imgui_table_get_column_width(const imgui_context *ctx,
                                             int column);
IMGUI_API const char *imgui_table_get_column_name(const imgui_context *ctx,
                                                  int column);
IMGUI_API imgui_table_column_flags imgui_table_get_column_flags(
    const imgui_context *ctx, int column);
IMGUI_API int imgui_table_get_hovered_column(const imgui_context *ctx);
IMGUI_API void imgui_table_end(imgui_context *ctx);

/* Numeric and text input. */
IMGUI_API imgui_bool imgui_slider_float(imgui_context *ctx,
                                        const char *label,
                                        float *value,
                                        float minimum,
                                        float maximum);
IMGUI_API imgui_bool imgui_slider_float_ex(imgui_context *ctx,
                                           const char *label,
                                           float *value,
                                           float minimum,
                                           float maximum,
                                           const char *format,
                                           imgui_slider_flags flags);
IMGUI_API imgui_bool imgui_vslider_float(imgui_context *ctx,
                                         const char *label,
                                         imgui_vec2 size,
                                         float *value,
                                         float minimum,
                                         float maximum);
IMGUI_API imgui_bool imgui_vslider_float_ex(
    imgui_context *ctx, const char *label, imgui_vec2 size, float *value,
    float minimum, float maximum, const char *format,
    imgui_slider_flags flags);
IMGUI_API imgui_bool imgui_slider_integer(imgui_context *ctx,
                                          const char *label,
                                          int *value,
                                          int minimum,
                                          int maximum);
IMGUI_API imgui_bool imgui_slider_integer_ex(imgui_context *ctx,
                                             const char *label,
                                             int *value,
                                             int minimum,
                                             int maximum,
                                             const char *format,
                                             imgui_slider_flags flags);
IMGUI_API imgui_bool imgui_vslider_integer(
    imgui_context *ctx, const char *label, imgui_vec2 size, int *value,
    int minimum, int maximum);
IMGUI_API imgui_bool imgui_vslider_integer_ex(
    imgui_context *ctx, const char *label, imgui_vec2 size, int *value,
    int minimum, int maximum, const char *format, imgui_slider_flags flags);
IMGUI_API imgui_bool imgui_drag_float(imgui_context *ctx,
                                      const char *label,
                                      float *value,
                                      float speed);
IMGUI_API imgui_bool imgui_drag_float_ex(imgui_context *ctx,
                                         const char *label,
                                         float *value,
                                         float speed,
                                         float minimum,
                                         float maximum,
                                         const char *format,
                                         imgui_slider_flags flags);
IMGUI_API imgui_bool imgui_drag_integer(imgui_context *ctx,
                                        const char *label,
                                        int *value,
                                        float speed);
IMGUI_API imgui_bool imgui_drag_integer_ex(imgui_context *ctx,
                                           const char *label,
                                           int *value,
                                           float speed,
                                           int minimum,
                                           int maximum,
                                           const char *format,
                                           imgui_slider_flags flags);
IMGUI_API imgui_bool imgui_input_integer(imgui_context *ctx,
                                         const char *label,
                                         int *value);
IMGUI_API imgui_bool imgui_input_integer_ex(imgui_context *ctx,
                                            const char *label,
                                            int *value,
                                            const char *format);
IMGUI_API imgui_bool imgui_input_float(imgui_context *ctx,
                                       const char *label,
                                       float *value);
IMGUI_API imgui_bool imgui_input_float_ex(imgui_context *ctx,
                                          const char *label,
                                          float *value,
                                          const char *format);
/* Vector scalar controls use 2-4 adjacent components and one shared label. */
IMGUI_API imgui_bool imgui_slider_float_n(
    imgui_context *ctx, const char *label, float *values, int components,
    float minimum, float maximum, const char *format,
    imgui_slider_flags flags);
IMGUI_API imgui_bool imgui_drag_float_n(
    imgui_context *ctx, const char *label, float *values, int components,
    float speed, float minimum, float maximum, const char *format,
    imgui_slider_flags flags);
IMGUI_API imgui_bool imgui_input_float_n(
    imgui_context *ctx, const char *label, float *values, int components,
    const char *format);
IMGUI_API imgui_bool imgui_slider_integer_n(
    imgui_context *ctx, const char *label, int *values, int components,
    int minimum, int maximum, const char *format, imgui_slider_flags flags);
IMGUI_API imgui_bool imgui_drag_integer_n(
    imgui_context *ctx, const char *label, int *values, int components,
    float speed, int minimum, int maximum, const char *format,
    imgui_slider_flags flags);
IMGUI_API imgui_bool imgui_input_integer_n(
    imgui_context *ctx, const char *label, int *values, int components,
    const char *format);
IMGUI_API imgui_bool imgui_input_text_fixed(imgui_context *ctx,
                                            const char *label,
                                            char *buffer,
                                            size_t capacity);
IMGUI_API imgui_bool imgui_input_text_fixed_ex(
    imgui_context *ctx,
    const imgui_input_text_desc *desc,
    char *buffer,
    size_t capacity);
IMGUI_API imgui_bool imgui_input_text_buffer(imgui_context *ctx,
                                             const char *label,
                                             imgui_text_buffer *buffer);
IMGUI_API imgui_bool imgui_input_text_buffer_ex(
    imgui_context *ctx,
    const imgui_input_text_desc *desc,
    imgui_text_buffer *buffer);

/* Last-item and window queries. */
IMGUI_API imgui_bool imgui_is_item_hovered(imgui_context *ctx,
                                           imgui_flags flags);
IMGUI_API imgui_bool imgui_is_item_visible(imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_item_active(imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_item_focused(imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_item_activated(imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_item_deactivated(imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_item_deactivated_after_edit(
    imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_item_edited(imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_item_toggled_open(imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_item_toggled_selection(imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_item_clicked(imgui_context *ctx,
                                           imgui_mouse_button button);
IMGUI_API imgui_bool imgui_is_mouse_hovering_rect(
    const imgui_context *ctx, imgui_vec2 minimum, imgui_vec2 maximum,
    imgui_bool clip);
IMGUI_API imgui_bool imgui_is_rect_visible(const imgui_context *ctx,
                                           imgui_vec2 minimum,
                                           imgui_vec2 maximum);
IMGUI_API imgui_bool imgui_is_any_item_hovered(const imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_any_item_active(const imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_any_item_focused(const imgui_context *ctx);
IMGUI_API imgui_id imgui_get_item_id(const imgui_context *ctx);
IMGUI_API imgui_rect imgui_get_item_rect(imgui_context *ctx);
IMGUI_API imgui_vec2 imgui_get_item_rect_min(const imgui_context *ctx);
IMGUI_API imgui_vec2 imgui_get_item_rect_max(const imgui_context *ctx);
IMGUI_API imgui_vec2 imgui_get_item_rect_size(const imgui_context *ctx);
IMGUI_API imgui_bool imgui_is_window_hovered(imgui_context *ctx,
                                             imgui_flags flags);
IMGUI_API imgui_bool imgui_is_window_focused(imgui_context *ctx,
                                             imgui_flags flags);
IMGUI_API imgui_draw_list *imgui_get_window_draw_list(imgui_context *ctx);

#ifdef __cplusplus
}
#endif

#endif
