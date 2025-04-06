import React from 'react';
import Home from '../components/home.jsx'
import Header from "../components/header.jsx";

const LandingPage = () => {
    return (
        <div className="min-h-screen flex flex-col">
            <Header />
            <main className="flex-grow">
                <div>
                    <Home />
                </div>
            </main>
        </div>
    );
};

export default LandingPage; 