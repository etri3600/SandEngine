#include "Linux.h"

#if __LINUX__
bool SLinux::Init()
{
	int screenp = 0;
	connection = xcb_connect(NULL, &screenp);

	if (xcb_connection_has_error(connection))
		exitOnError("Failed to connect to X server using XCB.");

	xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(connection));

	for (int s = screenp; s > 0; s--)
		xcb_screen_next(&iter);

	screen = iter.data;

	uint32_t eventMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t valueList[] = { screen->black_pixel, 0 };
	window = xcb_generate_id(connection);

	xcb_create_window(
		connection,
		XCB_COPY_FROM_PARENT,
		window,
		screen->root,
		0,
		0,
		windowWidth,
		windowHeight,
		0,
		XCB_WINDOW_CLASS_INPUT_OUTPUT,
		screen->root_visual,
		eventMask,
		valueList);

	return true;
}

bool SLinux::Tick()
{
	bool running = true;
	xcb_generic_event_t *event;
	xcb_client_message_event_t *cm;

	while ((event = xcb_poll_for_event(connection, 0))) {
		switch (event->response_type & ~0x80) {
			case XCB_CLIENT_MESSAGE: {
				cm = (xcb_client_message_event_t *)event;
				if (cm->data.data32[0] == wmDeleteWin)
					running = false;
				break;
			}
		}
	}

	return running;
}

#endif