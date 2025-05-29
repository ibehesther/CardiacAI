"""
This module is responsible for creating the FastAPI app instance.
"""

from fastapi import FastAPI
from web.machine import router as machine_router
from web.websocket import router as websocket_router
# from web.reading import router as reading_router
from web.auth import router as auth_router


def create_app() -> FastAPI:
    """
    Creates the FastAPI app instance and includes the user router.

    Returns:
        FastAPI: The FastAPI app instance.
    """
    app = FastAPI()
    app.router.prefix = "/api"
    app.include_router(machine_router)
    app.include_router(websocket_router)
    app.include_router(auth_router)
    return app