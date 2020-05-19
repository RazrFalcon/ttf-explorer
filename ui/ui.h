#pragma once

typedef struct ttfe_ui_app ttfe_ui_app;

extern "C" {
    ttfe_ui_app* ttfe_ui_init();
    void ttfe_ui_exec(ttfe_ui_app *app, const char *path);
}
