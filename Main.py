# Reference Video: https://www.youtube.com/watch?v=OJtpA_qTNL0 #

# The Raspberry Pi acts as a computational machine. The arduino acts as a slave to accept inputs and perform outputs#

# Author: Shamir Kevin and Gokul Selvaraj

# imports
import math
import os
import serial
import time
import pandas as pd
from natsort import natsorted

# Constants
INPUT_FILE_NAME = "Dummy List from SD.xlsx"
SORTING_TEMPLATE_FILE = "Sorting Template File.xlsx"
COORDINATES_FILE = "Coordinates Template File.xlsx"

# Column Mappings

NB_NUMBER_COL_NAME = "NB Number"
SORTING_COLUMN_MAPPING = {
    "Pathologist": "Pathologist Name",
    "Slides": NB_NUMBER_COL_NAME,
}
COORDINATES_COLUMN_MAPPING = {"Coordinates": "Location"}

# Other Constants
EXIT_COMMAND = "exit"
MENU_COMMAND = (
    'Available Commands:\n1. "Start" or 1\n2. "Eject" or 2\n3. "Finished" or 3\n4. "Faulty" or 4\nENTER '
    "YOUR DESIRED COMMAND:\n"
)

# https://codereview.stackexchange.com/questions/41298/producing-ordinal-numbers#:~:text=29-,def%20ordinal(self%2C%20num)%3A%20%22%22%22%20Returns%20ordinal,%2C%202nd%2C%203rd%2C%20etc.
SUFFIXES = {1: "st", 2: "nd", 3: "rd"}


def ordinal(num):
    # I'm checking for 10-20 because those are the digits that
    # don't follow the normal counting scheme.
    if 10 <= num % 100 <= 20:
        suffix = "th"
    else:
        # the second parameter is a default.
        suffix = SUFFIXES.get(num % 10, "th")
    return str(num) + suffix


# Functions
def datafile_to_sorted_file(input_file_name: str, output_file_name=None) -> str:
    """
    Convert data from an Excel file to a sorted Excel file with customized column mapping.

    Args:
        input_file_name (str): The name of the input Excel data file.
        output_file_name (str, optional): The name of the output Excel file. If not provided, a default name
            based on the 'End Date' column in the input data will be used.

    Returns:
        str: The name of the generated output Excel file.

    Example:
        >>> datafile_to_sorted_file("input_data.xlsx", "output_data.xlsx")
    """
    # Read input data from Excel
    input_data = pd.read_excel(input_file_name)

    # Set the output file name if not provided
    if output_file_name is None:
        output_file_name = generate_default_output_name(input_data)
    if os.path.exists(output_file_name):
        return output_file_name

    # Process and sort data
    sorted_data = sort_data(input_data)

    # Load the sorting template
    sorting_template = pd.read_excel(SORTING_TEMPLATE_FILE)

    # Map columns from sorted data to the sorting template
    for source_col, target_col in SORTING_COLUMN_MAPPING.items():
        sorting_template[target_col] = sorted_data[source_col]

    # Save the sorted data to the output Excel file
    sorting_template.to_excel(output_file_name, index=False)

    return output_file_name


def custom_sort(group: pd.DataFrame):
    # Custom sorting function to sort values within each group
    # Define your custom sorting logic here,
    # For example, you can split the "Slides" values and sort them based on custom criteria
    # print(group)
    sorted_indices = natsorted(group.index, key=lambda x: group.loc[x, "Slides"])
    return group.loc[sorted_indices]


def sort_data(data: pd.DataFrame) -> pd.DataFrame:
    """
    Sort the input data by 'Pathologist' and 'Slides' in ascending order.

    Args:
        data (DataFrame): Input data to be sorted.

    Returns:
        DataFrame: Sorted data.
    """
    doctor_slide_list = data[["Pathologist", "Slides"]]
    sorted_doctor_slide_list = (
        doctor_slide_list.groupby("Pathologist", group_keys=True)
        .apply(custom_sort)
        .reset_index(drop=True)
    )
    return sorted_doctor_slide_list


