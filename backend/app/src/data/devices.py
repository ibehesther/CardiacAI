from data import async_db
from app.src.models.device import DeviceCreate, DeviceInDB
from app.src.utils.security import hash_password, verify_password


async def create_device(device: DeviceCreate):
    hashed = hash_password(device.password)
    device_doc = {
        "device_id": device.device_id,
        "hashed_password": hashed
    }
    result = await async_db.devices.insert_one(device_doc)
    return str(result.inserted_id)


async def get_device_by_id(device_id: str) -> DeviceInDB | None:
    data = await async_db.devices.find_one({"device_id": device_id})
    if data:
        return DeviceInDB(**data)
    return None


async def authenticate_device(device_id: str, password: str) -> bool:
    device = await get_device_by_id(device_id)
    if not device:
        return False
    return verify_password(password, device.hashed_password)
