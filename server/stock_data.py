"""
=============================================================================
File:        stock_data.py
Project:     Stock Market Momentum Classifier
Description: Data fetching layer using Yahoo Finance (yfinance library).

             This module fetches historical OHLCV (Open, High, Low, Close, Volume)
             stock data for NSE (National Stock Exchange of India) listed stocks.

             NSE stocks require a '.NS' suffix in Yahoo Finance's API. This module
             automatically appends '.NS' to the user-provided ticker symbol.

How it works:
    1. User provides a plain NSE ticker (e.g., 'RELIANCE')
    2. This module converts it to 'RELIANCE.NS' for yfinance
    3. yfinance fetches data from Yahoo Finance servers
    4. The data is returned as a CSV string for the C engine to parse

Why yfinance?
    - Free, no API key required
    - Reliable data source (Yahoo Finance)
    - Supports all major Indian exchanges (NSE via .NS, BSE via .BO)
    - Returns pandas DataFrame with clean OHLCV data
=============================================================================
"""

import yfinance as yf  # Yahoo Finance API wrapper
import pandas as pd    # Data manipulation library


def fetch_stock_data_csv(ticker: str, period: str = '2y') -> str:
    """
    Fetches historical OHLCV data from Yahoo Finance for an NSE stock
    and returns it as a CSV string suitable for piping to the C engine.

    The function automatically appends '.NS' (NSE suffix) to the ticker
    if not already present. This means users can simply type 'RELIANCE'
    instead of 'RELIANCE.NS'.

    Parameters:
        ticker (str): NSE stock symbol (e.g., 'RELIANCE', 'TCS', 'INFY')
        period (str): Historical data period. Default '2y' means 2 years.
                      Other options: '1y', '6mo', '3mo', '1mo', '5d', etc.

    Returns:
        str: CSV string with columns: Date, Open, High, Low, Close, Volume
             Each row represents one trading day.

    Raises:
        ValueError: If no data is found for the given ticker
                    (stock might be delisted or ticker might be incorrect)

    Example CSV output:
        Date,Open,High,Low,Close,Volume
        2024-05-02,2850.0,2875.0,2840.0,2860.0,12345678
        2024-05-03,2860.0,2890.0,2855.0,2880.0,9876543
        ...
    """
    # Auto-append .NS for NSE stocks if not already present
    # Also handles .BO (BSE) tickers without double-suffixing
    yf_ticker = ticker.upper()
    if not yf_ticker.endswith('.NS') and not yf_ticker.endswith('.BO'):
        yf_ticker = yf_ticker + '.NS'  # National Stock Exchange suffix

    # Create a yfinance Ticker object and fetch historical data
    stock = yf.Ticker(yf_ticker)
    df = stock.history(period=period)

    # Check if any data was returned
    if df.empty:
        raise ValueError(f"No data found for NSE ticker {ticker} (tried {yf_ticker})")

    # Reset the index so 'Date' becomes a regular column instead of the index
    df = df.reset_index()

    # Format Date to YYYY-MM-DD string (the C engine expects this format)
    df['Date'] = df['Date'].dt.strftime('%Y-%m-%d')

    # Select only the columns the C engine needs (in the exact expected order)
    # The C engine parses CSV with sscanf(): Date,Open,High,Low,Close,Volume
    df = df[['Date', 'Open', 'High', 'Low', 'Close', 'Volume']]

    # Convert DataFrame to CSV string (without the pandas row index)
    return df.to_csv(index=False)
