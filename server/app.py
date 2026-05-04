"""
=============================================================================
File:        app.py
Project:     Stock Market Momentum Classifier
Description: Flask REST API server — the main communication layer.

             This is the central hub that connects all components:
             1. Receives HTTP requests from the React frontend
             2. Fetches NSE stock data via yfinance (stock_data.py)
             3. Pipes data to the C engine via subprocess (c_bridge.py)
             4. Caches results in SQLite (db.py)
             5. Returns JSON responses to the React frontend

API Endpoints:
    GET  /api/health   — Health check (is the server running?)
    POST /api/predict  — Main prediction endpoint
    GET  /api/history  — Retrieve past prediction results

Request/Response Flow:
    React UI → POST /api/predict {"ticker": "RELIANCE"}
    Flask    → yfinance fetches 2 years of OHLCV data for RELIANCE.NS
    Flask    → Pipes CSV to C binary via subprocess
    C Engine → Trains logistic regression, evaluates on test set, predicts
    C Engine → Returns JSON with prediction, metrics, features via stdout
    Flask    → Combines C results with stock info from yfinance
    Flask    → Saves to SQLite cache
    Flask    → Returns JSON response to React
=============================================================================
"""

from flask import Flask, request, jsonify  # Web framework for REST API
from flask_cors import CORS                # Cross-Origin Resource Sharing for React
from stock_data import fetch_stock_data_csv  # Yahoo Finance data fetcher
from c_bridge import run_c_engine           # Subprocess bridge to C binary
from db import init_db, save_prediction, get_history  # SQLite database layer
import yfinance as yf                       # Additional stock info (company name, etc.)

# Create the Flask application instance
app = Flask(__name__)

# Enable CORS so the React frontend (localhost:5173) can make requests
# to this API server (localhost:5000) without browser security blocking
CORS(app)

# Initialize the SQLite database on server startup
# Creates the 'predictions' table if it doesn't exist
init_db()


@app.route('/api/health', methods=['GET'])
def health():
    """
    Health check endpoint.
    Used to verify the API server is running and responsive.

    Returns: {"status": "ok", "message": "..."}
    """
    return jsonify({"status": "ok", "message": "Stock Momentum Classifier API is running (NSE)"})


@app.route('/api/predict', methods=['POST'])
def predict():
    """
    Main prediction endpoint — the core of the application.

    Accepts a JSON body with:
        ticker (str, required): NSE stock symbol (e.g., 'RELIANCE', 'TCS')
        period (str, optional): Data period, default '2y'

    Processing Steps:
        1. Validate input and normalize ticker (strip .NS/.BO if present)
        2. Fetch OHLCV data from Yahoo Finance via yfinance
        3. Get company metadata (name, sector, market cap)
        4. Format OHLCV data as JSON for React's candlestick chart
        5. Run the C engine (pipes CSV → receives JSON prediction)
        6. Combine all data into a unified response
        7. Cache the result in SQLite

    Returns: JSON with prediction, confidence, metrics, ohlcv, features
    """
    # Parse the JSON request body
    data = request.get_json()

    # Validate that 'ticker' field is present
    if not data or 'ticker' not in data:
        return jsonify({"error": "Missing 'ticker' in request body"}), 400

    # Normalize the ticker: uppercase, strip any exchange suffix
    ticker = data['ticker'].upper().replace('.NS', '').replace('.BO', '')
    period = data.get('period', '2y')  # Default: 2 years of historical data

    # Build yfinance-compatible ticker (append .NS for NSE)
    yf_ticker = ticker + '.NS'

    try:
        # ── Step 1: Fetch raw OHLCV data as CSV for the C engine ──
        csv_data = fetch_stock_data_csv(ticker, period)

        # ── Step 2: Fetch the same data as a DataFrame for React charts ──
        stock = yf.Ticker(yf_ticker)
        hist = stock.history(period=period)

        if hist.empty:
            return jsonify({"error": f"No data found for NSE ticker '{ticker}'"}), 404

        # ── Step 3: Get company metadata from yfinance ──
        try:
            info = stock.info
            company_name = info.get('shortName', ticker)    # e.g., "Reliance Industries"
            current_price = info.get('currentPrice', hist['Close'].iloc[-1])
            market_cap = info.get('marketCap', 0)           # Market cap in INR
            sector = info.get('sector', 'N/A')              # e.g., "Energy"
        except Exception:
            # Fallback if company info is not available
            company_name = ticker
            current_price = float(hist['Close'].iloc[-1])
            market_cap = 0
            sector = 'N/A'

        # ── Step 4: Format OHLCV data as JSON for React's candlestick chart ──
        # Each entry needs: time (YYYY-MM-DD), open, high, low, close, volume
        hist_reset = hist.reset_index()
        ohlcv = []
        for _, row in hist_reset.iterrows():
            ohlcv.append({
                "time": row['Date'].strftime('%Y-%m-%d'),
                "open": round(float(row['Open']), 2),
                "high": round(float(row['High']), 2),
                "low": round(float(row['Low']), 2),
                "close": round(float(row['Close']), 2),
                "volume": int(row['Volume'])
            })

        # ── Step 5: Run C engine for ML prediction ──
        # Pipes CSV to C binary via subprocess stdin/stdout
        c_result = run_c_engine(csv_data)

        # ── Step 6: Build unified response combining all data ──
        response = {
            "ticker": ticker,                                          # NSE symbol
            "company_name": company_name,                              # Full company name
            "current_price": round(float(current_price), 2),           # Latest price in ₹
            "market_cap": market_cap,                                  # Market cap in INR
            "sector": sector,                                          # Industry sector
            "prediction": c_result.get("prediction", "N/A"),           # "UP" or "DOWN"
            "confidence": round(c_result.get("confidence", 0) * 100, 2),  # Convert 0-1 to 0-100%
            "metrics": c_result.get("metrics", {}),                    # accuracy, precision, recall, confusion_matrix
            "features": c_result.get("features", []),                  # MA5, MA10, RSI per day
            "train_size": c_result.get("train_size", 0),               # Number of training samples (80%)
            "test_size": c_result.get("test_size", 0),                 # Number of test samples (20%)
            "ohlcv": ohlcv                                             # Raw price data for charts
        }

        # ── Step 7: Cache the result in SQLite ──
        save_prediction(ticker, response)

        return jsonify(response)

    # ── Error handling ──
    except ValueError as e:
        return jsonify({"error": str(e)}), 404           # Stock not found
    except FileNotFoundError as e:
        return jsonify({"error": str(e)}), 500           # C binary not compiled
    except RuntimeError as e:
        return jsonify({"error": str(e)}), 500           # C engine failed
    except Exception as e:
        return jsonify({"error": f"Unexpected error: {str(e)}"}), 500


@app.route('/api/history', methods=['GET'])
def history():
    """
    Retrieve past prediction sessions from the SQLite database.

    Query Parameters:
        limit (int, optional): Maximum number of records to return. Default: 10.

    Returns: JSON array of past predictions with ticker, timestamp,
             prediction, confidence, and metrics.
    """
    try:
        limit = request.args.get('limit', 10, type=int)
        records = get_history(limit)
        return jsonify(records)
    except Exception as e:
        return jsonify({"error": str(e)}), 500


# ── Entry Point ──
# Run the Flask development server on port 5000
if __name__ == '__main__':
    print("Starting Stock Momentum Classifier API (NSE) on http://localhost:5000")
    app.run(debug=True, port=5000)
