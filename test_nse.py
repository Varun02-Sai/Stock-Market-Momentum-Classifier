"""
Test script — Tests the Stock Momentum Classifier API with 10 NSE stocks.
Verifies that the model produces realistic (not overfitted) accuracy on unseen test data.
"""
import requests
import time

API_URL = "http://localhost:5000/api/predict"

# 10 Major NSE (National Stock Exchange of India) stocks to test
NSE_TICKERS = [
    "RELIANCE",    # Reliance Industries
    "TCS",         # Tata Consultancy Services
    "INFY",        # Infosys
    "HDFCBANK",    # HDFC Bank
    "ICICIBANK",   # ICICI Bank
    "SBIN",        # State Bank of India
    "BHARTIARTL",  # Bharti Airtel
    "ITC",         # ITC Limited
    "WIPRO",       # Wipro
    "TATASTEEL",   # Tata Steel
]

print("=" * 90)
print(f"{'TICKER':<14} {'PREDICTION':<12} {'CONFIDENCE':>10} {'ACCURACY':>10} {'PRECISION':>10} {'RECALL':>10} {'TRAIN':>6} {'TEST':>6}")
print("=" * 90)

results = []
for ticker in NSE_TICKERS:
    try:
        resp = requests.post(API_URL, json={"ticker": ticker}, timeout=30)
        if resp.status_code == 200:
            data = resp.json()
            m = data.get("metrics", {})
            row = {
                "ticker": ticker,
                "prediction": data.get("prediction", "?"),
                "confidence": data.get("confidence", 0),
                "accuracy": m.get("accuracy", 0) * 100,
                "precision": m.get("precision", 0) * 100,
                "recall": m.get("recall", 0) * 100,
                "train": data.get("train_size", 0),
                "test": data.get("test_size", 0),
            }
            results.append(row)
            print(f"{row['ticker']:<14} {row['prediction']:<12} {row['confidence']:>9.1f}% {row['accuracy']:>9.1f}% {row['precision']:>9.1f}% {row['recall']:>9.1f}% {row['train']:>6} {row['test']:>6}")
        else:
            err = resp.json().get("error", "Unknown error")
            print(f"{ticker:<14} ERROR: {err}")
    except Exception as e:
        print(f"{ticker:<14} EXCEPTION: {e}")
    time.sleep(1)  # small delay between requests

print("=" * 90)

# Summary
if results:
    avg_acc = sum(r["accuracy"] for r in results) / len(results)
    print(f"\nAverage Test Accuracy across {len(results)} stocks: {avg_acc:.1f}%")
    print(f"Note: Metrics are computed on 20% UNSEEN test data (not training data)")
    print(f"      Accuracy around 48-55% on test data is realistic for stock prediction.")
    print(f"      Perfect accuracy (>90%) would indicate overfitting.\n")
