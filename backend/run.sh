#!/bin/bash

export PYTHONPATH=/home/azureuser/CardiacAI/backend/app/src/
# poetry run which uvicorn) app.main:app --host 0.0.0.0 --port 8000 --log-level debug

# Create systemd service file
echo "Creating service file for app..."
sudo tee /etc/systemd/system/cardiac.service >/dev/null <<EOF
[Unit]
Description=cardiac service
After=network.target

[Service]
Type=simple
WorkingDirectory=$(echo $HOME)/CardiacAI/backend
StandardOutput=append:/var/log/cardiac.log
StandardError=append:/var/log/cardiac.err.log
Environment="PATH=$(poetry env info --path)/bin"
Environment="PYTHONPATH=$(echo $HOME)/CardiacAI/backend/app/src"
ExecStart=$(poetry run which uvicorn) app.main:app --host 0.0.0.0 --port 8000 --workers 4 --log-level debug
Restart=always
RestartSec=3

[Install]
WantedBy=multi-user.target
EOF
sudo systemctl daemon-reload
sudo systemctl enable cardiac.service
sudo systemctl start cardiac.service
echo "Service created and started successfully."

# Install Nginx
echo "Installing Nginx..."
sudo apt install -y nginx
echo "Nginx installed successfully."

# Create Nginx config file
echo "Creating Nginx config file..."
sudo tee /etc/nginx/sites-available/cardiac >/dev/null <<EOF
# /etc/nginx/sites-available/fastapi_proxy

server {
    listen 80; # Nginx listens on port 80 for incoming HTTP requests
    server_name 172.187.168.3; # Replace with your server's public IP address

    location / {
        proxy_pass http://127.0.0.1:8000; # Forward requests to your FastAPI server

        # Standard headers for HTTP proxying
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;

        # WebSocket specific headers
        proxy_http_version 1.1; # Crucial for WebSocket upgrade
        proxy_set_header Upgrade $http_upgrade; # Passes the Upgrade header from the client
        proxy_set_header Connection "upgrade"; # Specifies the connection upgrade

        # Optional: Adjust read timeout for long-lived WebSocket connections
        proxy_read_timeout 86400s; # Example: 24 hours, adjust as needed
    }
}
EOF
sudo ln -s /etc/nginx/sites-available/cardiac /etc/nginx/sites-enabled
sudo nginx -t
sudo systemctl reload nginx
echo "Nginx configured successfully."
echo "Setup complete. Remember to start your database before running the app."

