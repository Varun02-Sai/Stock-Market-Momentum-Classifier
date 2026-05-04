/*
 * ============================================================================
 * File:        features.c
 * Project:     Stock Market Momentum Classifier
 * Description: Implements feature engineering functions for technical analysis.
 *              Computes Moving Averages (MA5, MA10) and RSI(14) from raw
 *              stock data. Uses POSIX threads (pthreads) to parallelize the
 *              moving average computation across data chunks.
 *
 * Topics:      Pointers, Dynamic Arrays, C Threads (pthreads), Functions
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>   /* POSIX threads library for parallel computation */
#include <math.h>
#include "features.h"

/*
 * calculate_moving_average
 * ------------------------
 * Computes a Simple Moving Average (SMA) over the closing prices.
 *
 * Algorithm:
 *   For each day i:
 *     If i < window_size - 1: not enough data, set MA to 0
 *     Else: MA[i] = (close[i] + close[i-1] + ... + close[i-window+1]) / window
 *
 * Example for window_size = 5 and closes [100, 102, 104, 103, 105, 107]:
 *   Day 0-3: MA = 0 (not enough data)
 *   Day 4:   MA = (100+102+104+103+105)/5 = 102.8
 *   Day 5:   MA = (102+104+103+105+107)/5 = 104.2
 */
void calculate_moving_average(StockRecord *records, int num_records, int window_size, double *ma_out) {
    int i, j;
    for (i = 0; i < num_records; i++) {
        if (i < window_size - 1) {
            /* Not enough historical data to compute the MA yet */
            ma_out[i] = 0.0;
        } else {
            /* Sum the last `window_size` closing prices */
            double sum = 0.0;
            for (j = 0; j < window_size; j++) {
                sum += records[i - j].close;  /* Access via pointer arithmetic */
            }
            ma_out[i] = sum / window_size;  /* Arithmetic mean */
        }
    }
}

/*
 * calculate_rsi
 * -------------
 * Computes the Relative Strength Index (RSI) using Wilder's smoothing method.
 *
 * Algorithm:
 *   1. For the first `window_size` days, compute the average gain and average
 *      loss from day-over-day price changes.
 *   2. For subsequent days, use exponential smoothing:
 *        avg_gain = (prev_avg_gain * (window-1) + current_gain) / window
 *        avg_loss = (prev_avg_loss * (window-1) + current_loss) / window
 *   3. RS = avg_gain / avg_loss
 *   4. RSI = 100 - (100 / (1 + RS))
 *
 * Interpretation:
 *   RSI > 70 → Overbought (stock may be due for a pullback)
 *   RSI < 30 → Oversold (stock may be due for a rally)
 *   RSI ~ 50 → Neutral momentum
 */
void calculate_rsi(StockRecord *records, int num_records, int window_size, double *rsi_out) {
    int i;

    /* Guard: need at least window_size+1 records to compute RSI */
    if (num_records <= window_size) {
        for (i = 0; i < num_records; i++) rsi_out[i] = 0.0;
        return;
    }

    double gain_sum = 0.0, loss_sum = 0.0;

    /* Step 1: Compute initial average gain and loss over the first window */
    for (i = 1; i <= window_size; i++) {
        double change = records[i].close - records[i - 1].close;
        if (change > 0)
            gain_sum += change;   /* Positive change = gain */
        else
            loss_sum -= change;   /* Negative change = loss (negate to make positive) */
    }

    double avg_gain = gain_sum / window_size;
    double avg_loss = loss_sum / window_size;

    /* Step 2: Compute RSI for each day */
    for (i = 0; i < num_records; i++) {
        if (i < window_size) {
            /* Not enough data for RSI calculation */
            rsi_out[i] = 0.0;
        } else {
            if (i > window_size) {
                /* Wilder's smoothing: exponential moving average of gains/losses */
                double change = records[i].close - records[i - 1].close;
                double gain = (change > 0) ? change : 0.0;
                double loss = (change < 0) ? -change : 0.0;
                avg_gain = ((avg_gain * (window_size - 1)) + gain) / window_size;
                avg_loss = ((avg_loss * (window_size - 1)) + loss) / window_size;
            }

            /* Compute RSI from relative strength */
            if (avg_loss == 0) {
                rsi_out[i] = 100.0;  /* No losses → RSI maxes at 100 */
            } else {
                double rs = avg_gain / avg_loss;  /* Relative Strength */
                rsi_out[i] = 100.0 - (100.0 / (1.0 + rs));
            }
        }
    }
}

