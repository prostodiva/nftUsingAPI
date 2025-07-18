# SOL Airdrop Troubleshooting Guide

## Issue: "Request test sol - failed"

### Root Cause Analysis
The airdrop failure is typically caused by **rate limiting** on Solana testnet/devnet. This is a common issue when:
1. Too many airdrop requests in a short time
2. Network congestion
3. RPC endpoint temporarily unavailable
4. Wallet not properly connected

### ✅ **Solutions (Try in Order)**

#### **Solution 1: Wait and Retry**
```bash
# Wait 2-5 minutes, then try again
# The rate limit usually resets after a few minutes
```

#### **Solution 2: Use Alternative RPC Endpoints**
The program now automatically tries multiple endpoints:
- **Testnet**: `https://api.testnet.solana.com` (primary)
- **Devnet**: `https://api.devnet.solana.com` (fallback)

#### **Solution 3: Manual Airdrop via CLI**
```bash
# Check current configuration
solana config get

# Try testnet airdrop
solana airdrop 0.05 YOUR_WALLET_ADDRESS --url https://api.testnet.solana.com

# If testnet fails, try devnet
solana airdrop 1 YOUR_WALLET_ADDRESS --url https://api.devnet.solana.com

# Alternative RPC endpoints
solana airdrop 0.05 YOUR_WALLET_ADDRESS --url https://solana-testnet.rpc.extrnode.com
solana airdrop 0.05 YOUR_WALLET_ADDRESS --url https://api.testnet.solana.com
```

#### **Solution 4: Check Wallet Connection**
```bash
# Verify wallet is connected
solana address

# Check current balance
solana balance

# Verify Solana CLI installation
solana --version
```

#### **Solution 5: Reset Solana Configuration**
```bash
# Set to testnet
solana config set --url https://api.testnet.solana.com

# Or set to devnet
solana config set --url https://api.devnet.solana.com

# Verify configuration
solana config get
```

### 🔧 **Program Improvements Made**

#### **Enhanced Error Handling**
- Better error messages for rate limiting
- Automatic fallback to devnet when testnet fails
- Detailed troubleshooting suggestions

#### **Improved User Feedback**
- Shows wallet address being used
- Displays current balance before/after airdrop
- Clear success/failure messages

### 📋 **Testing Steps**

#### **Step 1: Verify Prerequisites**
```bash
# 1. Check Solana CLI
solana --version

# 2. Check wallet connection
solana address

# 3. Check current balance
solana balance

# 4. Check configuration
solana config get
```

#### **Step 2: Test Manual Airdrop**
```bash
# Get your wallet address
WALLET_ADDRESS=$(solana address)

# Try testnet airdrop
solana airdrop 0.05 $WALLET_ADDRESS --url https://api.testnet.solana.com

# If that fails, try devnet
solana airdrop 1 $WALLET_ADDRESS --url https://api.devnet.solana.com
```

#### **Step 3: Test Program Airdrop**
1. **Login to your account** (option 2)
2. **Connect Phantom Wallet** (option 14)
3. **Request test SOL** (option 15)
4. **Check the detailed error messages**

### 🚨 **Common Error Messages & Solutions**

#### **"Rate limit reached"**
- **Solution**: Wait 2-5 minutes and try again
- **Alternative**: Use devnet instead of testnet

#### **"airdrop request failed"**
- **Solution**: Network congestion, try again later
- **Alternative**: Use different RPC endpoint

#### **"Wallet not properly connected"**
- **Solution**: Run option 14 (Connect Phantom Wallet) first
- **Check**: Verify `solana address` returns a valid address

#### **"Login first"**
- **Solution**: Login to your account before requesting SOL
- **Check**: Use option 2 to login

### 🔄 **Alternative Testing Approaches**

#### **Option A: Use Devnet Instead**
```bash
# Switch to devnet
solana config set --url https://api.devnet.solana.com

# Test airdrop on devnet
solana airdrop 1 $(solana address)
```

#### **Option B: Use Different RPC Provider**
```bash
# Try alternative RPC endpoints
solana airdrop 0.05 $(solana address) --url https://solana-testnet.rpc.extrnode.com
```

#### **Option C: Create New Wallet**
```bash
# Generate new keypair
solana-keygen new --no-bip39-passphrase -o new-wallet.json

# Set as default
solana config set --keypair new-wallet.json

# Try airdrop with new wallet
solana airdrop 0.05 $(solana address)
```

### 📊 **Rate Limit Information**

#### **Testnet Limits**
- **Rate**: 1 airdrop per 20 seconds
- **Daily**: 2 airdrops per day per wallet
- **Amount**: 0.05 SOL per airdrop

#### **Devnet Limits**
- **Rate**: 1 airdrop per 20 seconds
- **Daily**: 2 airdrops per day per wallet
- **Amount**: 1 SOL per airdrop

### 🎯 **Quick Fix Checklist**

- [ ] **Wait 2-5 minutes** between airdrop attempts
- [ ] **Verify wallet connection** (`solana address`)
- [ ] **Check Solana CLI** (`solana --version`)
- [ ] **Try devnet** if testnet fails
- [ ] **Use alternative RPC** endpoints
- [ ] **Restart program** if needed
- [ ] **Check network status** at https://status.solana.com

### 💡 **Pro Tips**

1. **Best Time**: Airdrops work better during off-peak hours
2. **Multiple Wallets**: Create multiple wallets for testing
3. **Persistence**: Wait between attempts - don't spam requests
4. **Monitoring**: Check Solana network status before testing
5. **Fallback**: Always have devnet as backup option

### 🆘 **Still Having Issues?**

If the problem persists:

1. **Check Solana Network Status**: https://status.solana.com
2. **Try Different RPC Providers**: 
   - https://solana-testnet.rpc.extrnode.com
   - https://api.testnet.solana.com
   - https://api.devnet.solana.com
3. **Wait Longer**: Sometimes rate limits can be 10-15 minutes
4. **Use Different Wallet**: Generate a new keypair for testing

### ✅ **Success Indicators**

When airdrop works, you should see:
```
Airdrop successful! 0.05 SOL added to your wallet
New balance: 0.05 SOL
Remaining airdrops today: 1
```

**Remember**: Rate limiting is normal and temporary. The improved error handling will now guide you through the process! 🚀 