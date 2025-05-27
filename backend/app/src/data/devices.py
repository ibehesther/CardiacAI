# app/src/data/device_repository.py
from typing import List, Optional, Dict, Any
from bson import ObjectId
from app.src.models.device import DeviceInDB, DeviceCreate
from app.src.utils.security import hash_password, verify_password
from data import async_db

class DeviceRepository:
    _collection = async_db.devices

    @staticmethod
    async def create_device(device_data: DeviceCreate) -> DeviceInDB:
        """
        Hashes passwords and stores a new DeviceInDB in the database.

        Args:
            device_data (DeviceCreate): The DeviceInDB data from the input.

        Returns:
            DeviceInDB: The created DeviceInDB object with hashed passwords and MongoDB _id.
        """
        hashed_pwd = hash_password(device_data.password)
        hashed_public_pwd = hash_password(device_data.public_password)

        device_to_insert = DeviceInDB(
            device_id=device_data.device_id,
            hashed_password=hashed_pwd,
            hashed_public_password=hashed_public_pwd
        )
        
        insert_result = await DeviceRepository._collection.insert_one(
            device_to_insert.dict(by_alias=True, exclude_none=True)
        )
        
        device_to_insert.id = insert_result.inserted_id
        return device_to_insert

    @staticmethod
    async def get_device_by_id(device_id: str) -> Optional[DeviceInDB]:
        """
        Retrieves a DeviceInDB by its device_id.

        Args:
            device_id (str): The unique identifier of the DeviceInDB.

        Returns:
            Optional[DeviceInDB]: The DeviceInDB object if found, otherwise None.
        """
        device_doc = await DeviceRepository._collection.find_one({"device_id": device_id})
        if device_doc:
            return DeviceInDB(**device_doc)
        return None

    @staticmethod
    async def get_device_by_object_id(object_id: str) -> Optional[DeviceInDB]:
        """
        Retrieves a DeviceInDB by its MongoDB ObjectId.

        Args:
            object_id (str): The MongoDB ObjectId of the DeviceInDB.

        Returns:
            Optional[DeviceInDB]: The DeviceInDB object if found, otherwise None.
        """
        try:
            device_doc = await DeviceRepository._collection.find_one({"_id": ObjectId(object_id)})
            if device_doc:
                return DeviceInDB(**device_doc)
            return None
        except Exception:
            return None

    @staticmethod
    async def get_all_devices() -> List[DeviceInDB]:
        """
        Retrieves all devices from the database.

        Returns:
            List[DeviceInDB]: A list of all DeviceInDB objects.
        """
        devices = []
        async for device_doc in DeviceRepository._collection.find({}):
            devices.append(DeviceInDB(**device_doc))
        return devices

    @staticmethod
    async def update_device(object_id: str, updates: Dict[str, Any]) -> Optional[DeviceInDB]:
        """
        Updates a DeviceInDB's fields in the database.
        Note: If updating passwords, ensure they are hashed *before* passing to this function.

        Args:
            object_id (str): The MongoDB ObjectId of the DeviceInDB to update.
            updates (Dict[str, Any]): A dictionary of fields to update and their new values.

        Returns:
            Optional[DeviceInDB]: The updated DeviceInDB object if found and updated, otherwise None.
        """
        try:
            if 'password' in updates:
                updates['hashed_password'] = hash_password(updates.pop('password'))
            if 'public_password' in updates:
                updates['hashed_public_password'] = hash_password(updates.pop('public_password'))

            result = await DeviceRepository._collection.update_one(
                {"_id": ObjectId(object_id)},
                {"$set": updates}
            )
            if result.modified_count == 1:
                return await DeviceRepository.get_device_by_object_id(object_id)
            return None
        except Exception:
            return None

    @staticmethod
    async def delete_device(object_id: str) -> bool:
        """
        Deletes a DeviceInDB from the database by its MongoDB ObjectId.

        Args:
            object_id (str): The MongoDB ObjectId of the DeviceInDB to delete.

        Returns:
            bool: True if the DeviceInDB was deleted, False otherwise.
        """
        try:
            result = await DeviceRepository._collection.delete_one({"_id": ObjectId(object_id)})
            return result.deleted_count == 1
        except Exception:
            return False

    @staticmethod
    async def authenticate_device(device_id: str, plain_password: str) -> Optional[DeviceInDB]:
        """
        Authenticates a DeviceInDB using its device_id and plain password.

        Args:
            device_id (str): The ID of the DeviceInDB.
            plain_password (str): The plain-text password provided for authentication.

        Returns:
            Optional[DeviceInDB]: The authenticated DeviceInDB object if credentials are valid, else None.
        """
        DeviceInDB = await DeviceRepository.get_device_by_id(device_id)
        if DeviceInDB and verify_password(plain_password, DeviceInDB.hashed_password):
            return DeviceInDB
        return None

    @staticmethod
    async def verify_public_password(device_id: str, plain_public_password: str) -> bool:
        """
        Verifies the public password for a given device_id.

        Args:
            device_id (str): The ID of the DeviceInDB.
            plain_public_password (str): The plain-text public password to verify.

        Returns:
            bool: True if the public password is valid, False otherwise.
        """
        DeviceInDB = await DeviceRepository.get_device_by_id(device_id)
        if DeviceInDB and verify_password(plain_public_password, DeviceInDB.hashed_public_password):
            return True
        return False