def generate_default_output_name(input_data: pd.DataFrame) -> str:
    """
    Generate a default output file name based on the 'End Date' column in the input data.

    Args:
        input_data (DataFrame): Input data used to generate the default name.

    Returns:
        str: Default output file name.
    """
    return str(input_data["End Date"].iloc[0]).replace("/", "_") + ".xlsx"


def update_coordinates(input_file_name: str, coordinate_file_name=COORDINATES_FILE):
    """
    Update coordinates in an Excel file with values from a coordinate template file.

    Args:
        input_file_name (str): The name of the input Excel file to be updated.
        coordinate_file_name (str, optional): The name of the coordinate template Excel file. If not provided,
            a default template file name will be used.

    Returns:
        None

    Example:
        >>> update_coordinates("input_data.xlsx", "coordinates_template.xlsx")
    """
    # Read input data from Excel
    input_data = pd.read_excel(input_file_name)

    # Read the coordinate template file
    coordinate_data = pd.read_excel(coordinate_file_name)

    # Update the 'Location' column in the input data with values from the template
    for source_col, target_col in COORDINATES_COLUMN_MAPPING.items():
        input_data[target_col] = coordinate_data[source_col]

    # Save the updated data back to the input Excel file
    input_data.to_excel(input_file_name, index=False)


def update_status_to_completed(file_name: str, qr_number: str) -> str:
    """
    Update the status in an Excel file based on a QR number.

    Args:
        file_name (str): The name of the Excel file to update.
        qr_number (str): The QR number to identify the row to update.

    Returns:
        str: The location associated with the QR number.

    Example:
        To mark a specific QR number as "Completed" in an Excel file:
        >>> update_status_to_completed('data.xlsx', 'QR12345')
    """
    df = pd.read_excel(file_name)
    df["Status"] = df["Status"].astype(str)

    # Update the 'Status' column where 'NB Number' matches qr_number to "Completed"
    df.loc[df[NB_NUMBER_COL_NAME] == qr_number, "Status"] = "Completed"

    # Write the updated DataFrame back to the Excel file without including the index
    df.to_excel(file_name, index=False)

    # Get locations associated with the QR number
    locations = df.loc[df[NB_NUMBER_COL_NAME] == qr_number, "Location"]

    if len(locations) == 0:
        return "Location not found"
    elif len(locations) == 1:
        # If there's only one location, return it
        return locations.item()
    else:
        # Handle multiple locations (you can choose any logic here)
        # For example, return a comma-separated list of locations
        return ", ".join(locations)


def eject_sequence(file_name: str, nb_numbers: str):
    df = pd.read_excel(file_name)

    # Step 3.1
    while not case_number_exists(df, nb_numbers):
        print("Check the case number entered and try again!\n")
        nb_numbers = input("Enter the correct case_number: \n")

    # Step 3.2
    if is_status_not_completed(df, nb_numbers):
        print("Wait for glass slide to reach the magazine!\n")
    # Step 3.3
    elif is_status_ejected(df, nb_numbers):
        print("The glass slide has already been removed from the machine.\n")

    return nb_numbers


def update_case_number_statuses(file_name: str, nb_numbers: list, new_status: str):
    df = pd.read_excel(file_name)

    df.loc[df[NB_NUMBER_COL_NAME].isin(nb_numbers), "Status"] = new_status

    df.to_excel(file_name, index=False)
    print(f"Status updated to '{new_status}' for specified case numbers.")


