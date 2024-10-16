#ifndef WUT_H
#define WUT_H

void wut_init(void);
int wut_create(void (*run)(void));
int wut_id(void);
int wut_yield(void);
int wut_cancel(int id);
int wut_join(int id);
void wut_exit(int status);

#endif
