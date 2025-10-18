#ifndef CONFIG_H
#define CONFIG_H


typedef struct  {
    int input_fd;
    int output_fd;
} AppConfig;

extern AppConfig app_config;

void init_app_config(int fd_in, int fd_out);

#endif