def print_magazine_and_row_number(file_name: str, nb_numbers: list):
    df = pd.read_excel(file_name)
    magazine_numbers = df[df[NB_NUMBER_COL_NAME].isin(nb_numbers)]["Number"].tolist()
    print_list = []
    for index, nb_number in enumerate(nb_numbers):
        magazine_number = math.ceil(magazine_numbers[index] / 30)
        row_number = magazine_numbers[index] % 30
        if row_number == 0:
            row_number = 30
        print_list.append((nb_number, magazine_number, row_number))
    print("You can remove the following Magazines:\n")
    for nb_number, magazine_number, row_number in print_list:
        print(
            f"Glass Slide {nb_number} in Magazine {magazine_number} in Row {row_number}.\n"
        )


def case_number_exists(df: pd.DataFrame, nb_number: str) -> bool:
    return any(df[NB_NUMBER_COL_NAME] == nb_number)


def is_status_not_completed(df: pd.DataFrame, nb_number: str) -> bool:
    return (
        df.loc[df[NB_NUMBER_COL_NAME] == nb_number, "Status"].any() == "Not Completed"
    )


def is_status_withdrawn(df: pd.DataFrame, nb_number: str) -> bool:
    return df.loc[df[NB_NUMBER_COL_NAME] == nb_number, "Status"].any() == "Withdrawn"


def is_status_ejected(df: pd.DataFrame, nb_number: str) -> bool:
    return df.loc[df[NB_NUMBER_COL_NAME] == nb_number, "Status"].any() == "Ejected"


def wait_for_insertion_confirmation(ser: serial.Serial):
    """Waits for the user to insert the magazine and type 'Inserted', then sends the confirmation to the Arduino."""
    insert_msg = input(
        'Insert magazine into the machine, and type "Inserted" when completed":\n'
    )
    while insert_msg != "Inserted":
        insert_msg = input(
            'Invalid value. Please insert magazine into the machine, and type "Inserted" when completed":\n'
        )
    send_command_to_arduino(ser, insert_msg)
    wait_for_received_from_arduino(ser)


# def send_coordinates_to_arduino(ser: serial.Serial, coordinates: str):
#     """Sends coordinates to the Arduino and waits for a 'Finished' response."""
#     formatted_coordinates = f"({coordinates})"
#     ser.write(formatted_coordinates.encode("utf-8"))


def send_command_to_arduino(ser: serial.Serial, command: str):
    """Sends a command to the Arduino and waits for a 'Received' response."""
    # print(command) # debug
    ser.write(command.encode("utf-8"))
    while ser.in_waiting <= 0:
        time.sleep(0.01)


def wait_for_received_from_arduino(ser: serial.Serial):
    """Waits for a 'Received' response."""
    while True:
        message = read_arduino_message(ser=ser)
        if message == "Received":
            break
        # else:
        #     print(
        #         "Not expected Arduino response, trying again for a valid response from Arduino!"
        #     )


def start_command(ser: serial.Serial, command="Start"):
    print("Starting! Please wait for calibration\n")
    send_command_to_arduino(ser, command)
    wait_for_received_from_arduino(ser)

    one_or_two = input("Do you want to insert 1 or 2 magazines:\n")
    while one_or_two not in ["1", "2"]:
        one_or_two = input("Invalid value. Please enter only the values 1 or 2:")
    print("Please wait for calibration!\n")
    send_command_to_arduino(ser, one_or_two)
    wait_for_received_from_arduino(ser)

    wait_for_insertion_confirmation(ser)

    return one_or_two == "2"


def wait_for_arduino_message(ser: serial.Serial):
    """Wait for a message from the Arduino"""
    while ser.in_waiting <= 0:
        time.sleep(0.01)  # Delay to prevent premature exit


def read_arduino_message(ser: serial.Serial, debug=True):
    """Read a message from the Arduino"""
    msg_from_arduino = ser.readline().decode("utf-8").rstrip()
    if debug:
        print(f"Received this message from Arduino: {msg_from_arduino}")
    return msg_from_arduino


