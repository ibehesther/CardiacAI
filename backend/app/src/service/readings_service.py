from typing import Dict, List
from fastapi import WebSocket, HTTPException
import json
from uuid import uuid4
from app.src.utils.plot import plot_ecg_and_return_image
from app.src.data.reading import ReadingRepository
from fastapi.responses import StreamingResponse
from fastapi import WebSocket, WebSocketDisconnect
from app.src.models.reading import ECGReading

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

async def toggle_reading_store_service(device_id: str, enable: bool):
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
        session_docs.pop(device_id, None)
        return {"device_id": device_id, "store_enabled": False}

async def download_ecg_service(session_id: str):
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


async def get_device_metadata_service(device_id: str) -> List[ECGReading]:
    """
    Fetches all metadata for a given device ID from the database.
    """
    metadata_list = await ReadingRepository.get_all_metadata(device_id)
    return metadata_list


async def handle_device_websocket_service(websocket: WebSocket):
    """
    Handles WebSocket connections from devices, manages real-time data forwarding to frontend clients,
    and conditionally stores incoming data to the database in buffered batches.
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
            data_point = json.loads(data)

            # Forward to frontend
            if device_id in frontend_connections:
                for client_ws in frontend_connections[device_id]:
                    await client_ws.send_text(data)

            # Store to DB if toggled on
            if store_reading_flags.get(device_id):
                if not isinstance(data_point, (int, float)):
                    data_point = data_point.get("value")
                if data_point is None:
                    continue

                reading_buffers[device_id].append(float(data_point))

                # Save when buffer reaches threshold
                if len(reading_buffers[device_id]) >= BUFFER_SIZE:
                    session_id = current_sessions[device_id]
                    buffer_copy = reading_buffers[device_id][:BUFFER_SIZE]

                    if device_id not in session_docs:
                        print("Storing first")
                        inserted_id = await ReadingRepository.store_reading_with_array({
                            "device_id": device_id,
                            "session_id": session_id
                        }, buffer_copy)
                        session_docs[device_id] = inserted_id
                    else:
                        # print("Storing subsequent")
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
                    await ReadingRepository.append_to_array(session_docs[device_id], buffer)

            # Clear session state (turn off save)
            store_reading_flags.pop(device_id, None)
            current_sessions.pop(device_id, None)
            reading_buffers.pop(device_id, None)
            session_docs.pop(device_id, None)


async def handle_frontend_websocket_service(websocket: WebSocket):
    """
    Handles WebSocket connections from the frontend for a specific device.
    """
    await websocket.accept()
    device_id = websocket.query_params.get("device_id")
    if not device_id:
        print("Frontend WebSocket connection denied: Missing device_id.")
        await websocket.close(code=1008) # Policy Violation
        return

    # Check if the device is currently connected via device_ws
    if device_id not in device_connections:
        print(f"Frontend WebSocket connection denied: Device {device_id} not active.")
        await websocket.close(code=1011) # Internal Error (or a custom close code if you define one for 'device not active')
        return

    if device_id not in frontend_connections:
        frontend_connections[device_id] = []
    frontend_connections[device_id].append(websocket)
    print(f"Frontend connected to device {device_id}. Total frontend connections for device: {len(frontend_connections[device_id])}")

    try:
        while True:
            message = await websocket.receive_text()
            print(f"Received message from frontend for device {device_id}: {message}")
    except WebSocketDisconnect:
        print(f"Frontend disconnected from device {device_id}.")
        if device_id in frontend_connections and websocket in frontend_connections[device_id]:
            frontend_connections[device_id].remove(websocket)
            if not frontend_connections[device_id]:
                frontend_connections.pop(device_id)
        print(f"Remaining frontend connections for device {device_id}: {len(frontend_connections.get(device_id, []))}")
    except Exception as e:
        print(f"Error in frontend WebSocket for device {device_id}: {e}")
        await websocket.close(code=1011)
