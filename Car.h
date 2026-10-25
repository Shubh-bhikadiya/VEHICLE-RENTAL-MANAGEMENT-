#ifndef CAR_H
#define CAR_H

typedef struct car
{
    char Brand[50];
    char Model[50];
    char reg_no[50];
    float rate;
} Car;

void addCar();
void viewCar();
void updateCar();
void deleteCar();
/* #new: Let customers search and sort available cars. */
void searchCar(void);
void sortCar(void);

#endif // CAR_H
