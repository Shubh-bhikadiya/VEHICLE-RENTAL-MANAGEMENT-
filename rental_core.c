#include "rental_core.h"

#include <ctype.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const char *vehicle_file(RcKind kind)
{
    return kind == RC_CAR ? "Car.txt" : "Bike.txt";
}

static const char *rental_file(RcKind kind)
{
    return kind == RC_CAR ? "rental_Car.txt" : "rental_Bike.txt";
}

const char *rc_kind_name(RcKind kind)
{
    return kind == RC_CAR ? "Car" : "Bike";
}

static void set_error(char *error, size_t size, const char *message)
{
    if (size > 0)
        snprintf(error, size, "%s", message);
}

static int same_registration(const char *first, const char *second)
{
    size_t i;
    for (i = 0; first[i] != '\0' && second[i] != '\0'; i++)
        if (toupper((unsigned char)first[i]) != toupper((unsigned char)second[i]))
            return 0;
    return first[i] == '\0' && second[i] == '\0';
}

static int save_vehicles(RcKind kind, RcVehicle *rows, size_t count,
                         char *error, size_t error_size)
{
    char temporary[40];
    FILE *fp;
    size_t i;
    int okay = 1;
    snprintf(temporary, sizeof(temporary), "%s.core.tmp", vehicle_file(kind));
    fp = fopen(temporary, "w");
    if (fp == NULL)
    {
        set_error(error, error_size, "Could not create a temporary vehicle file.");
        return 0;
    }
    for (i = 0; i < count; i++)
        if (fprintf(fp, "%s %s %s %.2f\n", rows[i].brand, rows[i].model,
                    rows[i].reg_no, rows[i].rate) < 0)
            okay = 0;
    if (fclose(fp) != 0)
        okay = 0;
    if (!okay || rename(temporary, vehicle_file(kind)) != 0)
    {
        remove(temporary);
        set_error(error, error_size, "Could not save the vehicle file.");
        return 0;
    }
    return 1;
}

static int read_one_rental(FILE *fp, RcRental *row)
{
    return fscanf(fp, "%d %99s %49s %49s %49s %d %f %19s %10s %10s %10s %d %f",
                  &row->rental_id, row->customer_name, row->brand, row->model,
                  row->reg_no, &row->days, &row->total_cost, row->phone_number,
                  row->start_date, row->due_date, row->return_date,
                  &row->late_days, &row->late_fee);
}

static int write_one_rental(FILE *fp, const RcRental *row)
{
    return fprintf(fp, "%d %s %s %s %s %d %.2f %s %s %s %s %d %.2f\n",
                   row->rental_id, row->customer_name, row->brand, row->model,
                   row->reg_no, row->days, row->total_cost, row->phone_number,
                   row->start_date, row->due_date, row->return_date,
                   row->late_days, row->late_fee) >= 0;
}

static int load_rentals_from(const char *file_name, RcRental *rows, size_t capacity,
                             size_t *count, RcKind kind, char *error, size_t error_size)
{
    FILE *fp = fopen(file_name, "r");
    int fields;
    *count = 0;
    if (fp == NULL)
    {
        set_error(error, error_size, "Could not open a rental data file.");
        return 0;
    }
    while (1)
    {
        RcRental row;
        fields = read_one_rental(fp, &row);
        if (fields == EOF) break;
        if (fields != 13)
        {
            fclose(fp);
            set_error(error, error_size, "Rental data has an invalid record.");
            return 0;
        }
        if (*count >= capacity)
        {
            fclose(fp);
            set_error(error, error_size, "Rental list has too many records.");
            return 0;
        }
        row.kind = kind;
        rows[*count] = row;
        (*count)++;
    }
    if (ferror(fp) || fclose(fp) != 0)
    {
        set_error(error, error_size, "Could not read a rental data file.");
        return 0;
    }
    return 1;
}

