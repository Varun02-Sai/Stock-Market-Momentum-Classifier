/*
 * ============================================================================
 * File:        logistic.h
 * Project:     Stock Market Momentum Classifier
 * Description: Header file for the Logistic Regression machine learning model.
 *              Defines the ModelWeights structure to store learned parameters
 *              and declares functions for initialization, training (gradient
 *              descent), and prediction (sigmoid activation).
 *
 * Topics:      Structures, Function Prototypes, Pointers, Dynamic Memory
 * ============================================================================
 */

#ifndef LOGISTIC_H
#define LOGISTIC_H

/*
 * ModelWeights Structure
 * ----------------------
 * Stores all parameters of a trained Logistic Regression model.
 *
 * In logistic regression, the prediction is:
 *   P(y=1|x) = sigmoid(w0*x0 + w1*x1 + ... + wn*xn + bias)
 *
 * Members:
 *   weights       - Dynamically allocated array of feature weights (w0..wn)
 *                   Allocated on the heap using malloc() in init_model()
 *   bias          - The bias (intercept) term of the model
 *   num_features  - Number of input features (determines size of weights array)
 *   learning_rate - Step size for gradient descent optimization (e.g., 0.01)
 *   epochs        - Number of complete passes through the training data
 */
typedef struct {
    double *weights;    /* Pointer to dynamically allocated weight vector */
    double bias;        /* Bias (intercept) term */
    int num_features;   /* Number of features in the input */
    double learning_rate;  /* Learning rate for gradient descent */
    int epochs;         /* Number of training iterations */
} ModelWeights;

/*
 * Function: init_model
 * --------------------
 * Allocates memory for the weight vector and initializes all weights to zero.
 * Must be called before training.
 *
 * Parameters:
 *   model         - Pointer to ModelWeights struct to initialize
 *   num_features  - Number of input features
 *   learning_rate - Learning rate (step size for gradient descent)
 *   epochs        - Number of training epochs
 */
void init_model(ModelWeights *model, int num_features, double learning_rate, int epochs);

/*
 * Function: free_model
 * --------------------
 * Frees the dynamically allocated weight vector to prevent memory leaks.
 * Should be called when the model is no longer needed.
 *
 * Parameters:
 *   model - Pointer to ModelWeights struct to free
 */
void free_model(ModelWeights *model);

/*
 * Function: sigmoid
 * -----------------
 * The sigmoid (logistic) activation function:
 *   sigmoid(z) = 1 / (1 + e^(-z))
 *
 * Maps any real number to the range (0, 1), which can be interpreted
 * as a probability. This is the core of logistic regression.
 *
 * Parameters:
 *   z - The input value (linear combination of features and weights)
 *
 * Returns:
 *   A double in the range (0, 1)
 */
double sigmoid(double z);

/*
 * Function: train_model
 * ---------------------
 * Trains the logistic regression model using batch gradient descent.
 *
 * For each epoch, it:
 *   1. Computes predictions for all training samples
 *   2. Calculates the gradient of the loss w.r.t. each weight
 *   3. Updates weights: w = w - learning_rate * gradient
 *
 * The loss function minimized is Binary Cross-Entropy (Log Loss).
 *
 * Parameters:
 *   model       - Pointer to the ModelWeights struct
 *   X           - 2D array of feature vectors (num_samples x num_features)
 *   y           - Array of labels (0 = DOWN, 1 = UP)
 *   num_samples - Number of training samples
 */
void train_model(ModelWeights *model, double **X, int *y, int num_samples);

/*
 * Function: predict_prob
 * ----------------------
 * Computes the probability that the stock goes UP (class 1).
 *
 * Parameters:
 *   model - Pointer to trained ModelWeights
 *   x     - Feature vector for a single sample
 *
 * Returns:
 *   Probability in range (0, 1) — higher means more likely UP
 */
double predict_prob(ModelWeights *model, double *x);

/*
 * Function: predict_class
 * -----------------------
 * Predicts the binary class label based on a probability threshold.
 *
 * Parameters:
 *   model     - Pointer to trained ModelWeights
 *   x         - Feature vector for a single sample
 *   threshold - Decision boundary (typically 0.5)
 *
 * Returns:
 *   1 if predicted UP, 0 if predicted DOWN
 */
int predict_class(ModelWeights *model, double *x, double threshold);

#endif /* LOGISTIC_H */
