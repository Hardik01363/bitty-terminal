//For terminal window GUI and Text Rendering, I'm using the leif library and its functions. Definitions will be a bit different for Wayland users. Thse definitions are tried and tested on my local machine using X11.
#define LF_X11
#define LF_RUNARA
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pty.h>
#include <sys/select.h>
#include <X11/Xlib.h>
#include <leif/leif.h>
#include <leif/win.h>
#include <leif/ui_core.h>

static int32_t masterfd;

int32_t decode_utf8(const char *s, uint32_t  *out_cp) {// Returns length of utf8 string
    unsigned char c = s[0];
    if (c < 0x80) {
        *out_cp = c;
        return 1;
    } else if ((c >> 5) == 0x6) {
        *out_cp = ((c & 0x1F) << 6) | (s[1] & 0x3F);
        return 2;
    } else if ((c >> 4) == 0xE) {
        *out_cp = ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        return 3;
    } else if ((c >> 3) == 0x1E) {
        *out_cp = ((c & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
        return 4;
    }
    return -1; // invalid UTF-8
}

//only useful parts of the function lf_ui_core_next_event() modified from ../bitty-terminal/reif/src/ui_core.c
void term_next_event(lf_ui_state_t* ui) {
  float cur_time = lf_ui_core_get_elapsed_time();
  ui->delta_time = cur_time - ui->_last_time;
  ui->_last_time = cur_time;

  bool rendered = lf_windowing_get_current_event() == LF_EVENT_WINDOW_REFRESH;
  lf_ui_core_shape_widgets_if_needed(ui, ui->root, false);

  if(ui->needs_render) {
    lf_win_make_gl_context(ui->win);
    vec2s winsize = lf_win_get_size(ui->win);
    ui->render_clear_color_area(lf_color_from_hex(0xa03cf2), LF_SCALE_CONTAINER(winsize.x, winsize.y), winsize.y);
    ui->render_begin(ui->render_state);
    ui->render_rect(ui->render_state, (vec2s){50, 50}, (vec2s){50, 50}, LF_RED, LF_NO_COLOR, 0.0f, 0.0f);
    ui->render_end(ui->render_state);
    lf_win_swap_buffers(ui->win);
    ui->needs_render = false;
    rendered = true;
  }

  lf_windowing_update();
}

size_t read_from_pty(void) {
    static char buff[SHRT_MAX];
    static uint32_t buff_len = 0;

    int32_t read_bytes = read(masterfd, buff + buff_len, sizeof(buff) - buff_len);
    if(read_bytes <= 0) return 0;
    buff_len += read_bytes;

    uint32_t iter = 0;
    while(iter < buff_len) {
        uint32_t codepoint;
        int32_t len = decode_utf8(&buff[iter], &codepoint);
        if(len == -1 || len > buff_len) break;
        iter += len;
    }

    if(iter < buff_len) {
        memmove(buff, buff + iter, buff_len - iter);
    }
    
    buff_len -= iter;
    return read_bytes;
}

int main(void) {
    if(forkpty(&masterfd, NULL, NULL, NULL) == 0) {
        execlp("usr/bin/bash", "bash", NULL);
        perror("execlp");
        exit(1);
    }

    lf_windowing_init();
    lf_window_t window = lf_ui_core_create_window(1560, 980, "bitty - custom terminal & shell"); //parameters for lf_ui_core_create_window(pixel width, pixel height, terminal title)
    lf_ui_state_t* ui = lf_ui_core_init(window); //initialising UI state

    fd_set fdset;
    int32_t x11fd = ConnectionNumber(lf_win_get_x11_display());
    bool first_run = true;
    while(ui->running) {
        //Clearing and rebuilding the fdset each loop
        FD_ZERO(&fdset);
        FD_SET(masterfd, &fdset);
        FD_SET(x11fd, &fdset);

        if(first_run) {
            term_next_event(ui);
            first_run = false;
        }

        select(MAX(x11fd, masterfd)+1, &fdset, NULL, NULL, NULL);
        
        if(FD_ISSET(masterfd, &fdset)) {
            read_from_pty();
        }

        if(FD_ISSET(x11fd, &fdset)) {
            lf_windowing_next_event();
            lf_event_type_t curr_event = lf_windowing_get_current_event();
            if(curr_event == LF_EVENT_KEY_PRESS || curr_event == LF_EVENT_TYPING_CHAR || curr_event == LF_EVENT_WINDOW_REFRESH || curr_event == LF_EVENT_WINDOW_RESIZE || curr_event == LF_EVENT_WINDOW_CLOSE) {
                term_next_event(ui);
            }
        }
    }

    printf("Hello World!");
    return EXIT_SUCCESS;
}
