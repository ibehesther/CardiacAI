from fastapi import APIRouter

router = APIRouter(prefix="/ecg", tags=["ecg"])

@router.post("/save")
async def save_ecg_data():
    return {"message": "Hello World from save ECG"}

@router.get("/history")
async def get_user_ecg_data():
    return {"message": "Hello World from ECG history"}

@router.get("/stream")
async def stream_latest_data():
    return {"message": "Hello World from ECG stream"}
