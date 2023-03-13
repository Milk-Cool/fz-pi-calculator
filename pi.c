#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <input/input.h>

#define PI_LENGTH 13
#define ITERS_LENGTH 6

bool state = false;
float pi = 3;
char pi_str[PI_LENGTH];
uint16_t iterations = 100;
char iterations_str[ITERS_LENGTH];

// Screen is 128x64 px
static void app_draw_callback(Canvas* canvas, void* ctx) {
	UNUSED(ctx);

	canvas_clear(canvas);
	canvas_set_font(canvas, FontPrimary);
	if(state) {
		snprintf(pi_str, PI_LENGTH, "%.10f", (double)pi);
		canvas_draw_str(canvas, 0, 12, pi_str); 
	} else {
		canvas_draw_str(canvas, 0, 12, "OK = calculate");
		canvas_draw_str(canvas, 0, 24, "Up/Down = iterations");
		canvas_draw_str(canvas, 0, 36, "Iterations:");
		snprintf(iterations_str, ITERS_LENGTH, "%d", iterations);
		canvas_draw_str(canvas, 64, 36, iterations_str);
	}
}

static void app_input_callback(InputEvent* input_event, void* ctx) {
	furi_assert(ctx);

	FuriMessageQueue* event_queue = ctx;
	furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t pi_main(void* p) {
	UNUSED(p);
	FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

	// Configure view port
	ViewPort* view_port = view_port_alloc();
	view_port_draw_callback_set(view_port, app_draw_callback, view_port);
	view_port_input_callback_set(view_port, app_input_callback, event_queue);

	// Register view port in GUI
	Gui* gui = furi_record_open(RECORD_GUI);
	gui_add_view_port(gui, view_port, GuiLayerFullscreen);

	InputEvent event;

	bool running = true;
	while(running) {
		if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
			if(event.type == InputTypePress && event.key == InputKeyBack) running = false;
			if(!state) {
				if(event.type == InputTypePress && event.key == InputKeyOk) {
					for(uint16_t i = 0; i < iterations; i++) {
						if(i % 2 == 0) pi += 4.0 / ((i * 2 + 2) * (i * 2 + 3) * (i * 2 + 4));
						else pi -= 4.0 / ((i * 2 + 2) * (i * 2 + 3) * (i * 2 + 4));
					}
					state = true;
				}
				if(event.type == InputTypePress && event.key == InputKeyUp) iterations += 1;
				if(event.type == InputTypeRepeat && event.key == InputKeyUp) iterations += 3;
				if(event.type == InputTypePress && event.key == InputKeyDown) iterations -= 1;
				if(event.type == InputTypeRepeat && event.key == InputKeyDown) iterations -= 3;
			}
		}
		view_port_update(view_port);
	}

	view_port_enabled_set(view_port, false);
	gui_remove_view_port(gui, view_port);
	view_port_free(view_port);
	furi_message_queue_free(event_queue);

	furi_record_close(RECORD_GUI);

	return 0;
}
