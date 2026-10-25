#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "Bike.h"
#include "rental_Bike.h"
#include "rental_core.h"

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

static int readNumber(int *number)
{
    int next;
    if (scanf("%d", number) != 1)
    {
        while ((next = getchar()) != '\n' && next != EOF) {}
        return 0;
    }
    while ((next = getchar()) != '\n' && next != EOF) {}
    return 1;
}

/* #new: Read a full name line so the terminal and GTK forms share name handling. */
static int readFullName(char *name, size_t size)
{
    size_t length;
    int next;
    if (fgets(name, (int)size, stdin) == NULL) return 0;
    length = strlen(name);
    if (length > 0 && name[length - 1] == '\n') name[--length] = '\0';
    else if (!feof(stdin))
    {
        while ((next = getchar()) != '\n' && next != EOF) {}
        return 0;
    }
    if (length > 0 && name[length - 1] == '\r') name[--length] = '\0';
    return length > 0;
}

void rentBike(void)
{
    char reg_no[50], name[100], phone[20], error[256] = "";
    int days;
    RcRental rental;
    viewBike();
    printf("Enter registration number to rent Bike: ");
    if (!readWord(reg_no, sizeof(reg_no))) { printf("Registration number is too long.\n"); return; }
    printf("Enter your full name: ");
    if (!readFullName(name, sizeof(name))) { printf("Name is empty or too long.\n"); return; }
    printf("Enter phone number (10 to 15 digits, optional +): ");
    if (!readWord(phone, sizeof(phone))) { printf("Phone number is too long.\n"); return; }
    printf("How many days do you need the bike? ");
    if (!readNumber(&days)) { printf("Enter a whole number of days.\n"); return; }
    if (!rc_rent_vehicle(RC_BIKE, reg_no, name, phone, days, &rental, error, sizeof(error)))
    {
        printf("%s\n", error);
        return;
    }
    printf("Bike rented successfully!\nRental ID: %d\nPhone: %s\nStart: %s\nDue: %s\nTotal: %.2f\n",
           rental.rental_id, rental.phone_number, rental.start_date,
           rental.due_date, rental.total_cost);
}

void returnBike(void)
{
    int rental_id;
    size_t count, i;
    RcRental active[RC_MAX_ROWS], returned;
    char error[256] = "";
    printf("Enter Rental ID to return Bike: ");
    if (!readNumber(&rental_id) || rental_id <= 0) { printf("Enter a valid rental ID.\n"); return; }
    if (!rc_load_active(active, RC_MAX_ROWS, &count, error, sizeof(error)))
    { printf("%s\n", error); return; }
    for (i = 0; i < count; i++)
        if (active[i].rental_id == rental_id && active[i].kind != RC_BIKE)
        { printf("That rental belongs to a bike. Select Bike to return it.\n"); return; }
    if (!rc_return_rental(rental_id, &returned, error, sizeof(error)))
    { printf("%s\n", error); return; }
    printf("Bike returned on %s.\nLate days: %d\nLate fee: %.2f\nFinal total: %.2f\n",
           returned.return_date, returned.late_days, returned.late_fee,
           returned.total_cost + returned.late_fee);
}

void viewActiveRental_Bikes(void)
{
    RcRental rows[RC_MAX_ROWS];
    size_t count, i;
    char error[256] = "", name[100];
    if (!rc_load_active(rows, RC_MAX_ROWS, &count, error, sizeof(error)))
    { printf("%s\n", error); return; }
    for (i = 0; i < count; i++)
        if (rows[i].kind == RC_BIKE)
        {
            rc_name_to_display(rows[i].customer_name, name, sizeof(name));
            printf("\nRental ID: %d\nCustomer: %s\nPhone: %s\nBike: %s %s\nRegistration: %s\n",
                   rows[i].rental_id, name, rows[i].phone_number, rows[i].brand,
                   rows[i].model, rows[i].reg_no);
            printf("Start: %s\nDue: %s\nDays: %d\nTotal: %.2f\n",
                   rows[i].start_date, rows[i].due_date, rows[i].days, rows[i].total_cost);
        }
}
