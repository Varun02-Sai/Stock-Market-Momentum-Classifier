/*
 * ============================================================================
 * File:        metrics.c
 * Project:     Stock Market Momentum Classifier
 * Description: Implements model evaluation metrics for binary classification.
 *              Computes the confusion matrix, accuracy, precision, recall,
 *              and F1 score from predicted vs. actual labels.
 *
 * Topics:      Functions, Pointers (output parameters via pointer dereferencing)
 * ============================================================================
 */

#include "metrics.h"

/*
 * compute_confusion_matrix
 * ------------------------
 * Iterates over all samples and classifies each prediction into one of
 * four categories based on the actual vs. predicted label:
 *
 *   TP (True Positive):  Actual=UP,   Predicted=UP   → Correct UP prediction
 *   FP (False Positive): Actual=DOWN, Predicted=UP   → Incorrectly predicted UP
 *   FN (False Negative): Actual=UP,   Predicted=DOWN → Missed an UP day
 *   TN (True Negative):  Actual=DOWN, Predicted=DOWN → Correct DOWN prediction
 *
 * The results are returned via pointers (output parameters), which is a
 * common C pattern to return multiple values from a single function.
 */
void compute_confusion_matrix(int *y_true, int *y_pred, int num_samples,
                               int *tp, int *fp, int *fn, int *tn) {
    /* Initialize all counters to zero */
    *tp = 0;
    *fp = 0;
    *fn = 0;
    *tn = 0;

    int i;
    for (i = 0; i < num_samples; i++) {
        if (y_true[i] == 1 && y_pred[i] == 1)
            (*tp)++;   /* Dereference pointer, then increment */
        else if (y_true[i] == 0 && y_pred[i] == 1)
            (*fp)++;
        else if (y_true[i] == 1 && y_pred[i] == 0)
            (*fn)++;
        else if (y_true[i] == 0 && y_pred[i] == 0)
            (*tn)++;
    }
}

/*
 * compute_accuracy
 * ----------------
 * Accuracy = (TP + TN) / Total
 *
 * Measures overall correctness: what fraction of all predictions were right?
 * Returns 0.0 if there are no samples to avoid division by zero.
 */
double compute_accuracy(int tp, int fp, int fn, int tn) {
    int total = tp + fp + fn + tn;
    if (total == 0) return 0.0;
    return (double)(tp + tn) / total;
}

/*
 * compute_precision
 * -----------------
 * Precision = TP / (TP + FP)
 *
 * Of all the times we predicted "UP", how often were we right?
 * High precision means the model rarely gives false alarms.
 * Returns 0.0 if no positive predictions were made.
 */
double compute_precision(int tp, int fp) {
    if (tp + fp == 0) return 0.0;
    return (double)tp / (tp + fp);
}

/*
 * compute_recall
 * --------------
 * Recall (Sensitivity) = TP / (TP + FN)
 *
 * Of all the actual "UP" days, how many did we correctly predict?
 * High recall means the model catches most UP movements.
 * Returns 0.0 if there are no actual positive samples.
 */
double compute_recall(int tp, int fn) {
    if (tp + fn == 0) return 0.0;
    return (double)tp / (tp + fn);
}

/*
 * compute_f1_score
 * ----------------
 * F1 Score = 2 * (Precision * Recall) / (Precision + Recall)
 *
 * The harmonic mean of precision and recall. It provides a single
 * metric that balances both concerns:
 *   - A model with high precision but low recall will have a low F1
 *   - A model with high recall but low precision will also have a low F1
 *   - Only when both are high does F1 become high
 *
 * Returns 0.0 if both precision and recall are zero.
 */
double compute_f1_score(double precision, double recall) {
    if (precision + recall == 0.0) return 0.0;
    return 2.0 * (precision * recall) / (precision + recall);
}
