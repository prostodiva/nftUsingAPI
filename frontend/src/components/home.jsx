import '../styles/home.css';

const Home = () => {
    try {
        console.log('home component rendering');
        return (
            <section className="hero-section">
                <div className="hero-content">
                    <h1>Welcome to NFT Marketplace</h1>
                    <p>Discover, collect, and trade unique digital assets</p>
                </div>
            </section>
        );
    } catch (error) {
        console.error('Error in Home component:', error);
        return null;
    }
};

export default Home;