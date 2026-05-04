import React, { useState } from 'react';
import { Search } from 'lucide-react';

const StockSearch = ({ onSearch, isLoading }) => {
  const [ticker, setTicker] = useState('RELIANCE');

  const handleSubmit = (e) => {
    e.preventDefault();
    if (ticker.trim()) {
      onSearch(ticker.trim().toUpperCase());
    }
  };

  return (
    <div className="glass-panel" style={{ padding: '24px', marginBottom: '24px' }}>
      <form onSubmit={handleSubmit} style={{ display: 'flex', gap: '16px' }}>
        <div style={{ position: 'relative', flex: 1 }}>
          <div style={{ position: 'absolute', left: '16px', top: '50%', transform: 'translateY(-50%)', color: 'var(--text-secondary)' }}>
            <Search size={20} />
          </div>
          <input
            type="text"
            value={ticker}
            onChange={(e) => setTicker(e.target.value)}
            placeholder="Enter NSE Ticker (e.g. RELIANCE, TCS, INFY, HDFCBANK)"
            style={{
              width: '100%',
              padding: '16px 16px 16px 48px',
              borderRadius: '8px',
              border: '1px solid var(--border-light)',
              background: 'rgba(0,0,0,0.2)',
              color: 'var(--text-primary)',
              fontSize: '1rem',
              outline: 'none',
              transition: 'border-color 0.2s'
            }}
            onFocus={(e) => e.target.style.borderColor = 'var(--accent-cyan)'}
            onBlur={(e) => e.target.style.borderColor = 'var(--border-light)'}
          />
        </div>
        <button 
          type="submit" 
          disabled={isLoading}
          style={{
            padding: '0 32px',
            borderRadius: '8px',
            border: 'none',
            background: 'linear-gradient(135deg, var(--accent-cyan), #0284c7)',
            color: 'white',
            fontWeight: '600',
            fontSize: '1rem',
            cursor: isLoading ? 'not-allowed' : 'pointer',
            opacity: isLoading ? 0.7 : 1,
            transition: 'opacity 0.2s',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            minWidth: '140px'
          }}
        >
          {isLoading ? <div className="spinner"></div> : 'Analyze'}
        </button>
      </form>
    </div>
  );
};

export default StockSearch;