static int save_rentals(const char *file_name, RcRental *rows, size_t count,
                        char *error, size_t error_size)
{
    char temporary[48];
    FILE *fp;
    size_t i;
    int okay = 1;
    snprintf(temporary, sizeof(temporary), "%s.core.tmp", file_name);
    fp = fopen(temporary, "w");
    if (fp == NULL)
    {
        set_error(error, error_size, "Could not create a temporary rental file.");
        return 0;
    }
    for (i = 0; i < count; i++)
        if (!write_one_rental(fp, &rows[i]))
            okay = 0;
    if (fclose(fp) != 0)
        okay = 0;
    if (!okay || rename(temporary, file_name) != 0)
    {
        remove(temporary);
        set_error(error, error_size, "Could not save the rental file.");
        return 0;
    }
    return 1;
}

int rc_load_vehicles(RcKind kind, RcVehicle *rows, size_t capacity, size_t *count,
                     char *error, size_t error_size)
{
    FILE *fp = fopen(vehicle_file(kind), "r");
    int fields;
    *count = 0;
    if (fp == NULL)
    {
        set_error(error, error_size, "Could not open the available vehicle file.");
        return 0;
    }
    while (1)
    {
        RcVehicle row;
        fields = fscanf(fp, "%49s %49s %49s %f", row.brand, row.model,
                        row.reg_no, &row.rate);
        if (fields == EOF) break;
        if (fields != 4)
        {
            fclose(fp);
            set_error(error, error_size, "Vehicle data has an invalid record.");
            return 0;
        }
        if (*count >= capacity)
        {
            fclose(fp);
            set_error(error, error_size, "Vehicle list has too many records.");
            return 0;
        }
        rows[*count] = row;
        (*count)++;
    }
    if (ferror(fp) || fclose(fp) != 0)
    {
        set_error(error, error_size, "Could not read the available vehicle file.");
        return 0;
    }
    return 1;
}

static int registration_is_used(const char *registration, char *error, size_t error_size)
{
    RcVehicle vehicles[RC_MAX_ROWS];
    RcRental rentals[RC_MAX_ROWS];
    size_t count, i;
    RcKind kind;
    for (kind = RC_CAR; kind <= RC_BIKE; kind++)
    {
        if (!rc_load_vehicles(kind, vehicles, RC_MAX_ROWS, &count, error, error_size))
            return -1;
        for (i = 0; i < count; i++)
            if (same_registration(vehicles[i].reg_no, registration))
                return 1;
    }
    for (kind = RC_CAR; kind <= RC_BIKE; kind++)
    {
        if (!load_rentals_from(rental_file(kind), rentals, RC_MAX_ROWS, &count,
                               kind, error, error_size))
            return -1;
        for (i = 0; i < count; i++)
            if (same_registration(rentals[i].reg_no, registration))
                return 1;
    }
    return 0;
}

int rc_add_vehicle(RcKind kind, const char *brand, const char *model,
                   const char *reg_no, float rate, char *error, size_t error_size)
{
    RcVehicle rows[RC_MAX_ROWS];
    size_t count;
    int used;
    if (brand == NULL || model == NULL || reg_no == NULL || !isfinite(rate) || rate <= 0.0f ||
        strlen(brand) == 0 || strlen(brand) >= 50 || strlen(model) == 0 ||
        strlen(model) >= 50 || strlen(reg_no) == 0 || strlen(reg_no) >= 50 ||
        strpbrk(brand, " \t\n") || strpbrk(model, " \t\n") || strpbrk(reg_no, " \t\n"))
    {
        set_error(error, error_size, "Enter a positive rate and one-word brand, model, and registration.");
        return 0;
    }
    used = registration_is_used(reg_no, error, error_size);
    if (used != 0)
    {
        if (used > 0)
            set_error(error, error_size, "That registration number is already used.");
        return 0;
    }
    if (!rc_load_vehicles(kind, rows, RC_MAX_ROWS, &count, error, error_size))
        return 0;
    if (count >= RC_MAX_ROWS)
    {
        set_error(error, error_size, "Vehicle list is full.");
        return 0;
    }
    snprintf(rows[count].brand, sizeof(rows[count].brand), "%s", brand);
    snprintf(rows[count].model, sizeof(rows[count].model), "%s", model);
    snprintf(rows[count].reg_no, sizeof(rows[count].reg_no), "%s", reg_no);
    rows[count].rate = rate;
    return save_vehicles(kind, rows, count + 1, error, error_size);
}