def find_first_empty_row_after_index(df: pd.DataFrame, start_index: int):
    """
    Find the first empty row (where 'Number' column has no value) after a given index.

    Args:
        df (DataFrame): The DataFrame to search.
        start_index (int): The index after which to start searching.

    Returns:
        int: Index of the first empty row after the given index, or None if not found.
    """
    # Filter rows where 'Number' column has no value and locate the first occurrence after start_index
    # empty_row_index = df.loc[df.index > start_index, "Number"].isna().idxmax()
    empty_row_index = df.loc[
        (df.index > start_index) & (df["NB Number"].isnull())
    ].index
    return empty_row_index[0] if not empty_row_index.empty else None


def modify_values_of_empty_row(
    file_name: str,
    start_index: int,
    columns: list[str],
    values: list[str],
    return_column: str,
):
    """
    Modify the values of the first empty row (where 'Number' column has no value) after a given index,
    and return the value of another column from the same row.

    Args:
        file_name (str): Name of the file to modify.
        start_index (int): The index after which to start searching for the empty row.
        columns (list): A list of column names to modify.
        values (list): A list of values to set in the specified columns.
        return_column (str): The name of the column to return the value from.

    Returns:
        object: The value of the specified column from the same row after modifications.
    """
    df = pd.read_excel(file_name)
    empty_row_index = find_first_empty_row_after_index(df, start_index)
    if empty_row_index is not None:
        for column, value in zip(columns, values):
            df.loc[empty_row_index, column] = value
        print(f"Values of row at index {empty_row_index + 1} updated.")
        df.to_excel(file_name, index=False)
        return df.loc[empty_row_index, return_column]
    else:
        print("No empty row found after the given start index.")
        return None


def nb_number_exists(df, nb_number):
    """
    Check if the given NB_number exists in the DataFrame.

    Args:
        df (DataFrame): The DataFrame to search.
        nb_number (str): The NB_number to check.

    Returns:
        bool: True if the NB_number exists in the DataFrame, False otherwise.
    """
    return nb_number in df[NB_NUMBER_COL_NAME].values


def scan_command(sorted_file_name: str, ser: serial.Serial, command="Scan") -> bool:
    print(f"Scanning is in progress, do not open {sorted_file_name}\n")
    ser.write(command.encode("utf-8"))
    wait_for_arduino_message(ser=ser)

    qr_number = read_arduino_message(ser=ser)
    if qr_number == "Finished" or qr_number == "Magazine":
        return True
    print(f"The number that was scanned was {qr_number}")
    if qr_number == "Faulty":
        coordinates = modify_values_of_empty_row(
            file_name=sorted_file_name,
            start_index=149,
            columns=[NB_NUMBER_COL_NAME, "Status"],
            values=["Unknown NB Number", "Faulty"],
            return_column="Location",
        )
        print(f"Updated location {coordinates} to Faulty.")
    # NB Number does not exist
    elif not nb_number_exists(pd.read_excel(sorted_file_name), qr_number):
        coordinates = modify_values_of_empty_row(
            file_name=sorted_file_name,
            start_index=149,
            columns=[NB_NUMBER_COL_NAME, "Status"],
            values=[qr_number, "Faulty"],
            return_column="Location",
        )
        print(
            f"Received {qr_number} not in magazine! Updated at location {coordinates}!"
        )
    else:
        coordinates = update_status_to_completed(
            file_name=sorted_file_name, qr_number=qr_number
        )
        print(f"Updated NB Number {qr_number} status to Completed!")

    # command = "Coordinates-Delivery"
    # ser.write(command.encode("utf-8"))
    # wait_for_arduino_message(ser=ser)

    # Send coordinates to Arduino
    print(f"The Coordinates I am going to send to arduino now are {coordinates}")
    formatted_coordinates = f"({coordinates})"
    ser.write(formatted_coordinates.encode("utf-8"))

    return False


