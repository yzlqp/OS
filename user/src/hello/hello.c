/**
 * @file hello.c
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-05-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "user.h"

char s[] = "                  \n    \
                              \n    \
        _|_|      _|_|_|      \n    \
      _|    _|  _|            \n    \
      _|    _|    _|_|        \n    \
      _|    _|        _|      \n    \
        _|_|     _|_|_|       \n    \
                              \n    \
                              \n    \
          Based on ARMv8      \n    \
          Hello, World        \n";

int main()
{
    printf("%s", s);
    exit(0);
}