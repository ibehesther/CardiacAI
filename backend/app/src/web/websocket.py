from fastapi import APIRouter, WebSocket, WebSocketDisconnect
from typing import Dict, List
from data import async_db
from bson import ObjectId
from datetime import datetime
import json
from uuid import uuid4
from app.src.utils.plot import plot_ecg_and_return_image
from app.src.data.reading import get_reading_session
from fastapi import HTTPException

router = APIRouter(prefix="/ws", tags=["websocket"])

# Track active WebSocket connections
device_connections: Dict[str, WebSocket] = {}          # device_id: device websocket
frontend_connections: Dict[str, List[WebSocket]] = {}  # device_id: list of frontend websockets
# Track which devices should store readings
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





@router.websocket("/device")
async def device_ws(websocket: WebSocket):
    await websocket.accept()
    device_id = websocket.query_params.get("device_id")
    if not device_id:
        await websocket.close()
        return

    device_connections[device_id] = websocket
    print(f"Device {device_id} connected.")

    try:
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
                        result = await async_db.readings.insert_one({
                            "device_id": device_id,
                            "timestamp": datetime.utcnow(),
                            "session_id": session_id,
                            "data": buffer_copy,
                        })
                        session_docs[device_id] = result.inserted_id
                    else:
                        # Append to existing document
                        print(f"Storing subsequent")
                        await async_db.readings.update_one(
                            {"_id": ObjectId(session_docs[device_id])},
                            {"$push": {"data": {"$each": buffer_copy}}}
                        )

                    reading_buffers[device_id].clear()

    except WebSocketDisconnect:
        print(f"Device {device_id} disconnected.")
        device_connections.pop(device_id, None)

        # 🧠 Handle pending buffer if session was active
        if store_reading_flags.get(device_id):
            buffer = reading_buffers.get(device_id, [])
            if buffer:
                session_id = current_sessions.get(device_id)
                if device_id not in session_docs:
                    # Insert fresh if buffer never got saved
                    result = await async_db.readings.insert_one({
                        "device_id": device_id,
                        "timestamp": datetime.utcnow(),
                        "session_id": session_id,
                        "data": buffer
                    })
                else:
                    # Append final buffer to existing doc
                    await async_db.readings.update_one(
                        {"_id": ObjectId(session_docs[device_id])},
                        {"$push": {"data": {"$each": buffer}}}
                    )

            # Clear session state (turn off save)
            store_reading_flags.pop(device_id, None)
            current_sessions.pop(device_id, None)
            reading_buffers.pop(device_id, None)
            session_docs.pop(device_id, None)


@router.websocket("/frontend")
async def frontend_ws(websocket: WebSocket):
    await websocket.accept()
    device_id = websocket.query_params.get("device_id")
    if not device_id:
        await websocket.close()
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

