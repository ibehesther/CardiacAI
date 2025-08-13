# app/src/api/routers/device_router.py
from fastapi import APIRouter, HTTPException, status, Body, Depends
from typing import List, Dict, Any
from app.src.models.device import DeviceCreate, DeviceInDB
from app.src.service.device_service import DeviceService
from app.src.service.readings_service import get_device_metadata_service
from app.src.models.reading import ECGReading
from app.src.web.auth import oauth2_scheme


router = APIRouter(prefix="/devices", tags=["Devices"])

@router.post("/", response_model=DeviceInDB, status_code=status.HTTP_201_CREATED)
async def create_device_endpoint(device_data: DeviceCreate):
    """
    **Creates a new DeviceInDB.**

    Args:
        `device_id`: The unique identifier for the device.
        `password`: Admin password for the device (Admins can control the device).
        `public_password`: Public password for the device (Users can only view device live data).

    Returns:
        Returns DeviceInDB object with its database ID.
    """
    new_device = await DeviceService.create_new_device(device_data)
    if new_device is None:
        raise HTTPException(
            status_code=status.HTTP_409_CONFLICT,
            detail=f"DeviceInDB with ID '{device_data.device_id}' already exists."
        )
    return new_device

@router.put("/{device_id}", response_model=DeviceInDB)
async def update_device_endpoint(
    device_id: str,
    updates: Dict[str, Any] = Body(...),
    token: str = Depends(oauth2_scheme)
):
    """
    **Updates an existing DeviceInDB.**

    Provide the `device_id` in the path and a JSON body with the fields to update.
    For example: `{"name": "New DeviceInDB Name", "password": "12345678"}`.
    If you include "password" or "public_password", they will be re-hashed.
    Returns the updated DeviceInDB object.
    """
    # First, get the DeviceInDB by device_id to get its MongoDB _id
    existing_device = await DeviceService.get_device_details_by_id(device_id)
    if not existing_device:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="DeviceInDB not found.")

    # Pass the MongoDB ObjectId to the repository for update
    updated_device = await DeviceService.update_existing_device(str(existing_device.id), updates)
    
    if not updated_device:
        raise HTTPException(status_code=status.HTTP_500_INTERNAL_SERVER_ERROR, detail="Failed to update DeviceInDB.")
    
    return updated_device

@router.delete("/{device_id}", status_code=status.HTTP_204_NO_CONTENT)
async def delete_device_endpoint(device_id: str, token: str = Depends(oauth2_scheme)):
    """
    **Deletes a DeviceInDB.**

    Provide the `device_id` of the DeviceInDB to be deleted.
    Returns a 204 No Content status on successful deletion.
    """
    # First, get the DeviceInDB by device_id to get its MongoDB _id
    existing_device = await DeviceService.get_device_details_by_id(device_id)
    if not existing_device:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="DeviceInDB not found.")

    # Pass the MongoDB ObjectId to the repository for deletion
    deleted = await DeviceService.delete_existing_device(str(existing_device.id))
    
    if not deleted:
        raise HTTPException(status_code=status.HTTP_500_INTERNAL_SERVER_ERROR, detail="Failed to delete DeviceInDB.")
    
    return # 204 No Content response


@router.get("/metadata/{device_id}", response_model=List[ECGReading])
async def get_metadata_for_device(device_id: str, token: str = Depends(oauth2_scheme)):
    """
    Endpoint to get all metadata for a specific DeviceInDB.

    The metadata includes pointers to all ECG readings saved for the device.
    In these json readings, you can extract the `session_id` and `timestamp` to download the ECG data.

    The last item on the list has a save_status field that indicates whether the device is currently storing readings or not.
    Args:
        device_id (str): The unique identifier for the device.
    Returns:
        List[ECGReading]: A list of ECG readings associated with the device.

    Sructure of the response:
    `[
    {
        "_id": "683618b1ff564b83d431dd60",
        "device_id": "victory",
        "session_id": "5f750ddd-177e-4669-9149-07e2e959aa29",
        "timestamp": "2025-05-27T19:55:29.106000",
        "ecg_array_id": "683618b1ff564b83d431dd5f"
    },
    {
        "_id": "6836192bff564b83d431dd62",
        "device_id": "victory",
        "session_id": "f3f2e168-e51b-47d5-bf10-aaac19a0386e",
        "timestamp": "2025-05-27T19:57:31.376000",
        "ecg_array_id": "6836192bff564b83d431dd61"
    }
    ]`
    """
    try:
        metadata = await get_device_metadata_service(device_id)
        if not metadata:
            # Optionally return 404 if no metadata found, or an empty list
            raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail=f"No metadata found for DeviceInDB ID: {device_id}")
        return metadata
    except Exception as e:
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Device has never saved any data to db.")