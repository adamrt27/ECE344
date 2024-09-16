#ifndef SSP
#define SSP

void ssp_init();
int ssp_create(char *const *argv, int fd0, int fd1, int fd2);
void ssp_send_signal(int ssp_id, int signum);
int ssp_get_status(int ssp_id);
void ssp_wait();
void ssp_print();

#endif