int rc_update_rate(RcKind kind, const char *reg_no, float rate,
                   char *error, size_t error_size)
{
    RcVehicle rows[RC_MAX_ROWS];
    size_t count, i;
    if (!isfinite(rate) || rate <= 0.0f)
    {
        set_error(error, error_size, "Rate must be greater than zero.");
        return 0;
    }
    if (!rc_load_vehicles(kind, rows, RC_MAX_ROWS, &count, error, error_size))
        return 0;
    for (i = 0; i < count; i++)
        if (strcmp(rows[i].reg_no, reg_no) == 0)
        {
            rows[i].rate = rate;
            return save_vehicles(kind, rows, count, error, error_size);
        }
    set_error(error, error_size, "Vehicle was not found.");
    return 0;
}

int rc_delete_vehicle(RcKind kind, const char *reg_no, char *error, size_t error_size)
{
    RcVehicle rows[RC_MAX_ROWS], remaining[RC_MAX_ROWS];
    size_t count, i, kept = 0;
    int found = 0;
    if (!rc_load_vehicles(kind, rows, RC_MAX_ROWS, &count, error, error_size))
        return 0;
    for (i = 0; i < count; i++)
    {
        if (strcmp(rows[i].reg_no, reg_no) == 0)
            found = 1;
        else
            remaining[kept++] = rows[i];
    }
    if (!found)
    {
        set_error(error, error_size, "Vehicle was not found.");
        return 0;
    }
    return save_vehicles(kind, remaining, kept, error, error_size);
}

int rc_load_active(RcRental *rows, size_t capacity, size_t *count,
                   char *error, size_t error_size)
{
    size_t used = 0, got;
    RcKind kind;
    for (kind = RC_CAR; kind <= RC_BIKE; kind++)
    {
        if (used >= capacity ||
            !load_rentals_from(rental_file(kind), rows + used, capacity - used,
                               &got, kind, error, error_size))
            return 0;
        used += got;
    }
    *count = used;
    return 1;
}

int rc_load_completed(RcRental *rows, size_t capacity, size_t *count,
                      char *error, size_t error_size)
{
    return load_rentals_from("completed_rentals.txt", rows, capacity, count,
                             RC_CAR, error, error_size);
}

static int next_id(int *id, char *error, size_t error_size)
{
    FILE *fp = fopen("next_rental_id.txt", "r");
    long value;
    if (fp == NULL || fscanf(fp, "%ld", &value) != 1 || value <= 0 || value >= 2147483647L)
    {
        if (fp != NULL) fclose(fp);
        set_error(error, error_size, "Rental ID counter is missing or invalid.");
        return 0;
    }
    fclose(fp);
    *id = (int)value;
    fp = fopen("next_rental_id.txt.core.tmp", "w");
    if (fp == NULL || fprintf(fp, "%d\n", *id + 1) < 0)
    {
        if (fp != NULL) fclose(fp);
        remove("next_rental_id.txt.core.tmp");
        set_error(error, error_size, "Could not update the rental ID counter.");
        return 0;
    }
    if (fclose(fp) != 0 || rename("next_rental_id.txt.core.tmp", "next_rental_id.txt") != 0)
    {
        remove("next_rental_id.txt.core.tmp");
        set_error(error, error_size, "Could not save the rental ID counter.");
        return 0;
    }
    return 1;
}

static int make_dates(int days, char start[11], char due[11])
{
    time_t now = time(NULL), due_time;
    struct tm *today_ptr = localtime(&now);
    struct tm today;
    if (today_ptr == NULL) return 0;
    today = *today_ptr;
    today.tm_hour = 12; today.tm_min = 0; today.tm_sec = 0;
    if (strftime(start, 11, "%Y-%m-%d", &today) == 0) return 0;
    today.tm_mday += days;
    due_time = mktime(&today);
    today_ptr = localtime(&due_time);
    return today_ptr != NULL && strftime(due, 11, "%Y-%m-%d", today_ptr) != 0;
}

static int valid_phone(const char *phone)
{
    size_t i = phone[0] == '+' ? 1 : 0, digits = 0;
    for (; phone[i] != '\0'; i++)
    {
        if (!isdigit((unsigned char)phone[i])) return 0;
        digits++;
    }
    return digits >= 10 && digits <= 15;
}

