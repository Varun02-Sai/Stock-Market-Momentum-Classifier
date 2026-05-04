# Stock Market Momentum Classifier

A stock market prediction system built for NSE (National Stock Exchange of India) stocks. The core ML algorithm — Logistic Regression — is written entirely in C from scratch, without using any ML libraries. The system predicts whether a stock will go **UP** or **DOWN** on the next trading day.

We built this as a full-stack application with a React dashboard for visualization, a Flask API for data handling, and the C engine doing all the heavy lifting for training and prediction.

## How It Works

```
React Dashboard  <-->  Flask API Server  <-->  C Engine (model_release.exe)
   (port 5173)          (port 5000)            Logistic Regression + pthreads
```

1. User enters an NSE stock ticker (like RELIANCE, TCS, INFY)
2. Flask fetches 2 years of historical data from Yahoo Finance
3. Data is piped as CSV into the C binary via subprocess
4. The C engine computes features (MA5, MA10, RSI), trains a logistic regression model, and predicts the next day's direction
5. Results are sent back as JSON and displayed on the dashboard

## Getting Started

**You need these installed:**
- GCC (MinGW-w64) for compiling C code
- Python 3 with pip
- Node.js with npm

**First time setup:**
```bash
mingw32-make install
```

**Run the project:**
```bash
mingw32-make run
```

This compiles the C code, starts the Flask server, and launches the React frontend. Open `http://localhost:5173` in your browser.

**Other commands:**
- `mingw32-make build` — just compile the C engine
- `mingw32-make test` — test with 10 different NSE stocks
- `mingw32-make clean` — remove compiled files

## Project Structure

```
├── Makefile                    # root build system
├── PROJECT_REPORT.md           # detailed report
├── test_nse.py                 # automated test script
│
├── c_engine/                   # C Algorithm Engine
│   ├── include/
│   │   ├── features.h          # struct definitions
│   │   ├── logistic.h          # model weights & functions
│   │   └── metrics.h           # evaluation metrics
│   └── src/
│       ├── main.c              # entry point, pipeline
│       ├── features.c          # moving averages, RSI (parallel)
│       ├── logistic.c          # gradient descent, L2 reg
│       └── metrics.c           # confusion matrix, accuracy
│
├── server/                     # Flask API
│   ├── app.py                  # REST endpoints
│   ├── stock_data.py           # Yahoo Finance fetcher
│   ├── c_bridge.py             # subprocess bridge to C
│   ├── db.py                   # SQLite cache
│   └── requirements.txt
│
└── frontend/                   # React Dashboard
    └── src/
        ├── App.jsx
        ├── index.css
        └── components/
            ├── CandlestickChart.jsx
            ├── IndicatorPanel.jsx
            ├── PredictionCard.jsx
            ├── ConfusionMatrix.jsx
            ├── StockSearch.jsx
            └── Navbar.jsx
```

## What We Implemented in C

- **Structures** — `StockRecord`, `ModelWeights`, `ThreadArgs` for organizing data
- **Pointers** — dynamic arrays, output parameters, struct access
- **Header files** — 3 modular headers with include guards
- **Makefile** — debug and release build targets
- **POSIX Threads** — 4 threads for parallel moving average computation
- **Dynamic memory** — `malloc()`/`free()` for feature vectors
- **Math functions** — sigmoid, gradient descent, z-score normalization

## How We Prevent Overfitting

Since stock prices are noisy, it's easy for a model to memorize patterns instead of learning real signals. We used:

- **80/20 chronological split** — train on first 80% of data, test on last 20% (never peek at the future)
- **L2 regularization** (lambda = 0.01) — penalizes large weights to keep the model simple
- **Z-score normalization** — standardizes features so the model works across stocks with different price ranges
- **Sigmoid clamping** — prevents numerical overflow in the exp() function

## Test Results

We tested on 10 major NSE stocks:

| Stock | Prediction | Accuracy |
|-------|-----------|----------|
| RELIANCE | UP | 54.2% |
| TCS | DOWN | 53.1% |
| INFY | UP | 59.4% |
| HDFCBANK | UP | 57.3% |
| ICICIBANK | UP | 52.1% |
| SBIN | DOWN | 49.0% |
| BHARTIARTL | UP | 55.2% |
| ITC | UP | 42.7% |
| WIPRO | UP | 54.2% |
| TATASTEEL | DOWN | 41.7% |

Average accuracy is around 52%, which is realistic for daily stock prediction — the model isn't memorizing training data.

## Tech Stack

- **C** (GCC, pthreads, math.h) — ML engine
- **Python** (Flask, yfinance, SQLite) — API server
- **React** (Vite, lightweight-charts) — frontend dashboard
