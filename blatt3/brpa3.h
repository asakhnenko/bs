#ifndef BRPA3_H
#define BRPA3_H

#define UNIQUE_DEV_NUMBER 13
#define BRPA3_SET_SECRET _IOW(UNIQUE_DEV_NUMBER, 1, unsigned short*)
#define BRPA3_SET_OPENKEY _IOW(UNIQUE_DEV_NUMBER, 2, unsigned short*)
#define BRPA3_GET_OPENKEY _IOR(UNIQUE_DEV_NUMBER, 3, unsigned short*)

#endif /*BRPA3_H*/
