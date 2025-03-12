import React from 'react';
import { Route, BrowserRouter as Router, Routes } from 'react-router-dom';
import './App.css';

function App() {
  return (
    <Router>
      <div className="App">
        <header className="App-header">
          <h1>NFT Marketplace</h1>
        </header>
        <main>
          <Routes>
            <Route path="/" element={<div>Welcome to NFT Marketplace</div>} />
          </Routes>
        </main>
      </div>
    </Router>
  );
}

export default App; 