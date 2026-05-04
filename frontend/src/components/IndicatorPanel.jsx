/**
 * =============================================================================
 * File:        IndicatorPanel.jsx
 * Project:     Stock Market Momentum Classifier
 * Description: RSI (Relative Strength Index) chart component using TradingView's
 *              lightweight-charts library (v5). Displays the RSI(14) oscillator
 *              with overbought (70) and oversold (30) reference lines.
 *
 * RSI Interpretation:
 *   RSI > 70 — Stock is "overbought" (may go DOWN soon)
 *   RSI < 30 — Stock is "oversold" (may go UP soon)
 *   RSI ~ 50 — Neutral momentum
 *
 * The RSI is computed in the C engine (features.c) using Wilder's smoothing
 * method over a 14-day window, then passed to this component via the
 * features array in the API response.
 * =============================================================================
 */

import React, { useEffect, useRef } from 'react';
import { createChart, LineSeries } from 'lightweight-charts';

/**
 * IndicatorPanel Component
 *
 * Props:
 *   features (array): Array of feature objects from C engine
 *                     [{date: "2024-01-01", ma5, ma10, rsi}, ...]
 *                     RSI values range from 0 to 100
 */
const IndicatorPanel = ({ features }) => {
  const chartContainerRef = useRef();
  const chartRef = useRef();

  useEffect(() => {
    if (!chartContainerRef.current || !features || features.length === 0) return;

    // ── Prepare RSI Data ──
    // Filter out initial days where RSI couldn't be calculated (value = 0)
    // and ensure RSI is within valid bounds (0-100)
    const rsiData = features
      .filter(f => f.rsi > 0 && f.rsi <= 100)
      .map(f => ({ time: f.date, value: f.rsi }));

    if (rsiData.length === 0) return;

    // ── Create the RSI chart ──
    const chart = createChart(chartContainerRef.current, {
      layout: {
        background: { type: 'solid', color: 'transparent' },
        textColor: '#94a3b8',
      },
      grid: {
        vertLines: { color: 'rgba(255, 255, 255, 0.05)' },
        horzLines: { color: 'rgba(255, 255, 255, 0.05)' },
      },
      rightPriceScale: {
        borderColor: 'rgba(255, 255, 255, 0.1)',
      },
      timeScale: {
        borderColor: 'rgba(255, 255, 255, 0.1)',
        timeVisible: true,
      },
      width: chartContainerRef.current.clientWidth,
      height: 250,  // Shorter than candlestick chart
    });

    chartRef.current = chart;

    // ── Add RSI Line Series ──
    // Orange/amber color to distinguish from price chart
    const rsiSeries = chart.addSeries(LineSeries, {
      color: '#f59e0b',   // Amber/orange
      lineWidth: 2,
      title: 'RSI(14)',
    });

    rsiSeries.setData(rsiData);

    // ── Add Overbought/Oversold Reference Lines ──
    // These horizontal dashed lines help traders identify extremes

    // Overbought line at RSI = 70 (red, dashed)
    rsiSeries.createPriceLine({
      price: 70,
      color: '#ef4444',      // Red
      lineWidth: 1,
      lineStyle: 2,          // Dashed line
      axisLabelVisible: true,
      title: 'Overbought',
    });

    // Oversold line at RSI = 30 (green, dashed)
    rsiSeries.createPriceLine({
      price: 30,
      color: '#10b981',      // Green
      lineWidth: 1,
      lineStyle: 2,          // Dashed line
      axisLabelVisible: true,
      title: 'Oversold',
    });

    // Auto-fit timeline
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
  }, [features]);

  return (
    <div className="glass-panel animate-fade-in" style={{ padding: '24px', height: '100%', display: 'flex', flexDirection: 'column' }}>
      <h2 style={{ margin: '0 0 16px 0', fontSize: '1.25rem', color: 'var(--text-secondary)' }}>Relative Strength Index (RSI)</h2>
      <div ref={chartContainerRef} style={{ flex: 1, minHeight: '250px' }} />
    </div>
  );
};

export default IndicatorPanel;
