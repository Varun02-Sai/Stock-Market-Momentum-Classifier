/**
 * =============================================================================
 * File:        CandlestickChart.jsx
 * Project:     Stock Market Momentum Classifier
 * Description: Interactive candlestick chart component using TradingView's
 *              lightweight-charts library (v5). Renders OHLCV price action
 *              with MA(5) and MA(10) moving average overlays.
 *
 * Features:
 *   - Japanese candlestick chart (green = up day, red = down day)
 *   - 5-day Moving Average line (cyan) — computed by C engine
 *   - 10-day Moving Average line (magenta) — computed by C engine
 *   - Responsive: auto-resizes with window
 *   - Crosshair for precise price reading
 *
 * Data Source: C engine features array + yfinance OHLCV data
 * Chart Library: lightweight-charts v5 (TradingView open-source)
 * =============================================================================
 */

import React, { useEffect, useRef } from 'react';
import { createChart, CrosshairMode, CandlestickSeries, LineSeries } from 'lightweight-charts';

/**
 * CandlestickChart Component
 *
 * Props:
 *   data (array):     Array of OHLCV objects from yfinance
 *                     [{time: "2024-01-01", open, high, low, close, volume}, ...]
 *   features (array): Array of feature objects from C engine
 *                     [{date: "2024-01-01", ma5, ma10, rsi}, ...]
 */
const CandlestickChart = ({ data, features }) => {
  // Refs to store DOM container and chart instance
  const chartContainerRef = useRef();
  const chartRef = useRef();

  useEffect(() => {
    // Guard: don't render if container or data is missing
    if (!chartContainerRef.current || !data || data.length === 0) return;

    // ── Create the chart instance ──
    const chart = createChart(chartContainerRef.current, {
      layout: {
        background: { type: 'solid', color: 'transparent' },  // Let CSS handle background
        textColor: '#94a3b8',                                  // Muted text for axes
      },
      grid: {
        vertLines: { color: 'rgba(255, 255, 255, 0.05)' },    // Very subtle grid
        horzLines: { color: 'rgba(255, 255, 255, 0.05)' },
      },
      crosshair: {
        mode: CrosshairMode.Normal,  // Show crosshair on hover
      },
      rightPriceScale: {
        borderColor: 'rgba(255, 255, 255, 0.1)',
      },
      timeScale: {
        borderColor: 'rgba(255, 255, 255, 0.1)',
        timeVisible: true,  // Show time on x-axis
      },
      width: chartContainerRef.current.clientWidth,
      height: 400,
    });

    chartRef.current = chart;

    // ── Add Candlestick Series ──
    // Each candle shows Open, High, Low, Close for one trading day
    // Green (up) candle: Close > Open
    // Red (down) candle: Close < Open
    const candlestickSeries = chart.addSeries(CandlestickSeries, {
      upColor: '#10b981',      // Green for up days
      downColor: '#ef4444',    // Red for down days
      borderDownColor: '#ef4444',
      borderUpColor: '#10b981',
      wickDownColor: '#ef4444',
      wickUpColor: '#10b981',
    });

    candlestickSeries.setData(data);

    // ── Add Moving Average Overlays ──
    // These overlay lines are computed by the C engine in features.c
    if (features && features.length > 0) {
      // MA5 (5-day Moving Average) — cyan line
      // Tracks short-term price momentum
      const ma5Data = features
        .filter(f => f.ma5 > 0)  // Filter out early days where MA can't be computed
        .map(f => ({ time: f.date, value: f.ma5 }));

      if (ma5Data.length > 0) {
        const ma5Series = chart.addSeries(LineSeries, {
          color: '#06b6d4',   // Cyan
          lineWidth: 2,
          title: 'MA(5)',
        });
        ma5Series.setData(ma5Data);
      }

      // MA10 (10-day Moving Average) — magenta line
      // Tracks medium-term price momentum
      const ma10Data = features
        .filter(f => f.ma10 > 0)
        .map(f => ({ time: f.date, value: f.ma10 }));

      if (ma10Data.length > 0) {
        const ma10Series = chart.addSeries(LineSeries, {
          color: '#d946ef',   // Magenta
          lineWidth: 2,
          title: 'MA(10)',
        });
        ma10Series.setData(ma10Data);
      }
    }

    // Auto-fit all data into the visible area
    chart.timeScale().fitContent();

    // ── Responsive Resize Handler ──
    const handleResize = () => {
      if (chartContainerRef.current && chartRef.current) {
        chartRef.current.applyOptions({ width: chartContainerRef.current.clientWidth });
      }
    };

    window.addEventListener('resize', handleResize);

    // ── Cleanup on unmount ──
    return () => {
      window.removeEventListener('resize', handleResize);
      chart.remove();
    };
  }, [data, features]);  // Re-render when data or features change

  return (
    <div className="glass-panel animate-fade-in" style={{ padding: '24px', height: '100%', display: 'flex', flexDirection: 'column' }}>
      <h2 style={{ margin: '0 0 16px 0', fontSize: '1.25rem', color: 'var(--text-secondary)' }}>Price Action & Moving Averages</h2>
      <div ref={chartContainerRef} style={{ flex: 1, minHeight: '400px' }} />
    </div>
  );
};

export default CandlestickChart;