def eject_command(sorted_file_name: str):
    counter = int(input("Enter the number of glass slides to be removed: \n"))
    total = counter + 1
    case_numbers = []
    while counter > 0:
        case_number = input(
            f"Enter {ordinal(total - counter)} case number to eject: \n"
        )
        if is_status_ejected(pd.read_excel(sorted_file_name), case_number):
            print(
                "Invalid case number, case number is ejected! Re-enter a valid case number!"
            )
            continue
        if is_status_withdrawn(pd.read_excel(sorted_file_name), case_number):
            print(
                "Invalid case number, case number is withdrawn! Re-enter a valid case number!"
            )
            continue
        counter -= 1

        if case_number == "exit":
            break

        case_number = eject_sequence(file_name=sorted_file_name, nb_numbers=case_number)
        case_numbers.append(case_number)
    update_case_number_statuses(sorted_file_name, case_numbers, "Ejected")
    print_magazine_and_row_number(sorted_file_name, case_numbers)
    while True:
        is_done = input("Enter Done when completed:")
        if is_done == "Done":
            break


def check_all_status_is_completed_or_ejected(file_name: str):
    df = pd.read_excel(file_name)

    def check_status(row):
        if row[NB_NUMBER_COL_NAME] != "":
            return row["Status"] in ["Completed", "Ejected"]
        return False

    # Apply the check_status function to the entire DataFrame
    results = df.apply(check_status, axis=1)

    # Check if any row in the DataFrame meets the conditions (True in results)
    return any(results)


def get_completed_cases(file_name: str):
    df = pd.read_excel(file_name)
    return df[df["Status"] == "Completed"]


def get_completed_or_ejected_cases(file_name: str):
    df = pd.read_excel(file_name)
    return df[df["Status"].isin(["Completed", "Ejected"])]


def get_not_completed_or_ejected_cases(file_name: str):
    df = pd.read_excel(file_name)
    return df[~df["Status"].isin(["Completed", "Ejected"])]


def finished_command(sorted_file_name: str):
    # 2) Once all the scanning is done - check in the Excel file if all the status is either Completed or Ejected.
    # This means all the glass slides have been scanned and all sorted
    if not check_all_status_is_completed_or_ejected(file_name=sorted_file_name):
        print("Not all glass slides have been scanned and sorted!")

    # 4) This list of completed cases sorted alphanumerically will then be displayed on the screen
    completed_cases_dataframe = get_completed_or_ejected_cases(sorted_file_name)
    nb_numbers = completed_cases_dataframe[NB_NUMBER_COL_NAME]
    # Achieves the purpose of only printing out only NB23 and the first 5 numbers
    completed_cases_number_set = {nb_number[:10] for nb_number in nb_numbers}

    not_completed_cases_dataframe = get_not_completed_or_ejected_cases(sorted_file_name)
    nb_numbers = (
        not_completed_cases_dataframe[NB_NUMBER_COL_NAME].fillna("").astype(str)
    )
    not_completed_cases_number_set = {nb_number[:10] for nb_number in nb_numbers}

    truly_completed_cases_number_set = (
        completed_cases_number_set - not_completed_cases_number_set
    )

    print("Glass slides starting with the following numbers are completed:")
    for nb_number in truly_completed_cases_number_set:
        print(nb_number)

    if len(truly_completed_cases_number_set) == 0:
        print("No more completed glass slides, returning to main menu!\n")
        return
    # 5) The user can then choose which case they want to withdraw from the machine and presses it
    while True:
        case_number_to_withdraw = input("Enter the case number to withdraw:\n")
        if case_number_to_withdraw in truly_completed_cases_number_set:
            break
        print(
            "Case Number entered is not part of Completed cases, enter a valid case number from the previously "
            "displayed list!"
        )

    rows_with_case_numbers = completed_cases_dataframe[
        completed_cases_dataframe[NB_NUMBER_COL_NAME].str.startswith(
            case_number_to_withdraw[:10]
        )
    ].copy()

    # Python updates in Excel the case numbers to “Withdrawn”
    nb_numbers_to_update_to_withdrawn = rows_with_case_numbers[
        NB_NUMBER_COL_NAME
    ].to_list()

    update_case_number_statuses(
        file_name=sorted_file_name,
        nb_numbers=nb_numbers_to_update_to_withdrawn,
        new_status="Withdrawn",
    )

    rows_with_case_numbers["Magazine"] = rows_with_case_numbers["Number"].apply(
        lambda x: math.ceil(x / 30)
    )
    rows_with_case_numbers["Row"] = rows_with_case_numbers["Number"].apply(
        lambda x: (x - 1) % 30 + 1
    )
    rows_with_case_numbers = rows_with_case_numbers[
        [NB_NUMBER_COL_NAME, "Magazine", "Row", "Location", "Status"]
    ]
    print(
        f"\nThere is a total of {rows_with_case_numbers.shape[0]} glass slides. Listed below:"
    )
    print(rows_with_case_numbers.to_string(index=False, justify="justify"))


