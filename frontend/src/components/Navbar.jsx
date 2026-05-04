import React from 'react';
import { Activity } from 'lucide-react';

const Navbar = () => {
  return (
    <nav className="glass-panel" style={{ padding: '16px 32px', marginBottom: '24px', display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
      <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
        <div style={{ 
          background: 'linear-gradient(135deg, var(--accent-cyan), var(--accent-magenta))',
          padding: '8px',
          borderRadius: '8px',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center'
        }}>
          <Activity size={24} color="#fff" />
        </div>
        <h1 style={{ margin: 0, fontSize: '1.5rem', fontWeight: 700 }} className="text-gradient">
          Momentum Classifier
        </h1>
      </div>
      
      <div style={{ color: 'var(--text-secondary)', fontSize: '0.875rem' }}>
        Powered by C Logistic Regression
      </div>
    </nav>
  );
};

export default Navbar;