static int encode_customer_name(const char *name, char saved[100])
{
    size_t i, used = 0;
    if (name == NULL || name[0] == '\0' || strchr(name, '~') != NULL ||
        strchr(name, '\t') != NULL || strchr(name, '\n') != NULL || strlen(name) > 99)
        return 0;
    for (i = 0; name[i] != '\0'; i++)
    {
        if (used + 1 >= 100) return 0;
        saved[used++] = name[i] == ' ' ? '~' : name[i];
    }
    saved[used] = '\0';
    return 1;
}

static void create_receipt(const RcRental *rental, int returning)
{
    char file_name[40];
    FILE *fp;
    snprintf(file_name, sizeof(file_name), "receipt_%d.txt", rental->rental_id);
    fp = fopen(file_name, returning ? "a" : "w");
    if (fp == NULL) return;
    if (!returning)
    {
        fprintf(fp, "VEHICLE RENTAL RECEIPT\nRental ID: %d\nCustomer: ", rental->rental_id);
        {
            char display[100];
            size_t i;
            rc_name_to_display(rental->customer_name, display, sizeof(display));
            for (i = 0; display[i] != '\0'; i++) fputc(display[i], fp);
        }
        fprintf(fp, "\nPhone: %s\n%s: %s %s\nRegistration: %s\nStart date: %s\nDue date: %s\n",
                rental->phone_number, rc_kind_name(rental->kind), rental->brand,
                rental->model, rental->reg_no, rental->start_date, rental->due_date);
        fprintf(fp, "Rental days: %d\nRate per day: %.2f\nRental total: %.2f\n",
                rental->days, rental->total_cost / rental->days, rental->total_cost);
    }
    else
        fprintf(fp, "\nRETURN DETAILS\nReturn date: %s\nLate days: %d\nLate fee: %.2f\nFinal total: %.2f\n",
                rental->return_date, rental->late_days, rental->late_fee,
                rental->total_cost + rental->late_fee);
    fclose(fp);
}

int rc_name_to_display(const char *saved_name, char *display, size_t size)
{
    size_t i = 0, used = 0;
    while (saved_name[i] != '\0')
    {
        char value = saved_name[i++];
        if (value == '~')
        {
            if (saved_name[i] == '~') i++;
            else value = ' ';
        }
        if (used + 1 >= size) return 0;
        display[used++] = value;
    }
    display[used] = '\0';
    return 1;
}

int rc_rent_vehicle(RcKind kind, const char *reg_no, const char *customer_name,
                    const char *phone, int days, RcRental *created,
                    char *error, size_t error_size)
{
    RcVehicle vehicles[RC_MAX_ROWS], remaining[RC_MAX_ROWS];
    RcRental rentals[RC_MAX_ROWS];
    size_t count, kept = 0, rental_count, i;
    int found = -1;
    if (days <= 0 || !valid_phone(phone))
    {
        set_error(error, error_size, "Enter a name, a valid phone, and a positive number of days.");
        return 0;
    }
    if (!rc_load_vehicles(kind, vehicles, RC_MAX_ROWS, &count, error, error_size) ||
        !load_rentals_from(rental_file(kind), rentals, RC_MAX_ROWS, &rental_count,
                           kind, error, error_size))
        return 0;
    for (i = 0; i < count; i++)
    {
        if (strcmp(vehicles[i].reg_no, reg_no) == 0)
            found = (int)i;
        else
            remaining[kept++] = vehicles[i];
    }
    if (found < 0)
    {
        set_error(error, error_size, "That vehicle is not available.");
        return 0;
    }
    if (rental_count >= RC_MAX_ROWS)
    {
        set_error(error, error_size, "Rental list is full.");
        return 0;
    }
    memset(created, 0, sizeof(*created));
    memset(created, 0, sizeof(*created));
    if (!encode_customer_name(customer_name, created->customer_name))
    {
        set_error(error, error_size, "Customer name is too long or contains an unsupported character.");
        return 0;
    }
    if (!next_id(&created->rental_id, error, error_size))
        return 0;
    created->kind = kind;
    snprintf(created->brand, sizeof(created->brand), "%s", vehicles[found].brand);
    snprintf(created->model, sizeof(created->model), "%s", vehicles[found].model);
    snprintf(created->reg_no, sizeof(created->reg_no), "%s", vehicles[found].reg_no);
    snprintf(created->phone_number, sizeof(created->phone_number), "%s", phone);
    created->days = days;
    created->total_cost = vehicles[found].rate * days;
    strcpy(created->return_date, "-");
    if (!make_dates(days, created->start_date, created->due_date))
    {
        set_error(error, error_size, "Could not calculate the rental dates.");
        return 0;
    }
    rentals[rental_count++] = *created;
    if (!save_rentals(rental_file(kind), rentals, rental_count, error, error_size))
        return 0;
    if (!save_vehicles(kind, remaining, kept, error, error_size))
        return 0;
    create_receipt(created, 0);
    return 1;
}

