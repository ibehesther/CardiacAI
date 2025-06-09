"""
This module is responsible for creating the FastAPI app instance.
"""

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.src.web.machine import router as machine_router
from app.src.web.websocket import router as websocket_router
# from web.reading import router as reading_router
from app.src.web.auth import router as auth_router


def create_app() -> FastAPI:
    """
    Creates the FastAPI app instance and includes the user router.

    Returns:
        FastAPI: The FastAPI app instance.
    """
    origins = [
        "http://localhost",  # For local development
        "http://localhost:8080", # Another common local development port
        "http://localhost:8000", # Ngninx reverse proxy
        "https://cardiacai.tech",  # production frontend domain
        "*", # For development phase testing
    ]

    app = FastAPI()

    app.add_middleware(
        CORSMiddleware,
        allow_origins=origins,
        allow_credentials=True, # Allow cookies and authorization headers
        allow_methods=["*"],    # Allow all HTTP methods (GET, POST, PUT, DELETE, etc.)
        allow_headers=["*"],    # Allow all headers
    )

    app.router.prefix = "/api"
    app.include_router(machine_router)
    app.include_router(websocket_router)
    app.include_router(auth_router)
    return app
