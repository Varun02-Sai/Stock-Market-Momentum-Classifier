/*
 * ============================================================================
 * File:        features.h
 * Project:     Stock Market Momentum Classifier
 * Description: Header file declaring the data structures and function
 *              prototypes for the feature engineering module. This module
 *              computes technical indicators (Moving Averages, RSI) from
 *              raw stock OHLCV data. It also defines a threaded parallel
 *              feature extraction interface using POSIX threads (pthreads).
 *
 * Topics:      Structures, Function Prototypes, Header Files, Pointers
 * ============================================================================
 */

#ifndef FEATURES_H
#define FEATURES_H

/*
 * StockRecord Structure
 * ---------------------
 * Holds one day of OHLCV (Open, High, Low, Close, Volume) market data.
 * Each field corresponds to a column in the CSV input piped from Python.
 *
 * Members:
 *   date   - The trading date as a string in "YYYY-MM-DD" format (10 chars + null)
 *   open   - The opening price of the stock for that day
 *   high   - The highest price reached during the trading day
 *   low    - The lowest price reached during the trading day
 *   close  - The closing price (most important for trend analysis)
 *   volume - The total number of shares traded that day
 */
typedef struct {
    char date[11];      /* "YYYY-MM-DD" format, 10 characters + null terminator */
    double open;        /* Opening price */
    double high;        /* Highest price of the day */
    double low;         /* Lowest price of the day */
    double close;       /* Closing price — used for MA and RSI calculations */
    double volume;      /* Trading volume (number of shares) */
} StockRecord;

/*
 * ThreadArgs Structure
 * --------------------
 * Argument structure passed to each pthread during parallel feature extraction.
 * Each thread receives a pointer to the shared data array and its assigned
 * index range [start_idx, end_idx) so it can compute features independently
 * for its chunk without data races.
 *
 * Members:
 *   records     - Pointer to the full array of StockRecord data (shared, read-only)
 *   start_idx   - Starting index (inclusive) for this thread's chunk
 *   end_idx     - Ending index (exclusive) for this thread's chunk
 *   num_records - Total number of records in the dataset
 *   ma5_out     - Pointer to output array for 5-day Moving Average values
 *   ma10_out    - Pointer to output array for 10-day Moving Average values
 *   rsi_out     - Pointer to output array for RSI values (computed sequentially)
 */
typedef struct {
    StockRecord *records;   /* Pointer to shared stock data array */
    int start_idx;          /* Start of this thread's data chunk */
    int end_idx;            /* End of this thread's data chunk */
    int num_records;        /* Total number of records */
    double *ma5_out;        /* Output: 5-day moving average array */
    double *ma10_out;       /* Output: 10-day moving average array */
    double *rsi_out;        /* Output: RSI(14) array */
} ThreadArgs;

/*
 * Function: calculate_moving_average
 * ----------------------------------
 * Computes a Simple Moving Average (SMA) over a sliding window of closing prices.
 *
 * The SMA for day i is the arithmetic mean of the previous `window_size` closing
 * prices (including day i). Days before the window is full are set to 0.0.
 *
 * Parameters:
 *   records     - Array of StockRecord structs containing historical data
 *   num_records - Number of records in the array
 *   window_size - Number of days to average (e.g., 5 for MA5, 10 for MA10)
 *   ma_out      - Output array (pre-allocated) to store computed MA values
 */
void calculate_moving_average(StockRecord *records, int num_records, int window_size, double *ma_out);

/*
 * Function: calculate_rsi
 * -----------------------
 * Computes the Relative Strength Index (RSI) using Wilder's smoothing method.
 *
 * RSI = 100 - (100 / (1 + RS)), where RS = Avg Gain / Avg Loss
 * RSI ranges from 0 to 100:
 *   - RSI > 70: Stock is "overbought" (may go down)
 *   - RSI < 30: Stock is "oversold" (may go up)
 *
 * Parameters:
 *   records     - Array of StockRecord structs
 *   num_records - Number of records
 *   window_size - RSI lookback period (typically 14)
 *   rsi_out     - Output array for RSI values
 */
void calculate_rsi(StockRecord *records, int num_records, int window_size, double *rsi_out);

/*
 * Function: parallel_feature_extract
 * ----------------------------------
 * Splits the dataset into chunks and uses POSIX threads (pthreads) to compute
 * moving averages in parallel. Each thread handles a contiguous slice of the
 * data array. After all threads complete, RSI is computed sequentially because
 * it depends on an exponentially-weighted running average (not parallelizable).
 *
 * Parameters:
 *   records     - Array of StockRecord structs
 *   num_records - Number of records
 *   ma5_out     - Output array for 5-day MA
 *   ma10_out    - Output array for 10-day MA
 *   rsi_out     - Output array for RSI
 *   num_threads - Number of threads to use for parallelization
 */
void parallel_feature_extract(StockRecord *records, int num_records, double *ma5_out, double *ma10_out, double *rsi_out, int num_threads);

#endif /* FEATURES_H */
