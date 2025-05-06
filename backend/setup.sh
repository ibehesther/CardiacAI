#!/bin/bash

# Run with sudo to install packages

# Alias python to python3
echo "Setting up python alias..."
echo "alias python=python3" >>~/.bashrc
source ~/.bashrc
echo "Python alias set up successfully."

# Install MongoDB
echo "Installing MongoDB..."
sudo apt-get install -y gnupg curl
curl -fsSL https://www.mongodb.org/static/pgp/server-8.0.asc | sudo gpg -o /usr/share/keyrings/mongodb-server-8.0.gpg --dearmor
echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-8.0.gpg ] https://repo.mongodb.org/apt/ubuntu jammy/mongodb-org/8.0 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-8.0.list
sudo apt-get update
sudo apt-get install -y mongodb-org
sudo systemctl start mongod
sudo systemctl daemon-reload
sudo systemctl enable mongod
mongod --version
echo "MongoDB installed successfully."

# Install pipx
echo "Installing pipx..."
sudo apt update
sudo apt install -y pipx
pipx ensurepath
pipx --version
echo "pipx installed successfully."

# Install Poetry
echo "Installing Poetry..."
pipx install poetry
export PATH="$HOME/.local/bin:$PATH"
poetry --version
echo "Poetry installed successfully."

# Install direnv
echo "Installing direnv..."
sudo apt install -y direnv
direnv --version
echo "direnv installed successfully."

# Add direnv to shell
echo "Adding direnv to shell..."
eval "$(direnv hook $SHELL)"
echo "direnv added to shell."

# Create direnv config directory
echo "Creating direnv config directory..."
mkdir -p ~/.config/direnv
touch ~/.config/direnv/direnvrc

# Add Poetry layout function to direnvrc
echo "Setting up direnvrc..."
cat <<'EOF' >~/.config/direnv/direnvrc
layout_poetry() {
  if [[ ! -f pyproject.toml ]]; then
    log_error 'No pyproject.toml found. Use `poetry new` or `poetry init` to create one first.'
    exit 2
  fi

  poetry run true

  export VIRTUAL_ENV=$(poetry env info --path)
  export POETRY_ACTIVE=1
  PATH_add "$VIRTUAL_ENV/bin"
}
EOF
echo "direnvrc set up successfully."

# Add direnv layout to .envrc
echo "Adding direnv layout to .envrc..."
echo 'layout poetry' >.envrc
echo 'export PYTHONPATH=$PWD/app/src' >>.envrc
direnv allow
echo ".envrc set up successfully."

# Install project dependencies
echo "Updating packages and installing dependencies..."
poetry install
echo "Dependencies installed successfully."

echo "Setup complete. Remember to start your database before running the app."