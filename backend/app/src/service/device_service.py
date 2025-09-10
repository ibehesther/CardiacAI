# app/src/services/device_service.py
from typing import Optional, List, Dict, Any
from app.src.models.device import DeviceCreate, DeviceInDB
from app.src.data.devices import DeviceRepository

class DeviceService:
    """
    Service layer for managing DeviceInDB-related business logic.
    Interacts with the DeviceRepository to perform CRUD and authentication operations.
    """

    @staticmethod
    async def create_new_device(device_data: DeviceCreate) -> Optional[DeviceInDB]:
        """
        Handles the business logic for creating a new DeviceInDB.
        Checks for existing device_id before creation.

        Args:
            device_data (DeviceCreate): The input data for the new DeviceInDB.

        Returns:
            Optional[DeviceInDB]: The newly created DeviceInDB object if successful, None if device_id already exists.
        """
        # Check if a DeviceInDB with this ID already exists
        existing_device = await DeviceRepository.get_device_by_id(device_data.device_id)
        if existing_device:
            return None # Indicate that creation failed due to conflict
        
        # If no conflict, proceed to create via the repository
        new_device = await DeviceRepository.create_device(device_data)
        return new_device

    @staticmethod
    async def get_device_details_by_id(device_id: str) -> Optional[DeviceInDB]:
        """
        Retrieves DeviceInDB details by its device_id.

        Args:
            device_id (str): The unique identifier of the DeviceInDB.

        Returns:
            Optional[DeviceInDB]: The DeviceInDB object if found, otherwise None.
        """
        DeviceInDB = await DeviceRepository.get_device_by_id(device_id)
        return DeviceInDB

    @staticmethod
    async def get_device_details_by_object_id(object_id: str) -> Optional[DeviceInDB]:
        """
        Retrieves DeviceInDB details by its MongoDB ObjectId.

        Args:
            object_id (str): The MongoDB ObjectId of the DeviceInDB.

        Returns:
            Optional[DeviceInDB]: The DeviceInDB object if found, otherwise None.
        """
        DeviceInDB = await DeviceRepository.get_device_by_object_id(object_id)
        return DeviceInDB

    @staticmethod
    async def list_all_devices() -> List[DeviceInDB]:
        """
        Retrieves a list of all devices.

        Returns:
            List[DeviceInDB]: A list of all DeviceInDB objects.
        """
        devices = await DeviceRepository.get_all_devices()
        return devices

    @staticmethod
    async def authenticate_device(device_id: str, password: str) -> Optional[DeviceInDB]:
        """
        Authenticates a DeviceInDB using its device_id and plain password.

        Args:
            device_id (str): The ID of the DeviceInDB.
            password (str): The plain-text password for authentication.

        Returns:
            Optional[DeviceInDB]: The authenticated DeviceInDB object if credentials are valid, else None.
        """
        authenticated_device = await DeviceRepository.authenticate_device(device_id, password)
        return authenticated_device

    @staticmethod
    async def verify_public_password_service(device_id: str, public_password: str) -> bool:
        """
        Service method to verify a DeviceInDB's public password.

        Args:
            device_id (str): The ID of the DeviceInDB.
            public_password (str): The plain-text public password to verify.

        Returns:
            bool: True if the public password is valid, False otherwise.
        """
        is_valid = await DeviceRepository.verify_public_password(device_id, public_password)
        return is_valid


    @staticmethod
    async def update_existing_device(object_id: str, updates: Dict[str, Any]) -> Optional[DeviceInDB]:
        """
        Updates an existing DeviceInDB's information.

        Args:
            object_id (str): The MongoDB ObjectId of the DeviceInDB to update.
            updates (Dict[str, Any]): A dictionary of fields to update.
                                    If 'password' or 'public_password' are in updates,
                                    they will be hashed by the repository.

        Returns:
            Optional[DeviceInDB]: The updated DeviceInDB object if successful, None otherwise.
        """
        updated_device = await DeviceRepository.update_device(object_id, updates)
        return updated_device
    
    @staticmethod
    async def delete_existing_device(object_id: str) -> bool:
        """
        Deletes a DeviceInDB from the database.

        Args:
            object_id (str): The MongoDB ObjectId of the DeviceInDB to delete.

        Returns:
            bool: True if the DeviceInDB was successfully deleted, False otherwise.
        """
        deleted = await DeviceRepository.delete_device(object_id)
        return deleted
