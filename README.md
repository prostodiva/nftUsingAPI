# NFT Marketplace API

A decentralized NFT marketplace built on the Solana blockchain, featuring a C++ backend and modern React frontend.

## Project Overview

This project consists of two main components:
- Backend: A C++ server using Crow framework for handling NFT marketplace operations
- Frontend: A React-based web application with Vite and Tailwind CSS

## Features

- User authentication and account management
- NFT listing and trading
- Solana blockchain integration
- Modern, responsive UI
- Docker containerization for easy deployment

## Prerequisites

- Docker and Docker Compose
- Node.js (for local development)
- C++ compiler (for local backend development)
- Solana CLI tools


### Using Docker (Recommended)

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd nftMarketplaceAPI
   ```

2. Build and start the containers:
   ```bash
   docker-compose up --build
   ```

3. Access the application:
   - Frontend: http://localhost:3001
   - Backend API: http://localhost:3000

### Local Development

#### Backend Setup

1. Navigate to the backend directory:
   ```bash
   cd backend
   ```

2. Build the project:
   ```bash
   make
   ```

3. Run the server:
   ```bash
   ./main
   ```

#### Frontend Setup

1. Navigate to the frontend directory:
   ```bash
   cd frontend
   ```

2. Install dependencies:
   ```bash
   npm install
   ```

3. Start the development server:
   ```bash
   npm run dev
   ```

## Environment Variables

### Backend
- `NODE_ENV`: Development/Production environment
- `SOLANA_RPC_URL`: Solana RPC endpoint
- `DATABASE_URL`: Database connection string

### Frontend
- `VITE_API_URL`: Backend API URL (default: http://localhost:3000)

## Acknowledgments

- Solana blockchain
- Crow C++ web framework
- React and Vite
- Tailwind CSS
