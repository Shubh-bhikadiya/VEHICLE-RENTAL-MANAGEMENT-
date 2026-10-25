#ifndef RENTAL_CORE_H
#define RENTAL_CORE_H

#include <stddef.h>

#define RC_MAX_ROWS 1000

typedef enum
{
    RC_CAR = 0,
    RC_BIKE = 1
} RcKind;

typedef struct
{
    char brand[50];
    char model[50];
    char reg_no[50];
    float rate;
} RcVehicle;

typedef struct
{
    int rental_id;
    char customer_name[100];
    char brand[50];
    char model[50];
    char reg_no[50];
    int days;
    float total_cost;
    char phone_number[20];
    char start_date[11];
    char due_date[11];
    char return_date[11];
    int late_days;
    float late_fee;
    RcKind kind;
} RcRental;

/* #new: These data functions are shared by the terminal menus and other front ends. */
int rc_load_vehicles(RcKind kind, RcVehicle *rows, size_t capacity, size_t *count,
                     char *error, size_t error_size);
int rc_add_vehicle(RcKind kind, const char *brand, const char *model,
                   const char *reg_no, float rate, char *error, size_t error_size);
int rc_update_rate(RcKind kind, const char *reg_no, float rate,
                   char *error, size_t error_size);
int rc_delete_vehicle(RcKind kind, const char *reg_no,
                      char *error, size_t error_size);
int rc_load_active(RcRental *rows, size_t capacity, size_t *count,
                   char *error, size_t error_size);
int rc_rent_vehicle(RcKind kind, const char *reg_no, const char *customer_name,
                    const char *phone, int days, RcRental *created,
                    char *error, size_t error_size);
int rc_return_rental(int rental_id, RcRental *returned,
                     char *error, size_t error_size);
int rc_name_to_display(const char *saved_name, char *display, size_t size);
const char *rc_kind_name(RcKind kind);

#endif
