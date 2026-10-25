#ifndef RENTAL_CAR_H
#define RENTAL_CAR_H

typedef struct 
{
    int rental_id;
    char customer_name[100];
    char Brand[50];
    char Model[50];
    char reg_no[50];
    int days;
    float total_cost;
    char phone_number[20];
    char start_date[11];
    char due_date[11];
    char return_date[11];
    int late_days;
    float late_fee;
} Rental_Car;

void rentCar(void);
void returnCar(void);
void viewActiveRental_Cars(void);

#endif // RENTAL_CAR_H
