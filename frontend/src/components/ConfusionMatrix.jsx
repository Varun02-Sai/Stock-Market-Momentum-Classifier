import React from 'react';

const ConfusionMatrix = ({ matrix, metrics }) => {
  if (!matrix) return null;
  
  const { tp, fp, fn, tn } = matrix;
  const total = tp + fp + fn + tn;
  
  const getCellColor = (value, max) => {
    const intensity = Math.min(value / (max || 1), 1);
    return `rgba(6, 182, 212, ${intensity * 0.8 + 0.1})`;
  };
  
  const maxVal = Math.max(tp, fp, fn, tn);

  return (
    <div className="glass-panel animate-fade-in" style={{ padding: '20px' }}>
      <h2 style={{ margin: '0 0 16px 0', fontSize: '1.1rem', color: 'var(--text-secondary)' }}>
        Model Performance (Test Set)
      </h2>
      
      {/* Confusion Matrix Grid */}
      <div style={{ marginBottom: '20px' }}>
        <div style={{ display: 'grid', gridTemplateColumns: 'auto 1fr 1fr', gap: '6px', textAlign: 'center', maxWidth: '280px', margin: '0 auto' }}>
          <div></div>
          <div style={{ fontSize: '0.7rem', color: 'var(--text-secondary)', padding: '4px' }}>Pred UP</div>
          <div style={{ fontSize: '0.7rem', color: 'var(--text-secondary)', padding: '4px' }}>Pred DOWN</div>
          
          <div style={{ fontSize: '0.7rem', color: 'var(--text-secondary)', display: 'flex', alignItems: 'center', justifyContent: 'flex-end', paddingRight: '6px' }}>
            Actual UP
          </div>
          <div style={{ 
            background: getCellColor(tp, maxVal), 
            padding: '10px 8px', 
            borderRadius: '6px',
            border: '1px solid rgba(255,255,255,0.1)'
          }}>
            <div style={{ fontSize: '1.2rem', fontWeight: 'bold' }}>{tp}</div>
            <div style={{ fontSize: '0.65rem', opacity: 0.7 }}>TP</div>
          </div>
          <div style={{ 
            background: getCellColor(fn, maxVal), 
            padding: '10px 8px', 
            borderRadius: '6px',
            border: '1px solid rgba(255,255,255,0.1)'
          }}>
            <div style={{ fontSize: '1.2rem', fontWeight: 'bold' }}>{fn}</div>
            <div style={{ fontSize: '0.65rem', opacity: 0.7 }}>FN</div>
          </div>
          
          <div style={{ fontSize: '0.7rem', color: 'var(--text-secondary)', display: 'flex', alignItems: 'center', justifyContent: 'flex-end', paddingRight: '6px' }}>
            Actual DOWN
          </div>
          <div style={{ 
            background: getCellColor(fp, maxVal), 
            padding: '10px 8px', 
            borderRadius: '6px',
            border: '1px solid rgba(255,255,255,0.1)'
          }}>
            <div style={{ fontSize: '1.2rem', fontWeight: 'bold' }}>{fp}</div>
            <div style={{ fontSize: '0.65rem', opacity: 0.7 }}>FP</div>
          </div>
          <div style={{ 
            background: getCellColor(tn, maxVal), 
            padding: '10px 8px', 
            borderRadius: '6px',
            border: '1px solid rgba(255,255,255,0.1)'
          }}>
            <div style={{ fontSize: '1.2rem', fontWeight: 'bold' }}>{tn}</div>
            <div style={{ fontSize: '0.65rem', opacity: 0.7 }}>TN</div>
          </div>
        </div>
      </div>

      {/* Metrics Bars */}
      <div style={{ display: 'flex', flexDirection: 'column', gap: '10px' }}>
        <div>
          <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '4px' }}>
            <span style={{ color: 'var(--text-secondary)', fontSize: '0.85rem' }}>Accuracy</span>
            <span style={{ fontWeight: 'bold', fontSize: '0.85rem' }}>{(metrics.accuracy * 100).toFixed(1)}%</span>
          </div>
          <div style={{ height: '6px', background: 'rgba(255,255,255,0.1)', borderRadius: '3px' }}>
            <div style={{ height: '100%', width: `${metrics.accuracy * 100}%`, background: 'var(--accent-cyan)', borderRadius: '3px', transition: 'width 1s ease' }} />
          </div>
        </div>
        
        <div>
          <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '4px' }}>
            <span style={{ color: 'var(--text-secondary)', fontSize: '0.85rem' }}>Precision</span>
            <span style={{ fontWeight: 'bold', fontSize: '0.85rem' }}>{(metrics.precision * 100).toFixed(1)}%</span>
          </div>
          <div style={{ height: '6px', background: 'rgba(255,255,255,0.1)', borderRadius: '3px' }}>
            <div style={{ height: '100%', width: `${metrics.precision * 100}%`, background: 'var(--accent-magenta)', borderRadius: '3px', transition: 'width 1s ease' }} />
          </div>
        </div>
        
        <div>
          <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '4px' }}>
            <span style={{ color: 'var(--text-secondary)', fontSize: '0.85rem' }}>Recall</span>
            <span style={{ fontWeight: 'bold', fontSize: '0.85rem' }}>{(metrics.recall * 100).toFixed(1)}%</span>
          </div>
          <div style={{ height: '6px', background: 'rgba(255,255,255,0.1)', borderRadius: '3px' }}>
            <div style={{ height: '100%', width: `${metrics.recall * 100}%`, background: '#f59e0b', borderRadius: '3px', transition: 'width 1s ease' }} />
          </div>
        </div>
      </div>
      
      <div style={{ fontSize: '0.75rem', color: 'var(--text-secondary)', textAlign: 'center', marginTop: '12px', borderTop: '1px solid var(--border-light)', paddingTop: '10px' }}>
        Evaluated on {total} unseen test-set trading days (20% split)
      </div>
    </div>
  );
};

export default ConfusionMatrix;
