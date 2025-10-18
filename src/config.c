#include "config.h"
#include "common.h"

AppConfig app_config;

void init_app_config(int fd_in, int fd_out) {
    app_config.input_fd = (fd_in > 0) ? fd_in : STDIN_FILENO;
    app_config.output_fd = (fd_out > 0) ? fd_in : STDOUT_FILENO;
}