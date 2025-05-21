#!/bin/bash

export PYTHONPATH=/home/robonish/victory/CardiacAI/backend/app/src/
poetry run uvicorn app.main:app --reload