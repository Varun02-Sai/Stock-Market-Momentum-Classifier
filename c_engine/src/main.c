/*
 * ============================================================================
 * File:        main.c
 * Project:     Stock Market Momentum Classifier
 * Description: Entry point for the C algorithm engine. Reads CSV stock data
 *              from stdin, performs feature engineering with threads, splits
 *              data into 80% train / 20% test for proper evaluation, trains
 *              a regularized logistic regression model, and outputs JSON.
 *
 * Anti-Overfitting Measures:
 *   1. 80/20 Train-Test Split — metrics are computed on UNSEEN test data
 *   2. L2 Regularization — in logistic.c, penalizes large weights
 *   3. Feature Normalization — scales features to similar ranges
 *
 * Data Flow:
 *   Python → (CSV via stdin) → C Engine → (JSON via stdout) → Python → React
 *
 * Topics:      Modular Programming, Headers, Structures, Pointers,
 *              Dynamic Memory, Functions, String Parsing
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "features.h"
#include "logistic.h"
#include "metrics.h"

#define MAX_RECORDS 10000
#define MAX_LINE 256

int main() {
    int i;

    /* ====================================================================
     * STEP 1: PARSE CSV INPUT
     * ==================================================================== */
    StockRecord *records = (StockRecord *)malloc(MAX_RECORDS * sizeof(StockRecord));
    int num_records = 0;

    char line[MAX_LINE];

    /* Read and skip CSV header if present */
    if (fgets(line, sizeof(line), stdin)) {
        if (strstr(line, "Date") || strstr(line, "date")) {
            /* Header line — skip */
        } else {
            sscanf(line, "%[^,],%lf,%lf,%lf,%lf,%lf",
                   records[num_records].date,
                   &records[num_records].open,
                   &records[num_records].high,
                   &records[num_records].low,
                   &records[num_records].close,
                   &records[num_records].volume);
            num_records++;
        }
    }

    while (fgets(line, sizeof(line), stdin) && num_records < MAX_RECORDS) {
        sscanf(line, "%[^,],%lf,%lf,%lf,%lf,%lf",
               records[num_records].date,
               &records[num_records].open,
               &records[num_records].high,
               &records[num_records].low,
               &records[num_records].close,
               &records[num_records].volume);
        num_records++;
    }

    if (num_records < 30) {
        printf("{\"error\": \"Not enough data (need at least 30 trading days)\"}\n");
        free(records);
        return 1;
    }

    /* ====================================================================
     * STEP 2: FEATURE ENGINEERING (with parallel threads)
     * ==================================================================== */
    double *ma5  = (double *)malloc(num_records * sizeof(double));
    double *ma10 = (double *)malloc(num_records * sizeof(double));
    double *rsi  = (double *)malloc(num_records * sizeof(double));

    parallel_feature_extract(records, num_records, ma5, ma10, rsi, 4);

    /* ====================================================================
     * STEP 3: COMPUTE NORMALIZATION STATS (mean & std for each feature)
     * ====================================================================
     * Proper normalization (z-score) ensures gradient descent converges
     * well regardless of the stock price range (₹50 vs ₹50000).
     * This makes the model GENERALIZED across different stocks.
     */
    int num_features = 8;
    int total_valid = 0;

    /* First pass: count valid samples and compute sums */
    double feat_sum[8] = {0};
    double feat_sq_sum[8] = {0};

    for (i = 14; i < num_records - 1; i++) {
        double feat[8];
        feat[0] = records[i].open;
        feat[1] = records[i].high;
        feat[2] = records[i].low;
        feat[3] = records[i].close;
        feat[4] = records[i].volume;
        feat[5] = ma5[i];
        feat[6] = ma10[i];
        feat[7] = rsi[i];

        int j;
        for (j = 0; j < num_features; j++) {
            feat_sum[j] += feat[j];
            feat_sq_sum[j] += feat[j] * feat[j];
        }
        total_valid++;
    }

    /* Compute mean and standard deviation for z-score normalization */
    double feat_mean[8], feat_std[8];
    int j;
    for (j = 0; j < num_features; j++) {
        feat_mean[j] = feat_sum[j] / total_valid;
        feat_std[j] = feat_sq_sum[j] / total_valid - feat_mean[j] * feat_mean[j];
        feat_std[j] = (feat_std[j] > 0) ? sqrt(feat_std[j]) : 1.0;
    }

    /* ====================================================================
     * STEP 4: BUILD FEATURE MATRIX with Z-SCORE NORMALIZATION
     * ==================================================================== */
    double **X = (double **)malloc(total_valid * sizeof(double *));
    int *y = (int *)malloc(total_valid * sizeof(int));
    int valid_samples = 0;

    for (i = 14; i < num_records - 1; i++) {
        X[valid_samples] = (double *)malloc(num_features * sizeof(double));

        /* Z-score normalization: (x - mean) / std */
        X[valid_samples][0] = (records[i].open   - feat_mean[0]) / feat_std[0];
        X[valid_samples][1] = (records[i].high   - feat_mean[1]) / feat_std[1];
        X[valid_samples][2] = (records[i].low    - feat_mean[2]) / feat_std[2];
        X[valid_samples][3] = (records[i].close  - feat_mean[3]) / feat_std[3];
        X[valid_samples][4] = (records[i].volume - feat_mean[4]) / feat_std[4];
        X[valid_samples][5] = (ma5[i]            - feat_mean[5]) / feat_std[5];
        X[valid_samples][6] = (ma10[i]           - feat_mean[6]) / feat_std[6];
        X[valid_samples][7] = (rsi[i]            - feat_mean[7]) / feat_std[7];

        /* Label: 1 if tomorrow's close > today's close */
        y[valid_samples] = (records[i + 1].close > records[i].close) ? 1 : 0;
        valid_samples++;
    }

    /* ====================================================================
     * STEP 5: 80/20 TRAIN-TEST SPLIT
     * ====================================================================
     * To prevent overfitting and properly evaluate the model, we split
     * the data chronologically: first 80% for training, last 20% for testing.
     * This simulates real-world usage where we train on past data and
     * predict future movements.
     */
    int train_size = (int)(valid_samples * 0.8);
    int test_size = valid_samples - train_size;

    /* Training set is X[0..train_size-1] */
    /* Test set is X[train_size..valid_samples-1] */

    /* ====================================================================
     * STEP 6: TRAIN MODEL (on training set only)
     * ==================================================================== */
    ModelWeights model;
    init_model(&model, num_features, 0.01, 1000);

    /* Train ONLY on the first 80% of data */
    train_model(&model, X, y, train_size);

    /* ====================================================================
     * STEP 7: EVALUATE ON TEST SET (unseen data)
     * ====================================================================
     * Metrics are computed on the 20% test set that the model has
     * NEVER seen during training. This gives an honest estimate
     * of how well the model will perform on future data.
     */
    int *y_pred_test = (int *)malloc(test_size * sizeof(int));
    for (i = 0; i < test_size; i++) {
        y_pred_test[i] = predict_class(&model, X[train_size + i], 0.5);
    }

    int tp, fp, fn, tn;
    compute_confusion_matrix(y + train_size, y_pred_test, test_size, &tp, &fp, &fn, &tn);

    double accuracy  = compute_accuracy(tp, fp, fn, tn);
    double precision = compute_precision(tp, fp);
    double recall    = compute_recall(tp, fn);

    /* ====================================================================
     * STEP 8: PREDICT NEXT DAY (using the very last record)
     * ==================================================================== */
    int last_idx = num_records - 1;
    double x_next[8] = {
        (records[last_idx].open   - feat_mean[0]) / feat_std[0],
        (records[last_idx].high   - feat_mean[1]) / feat_std[1],
        (records[last_idx].low    - feat_mean[2]) / feat_std[2],
        (records[last_idx].close  - feat_mean[3]) / feat_std[3],
        (records[last_idx].volume - feat_mean[4]) / feat_std[4],
        (ma5[last_idx]            - feat_mean[5]) / feat_std[5],
        (ma10[last_idx]           - feat_mean[6]) / feat_std[6],
        (rsi[last_idx]            - feat_mean[7]) / feat_std[7]
    };

    double next_prob = predict_prob(&model, x_next);
    int next_pred = (next_prob >= 0.5) ? 1 : 0;

    /* ====================================================================
     * STEP 9: OUTPUT JSON RESULTS
     * ==================================================================== */
    printf("{\n");
    printf("  \"prediction\": \"%s\",\n", next_pred ? "UP" : "DOWN");
    printf("  \"confidence\": %.2f,\n", next_prob);
    printf("  \"train_size\": %d,\n", train_size);
    printf("  \"test_size\": %d,\n", test_size);
    printf("  \"metrics\": {\n");
    printf("    \"accuracy\": %.4f,\n", accuracy);
    printf("    \"precision\": %.4f,\n", precision);
    printf("    \"recall\": %.4f,\n", recall);
    printf("    \"confusion_matrix\": {\n");
    printf("      \"tp\": %d,\n", tp);
    printf("      \"fp\": %d,\n", fp);
    printf("      \"fn\": %d,\n", fn);
    printf("      \"tn\": %d\n", tn);
    printf("    }\n");
    printf("  },\n");

    /* Output feature data for charting */
    printf("  \"features\": [\n");
    for (i = 0; i < num_records; i++) {
        printf("    {\"date\": \"%s\", \"ma5\": %.2f, \"ma10\": %.2f, \"rsi\": %.2f}",
               records[i].date, ma5[i], ma10[i], rsi[i]);
        if (i < num_records - 1) printf(",\n");
        else printf("\n");
    }
    printf("  ]\n");
    printf("}\n");

    /* ====================================================================
     * STEP 10: CLEANUP — FREE ALL DYNAMICALLY ALLOCATED MEMORY
     * ==================================================================== */
    for (i = 0; i < valid_samples; i++)
        free(X[i]);
    free(X);
    free(y);
    free(y_pred_test);
    free(records);
    free(ma5);
    free(ma10);
    free(rsi);
    free_model(&model);

    return 0;
}
