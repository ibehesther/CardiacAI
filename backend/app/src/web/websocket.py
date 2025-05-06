from fastapi import APIRouter, WebSocket, WebSocketDisconnect

router = APIRouter(prefix="/ws", tags=["websocket"])

@router.websocket("/ecg")
async def ecg_websocket(websocket: WebSocket):
    await websocket.accept()
    try:
        await websocket.send_text("Hello World from WebSocket")
        while True:
            await websocket.receive_text()
    except WebSocketDisconnect:
        print("Disconnected")
