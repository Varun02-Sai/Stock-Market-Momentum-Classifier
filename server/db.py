"""
=============================================================================
File:        db.py
Project:     Stock Market Momentum Classifier
Description: SQLite database layer for caching prediction results.

             Every time a user runs a prediction, the results are stored
             in a local SQLite database (momentum.db). This enables:
             - Viewing prediction history without re-running the C engine
             - Tracking model performance over time
             - Offline access to past analyses

Database Schema:
    predictions table:
        id          INTEGER PRIMARY KEY AUTOINCREMENT
        ticker      TEXT        — NSE stock symbol (e.g., 'RELIANCE')
        timestamp   DATETIME    — When the prediction was made
        prediction  TEXT        — 'UP' or 'DOWN'
        confidence  REAL        — Confidence percentage (0-100)
        metrics     JSON        — Serialized accuracy, precision, recall, confusion matrix
        features    JSON        — Serialized feature data (MA5, MA10, RSI per day)

Why SQLite?
    - Zero-configuration: no server setup, just a file
    - Built into Python standard library
    - Sufficient for single-user academic project
=============================================================================
"""

import sqlite3    # Python's built-in SQLite database interface
import json       # For serializing/deserializing complex data as JSON strings
from datetime import datetime  # For timestamping predictions

# Database file path (created in the server/ directory)
DB_FILE = 'momentum.db'


def init_db():
    """
    Initializes the SQLite database and creates the predictions table
    if it doesn't already exist. Called once when the Flask app starts.

    Uses 'CREATE TABLE IF NOT EXISTS' to be idempotent — safe to call
    multiple times without error.
    """
    conn = sqlite3.connect(DB_FILE)
    c = conn.cursor()

    # Create the predictions table with all necessary columns
    c.execute('''
        CREATE TABLE IF NOT EXISTS predictions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            ticker TEXT,
            timestamp DATETIME,
            prediction TEXT,
            confidence REAL,
            metrics JSON,
            features JSON
        )
    ''')

    conn.commit()  # Save changes to disk
    conn.close()   # Release the database connection


def save_prediction(ticker, prediction_data):
    """
    Saves a prediction result to the database.

    Parameters:
        ticker (str): NSE stock symbol (e.g., 'RELIANCE')
        prediction_data (dict): Full response from the API containing
                                prediction, confidence, metrics, and features

    The metrics and features are complex nested objects, so they are
    serialized to JSON strings before storage using json.dumps().
    """
    conn = sqlite3.connect(DB_FILE)
    c = conn.cursor()

    # Insert the prediction with current timestamp
    c.execute('''
        INSERT INTO predictions (ticker, timestamp, prediction, confidence, metrics, features)
        VALUES (?, ?, ?, ?, ?, ?)
    ''', (
        ticker.upper(),                                    # Normalize ticker to uppercase
        datetime.now().isoformat(),                        # ISO 8601 timestamp
        prediction_data.get('prediction'),                 # 'UP' or 'DOWN'
        prediction_data.get('confidence'),                 # Confidence percentage
        json.dumps(prediction_data.get('metrics', {})),    # Serialize metrics dict to JSON string
        json.dumps(prediction_data.get('features', []))    # Serialize features list to JSON string
    ))

    conn.commit()
    conn.close()


def get_history(limit=10):
    """
    Retrieves the most recent prediction records from the database.

    Parameters:
        limit (int): Maximum number of records to return (default: 10)

    Returns:
        list[dict]: List of prediction records, each containing:
                    id, ticker, timestamp, prediction, confidence, metrics
                    (features are excluded to reduce payload size)
    """
    conn = sqlite3.connect(DB_FILE)
    conn.row_factory = sqlite3.Row  # Enable dict-like access to rows

    c = conn.cursor()
    c.execute('SELECT * FROM predictions ORDER BY timestamp DESC LIMIT ?', (limit,))
    rows = c.fetchall()
    conn.close()

    # Convert SQLite Row objects to plain Python dicts
    history = []
    for row in rows:
        history.append({
            'id': row['id'],
            'ticker': row['ticker'],
            'timestamp': row['timestamp'],
            'prediction': row['prediction'],
            'confidence': row['confidence'],
            'metrics': json.loads(row['metrics']),  # Deserialize JSON string back to dict
            # 'features' intentionally omitted to save payload size
        })

    return history
