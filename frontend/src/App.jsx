import React from 'react';
import {Route, BrowserRouter as Router, Routes, Navigate} from 'react-router-dom';
import LandingPage from './pages/LandingPage';

const AppRoutes =() => {
    console.log('App rendering');
  return (
    <Routes>
        <Route path="/" element={<Navigate to="/home" replace />} />
        <Route path="/home" element={<LandingPage />} />
    </Routes>
  );
};

const App = () => {
    return (
        <>
            <AppRoutes />
        </>
    )
}

export default App; 