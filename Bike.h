#ifndef BIKE_H
#define BIKE_H

typedef struct 
{
    char Brand[50];
    char Model[50];
    char reg_no[50];
    float rate;
} Bike;

void addBike();
void viewBike();
void updateBike();
void deleteBike();
/* #new: Let customers search and sort available bikes. */
void searchBike(void);
void sortBike(void);

#endif // BIKE_H