static int late_days_for(const char *due_date, const char *return_date)
{
    int y1, m1, d1, y2, m2, d2;
    struct tm due = {0}, returned = {0};
    time_t due_time, return_time;
    double difference;
    if (sscanf(due_date, "%4d-%2d-%2d", &y1, &m1, &d1) != 3 ||
        sscanf(return_date, "%4d-%2d-%2d", &y2, &m2, &d2) != 3)
        return 0;
    due.tm_year = y1 - 1900; due.tm_mon = m1 - 1; due.tm_mday = d1;
    returned.tm_year = y2 - 1900; returned.tm_mon = m2 - 1; returned.tm_mday = d2;
    due.tm_hour = returned.tm_hour = 12;
    due.tm_isdst = returned.tm_isdst = -1;
    due_time = mktime(&due); return_time = mktime(&returned);
    difference = difftime(return_time, due_time);
    return difference > 0 ? (int)((difference + 43200.0) / 86400.0) : 0;
}

int rc_return_rental(int rental_id, RcRental *returned, char *error, size_t error_size)
{
    RcRental active[RC_MAX_ROWS], remaining[RC_MAX_ROWS], completed[RC_MAX_ROWS];
    RcVehicle vehicles[RC_MAX_ROWS];
    size_t count, kept = 0, completed_count = 0, vehicle_count = 0, i;
    int found = -1;
    time_t now = time(NULL);
    struct tm *today = localtime(&now);
    char today_text[11];
    if (!rc_load_active(active, RC_MAX_ROWS, &count, error, error_size)) return 0;
    for (i = 0; i < count; i++)
    {
        if (active[i].rental_id == rental_id)
            found = (int)i;
        else
            remaining[kept++] = active[i];
    }
    if (found < 0)
    {
        set_error(error, error_size, "No active rental has that ID.");
        return 0;
    }
    *returned = active[found];
    if (today == NULL || strftime(today_text, sizeof(today_text), "%Y-%m-%d", today) == 0)
    {
        set_error(error, error_size, "Could not read today's date.");
        return 0;
    }
    if (returned->days <= 0)
    {
        set_error(error, error_size, "This rental has an invalid day count.");
        return 0;
    }
    snprintf(returned->return_date, sizeof(returned->return_date), "%s", today_text);
    returned->late_days = late_days_for(returned->due_date, today_text);
    returned->late_fee = returned->late_days * (returned->total_cost / returned->days);
    if (!rc_load_completed(completed, RC_MAX_ROWS, &completed_count, error, error_size) ||
        completed_count >= RC_MAX_ROWS)
    {
        if (completed_count >= RC_MAX_ROWS) set_error(error, error_size, "Completed rental history is full.");
        return 0;
    }
    completed[completed_count++] = *returned;
    if (!rc_load_vehicles(returned->kind, vehicles, RC_MAX_ROWS, &vehicle_count, error, error_size) ||
        vehicle_count >= RC_MAX_ROWS)
    {
        if (vehicle_count >= RC_MAX_ROWS) set_error(error, error_size, "Available vehicle list is full.");
        return 0;
    }
    snprintf(vehicles[vehicle_count].brand, sizeof(vehicles[vehicle_count].brand), "%s", returned->brand);
    snprintf(vehicles[vehicle_count].model, sizeof(vehicles[vehicle_count].model), "%s", returned->model);
    snprintf(vehicles[vehicle_count].reg_no, sizeof(vehicles[vehicle_count].reg_no), "%s", returned->reg_no);
    vehicles[vehicle_count].rate = returned->total_cost / returned->days;
    vehicle_count++;

    /* #new: Persist the return, restore the vehicle, and keep completed history. */
    {
        RcRental bikes[RC_MAX_ROWS];
        size_t car_count = 0, bike_count = 0, j;
        for (j = 0; j < kept; j++)
            if (remaining[j].kind == RC_BIKE) bikes[bike_count++] = remaining[j];
            else active[car_count++] = remaining[j];
        if (!save_rentals(rental_file(RC_CAR), active, car_count, error, error_size) ||
            !save_rentals(rental_file(RC_BIKE), bikes, bike_count, error, error_size))
            return 0;
    }
    if (!save_vehicles(returned->kind, vehicles, vehicle_count, error, error_size) ||
        !save_rentals("completed_rentals.txt", completed, completed_count, error, error_size))
        return 0;
    create_receipt(returned, 1);
    return 1;
}

