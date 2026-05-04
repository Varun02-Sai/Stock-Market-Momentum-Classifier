"""
=============================================================================
File:        c_bridge.py
Project:     Stock Market Momentum Classifier
Description: Subprocess bridge between Python and the C algorithm engine.
             
             This module handles Inter-Process Communication (IPC) between
             the Flask server (Python) and the compiled C binary. It pipes
             CSV stock data into the C binary's standard input (stdin) and
             reads the JSON prediction results from standard output (stdout).

How it works:
    1. Python calls subprocess.run() to launch model_release.exe
    2. The CSV data string is passed via the 'input' parameter (→ C's stdin)
    3. The C binary processes the data, trains the model, and prints JSON
    4. Python captures stdout and parses the JSON response

Why subprocess?
    - Clean separation: C handles ML computation, Python handles I/O
    - No need for C FFI (Foreign Function Interface) or shared libraries
    - Each prediction runs in its own process (isolation)
=============================================================================
"""

import subprocess  # For launching the C binary as a child process
import json        # For parsing the JSON output from the C binary
import os          # For path operations and file existence checks

# Construct the absolute path to the compiled C binary
# The binary is located at: C_project/c_engine/model_release.exe
# This path is relative to this script's location (server/ directory)
C_BINARY_PATH = os.path.join(
    os.path.dirname(os.path.dirname(__file__)),  # Go up from server/ to C_project/
    'c_engine',                                   # Enter c_engine/ directory
    'model_release.exe'                           # The compiled binary
)


def run_c_engine(csv_data: str) -> dict:
    """
    Pipes CSV stock data into the C binary and returns parsed JSON results.

    Parameters:
        csv_data (str): A CSV string with columns: Date, Open, High, Low, Close, Volume
                        This is the output of stock_data.fetch_stock_data_csv()

    Returns:
        dict: Parsed JSON containing:
            - prediction: "UP" or "DOWN"
            - confidence: float (0.0 to 1.0)
            - train_size: int (number of training samples)
            - test_size:  int (number of test samples)
            - metrics:    dict with accuracy, precision, recall, confusion_matrix
            - features:   list of dicts with date, ma5, ma10, rsi

    Raises:
        FileNotFoundError: If the C binary has not been compiled yet
        RuntimeError:      If the C binary exits with a non-zero exit code
        RuntimeError:      If the C binary output is not valid JSON
    """
    # Check that the C binary exists before attempting to run it
    if not os.path.exists(C_BINARY_PATH):
        raise FileNotFoundError(
            f"C binary not found at {C_BINARY_PATH}. "
            "Please compile it first with: gcc -Wall -Wextra -pthread -Iinclude "
            "src/*.c -o model_release.exe -O2 -lm -static"
        )

    try:
        # Launch the C binary as a subprocess
        # - input=csv_data:     Sends CSV data to C binary's stdin
        # - text=True:          Treat I/O as text strings (not bytes)
        # - capture_output=True: Capture both stdout and stderr
        # - check=True:         Raise CalledProcessError if exit code != 0
        result = subprocess.run(
            [C_BINARY_PATH],
            input=csv_data,
            text=True,
            capture_output=True,
            check=True
        )

        # Parse the JSON output from the C binary's stdout
        output_json = json.loads(result.stdout)
        return output_json

    except subprocess.CalledProcessError as e:
        # The C binary exited with a non-zero exit code
        print(f"C Engine Error Output: {e.stderr}")
        raise RuntimeError(f"C Engine failed with exit code {e.returncode}: {e.stderr}")

    except json.JSONDecodeError as e:
        # The C binary produced output that isn't valid JSON
        print(f"Failed to parse C engine output. Raw output:\n{result.stdout}")
        raise RuntimeError("Failed to parse JSON from C engine.")
