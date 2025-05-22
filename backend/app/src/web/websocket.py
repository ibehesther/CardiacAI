# app/src/api/websocket_routes.py
from fastapi import APIRouter, WebSocket
from fastapi import Depends, HTTPException, status
from app.src.service.readings_service import (
    toggle_reading_store_service,
    download_ecg_service,
    handle_device_websocket_service,
    handle_frontend_websocket_service
)
from app.src.web.auth import oauth2_scheme
from app.src.service.auth import check_for_admin

router = APIRouter(tags=["websocket"])

@router.post("/readings/save/{device_id}")
async def toggle_reading_store(device_id: str, enable: bool, token: str = Depends(oauth2_scheme)):
    """
    Toggle storing of readings. When enabled, starts a new session.
    """
    # Check if the user is an admin
    print(f"Token: {token}")
    is_admin = await check_for_admin(token)
    if is_admin:
        return await toggle_reading_store_service(device_id, enable)
    raise HTTPException(
        status_code=status.HTTP_403_FORBIDDEN,
        detail="You do not have permission to perform this action.",
    )

@router.get("/readings/download/{session_id}")
async def download_ecg(session_id: str, token: str = Depends(oauth2_scheme)):
    """
    Download the ECG data for a given session ID.
    """
    return await download_ecg_service(session_id)

@router.websocket("/ws/device")
async def device_ws(websocket: WebSocket):
    """
    Handles WebSocket connections from devices.
    """
    await handle_device_websocket_service(websocket)

@router.websocket("/ws/frontend")
async def frontend_ws(websocket: WebSocket, token: str = Depends(oauth2_scheme)):
    """
    Handles WebSocket connections from the frontend.
    """
    await handle_frontend_websocket_service(websocket)