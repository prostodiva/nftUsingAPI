FROM node:18-alpine as builder

WORKDIR /app

# Copy package files
COPY package*.json ./

# Install dependencies with specific versions
RUN npm cache clean --force && \
    npm install vite@5.1.4 && \
    npm install @vitejs/plugin-react@4.2.1 && \
    npm install esbuild@0.21.5 && \
    npm install

# Copy the rest of the application
COPY . .

# Build the application
RUN npm run build

# Use Nginx to serve the application
FROM nginx:alpine

# Copy the built files to Nginx serve directory
COPY --from=builder /app/dist /usr/share/nginx/html

# Copy Nginx configuration
RUN echo 'server { \
    listen 3000; \
    location / { \
        root /usr/share/nginx/html; \
        try_files $uri $uri/ /index.html; \
    } \
}' > /etc/nginx/conf.d/default.conf

EXPOSE 3000

CMD ["nginx", "-g", "daemon off;"]