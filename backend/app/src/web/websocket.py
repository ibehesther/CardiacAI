from fastapi import APIRouter, WebSocket, WebSocketDisconnect
from typing import Dict, List
import json
from uuid import uuid4
from app.src.utils.plot import plot_ecg_and_return_image
from fastapi import HTTPException
from fastapi.responses import StreamingResponse
from app.src.data.reading import ReadingRepository

router = APIRouter(prefix="/ws", tags=["websocket"])

# Track active WebSocket connections
device_connections: Dict[str, WebSocket] = {}          # device_id: device websocket
frontend_connections: Dict[str, List[WebSocket]] = {}  # device_id: list of frontend websockets

# Cache created document ID
session_docs = {}  # device_id -> inserted document _id

# In-memory flags and session/buffer per device
store_reading_flags = {}      # device_id -> bool
current_sessions = {}         # device_id -> session_id
reading_buffers = {}          # device_id -> List[float]
BUFFER_SIZE = 100

@router.post("/toggle/{device_id}")
async def toggle_reading_store(device_id: str, enable: bool):
    """
    Toggle storing of readings. When enabled, starts a new session.

    Args:
        device_id (str): The ID of the device.
        enable (bool): True to enable storing, False to disable.
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
    """
    Download the ECG data for a given session ID.
    Args:
        session_id (str): The ID of the session to download.
    """
    try:
        session = await ReadingRepository.get_reading_session(session_id)
        return StreamingResponse(plot_ecg_and_return_image(session), media_type="image/png", headers={
        "Content-Disposition": f"attachment; filename=ecg_session_{session_id}.png"
    })
    except ValueError:
        raise HTTPException(status_code=404, detail="Session not found")


@router.websocket("/device")
async def device_ws(websocket: WebSocket):
    """
    Handles WebSocket connections from devices, manages real-time data forwarding to frontend clients,
    and conditionally stores incoming data to the database in buffered batches.
    Workflow:
    - Accepts a WebSocket connection and retrieves the device ID from query parameters.
    - Registers the device connection and listens for incoming data.
    - Forwards received data to all connected frontend clients associated with the device.
    - If data storage is enabled for the device, buffers incoming readings and saves them to the database
        in batches when the buffer reaches a predefined size.
    - On disconnection, ensures any remaining buffered data is saved and cleans up session state.
    Args:
            websocket (WebSocket): The WebSocket connection instance from the client device.
    Raises:
            None directly, but handles WebSocketDisconnect internally to perform cleanup.
    """

    await websocket.accept()
    device_id = websocket.query_params.get("device_id")
    if not device_id:
        await websocket.close()
        return

    device_connections[device_id] = websocket
    print(f"Device {device_id} connected.")

    try:
        inserted_id = ""
        while True:
            data = await websocket.receive_text()
            data_point = json.loads(data)  # Expecting a float or JSON containing "value"

            # Forward to frontend
            if device_id in frontend_connections:
                for client_ws in frontend_connections[device_id]:
                    await client_ws.send_text(data)

            # Store to DB if toggled on
            if store_reading_flags.get(device_id):
                if not isinstance(data_point, (int, float)):
                    # If payload is a dict like {"value": 0.5}
                    data_point = data_point.get("value")
                if data_point is None:
                    continue

                reading_buffers[device_id].append(float(data_point))

                # Save when buffer reaches threshold
                if len(reading_buffers[device_id]) >= BUFFER_SIZE:
                    session_id = current_sessions[device_id]
                    buffer_copy = reading_buffers[device_id][:BUFFER_SIZE]

                    if device_id not in session_docs:
                        # First save: insert new document
                        print(f"Storing first")
                        inserted_id = await ReadingRepository.store_reading_with_array({
                            "device_id": device_id,
                            "session_id": session_id
                        }, buffer_copy)
                        session_docs[device_id] = inserted_id

                    else:
                        # Append to existing document
                        print(f"Storing subsequent")
                        await ReadingRepository.append_to_array(inserted_id, buffer_copy)

                    reading_buffers[device_id].clear()

    except WebSocketDisconnect:
        print(f"Device {device_id} disconnected.")
        device_connections.pop(device_id, None)

        # Handle pending buffer if session was active
        if store_reading_flags.get(device_id):
            buffer = reading_buffers.get(device_id, [])
            if buffer:
                session_id = current_sessions.get(device_id)
                if device_id not in session_docs:
                    inserted_id = await ReadingRepository.store_reading_with_array({
                            "device_id": device_id,
                            "session_id": session_id
                        }, buffer)
                    session_docs[device_id] = inserted_id
                else:
                    await ReadingRepository.append_to_array(inserted_id, buffer)

            # Clear session state (turn off save)
            store_reading_flags.pop(device_id, None)
            current_sessions.pop(device_id, None)
            reading_buffers.pop(device_id, None)
            session_docs.pop(device_id, None)


@router.websocket("/frontend")
async def frontend_ws(websocket: WebSocket):
    """
    Handles WebSocket connections from the frontend for a specific device.
    Accepts a WebSocket connection, validates the provided device_id from query parameters,
    and manages the connection in the frontend_connections mapping. If the device_id is invalid
    or the device is offline (not present in device_connections), the connection is closed.
    Once connected, the function listens for incoming messages from the frontend (e.g., control commands).
    On disconnection, it cleans up the connection from the frontend_connections mapping.
    Args:
        websocket (WebSocket): The WebSocket connection instance from the frontend.
    Raises:
        None. (HTTPException is commented out; connection is closed on error.)
    """

    await websocket.accept()
    device_id = websocket.query_params.get("device_id")
    if not device_id:
        await websocket.close()
        # raise HTTPException(status_code=404, detail="Invalid device_id")
        return
    
    if device_id not in device_connections:
        await websocket.close()
        # raise HTTPException(status_code=404, detail="Device is Offline")
        return

    if device_id not in frontend_connections:
        frontend_connections[device_id] = []
    frontend_connections[device_id].append(websocket)
    print(f"Frontend connected to device {device_id}.")

    try:
        while True:
            await websocket.receive_text()  # Optional: listen to frontend messages (e.g., control commands)
    except WebSocketDisconnect:
        print(f"Frontend disconnected from device {device_id}.")
        frontend_connections[device_id].remove(websocket)
        if not frontend_connections[device_id]:
            frontend_connections.pop(device_id)

