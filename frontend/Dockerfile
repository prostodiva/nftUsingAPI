FROM node:18-alpine

WORKDIR /app

# Copy package files
COPY package*.json ./

# Install dependencies
RUN npm install

# Copy the rest of the application
COPY . .

# Build the application (if using a framework like React/Vue)
RUN npm run build

EXPOSE 3000

CMD ["npm", "start"]