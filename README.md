# Vehicle Rental Management System

A beginner-friendly car and bike rental program written in C. It uses structures, functions, menus, and text files to save the vehicle and rental information.

## What the program can do

### Admin menu

- Add, view, update, and delete cars and bikes.
- See current rentals and their rental IDs.
- View a report with available vehicles, active rentals, completed rentals, and rental totals.
- Make a dated backup of the vehicle, rental, password, ID, and receipt files.
- Change the admin password from inside the program.

### Customer menu

- View cars and bikes that are available.
- Search by part of a brand, model, or registration number.
- Sort the list by brand or daily rate.
- Rent a vehicle with a full name and phone number.
- Return a vehicle using its rental ID.

When a vehicle is rented, the program saves its start date and due date. Returning it saves the return date. Each late day adds one daily rental rate to the late fee. The program keeps returned rentals in `completed_rentals.txt` so the report can include them.

## Files used by the program

```text
Car.txt                  Available cars
Bike.txt                 Available bikes
rental_Car.txt           Current car rentals
rental_Bike.txt          Current bike rentals
completed_rentals.txt    Returned rentals
next_rental_id.txt       Number used for the next rental ID
admin_password.txt       Admin password
receipt_<ID>.txt         Receipt for one rental ID
backup_<date>_<file>     Files made by the backup menu
```

Rental records are saved as space-separated values. Customer names are stored with `~` in place of spaces so the program can read the whole record; `~~` represents a real tilde in a name. The program displays the name with its spaces restored.

## Admin password

The first password is `Admin@123`. Change it from **Admin Menu → Change Admin Password**. The password is stored as plain text in `admin_password.txt`, which keeps it out of the C source code but is intended only for this small local learning project.

## Build and run

Build the program with the Makefile:

```sh
make
```

Run it from the project folder so it can find the text files:

```sh
./main
```

Remove compiled files with:

```sh
make clean
```

## Rental IDs and receipts

`next_rental_id.txt` contains the next number to assign. It starts at `1001`. Give the rental ID to the customer so they can return the correct booking. Each booking creates a `receipt_<ID>.txt` file; returning the vehicle adds its return date and final charge to that receipt.
