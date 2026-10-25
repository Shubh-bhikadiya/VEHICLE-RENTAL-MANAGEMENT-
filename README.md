# Vehicle Rental Management System

A beginner-friendly vehicle rental program written in C. It manages cars and bikes, saves its information in text files, gives each rental a unique ID, and calculates a late fee when a vehicle is returned after its due date.

This project currently runs in a terminal. It does not include a graphical interface.

## What you can do

### Admin

- Add cars and bikes to the available inventory.
- View, update the daily rate of, or delete available vehicles.
- See active rentals and their rental IDs.
- View reports with vehicle and rental totals.
- Make timestamped copies of the data files and receipts.
- Change the admin password.

### Customer

- View available cars and bikes.
- Search available vehicles by part of their brand, model, or registration number.
- Sort the displayed vehicles by brand or daily rate.
- Rent a vehicle using a customer name and phone number.
- Return a rental using its rental ID.

## Project files

| File | Purpose |
| --- | --- |
| `main.c` | Starts the program and displays the main, admin, and customer menus. It also handles the admin password, reports, and backups. |
| `Car.c`, `Car.h` | Car inventory actions, including viewing, adding, updating, deleting, searching, and sorting cars. |
| `Bike.c`, `Bike.h` | The same inventory actions for bikes. |
| `rental_Car.c`, `rental_Car.h` | Car rental, return, and active-rental display actions. |
| `rental_Bike.c`, `rental_Bike.h` | Bike rental, return, and active-rental display actions. |
| `rental_core.c`, `rental_core.h` | Shared functions for reading and saving records, checking input, assigning IDs, and calculating rentals and returns. |
| `Makefile` | Build instructions used by the `make` command. |
| `Car.txt`, `Bike.txt` | Vehicles that are currently available to rent. |
| `rental_Car.txt`, `rental_Bike.txt` | Rentals that have not yet been returned. |
| `completed_rentals.txt` | Rental records after the vehicles have been returned. |
| `next_rental_id.txt` | The ID that will be assigned to the next new rental. |
| `admin_password.txt` | The current admin password, saved as plain text. |
| `receipt_<ID>.txt` | Receipt created for a rental. Return information is added to it when the vehicle is returned. |

## Requirements

- A C compiler such as GCC or Clang.
- `make`.
- A terminal opened in this project folder.

The Makefile compiles the program using the C11 standard and enables compiler warnings.

## Build and run

Open Terminal and go to the project folder:

```sh
cd /Users/shubhbhikadiya/Downloads/vehicle_rental
```

Build the program:

```sh
make
```

If the build finishes without an error, start the program:

```sh
./main
```

Run `./main` while your terminal is in the project folder. The program opens data files by their names, so starting it from another folder may make those files unavailable.

To remove the compiled program and object files, run:

```sh
make clean
```

Then run `make` again to build a fresh copy.

## Menu guide

At the first menu, enter:

- `1` to open the Admin Menu.
- `2` to open the Customer Menu.
- `3` to exit.

The starter admin password is `Admin@123`. In the Admin Menu, option `9` changes it. After login, the admin menu lets you add, view, update, and delete vehicles; view active rentals; open reports; create backups; or return to the main menu. For vehicle actions, choose Car or Bike when prompted.

The Customer Menu options are:

1. View available vehicles.
2. Rent a vehicle.
3. Return a vehicle.
4. Go back to the main menu.
5. Search vehicles.
6. Sort vehicles.

For actions involving vehicles, choose Car or Bike when asked. After renting, write down the rental ID shown by the program. You need that ID to return the correct rental.

## How rental costs work

When a vehicle is rented, the program saves the rental start date and due date in `YYYY-MM-DD` format. The dates are based on the computer's local date. The rental cost is the daily rate multiplied by the number of rental days.

When a vehicle is returned, the program compares the return date with the due date. The late fee uses the same daily rate that was saved for that rental:

```text
late fee = late days × daily rate
final total = rental total + late fee
```

The returned rental is removed from the active rental file, the vehicle is put back in its available inventory, and the completed rental is saved in `completed_rentals.txt`.

## How the text data is formatted

The text files are plain text with no heading row. Each record is on its own line, and the values are separated by spaces. Do not put spaces inside a vehicle brand, model, or registration value because the program reads each of those as one value.

A vehicle record has four values:

```text
Brand Model Registration DailyRate
```

For example:

```text
Honda City GJ10GH4455 1600.00
```

A rental record has 13 values in this order:

```text
RentalID CustomerName Brand Model Registration Days RentalTotal Phone StartDate DueDate ReturnDate LateDays LateFee
```

For example:

```text
1001 Aarav~Patel Honda City GJ10GH4455 3 4800.00 +919876500001 2026-10-25 2026-10-28 - 0 0.00
```

The `~` in a saved customer name stands for a space. The program displays it as a normal space. An active rental uses `-` for the return date until it is returned. Avoid editing a data file while the program is running, and keep a backup before making manual changes.

## Sample data in this folder

The included data currently has 20 available cars, 21 available bikes, two active car rentals, two active bike rentals, and one completed rental. The rental records use IDs `1001` through `1005`; `next_rental_id.txt` contains `1006` for the next rental. Receipt examples for IDs 1001 through 1005 are also included.

These are demonstration records. Replace names and phone numbers with fictional data before publishing the project if they identify real people.

## Password and GitHub notes

`admin_password.txt` stores the password as plain text to keep the project simple. Do not put a real or reused password in this file or commit one to GitHub. If you share the starter file, treat `Admin@123` as a public demonstration password and change it after downloading the project.

For GitHub, include the `.c` and `.h` source files, `Makefile`, this README, and the data files needed to run the sample project. Before committing, check that the data and receipt files contain only information you are comfortable sharing.

Do not include macOS metadata (`.DS_Store`), compiler output (`*.o` and the built `main` program), temporary `*.core.tmp` files, or generated backup files (`backup_*`). These can be removed from a repository with a `.gitignore` file.
