import React from 'react';
import { TrendingUp, TrendingDown } from 'lucide-react';

const PredictionCard = ({ prediction, confidence, companyName, currentPrice }) => {
  const isUp = prediction === 'UP';
  const color = isUp ? 'var(--up-color)' : 'var(--down-color)';
  const glowClass = isUp ? 'glow-up' : 'glow-down';
  const Icon = isUp ? TrendingUp : TrendingDown;

  return (
    <div className={`glass-panel ${glowClass} animate-fade-in`} style={{ padding: '20px', display: 'flex', flexDirection: 'column' }}>
      <h2 style={{ margin: '0 0 8px 0', fontSize: '1.1rem', color: 'var(--text-secondary)' }}>
        Next Day Prediction
      </h2>
      <div style={{ fontSize: '1.1rem', fontWeight: 'bold', marginBottom: '4px' }}>{companyName}</div>
      <div style={{ fontSize: '0.95rem', color: 'var(--text-secondary)', marginBottom: '16px' }}>Current: ₹{currentPrice}</div>

      <div style={{ flex: 1, display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'center' }}>
        <Icon size={56} color={color} style={{ marginBottom: '8px' }} />
        <div style={{ fontSize: '2.8rem', fontWeight: '800', color: color, lineHeight: 1 }}>
          {prediction}
        </div>
      </div>
      
      <div style={{ marginTop: '16px' }}>
        <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '6px' }}>
          <span style={{ color: 'var(--text-secondary)', fontSize: '0.85rem' }}>Confidence Level</span>
          <span style={{ fontWeight: 'bold', fontSize: '0.85rem' }}>{confidence}%</span>
        </div>
        <div style={{ height: '8px', background: 'rgba(255,255,255,0.1)', borderRadius: '4px', overflow: 'hidden' }}>
          <div style={{ 
            height: '100%', 
            width: `${confidence}%`, 
            background: `linear-gradient(90deg, ${color}88, ${color})`,
            transition: 'width 1s cubic-bezier(0.4, 0, 0.2, 1)',
            borderRadius: '4px'
          }} />
        </div>
      </div>
    </div>
  );
};

export default PredictionCard;
