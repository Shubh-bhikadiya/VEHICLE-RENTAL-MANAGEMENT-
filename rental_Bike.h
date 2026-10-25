#ifndef RENTAL_BIKE_H
#define RENTAL_BIKE_H
typedef struct
{
    int rental_id;
    char customer_name[100];
    char Brand[50];
    char Model[50];
    char reg_no[50];
    int days;
    float total_cost;
    /* #new: Keep contact and date details with each booking. */
    char phone_number[20];
    char start_date[11];
    char due_date[11];
    char return_date[11];
    int late_days;
    float late_fee;
} Rental_Bike;

void rentBike(void);
void returnBike(void);
void viewActiveRental_Bikes(void);

#endif // RENTAL_BIKE_H
