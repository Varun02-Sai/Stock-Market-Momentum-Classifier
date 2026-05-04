/*
 * ============================================================================
 * File:        logistic.c
 * Project:     Stock Market Momentum Classifier
 * Description: Implements a Logistic Regression model from scratch in C.
 *              Includes sigmoid activation, batch gradient descent with
 *              L2 regularization to prevent overfitting, and prediction.
 *
 * Topics:      Functions, Structures, Pointers, Dynamic Memory (malloc/free)
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>   /* malloc(), free() */
#include <math.h>     /* exp() for sigmoid */
#include "logistic.h"

/*
 * init_model
 * ----------
 * Allocates memory for the weight vector and sets initial values.
 * All weights are initialized to 0.0 (zero initialization).
 */
void init_model(ModelWeights *model, int num_features, double learning_rate, int epochs) {
    model->num_features = num_features;
    model->learning_rate = learning_rate;
    model->epochs = epochs;
    model->bias = 0.0;

    /* Dynamically allocate the weight vector on the heap */
    model->weights = (double *)malloc(num_features * sizeof(double));

    /* Initialize all weights to zero */
    int i;
    for (i = 0; i < num_features; i++) {
        model->weights[i] = 0.0;
    }
}

/*
 * free_model
 * ----------
 * Frees the dynamically allocated weight vector.
 */
void free_model(ModelWeights *model) {
    if (model->weights) {
        free(model->weights);
        model->weights = NULL;   /* Prevent dangling pointer */
    }
}

/*
 * sigmoid
 * -------
 * The logistic (sigmoid) activation function:
 *   sigmoid(z) = 1 / (1 + e^(-z))
 *
 * Maps any real number to the range (0, 1) for probability interpretation.
 * Clamps input to [-500, 500] to avoid overflow in exp().
 */
double sigmoid(double z) {
    /* Clamp to prevent overflow */
    if (z > 500.0) z = 500.0;
    if (z < -500.0) z = -500.0;
    return 1.0 / (1.0 + exp(-z));
}

/*
 * predict_prob
 * ------------
 * Computes the probability that the stock goes UP (class 1).
 * P(UP) = sigmoid(bias + w0*x0 + w1*x1 + ... + wn*xn)
 */
double predict_prob(ModelWeights *model, double *x) {
    double z = model->bias;
    int i;
    for (i = 0; i < model->num_features; i++) {
        z += model->weights[i] * x[i];
    }
    return sigmoid(z);
}

/*
 * predict_class
 * -------------
 * Returns 1 (UP) if probability >= threshold, else 0 (DOWN).
 */
int predict_class(ModelWeights *model, double *x, double threshold) {
    double prob = predict_prob(model, x);
    return (prob >= threshold) ? 1 : 0;
}

/*
 * train_model
 * -----------
 * Trains logistic regression using Batch Gradient Descent with L2 Regularization.
 *
 * L2 Regularization prevents overfitting by penalizing large weights:
 *   Loss = CrossEntropy + (lambda/2) * sum(wi^2)
 *   Gradient update: w[j] -= lr * (dw[j] + lambda * w[j])
 *
 * This discourages the model from relying too heavily on any single feature,
 * which is especially important with financial data that has noise.
 *
 * The regularization parameter lambda = 0.01 provides a good balance between
 * fitting the training data and generalizing to unseen data.
 */
void train_model(ModelWeights *model, double **X, int *y, int num_samples) {
    int epoch, i, j;

    /* L2 regularization strength — prevents overfitting */
    double lambda = 0.01;

    for (epoch = 0; epoch < model->epochs; epoch++) {
        double db = 0.0;

        /* Allocate gradient array for weights */
        double *dw = (double *)malloc(model->num_features * sizeof(double));
        for (j = 0; j < model->num_features; j++)
            dw[j] = 0.0;

        /* Accumulate gradients over all training samples */
        for (i = 0; i < num_samples; i++) {
            double prob = predict_prob(model, X[i]);
            double dz = prob - y[i];

            db += dz;

            for (j = 0; j < model->num_features; j++) {
                dw[j] += dz * X[i][j];
            }
        }

        /* Average the gradients */
        db /= num_samples;
        for (j = 0; j < model->num_features; j++) {
            dw[j] /= num_samples;
        }

        /* Update bias (no regularization on bias) */
        model->bias -= model->learning_rate * db;

        /* Update weights WITH L2 regularization penalty */
        for (j = 0; j < model->num_features; j++) {
            model->weights[j] -= model->learning_rate * (dw[j] + lambda * model->weights[j]);
        }

        free(dw);
    }
}
