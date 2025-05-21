from data import async_db
from bson import ObjectId

async def get_reading_session(session_id: str):
    session = await async_db.ecg_readings.find_one({"_id": ObjectId(session_id)})
    if not session:
        raise ValueError("Session not found")
    return session
