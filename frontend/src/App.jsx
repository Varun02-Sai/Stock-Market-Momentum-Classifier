import React, { useState } from 'react';
import axios from 'axios';
import Navbar from './components/Navbar';
import StockSearch from './components/StockSearch';
import CandlestickChart from './components/CandlestickChart';
import IndicatorPanel from './components/IndicatorPanel';
import PredictionCard from './components/PredictionCard';
import ConfusionMatrix from './components/ConfusionMatrix';

const App = () => {
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [data, setData] = useState(null);

  const handleSearch = async (ticker) => {
    setLoading(true);
    setError(null);
    setData(null);
    try {
      const response = await axios.post('http://localhost:5000/api/predict', { ticker });
      setData(response.data);
    } catch (err) {
      console.error(err);
      setError(err.response?.data?.error || 'Failed to fetch prediction. Make sure Flask server is running.');
    } finally {
      setLoading(false);
    }
  };

  return (
    <div style={{ maxWidth: '1400px', margin: '0 auto', padding: '24px' }}>
      <Navbar />
      
      <StockSearch onSearch={handleSearch} isLoading={loading} />

      {error && (
        <div className="glass-panel animate-fade-in" style={{ 
          padding: '16px 20px', 
          color: '#ef4444', 
          backgroundColor: 'rgba(239, 68, 68, 0.1)', 
          borderLeft: '4px solid #ef4444', 
          marginBottom: '24px',
          fontSize: '0.95rem'
        }}>
          <strong>Error:</strong> {error}
        </div>
      )}

      {loading && (
        <div className="glass-panel" style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center', height: '400px' }}>
          <div className="spinner" style={{ width: '48px', height: '48px', marginBottom: '16px', borderWidth: '4px' }}></div>
          <p style={{ color: 'var(--text-secondary)', fontSize: '1rem' }}>Fetching NSE data & running C engine prediction...</p>
        </div>
      )}

      {data && !loading && (
        <>
          {/* Top Row: Prediction + Stock Info */}
          <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr 1fr', gap: '16px', marginBottom: '20px' }}>
            {/* Prediction Card */}
            <PredictionCard 
              prediction={data.prediction}
              confidence={data.confidence}
              companyName={data.company_name}
              currentPrice={data.current_price}
            />
            
            {/* Stock Info Card */}
            <div className="glass-panel animate-fade-in" style={{ padding: '20px', display: 'flex', flexDirection: 'column', justifyContent: 'center' }}>
              <h2 style={{ margin: '0 0 16px 0', fontSize: '1.1rem', color: 'var(--text-secondary)' }}>Stock Details</h2>
              <div style={{ display: 'flex', flexDirection: 'column', gap: '12px' }}>
                <div style={{ display: 'flex', justifyContent: 'space-between' }}>
                  <span style={{ color: 'var(--text-secondary)', fontSize: '0.9rem' }}>Ticker</span>
                  <span style={{ fontWeight: '600', fontSize: '0.9rem' }}>{data.ticker}</span>
                </div>
                <div style={{ display: 'flex', justifyContent: 'space-between' }}>
                  <span style={{ color: 'var(--text-secondary)', fontSize: '0.9rem' }}>Sector</span>
                  <span style={{ fontWeight: '600', fontSize: '0.9rem' }}>{data.sector}</span>
                </div>
                <div style={{ display: 'flex', justifyContent: 'space-between' }}>
                  <span style={{ color: 'var(--text-secondary)', fontSize: '0.9rem' }}>Market Cap</span>
                  <span style={{ fontWeight: '600', fontSize: '0.9rem' }}>
                    {data.market_cap ? `₹${(data.market_cap / 10000000).toFixed(0)} Cr` : 'N/A'}
                  </span>
                </div>
                <div style={{ display: 'flex', justifyContent: 'space-between' }}>
                  <span style={{ color: 'var(--text-secondary)', fontSize: '0.9rem' }}>Training Days</span>
                  <span style={{ fontWeight: '600', fontSize: '0.9rem' }}>{data.train_size || '—'}</span>
                </div>
                <div style={{ display: 'flex', justifyContent: 'space-between' }}>
                  <span style={{ color: 'var(--text-secondary)', fontSize: '0.9rem' }}>Test Days</span>
                  <span style={{ fontWeight: '600', fontSize: '0.9rem' }}>{data.test_size || '—'}</span>
                </div>
              </div>
            </div>

            {/* Confusion Matrix */}
            <ConfusionMatrix 
              matrix={data.metrics.confusion_matrix}
              metrics={data.metrics}
            />
          </div>

          {/* Charts Row */}
          <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
            <CandlestickChart data={data.ohlcv} features={data.features} />
            <IndicatorPanel features={data.features} />
          </div>
        </>
      )}
      
      {!data && !loading && !error && (
        <div className="glass-panel" style={{ padding: '60px 24px', display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center', color: 'var(--text-secondary)' }}>
          <div style={{ fontSize: '56px', marginBottom: '20px' }}>📈</div>
          <p style={{ fontSize: '1.1rem', marginBottom: '8px' }}>Enter an NSE stock ticker above to run the C-based Momentum Classifier</p>
          <p style={{ fontSize: '0.85rem', opacity: 0.7 }}>
            Try: RELIANCE · TCS · INFY · HDFCBANK · ICICIBANK · SBIN · BHARTIARTL · ITC · WIPRO · TATASTEEL
          </p>
        </div>
      )}
    </div>
  );
};

export default App;
