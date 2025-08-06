#!/bin/bash

export PYTHONPATH=/home/azureuser/CardiacAI/backend/app/src/
# export PYTHONPATH=/home/robonish/victory/CardiacAI/backend/app/src/
# poetry run which uvicorn) app.main:app --host 0.0.0.0 --port 8000 --log-level debug

# Create systemd service file
echo "Creating service file for app..."
sudo tee /etc/systemd/system/cardiac.service >/dev/null <<EOF
[Unit]
Description=cardiac service
After=network.target

[Service]
Type=simple
WorkingDirectory=${HOME}/CardiacAI/backend
StandardOutput=append:/var/log/cardiac.log
StandardError=append:/var/log/cardiac.err.log
Environment="PATH=$(poetry env info --path)/bin"
Environment="PYTHONPATH=${HOME}/CardiacAI/backend/app/src"

ExecStart=$(poetry run which gunicorn) -w 4 -k uvicorn.workers.UvicornWorker \
  app.main:app --bind 0.0.0.0:8000 --log-level debug

Restart=always
RestartSec=3
KillMode=process
TimeoutStopSec=30

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
echo "Creating Nginx config file with HTTPS..."
sudo tee /etc/nginx/sites-available/cardiac >/dev/null <<'EOF'
# /etc/nginx/sites-available/cardiac

# Redirect all HTTP to HTTPS
server {
    listen 80;
    server_name api.cardiacai.tech;
    return 301 https://$host$request_uri;
}

# Main HTTPS server
server {
    listen 443 ssl;
    server_name api.cardiacai.tech;

    # SSL certificates from Let's Encrypt
    ssl_certificate /etc/letsencrypt/live/api.cardiacai.tech/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/api.cardiacai.tech/privkey.pem;
    ssl_trusted_certificate /etc/letsencrypt/live/api.cardiacai.tech/chain.pem;

    # SSL settings for better security
    ssl_protocols TLSv1.2 TLSv1.3;
    ssl_prefer_server_ciphers on;
    ssl_ciphers 'ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384';
    ssl_session_timeout 1d;
    ssl_session_cache shared:SSL:10m;
    ssl_session_tickets off;

    # Optional: HSTS (forces HTTPS in browsers)
    add_header Strict-Transport-Security "max-age=31536000" always;

    location / {
        proxy_pass http://127.0.0.1:8000;

        # Standard headers for HTTP proxying
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;

        # WebSocket support
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";

        # Optional: Adjust read timeout for long-lived connections
        proxy_read_timeout 86400s;
    }
}
EOF

# Ensure the symlink exists and is updated
sudo ln -sf /etc/nginx/sites-available/cardiac /etc/nginx/sites-enabled/cardiac

# Test nginx configuration
sudo nginx -t

# Reload nginx only if test is successful
if [ $? -eq 0 ]; then
    sudo systemctl reload nginx
    echo "Nginx reloaded successfully with new config."
else
    echo "Nginx config test failed. Check the error above."
fi

echo "Setup complete. Remember to start your database before running the app."