def faulty_command(sorted_file_name: str):
    df: pd.DataFrame = pd.read_excel(sorted_file_name)
    faulty_df = df[
        df["Status"] == "Faulty"
    ].copy()  # Ensure you are working with a copy
    faulty_df.loc[:, "Magazine"] = faulty_df["Number"].apply(
        lambda x: math.ceil(x / 30)
    )
    faulty_df.loc[:, "Row"] = faulty_df["Number"].apply(lambda x: (x - 1) % 30 + 1)

    faulty_rows = faulty_df[
        [NB_NUMBER_COL_NAME, "Magazine", "Row", "Location", "Status"]
    ]

    print(
        f"\nThere is a total of {faulty_rows.shape[0]} faulty glass slides. Listed below:"
    )
    print(faulty_rows.to_string(index=False, justify="justify"))


if __name__ == "__main__":
    ser = serial.Serial("/dev/ttyACM0", 9600, timeout=1.0)  # Uno is com 5
    time.sleep(3)
    ser.reset_input_buffer()
    print("Serial OK")

    sorted_file_name: str = ""  # Default name

    while True:
        try:
            # Main program
            sorted_file_name = datafile_to_sorted_file(
                INPUT_FILE_NAME, output_file_name="Sorted Cases.xlsx"
            )
            update_coordinates(sorted_file_name)
            while True:
                try:
                    time.sleep(1)
                    command = input(MENU_COMMAND)

                    if command.casefold() == "Start".casefold() or command == "1":
                        there_is_two_mags = start_command(ser=ser)
                        if there_is_two_mags:
                            # scan the first magazine
                            is_finished = False
                            while not is_finished:
                                is_finished = scan_command(
                                    sorted_file_name=sorted_file_name, ser=ser
                                )

                            wait_for_insertion_confirmation(ser=ser)

                            # Scan the second magazine
                            is_finished = False
                            while not is_finished:
                                is_finished = scan_command(
                                    sorted_file_name=sorted_file_name, ser=ser
                                )
                        else:
                            # Scan only one magazine
                            is_finished = False
                            while not is_finished:
                                is_finished = scan_command(
                                    sorted_file_name=sorted_file_name, ser=ser
                                )
                    if command.casefold() == "Eject".casefold() or command == "2":
                        eject_command(sorted_file_name=sorted_file_name)
                    if command.casefold() == "Finished".casefold() or command == "3":
                        finished_command(sorted_file_name=sorted_file_name)
                    if command.casefold() == "Faulty".casefold() or command == "4":
                        faulty_command(sorted_file_name=sorted_file_name)

                except KeyboardInterrupt:
                    print("Close Serial Communication.")
                    ser.close()
        except PermissionError:
            print(
                "Error: Permission denied to open excel files. Ensure all excel files are closed"
            )
            while True:
                response = input(
                    "Enter 'Closed' once you have closed the excel files\n"
                )
                if response == "Closed":
                    break
            continue