/*
 * feature_thread_func
 * -------------------
 * Thread worker function for parallel feature extraction.
 *
 * Each thread receives a ThreadArgs struct specifying its index range
 * [start_idx, end_idx). It computes MA5 and MA10 for its assigned
 * chunk of the dataset. Since each thread writes to different indices
 * of the output arrays, no mutex or synchronization is needed.
 *
 * Note: RSI is NOT computed here because it depends on a running
 * exponential average that spans the entire dataset sequentially.
 *
 * Parameters:
 *   arg - void pointer cast to ThreadArgs* (required by pthread_create)
 *
 * Returns: NULL (required by pthread function signature)
 */
void *feature_thread_func(void *arg) {
    /* Cast the void pointer back to our ThreadArgs struct */
    ThreadArgs *t_args = (ThreadArgs *)arg;
    int i, j;

    /* Iterate over this thread's assigned chunk of data */
    for (i = t_args->start_idx; i < t_args->end_idx; i++) {

        /* Compute 5-day Moving Average for this index */
        if (i >= 4) {
            double sum = 0.0;
            for (j = 0; j < 5; j++)
                sum += t_args->records[i - j].close;
            t_args->ma5_out[i] = sum / 5.0;
        } else {
            t_args->ma5_out[i] = 0.0;  /* Not enough data */
        }

        /* Compute 10-day Moving Average for this index */
        if (i >= 9) {
            double sum = 0.0;
            for (j = 0; j < 10; j++)
                sum += t_args->records[i - j].close;
            t_args->ma10_out[i] = sum / 10.0;
        } else {
            t_args->ma10_out[i] = 0.0;  /* Not enough data */
        }
    }

    return NULL;
}

/*
 * parallel_feature_extract
 * ------------------------
 * Orchestrates multi-threaded feature extraction.
 *
 * Strategy:
 *   1. Divide the dataset into `num_threads` equal chunks
 *   2. Create a pthread for each chunk to compute MA5 and MA10
 *   3. Wait for all threads to complete (pthread_join)
 *   4. Compute RSI sequentially (due to its recursive dependency)
 *
 * Why parallelize?
 *   Moving averages are "embarrassingly parallel" — each day's MA
 *   only depends on the raw closing prices (which are read-only).
 *   By splitting the data across threads, we achieve near-linear
 *   speedup on multi-core CPUs.
 *
 * Thread Safety:
 *   - Each thread writes to a DIFFERENT range of indices in the output arrays
 *   - The input records[] array is only READ, never written
 *   - Therefore, no mutexes or locks are needed
 */
void parallel_feature_extract(StockRecord *records, int num_records,
                               double *ma5_out, double *ma10_out,
                               double *rsi_out, int num_threads) {
    pthread_t threads[num_threads];   /* Array of thread handles */
    ThreadArgs args[num_threads];     /* Array of arguments for each thread */

    int chunk_size = num_records / num_threads;  /* Records per thread */
    int i;

    /* Step 1: Create threads, each handling a chunk of the data */
    for (i = 0; i < num_threads; i++) {
        args[i].records = records;         /* Shared read-only data */
        args[i].start_idx = i * chunk_size;
        /* Last thread handles any remainder records */
        args[i].end_idx = (i == num_threads - 1) ? num_records : (i + 1) * chunk_size;
        args[i].num_records = num_records;
        args[i].ma5_out = ma5_out;         /* Shared output arrays */
        args[i].ma10_out = ma10_out;
        args[i].rsi_out = rsi_out;

        /* Create the thread — passes feature_thread_func as the worker */
        pthread_create(&threads[i], NULL, feature_thread_func, &args[i]);
    }

    /* Step 2: Wait for all threads to finish (barrier synchronization) */
    for (i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    /* Step 3: RSI must be computed sequentially due to its recursive nature */
    calculate_rsi(records, num_records, 14, rsi_out);
}
