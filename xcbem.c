#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <xcb/xcb.h>

int main(void) {
    xcb_connection_t *connection;
    xcb_screen_t *screen;
    xcb_window_t root, window;
    xcb_gcontext_t gc;
    xcb_font_t font;

    // Open the connection to the X server
    connection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(connection)) {
        fprintf(stderr, "Error: Could not connect to the X server.\n");
        return EXIT_FAILURE;
    }

    // Get the first screen
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    root = screen->root;

    // Create a window
    window = xcb_generate_id(connection);
    uint32_t mask = XCB_CW_EVENT_MASK;
    uint32_t values[1] = {XCB_EVENT_MASK_EXPOSURE |
                          XCB_EVENT_MASK_BUTTON_PRESS |
                          XCB_EVENT_MASK_BUTTON_RELEASE |
                          XCB_EVENT_MASK_KEY_PRESS |
                          XCB_EVENT_MASK_KEY_RELEASE |
                          XCB_EVENT_MASK_ENTER_WINDOW |
                          XCB_EVENT_MASK_LEAVE_WINDOW |
                          XCB_EVENT_MASK_POINTER_MOTION |
                          XCB_EVENT_MASK_STRUCTURE_NOTIFY};

    xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, root, 0, 0, 560, 50, 10,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);

    // Set the window title
    const char *window_title = "XCB Event Monitor";
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING, 8, strlen(window_title), window_title);

    // Set the background color to black
    uint32_t black_pixel = 0x000000;
    xcb_change_window_attributes(connection, window, XCB_CW_BACK_PIXEL, &black_pixel);

    // Create a green color and get its pixel value
    xcb_colormap_t colormap = xcb_generate_id(connection);
    xcb_create_colormap(connection, XCB_COLORMAP_ALLOC_NONE, colormap, root, screen->root_visual);
    uint16_t green_values[] = {0, 65535, 0};
    xcb_alloc_color_reply_t *green_color = xcb_alloc_color_reply(connection, xcb_alloc_color(connection, colormap, green_values[0], green_values[1], green_values[2]), NULL);
    uint32_t green_pixel = green_color->pixel;
    free(green_color);

    // Map the window on the screen
    xcb_map_window(connection, window);
    xcb_flush(connection);

    // Create a graphics context with green text color and black background color
    gc = xcb_generate_id(connection);
    xcb_create_gc(connection, gc, window, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND, (const uint32_t[]){black_pixel, green_pixel});

    // Load a larger font (change the font name to an available font on your system)
    font = xcb_generate_id(connection);
    const char *font_name = "-misc-fixed-bold-r-normal--18-120-100-100-c-90-iso8859-1";
    xcb_open_font(connection, font, strlen(font_name), font_name);

    // Set the font for the graphics context
    xcb_change_gc(connection, gc, XCB_GC_FONT, &font);

    // Event loop
    xcb_generic_event_t *event;
    while ((event = xcb_wait_for_event(connection))) {
        char event_text[100];
        uint16_t width;
        uint16_t height;

        switch (event->response_type & ~0x80) {
            case XCB_EXPOSE: {
                xcb_expose_event_t *expose_event = (xcb_expose_event_t *)event;
                snprintf(event_text, sizeof(event_text), "Expose Event: x=%d, y=%d, width=%d, height=%d", expose_event->x, expose_event->y, expose_event->width, expose_event->height);
                break;
            }
            case XCB_BUTTON_PRESS: {
                xcb_button_press_event_t *bp_event = (xcb_button_press_event_t *)event;
                snprintf(event_text, sizeof(event_text), "Button Press Event: x=%d, y=%d, button=%d", bp_event->event_x, bp_event->event_y, bp_event->detail);
                break;
            }
            case XCB_BUTTON_RELEASE: {
                xcb_button_release_event_t *br_event = (xcb_button_release_event_t *)event;
                snprintf(event_text, sizeof(event_text), "Button Release Event: x=%d, y=%d, button=%d", br_event->event_x, br_event->event_y, br_event->detail);
                break;
            }
            case XCB_KEY_PRESS: {
                xcb_key_press_event_t *kp_event = (xcb_key_press_event_t *)event;
                snprintf(event_text, sizeof(event_text), "Key Press Event: keycode=%d, state=%d", kp_event->detail, kp_event->state);
                break;
            }
            case XCB_KEY_RELEASE: {
                xcb_key_release_event_t *kr_event = (xcb_key_release_event_t *)event;
                snprintf(event_text, sizeof(event_text), "Key Release Event: keycode=%d, state=%d", kr_event->detail, kr_event->state);
                break;
            }
            case XCB_ENTER_NOTIFY: {
                xcb_enter_notify_event_t *en_event = (xcb_enter_notify_event_t *)event;
                snprintf(event_text, sizeof(event_text), "Enter Notify Event: x=%d, y=%d", en_event->event_x, en_event->event_y);
                break;
            }
            case XCB_LEAVE_NOTIFY: {
                xcb_leave_notify_event_t *lv_event = (xcb_leave_notify_event_t *)event;
                snprintf(event_text, sizeof(event_text), "Leave Notify Event: x=%d, y=%d", lv_event->event_x, lv_event->event_y);
                break;
            }
            case XCB_MOTION_NOTIFY: {
                xcb_motion_notify_event_t *mn_event = (xcb_motion_notify_event_t *)event;
                snprintf(event_text, sizeof(event_text), "Motion Notify Event: x=%d, y=%d", mn_event->event_x, mn_event->event_y);
                break;
            }
            case XCB_CONFIGURE_NOTIFY: {
                xcb_configure_notify_event_t *cn_event = (xcb_configure_notify_event_t *)event;
                width = cn_event->width;
                height = cn_event->height;
                snprintf(event_text, sizeof(event_text), "Configure Notify Event: x=%d, y=%d, width=%d, height=%d", cn_event->x, cn_event->y, cn_event->width, cn_event->height);
                break;
            }
            default:
                snprintf(event_text, sizeof(event_text), "Unknown Event");
                break;
        }

        // Clear the window and draw the updated text
        xcb_rectangle_t rect = {0, 0, width, height};
        xcb_poly_fill_rectangle(connection, window, gc, 1, &rect);
        xcb_image_text_8(connection, strlen(event_text), window, gc, 20, 30, event_text);
        xcb_flush(connection);

        free(event);
    }

    // Close the font
    xcb_close_font(connection, font);

    xcb_disconnect(connection);
    return EXIT_SUCCESS;
}