#include "slkscr_rus.h"
#include "cp1251.h"

char* recode_upper_cp1251(const char* str) {
    char* result = malloc(strlen(str) + 1);
    int j = 0;

    for(int i = 0; str[i]; i++, j++) {
        if(str[i] == 0xD0) {
            char ch = str[++i];
            result[j] = ch == 0x81 ? 0xA8 : ch + 0x30;
        } else {
            result[j] = str[i];
        }
    }

    result[j] = 0;

    return result;
}

void cp1251_render_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_custom_u8g2_font(canvas, silkscr_rus);

    char* str = recode_upper_cp1251("ВЫВОД РУССКОГО ЯЗЫКА В КОМПОНЕНТ");
    elements_multiline_text_aligned(canvas, 0, 0, AlignLeft, AlignTop, str);
    free(str);
}

static void cp1251_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    furi_message_queue_put((FuriMessageQueue*)ctx, input_event, FuriWaitForever);
}

CP1251App* cp1251_app_alloc() {
    CP1251App* app = malloc(sizeof(CP1251App));

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, cp1251_render_callback, app);
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    view_port_input_callback_set(app->view_port, cp1251_input_callback, app->event_queue);

    return app;
}

void cp1251_app_free(CP1251App** app) {
    furi_assert(*app);

    view_port_enabled_set((*app)->view_port, false);
    gui_remove_view_port((*app)->gui, (*app)->view_port);
    view_port_free((*app)->view_port);

    furi_record_close(RECORD_GUI);
    furi_message_queue_free((*app)->event_queue);

    free(*app);
}

int32_t cp1251_main(void* p) {
    UNUSED(p);
    __attribute__((__cleanup__(cp1251_app_free))) CP1251App* app = cp1251_app_alloc();

    for(InputEvent event;;) {
        furi_check(
            furi_message_queue_get(app->event_queue, &event, FuriWaitForever) == FuriStatusOk);

        if(event.type == InputTypeShort) break;
    }

    return 0;
}
