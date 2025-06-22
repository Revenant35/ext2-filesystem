//
// Created by Zach Brown on 6/18/25.
//

#ifndef GLOBALS_H
#define GLOBALS_H

#define SUCCESS 0
#define INVALID_PARAMETER -1
#define IO_ERROR -2
#define ERROR -3

#define log_error(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)

#endif //GLOBALS_H
