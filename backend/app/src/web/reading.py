# app/src/web/reading.py

from fastapi import APIRouter
from uuid import uuid4
from app.src.utils.plot import plot_ecg_and_return_image
from app.src.data.reading import get_reading_session
from fastapi import HTTPException

router = APIRouter(prefix="/reading", tags=["reading"])

# In-memory flags and session/buffer per device
store_reading_flags = {}      # device_id -> bool
current_sessions = {}         # device_id -> session_id
reading_buffers = {}          # device_id -> List[float]
BUFFER_SIZE = 100

@router.post("/toggle/{device_id}")
async def toggle_reading_store(device_id: str, enable: bool):
    """
    Toggle storing of readings. When enabled, starts a new session.
    """
    store_reading_flags[device_id] = enable

    if enable:
        # New session
        session_id = str(uuid4())
        current_sessions[device_id] = session_id
        reading_buffers[device_id] = []
        return {"device_id": device_id, "store_enabled": True, "session_id": session_id}
    else:
        # Clear session
        current_sessions.pop(device_id, None)
        reading_buffers.pop(device_id, None)
        return {"device_id": device_id, "store_enabled": False}


@router.get("/download/{session_id}")
async def download_ecg(session_id: str):
    try:
        session = await get_reading_session(session_id)
        return plot_ecg_and_return_image(session)
    except ValueError:
        raise HTTPException(status_code=404, detail="Session not found")

