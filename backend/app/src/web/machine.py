# app/src/api/routers/device_router.py
from fastapi import APIRouter, HTTPException, status, Body, Depends
from typing import List, Dict, Any
from app.src.models.device import DeviceCreate, DeviceInDB
from app.src.service.device_service import DeviceService # Import your service
from app.src.service.readings_service import get_device_metadata_service
from app.src.models.reading import ECGReading
from app.src.web.auth import oauth2_scheme


router = APIRouter(prefix="/devices", tags=["Devices"])

@router.post("/", response_model=DeviceInDB, status_code=status.HTTP_201_CREATED)
async def create_device_endpoint(device_data: DeviceCreate):
    """
    **Creates a new DeviceInDB.**

    Takes `device_id`, `password`, and `public_password` as input.
    The passwords will be hashed before storage.
    Returns the created DeviceInDB object with its database ID.
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
    For example: `{"name": "New DeviceInDB Name", "location": "Warehouse A"}`.
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
    """
    try:
        metadata = await get_device_metadata_service(device_id)
        print(f"metadata: {metadata}")
        if not metadata:
            # Optionally return 404 if no metadata found, or an empty list
            raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail=f"No metadata found for DeviceInDB ID: {device_id}")
        return metadata
    except Exception as e:
        print(f"Error fetching metadata for DeviceInDB {device_id}: {e}")
        raise HTTPException(status_code=status.HTTP_404_NOT_FOUND, detail="Device has never saved any data to db.")