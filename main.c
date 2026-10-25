#include <stdio.h>
#include "Bike.h"
#include "Car.h"
#include "rental_Bike.h"
#include "rental_Car.h"
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <dirent.h>

static int readWord(char *text, size_t size);

/* #new: Check the saved password so it can be changed without editing main.c. */
static int passwordIsCorrect(const char *entered)
{
    FILE *fp = fopen("admin_password.txt", "r");
    char saved[50];
    int matches = 0;

    if (fp == NULL)
    {
        perror("Could not open admin_password.txt");
        return 0;
    }
    if (fscanf(fp, "%49s", saved) == 1 && strcmp(entered, saved) == 0)
        matches = 1;
    fclose(fp);
    return matches;
}

/* #new: Let an administrator replace the password stored in the local file. */
static void changeAdminPassword(void)
{
    char current[50], new_password[50], confirm[50];
    FILE *fp;

    printf("Enter current password: ");
    if (!readWord(current, sizeof(current)) || !passwordIsCorrect(current))
    {
        printf("Current password is incorrect. Password was not changed.\n");
        return;
    }
    printf("Enter new password (at least 8 characters): ");
    if (!readWord(new_password, sizeof(new_password)) || strlen(new_password) < 8)
    {
        printf("New password must be at least 8 characters.\n");
        return;
    }
    printf("Enter new password again: ");
    if (!readWord(confirm, sizeof(confirm)) || strcmp(new_password, confirm) != 0)
    {
        printf("The passwords did not match. Password was not changed.\n");
        return;
    }

    fp = fopen("admin_password.tmp", "w");
    if (fp == NULL)
    {
        perror("Could not save the new password");
        return;
    }
    int saved = fprintf(fp, "%s\n", new_password) >= 0;
    if (fclose(fp) != 0)
        saved = 0;
    if (!saved)
    {
        remove("admin_password.tmp");
        printf("Could not save the new password.\n");
        return;
    }
    if (rename("admin_password.tmp", "admin_password.txt") != 0)
    {
        perror("Could not replace admin_password.txt");
        remove("admin_password.tmp");
        return;
    }
    printf("Admin password changed successfully.\n");
}

/* #new: Copy one file into a timestamped backup file. */
static int copyBackupFile(const char *source, const char *backup)
{
    FILE *input = fopen(source, "rb");
    FILE *output;
    int character;
    int okay = 1;

    if (input == NULL)
    {
        perror(source);
        return 0;
    }
    output = fopen(backup, "wb");
    if (output == NULL)
    {
        perror(backup);
        fclose(input);
        return 0;
    }
    while ((character = fgetc(input)) != EOF)
        if (fputc(character, output) == EOF)
        {
            okay = 0;
            break;
        }
    if (ferror(input))
        okay = 0;
    if (fclose(input) != 0)
        okay = 0;
    if (fclose(output) != 0)
        okay = 0;
    return okay;
}