int rc_password_matches(const char *password)
{
    FILE *fp = fopen("admin_password.txt", "r");
    char saved[100];
    int matches = 0;
    if (fp == NULL) return 0;
    if (fscanf(fp, "%99s", saved) == 1 && strcmp(saved, password) == 0)
        matches = 1;
    fclose(fp);
    return matches;
}

int rc_change_password(const char *current, const char *next,
                       char *error, size_t error_size)
{
    FILE *fp;
    if (!rc_password_matches(current))
    {
        set_error(error, error_size, "Current password is incorrect.");
        return 0;
    }
    if (strlen(next) < 8 || strlen(next) >= 100 || strpbrk(next, " \t\n"))
    {
        set_error(error, error_size, "New password must be at least 8 characters with no spaces.");
        return 0;
    }
    fp = fopen("admin_password.txt.core.tmp", "w");
    if (fp == NULL || fprintf(fp, "%s\n", next) < 0)
    {
        if (fp != NULL) fclose(fp);
        remove("admin_password.txt.core.tmp");
        set_error(error, error_size, "Could not save the new password.");
        return 0;
    }
    if (fclose(fp) != 0 || rename("admin_password.txt.core.tmp", "admin_password.txt") != 0)
    {
        remove("admin_password.txt.core.tmp");
        set_error(error, error_size, "Could not replace the admin password file.");
        return 0;
    }
    return 1;
}

static int copy_file(const char *source, const char *target)
{
    FILE *input = fopen(source, "rb"), *output;
    int ch, okay = 1;
    if (input == NULL) return 0;
    output = fopen(target, "wb");
    if (output == NULL) { fclose(input); return 0; }
    while ((ch = fgetc(input)) != EOF)
        if (fputc(ch, output) == EOF) { okay = 0; break; }
    if (ferror(input)) okay = 0;
    if (fclose(input) != 0) okay = 0;
    if (fclose(output) != 0) okay = 0;
    return okay;
}

int rc_create_backup(char *error, size_t error_size)
{
    const char *files[] = {"Car.txt", "Bike.txt", "rental_Car.txt", "rental_Bike.txt",
                           "completed_rentals.txt", "next_rental_id.txt", "admin_password.txt"};
    char stamp[20], target[160];
    time_t now = time(NULL);
    struct tm *current = localtime(&now);
    size_t i;
    int okay = 1;
    if (current == NULL || strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", current) == 0)
    {
        set_error(error, error_size, "Could not create the backup date.");
        return 0;
    }
    for (i = 0; i < sizeof(files) / sizeof(files[0]); i++)
    {
        snprintf(target, sizeof(target), "backup_%s_%s", stamp, files[i]);
        if (!copy_file(files[i], target)) okay = 0;
    }
    {
        DIR *folder = opendir(".");
        struct dirent *entry;
        if (folder == NULL)
            okay = 0;
        else
        {
            while ((entry = readdir(folder)) != NULL)
                if (strncmp(entry->d_name, "receipt_", 8) == 0)
                {
                    snprintf(target, sizeof(target), "backup_%s_%s", stamp, entry->d_name);
                    if (!copy_file(entry->d_name, target)) okay = 0;
                }
            closedir(folder);
        }
    }
    if (!okay)
        set_error(error, error_size, "Backup was incomplete; check that all data files are available.");
    return okay;
}
