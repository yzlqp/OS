/**
 * @file dev_t.h
 * @author ylp
 * @brief 
 * @version 0.1
 * @date 2022-05-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef DEV_T_H
#define DEV_T_H

#define MINORBITS	    20
#define MINORMASKK	    ((1U << MINORBITS) - 1)

#define MAJOR(dev)	    ((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev)	    ((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma, mi)	(((ma) << MINORBITS) | (mi))

#endif /* DEV_T_H */