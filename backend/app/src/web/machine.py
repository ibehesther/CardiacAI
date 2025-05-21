from fastapi import APIRouter
from app.src.data.devices import create_device
from app.src.models.device import DeviceCreate

router = APIRouter(prefix="/device", tags=["device"])

@router.post("/register")
async def register_device(device: DeviceCreate):
    device_id = await create_device(device)
    return {"id": device_id}

@router.post("/save")
async def save_ecg_data():
    return {"message": "Hello World from save ECG"}

@router.get("/history")
async def get_user_ecg_data():
    return {"message": "Hello World from ECG history"}

@router.get("/stream")
async def stream_latest_data():
    return {"message": "Hello World from ECG stream"}