/* #new: Save all main vehicle, rental, ID, and login data files together. */
static void createBackup(void)
{
    const char *files[] = {"Car.txt", "Bike.txt", "rental_Car.txt", "rental_Bike.txt",
                           "completed_rentals.txt", "next_rental_id.txt", "admin_password.txt"};
    char stamp[20], backup_name[100];
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    size_t i;
    int okay = 1;

    if (current_time == NULL || strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", current_time) == 0)
    {
        printf("Could not create a time for the backup name.\n");
        return;
    }
    for (i = 0; i < sizeof(files) / sizeof(files[0]); i++)
    {
        snprintf(backup_name, sizeof(backup_name), "backup_%s_%s", stamp, files[i]);
        if (!copyBackupFile(files[i], backup_name))
            okay = 0;
    }
    /* #new: Copy existing receipt files too, so backups keep customer receipts. */
    DIR *folder = opendir(".");
    if (folder == NULL)
    {
        perror("Could not open project folder to find receipts");
        okay = 0;
    }
    else
    {
        struct dirent *entry;
        while ((entry = readdir(folder)) != NULL)
        {
            if (strncmp(entry->d_name, "receipt_", 8) == 0)
            {
                snprintf(backup_name, sizeof(backup_name), "backup_%s_%s", stamp, entry->d_name);
                if (!copyBackupFile(entry->d_name, backup_name))
                    okay = 0;
            }
        }
        closedir(folder);
    }
    if (okay)
        printf("Backup files created with the name backup_%s_...\n", stamp);
    else
        printf("Backup was incomplete. Check the file errors above.\n");
}

/* #new: Count stored vehicles and summarize current and completed rentals. */
static void viewReports(void)
{
    const char *vehicle_files[] = {"Car.txt", "Bike.txt"};
    const char *rental_files[] = {"rental_Car.txt", "rental_Bike.txt", "completed_rentals.txt"};
    int vehicle_counts[2] = {0, 0};
    int rental_counts[3] = {0, 0, 0};
    float active_value = 0.0f, completed_value = 0.0f;
    size_t i;

    for (i = 0; i < 2; i++)
    {
        FILE *fp = fopen(vehicle_files[i], "r");
        char brand[50], model[50], reg_no[50];
        float rate;
        if (fp == NULL)
        {
            perror(vehicle_files[i]);
            return;
        }
        while (fscanf(fp, "%49s %49s %49s %f", brand, model, reg_no, &rate) == 4)
            vehicle_counts[i]++;
        if (ferror(fp))
        {
            perror(vehicle_files[i]);
            fclose(fp);
            return;
        }
        fclose(fp);
    }

    for (i = 0; i < 3; i++)
    {
        FILE *fp = fopen(rental_files[i], "r");
        int id, days, late_days;
        char customer[100], brand[50], model[50], reg_no[50], phone[20];
        char start[11], due[11], returned[11];
        float total, late_fee;
        if (fp == NULL)
        {
            perror(rental_files[i]);
            return;
        }
        while (fscanf(fp, "%d %99s %49s %49s %49s %d %f %19s %10s %10s %10s %d %f",
                      &id, customer, brand, model, reg_no, &days, &total, phone,
                      start, due, returned, &late_days, &late_fee) == 13)
        {
            rental_counts[i]++;
            if (i < 2)
                active_value += total;
            else
                completed_value += total + late_fee;
        }
        if (ferror(fp))
        {
            perror(rental_files[i]);
            fclose(fp);
            return;
        }
        fclose(fp);
    }
    printf("\n================ ADMIN REPORT =================\n");
    printf("Available cars: %d\n", vehicle_counts[0]);
    printf("Available bikes: %d\n", vehicle_counts[1]);
    printf("Active rentals: %d\n", rental_counts[0] + rental_counts[1]);
    printf("Completed rentals: %d\n", rental_counts[2]);
    printf("Value of active bookings: %.2f\n", active_value);
    printf("Completed rental total (including late fees): %.2f\n", completed_value);
}

/* FIX: Read a bounded word and reject extra characters instead of overflowing an array. */
static int readWord(char *text, size_t size)
{
    char format[20];
    int next;
    int too_long = 0;

    if (size < 2)
        return 0;

    snprintf(format, sizeof(format), "%%%zus", size - 1);
    if (scanf(format, text) != 1)
    {
        while ((next = getchar()) != '\n' && next != EOF)
        {
        }
        return 0;
    }

    next = getchar();
    if (next != '\n' && next != EOF)
    {
        too_long = !isspace((unsigned char)next);
        while (next != '\n' && next != EOF)
            next = getchar();
    }
    return !too_long;
}

/* FIX: Clear invalid menu input so a bad character cannot trap the menu in a loop. */
static int readInt(int *value)
{
    int next;

    if (scanf("%d", value) != 1)
    {
        while ((next = getchar()) != '\n' && next != EOF)
        {
        }
        return 0;
    }
    while ((next = getchar()) != '\n' && next != EOF)
    {
    }
    return 1;
}

int main(void)
{
    printf("\n***************************************************************\n");
    printf("*                                                             *\n");
    printf("*              VEHICLE RENTAL MANAGEMENT SYSTEM               *\n");
    printf("*                                                             *\n");
    printf("***************************************************************\n");

    while (1)
    {
        int choice;
    main:
        printf("\n==================== MAIN MENU ====================\n");
        printf("1. Admin Menu\n");
        printf("2. Customer Menu\n");
        printf("3. Exit\n");
        printf("====================================================\n");
        printf("Enter your choice(1/2/3): ");
        if (!readInt(&choice))
        {
            printf("Please enter a number from the menu.\n");
            goto main;
        }
        if (choice == 1)
        {
            char password[50];
            printf("\nEnter Password: ");
            if (!readWord(password, sizeof(password)))
            {
                printf("Password is too long or invalid.\n");
                goto main;
            }

            if (passwordIsCorrect(password))
            {
                int admin_choice;
            admin:
                printf("\n==================== ADMIN MENU ====================\n");
                printf("1. Add Vehicle\n");
                printf("2. View Available Vehicles\n");
                printf("3. Update Vehicle\n");
                printf("4. Delete Vehicle\n");
                printf("5. View Active Rentals\n");
                printf("6. Back to Main Menu\n");
                /* #new: Add the report, backup, and password controls for admins. */
                printf("7. View Reports\n");
                printf("8. Create Backup\n");
                printf("9. Change Admin Password\n");
                printf("====================================================\n");
                printf("Enter your choice: ");
                if (!readInt(&admin_choice))
                {
                    printf("Please enter a number from the menu.\n");
                    goto admin;
                }
                if (admin_choice == 6)
                {
                    goto main;
                }
                else if (admin_choice == 1)
                {
                    int op;
                admin1:
                    printf("\n================ VEHICLE TYPE ================\n");
                    printf("1. Car\n");
                    printf("2. Bike\n");
                    printf("3. Back\n");
                    printf("==============================================\n");
                    printf("Enter your choice: ");
                    if (!readInt(&op))
                    {
                        printf("Please enter a number from the menu.\n");
                        goto admin1;
                    }
                    printf("\n");

                    if (op == 1)
                    {
                        addCar();
                        goto admin;
                    }
                    else if (op == 2)
                    {
                        addBike();
                        goto admin;
                    }
                    else if (op == 3)
                    {
                        goto admin;
                    }
                    else
                    {
                        printf("\n[ ERROR ] Invalid choice! Select a number from the menu.\n\n");
                        goto admin1;
                    }
                }
                else if (admin_choice == 2)
                {
                    int op;
                admin2:
                    printf("\n================ VEHICLE TYPE ================\n");
                    printf("1. Car\n");
                    printf("2. Bike\n");
                    printf("3. Back\n");
                    printf("==============================================\n");
                    printf("Enter your choice: ");
                    if (!readInt(&op))
                    {
                        printf("Please enter a number from the menu.\n");
                        goto admin2;
                    }
                    printf("\n");

                    if (op == 1)
                    {
                        viewCar();
                        goto admin;
                    }
                    else if (op == 2)
                    {
                        viewBike();
                        goto admin;
                    }
                    else if (op == 3)
                    {
                        goto admin;
                    }
                    else
                    {
                        printf("\n[ ERROR ] Invalid choice! Select a number from the menu.\n\n");
                        goto admin2;
                    }
                }
                else if (admin_choice == 3)
                {
                    int op;
                admin3:
                    printf("\n================ VEHICLE TYPE ================\n");
                    printf("1. Car\n");
                    printf("2. Bike\n");
                    printf("3. Back\n");
                    printf("==============================================\n");
                    printf("Enter your choice: ");
                    if (!readInt(&op))
                    {
                        printf("Please enter a number from the menu.\n");
                        goto admin3;
                    }
                    printf("\n");

                    if (op == 1)
                    {
                        updateCar();
                        goto admin;
                    }
                    else if (op == 2)
                    {
                        updateBike();
                        goto admin;
                    }
                    else if (op == 3)
                    {
                        goto admin;
                    }
                    else
                    {
                        printf("\n[ ERROR ] Invalid choice! Select a number from the menu.\n\n");
                        goto admin3;
                    }
                }
                else if (admin_choice == 4)
                {
                    int op;
                admin4:
                    printf("\n================ VEHICLE TYPE ================\n");
                    printf("1. Car\n");
                    printf("2. Bike\n");
                    printf("3. Back\n");
                    printf("==============================================\n");
                    printf("Enter your choice: ");
                    if (!readInt(&op))
                    {
                        printf("Please enter a number from the menu.\n");
                        goto admin4;
                    }
                    printf("\n");

                    if (op == 1)
                    {
                        deleteCar();
                        goto admin;
                    }
                    else if (op == 2)
                    {
                        deleteBike();
                        goto admin;
                    }
                    else if (op == 3)
                    {
                        goto admin;
                    }
                    else
                    {
                        printf("\n[ ERROR ] Invalid choice! Select a number from the menu.\n\n");
                        goto admin4;
                    }
                }
                else if (admin_choice == 5)
                {
                    int op;
                admin5:
                    printf("\n================ VEHICLE TYPE ================\n");
                    printf("1. Car\n");
                    printf("2. Bike\n");
                    printf("3. Back\n");
                    printf("==============================================\n");
                    printf("Enter your choice: ");
                    if (!readInt(&op))
                    {
                        printf("Please enter a number from the menu.\n");
                        goto admin5;
                    }
                    printf("\n");

                    if (op == 1)
                    {
                        viewActiveRental_Cars();
                        goto admin;
                    }
                    else if (op == 2)
                    {
                        viewActiveRental_Bikes();
                        goto admin;
                    }
                    else if (op == 3)
                    {
                        goto admin;
                    }
                    else
                    {
                        printf("\n[ ERROR ] Invalid choice! Select a number from the menu.\n\n");
                        goto admin5;
                    }
                }
                else if (admin_choice == 7)
                {
                    viewReports();
                    goto admin;
                }
                else if (admin_choice == 8)
                {
                    createBackup();
                    goto admin;
                }
                else if (admin_choice == 9)
                {
                    changeAdminPassword();
                    goto admin;
                }
                else
                {
                    printf("\n[ ERROR ] Invalid choice! Select a number from the menu.\n\n");
                    goto admin;
                }
            }
            else
            {
                printf("\n****Invalid Password****\n");
                goto main;
            }
        }
        else if (choice == 2)
        {
            int customer_choice;
        customer:
            printf("\n=================== CUSTOMER MENU ==================\n");
            printf("1. View Available Vehicles\n");
            printf("2. Rent a Vehicle\n");
            printf("3. Return a Vehicle\n");
            printf("4. Back to Main Menu\n");
            /* #new: Make vehicle search and sorting available to customers. */
            printf("5. Search Vehicles\n");
            printf("6. Sort Vehicles\n");
            printf("====================================================\n");
            printf("Enter your choice: ");
            if (!readInt(&customer_choice))
            {
                printf("Please enter a number from the menu.\n");
                goto customer;
            }
            if (customer_choice == 4)
            {
                goto main;
            }
            else if (customer_choice == 1)
            {
                int op;
            customer1:
                printf("\n================ VEHICLE TYPE ================\n");
                printf("1. Car\n");
                printf("2. Bike\n");
                printf("3. Back\n");
                printf("==============================================\n");
                printf("Enter your choice: ");
                if (!readInt(&op))
                {
                    printf("Please enter a number from the menu.\n");
                    goto customer1;
                }
                printf("\n");

                if (op == 1)
                {
                    viewCar();
                    goto customer;
                }
                else if (op == 2)
                {
                    viewBike();
                    goto customer;
                }
                else if (op == 3)
                {
                    goto customer;
                }
                else
                {
                    printf("\n[ ERROR ] Invalid choice! Select a number from the menu.\n\n");
                    goto customer1;
                }
            }
            else if (customer_choice == 2)
            {
                int op;
            customer2:
                printf("\n================ VEHICLE TYPE ================\n");
                printf("1. Car\n");
                printf("2. Bike\n");
                printf("3. Back\n");
                printf("==============================================\n");
                printf("Enter your choice: ");
                if (!readInt(&op))
                {
                    printf("Please enter a number from the menu.\n");
                    goto customer2;
                }
                printf("\n");

                if (op == 1)
                {
                    rentCar();
                    goto customer;
                }
                else if (op == 2)
                {
                    rentBike();
                    goto customer;
                }
                else if (op == 3)
                {
                    goto customer;
                }
                else
                {
                    printf("\n[ ERROR ] Invalid choice! Select a number from the menu.\n\n");
                    goto customer2;
                }
            }
            else if (customer_choice == 3)
            {
                int op;
            customer3:
                printf("\n================ VEHICLE TYPE ================\n");
                printf("1. Car\n");
                printf("2. Bike\n");
                printf("3. Back\n");
                printf("==============================================\n");
                printf("Enter your choice: ");
                if (!readInt(&op))
                {
                    printf("Please enter a number from the menu.\n");
                    goto customer3;
                }
                printf("\n");
                if (op == 1)
                {
                    returnCar();
                    goto customer;
                }
                else if (op == 2)
                {
                    returnBike();
                    goto customer;
                }
                else if (op == 3)
                {
                    goto customer;
                }
                else
                {
                    printf("\n[ ERROR ] Invalid choice! Select a number from the menu.\n\n");
                    goto customer3;
                }
            }
            else if (customer_choice == 5 || customer_choice == 6)
            {
                int op;
                printf("\n================ VEHICLE TYPE ================\n");
                printf("1. Car\n2. Bike\n3. Back\n");
                printf("==============================================\nEnter your choice: ");
                if (!readInt(&op) || (op != 1 && op != 2 && op != 3))
                {
                    printf("Please enter 1, 2, or 3.\n");
                    goto customer;
                }
                if (op == 1)
                {
                    if (customer_choice == 5) searchCar();
                    else sortCar();
                }
                else if (op == 2)
                {
                    if (customer_choice == 5) searchBike();
                    else sortBike();
                }
                goto customer;
            }
            else
            {
                printf("\n[ ERROR ] Invalid choice! Select a number from the menu.\n\n");
                goto customer;
            }
        }
        else if (choice == 3)
        {
            printf("\n***************************************************************\n");
            printf("*                                                             *\n");
            printf("*       THANK YOU FOR USING VEHICLE RENTAL MANAGEMENT         *\n");
            printf("*                                                             *\n");
            printf("***************************************************************\n\n");
            break;
        }
        else
        {
            printf("\n[ ERROR ] Invalid choice! Select a number from the menu.\n\n");
            goto main;
        }
    }

    return 0;
}
