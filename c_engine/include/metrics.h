/*
 * ============================================================================
 * File:        metrics.h
 * Project:     Stock Market Momentum Classifier
 * Description: Header file for model evaluation metrics. Provides functions
 *              to compute a confusion matrix and derive precision, recall,
 *              accuracy, and F1-score from the matrix values.
 *
 * Topics:      Functions, Pointers (output parameters)
 * ============================================================================
 */

#ifndef METRICS_H
#define METRICS_H

/*
 * Function: compute_confusion_matrix
 * ----------------------------------
 * Computes a 2x2 confusion matrix from predicted and actual labels.
 *
 * The confusion matrix is:
 *                    Predicted UP   Predicted DOWN
 *   Actual UP     |     TP      |      FN       |
 *   Actual DOWN   |     FP      |      TN       |
 *
 * Parameters:
 *   y_true      - Array of actual labels (0 or 1)
 *   y_pred      - Array of predicted labels (0 or 1)
 *   num_samples - Number of samples
 *   tp          - Pointer to store True Positives count (output)
 *   fp          - Pointer to store False Positives count (output)
 *   fn          - Pointer to store False Negatives count (output)
 *   tn          - Pointer to store True Negatives count (output)
 *
 * Note: tp, fp, fn, tn are passed as pointers so the function can
 * return multiple values through pointer dereferencing.
 */
void compute_confusion_matrix(int *y_true, int *y_pred, int num_samples, int *tp, int *fp, int *fn, int *tn);

/*
 * Function: compute_accuracy
 * --------------------------
 * Accuracy = (TP + TN) / (TP + FP + FN + TN)
 * Measures the overall correctness of the model.
 */
double compute_accuracy(int tp, int fp, int fn, int tn);

/*
 * Function: compute_precision
 * ---------------------------
 * Precision = TP / (TP + FP)
 * Of all samples predicted as UP, what fraction were actually UP?
 * High precision means few false positives.
 */
double compute_precision(int tp, int fp);

/*
 * Function: compute_recall
 * ------------------------
 * Recall = TP / (TP + FN)
 * Of all actual UP samples, what fraction did the model catch?
 * High recall means few false negatives.
 */
double compute_recall(int tp, int fn);

/*
 * Function: compute_f1_score
 * --------------------------
 * F1 = 2 * (Precision * Recall) / (Precision + Recall)
 * Harmonic mean of precision and recall. Balances both metrics.
 */
double compute_f1_score(double precision, double recall);

#endif /* METRICS_H */
