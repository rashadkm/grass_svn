
#ifndef __GLOBALS_H_
#define __GLOBALS_H_

extern volatile int floating_point_exception;
extern volatile int floating_point_exception_occurred;
extern int overflow_occurred;
extern int overwrite_flag;
extern long seed_value;
extern long seeded;

extern int current_depth, current_row;
extern int depths, rows, columns;

#endif /* __GLOBALS_H_ */
