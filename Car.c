#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "Car.h"
#include "rental_core.h"

/* FIX: Read one bounded word and clear the rest of an overlong input line. */
static int readWord(char *text, size_t size)
{
    char format[20];
    int next, too_long = 0;
    if (size < 2) return 0;
    snprintf(format, sizeof(format), "%%%zus", size - 1);
    if (scanf(format, text) != 1)
    {
        while ((next = getchar()) != '\n' && next != EOF) {}
        return 0;
    }
    next = getchar();
    if (next != '\n' && next != EOF)
    {
        too_long = !isspace((unsigned char)next);
        while (next != '\n' && next != EOF) next = getchar();
    }
    return !too_long;
}

/* FIX: Check rate input before passing it to the shared vehicle function. */
static int readRate(float *rate)
{
    int next;
    if (scanf("%f", rate) != 1)
    {
        while ((next = getchar()) != '\n' && next != EOF) {}
        return 0;
    }
    while ((next = getchar()) != '\n' && next != EOF) {}
    return 1;
}

static void showVehicles(RcVehicle *rows, size_t count)
{
    size_t i;
    if (count == 0) printf("No available cars found.\n");
    for (i = 0; i < count; i++)
        printf("Brand: %s\nModel: %s\nReg_no: %s\nRate per day: %.2f\n\n",
               rows[i].brand, rows[i].model, rows[i].reg_no, rows[i].rate);
}

void addCar(void)
{
    char brand[50], model[50], reg_no[50], error[256] = "";
    float rate;
    printf("\nBrand: ");
    if (!readWord(brand, sizeof(brand))) { printf("Please enter a shorter brand name.\n"); return; }
    printf("Model: ");
    if (!readWord(model, sizeof(model))) { printf("Please enter a shorter model name.\n"); return; }
    printf("Reg_no: ");
    if (!readWord(reg_no, sizeof(reg_no))) { printf("Please enter a shorter registration number.\n"); return; }
    printf("Rate: ");
    if (!readRate(&rate)) { printf("Rate must be a number greater than zero.\n"); return; }
    if (!rc_add_vehicle(RC_CAR, brand, model, reg_no, rate, error, sizeof(error)))
        printf("%s\n", error);
    else
        printf("Vehicle added successfully!\n");
}

void viewCar(void)
{
    RcVehicle rows[RC_MAX_ROWS];
    size_t count;
    char error[256] = "";
    if (!rc_load_vehicles(RC_CAR, rows, RC_MAX_ROWS, &count, error, sizeof(error)))
        printf("%s\n", error);
    else
        showVehicles(rows, count);
}

void updateCar(void)
{
    char reg_no[50], error[256] = "";
    float rate;
    viewCar();
    printf("Enter registration number to edit: ");
    if (!readWord(reg_no, sizeof(reg_no))) { printf("Registration number is too long.\n"); return; }
    printf("Enter new rate: ");
    if (!readRate(&rate)) { printf("Rate must be a number greater than zero.\n"); return; }
    if (!rc_update_rate(RC_CAR, reg_no, rate, error, sizeof(error)))
        printf("%s\n", error);
    else
        printf("Car details updated successfully!\n");
}

void deleteCar(void)
{
    char reg_no[50], error[256] = "";
    viewCar();
    printf("Enter registration number to delete: ");
    if (!readWord(reg_no, sizeof(reg_no))) { printf("Registration number is too long.\n"); return; }
    if (!rc_delete_vehicle(RC_CAR, reg_no, error, sizeof(error)))
        printf("%s\n", error);
    else
        printf("Car deleted successfully.\n");
}

/* #new: Search and sort use the same saved vehicle rows as the GTK screen. */
static int containsText(const char *text, const char *search)
{
    size_t i, j;
    for (i = 0; text[i] != '\0'; i++)
    {
        for (j = 0; search[j] != '\0' && text[i + j] != '\0'; j++)
            if (tolower((unsigned char)text[i + j]) != tolower((unsigned char)search[j])) break;
        if (search[j] == '\0') return 1;
    }
    return search[0] == '\0';
}

void searchCar(void)
{
    RcVehicle rows[RC_MAX_ROWS];
    size_t count, i;
    char search[50], error[256] = "";
    if (!rc_load_vehicles(RC_CAR, rows, RC_MAX_ROWS, &count, error, sizeof(error)))
    { printf("%s\n", error); return; }
    printf("Enter brand, model, or registration to search: ");
    if (!readWord(search, sizeof(search))) { printf("Search word is too long.\n"); return; }
    for (i = 0; i < count; i++)
        if (containsText(rows[i].brand, search) || containsText(rows[i].model, search) ||
            containsText(rows[i].reg_no, search))
            printf("%s %s %s - %.2f per day\n", rows[i].brand, rows[i].model,
                   rows[i].reg_no, rows[i].rate);
}

void sortCar(void)
{
    RcVehicle rows[RC_MAX_ROWS], temp;
    size_t count, i, j;
    int choice, next;
    char error[256] = "";
    if (!rc_load_vehicles(RC_CAR, rows, RC_MAX_ROWS, &count, error, sizeof(error)))
    { printf("%s\n", error); return; }
    printf("Sort by: 1. Brand  2. Rate per day\nEnter choice: ");
    if (scanf("%d", &choice) != 1)
    {
        while ((next = getchar()) != '\n' && next != EOF) {}
        printf("Please enter 1 or 2.\n"); return;
    }
    while ((next = getchar()) != '\n' && next != EOF) {}
    if (choice != 1 && choice != 2) { printf("Please enter 1 or 2.\n"); return; }
    for (i = 0; i < count; i++)
        for (j = 0; j + 1 < count - i; j++)
        {
            int swap = choice == 1 ? strcmp(rows[j].brand, rows[j + 1].brand) > 0
                                   : rows[j].rate > rows[j + 1].rate;
            if (swap) { temp = rows[j]; rows[j] = rows[j + 1]; rows[j + 1] = temp; }
        }
    showVehicles(rows, count);
}
