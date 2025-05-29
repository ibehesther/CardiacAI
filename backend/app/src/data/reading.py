from data import async_db
"""
Module for managing ECG reading and array data in an asynchronous MongoDB database.
Classes:
    ReadingRepository: Provides static methods for storing, retrieving, updating, and deleting ECG readings and arrays.
Methods:
    store_reading_with_array(reading_data: dict, array_data: List[float]) -> ObjectId
        Stores a new ECG reading and its associated array in the database.
    get_all_metadata() -> List[dict]
        Retrieves metadata for all ECG readings.
    get_reading_session(session_id: str) -> dict
        Retrieves a reading session and its associated array data by session ID.
    get_array(array_id: str) -> dict
        Retrieves an ECG array by its ObjectId.
    get_all_arrays() -> List[dict]
        Retrieves all ECG arrays from the database.
    update_metadata(session_id: str, updates: dict) -> dict
        Updates metadata for a specific reading session.
    update_array(array_id: str, updates: dict) -> dict
        Updates an ECG array by its ObjectId.
    delete_reading(session_id: str) -> None
        Deletes a reading and its associated array by session ID.
    delete_array(array_id: str) -> None
        Deletes an ECG array by its ObjectId.
    append_to_array(array_id: str, new_values: List[float]) -> dict
        Appends new values to an existing ECG array.
"""
from bson import ObjectId
from bson import ObjectId
from typing import List, Optional
from app.src.models.reading import ECGReading, ECGArray
from data import async_db

class ReadingRepository:
    @staticmethod
    async def store_reading_with_array(
        reading_data: dict, array_data: List[float]
    ):
        """
        Store a new ECG reading and its associated array in the database.
        Args:
            reading_data (dict): The ECG reading data to store.
            array_data (List[float]): The ECG array data to store.
        """
        # Create and insert the ECGArray
        ecg_array = ECGArray(data=array_data)
        result = await async_db.ecg_arrays.insert_one(ecg_array.dict(by_alias=True))

        # Create and insert the ECGReading
        reading = ECGReading(**reading_data, ecg_array_id=result.inserted_id)
        await async_db.readings.insert_one(reading.dict(by_alias=True))
        return result.inserted_id

    @staticmethod
    async def get_all_metadata(device_id: str) -> List[ECGReading]:
        """
        Retrieve metadata for all ECG readings.
        Returns:
            List[dict]: A list of dictionaries containing metadata for all ECG readings.
        """
        cursor = async_db.readings.find({"device_id": device_id})
        readings = [ECGReading(**doc) async for doc in cursor]
        return readings

    @staticmethod
    async def get_reading_session(session_id: str) -> dict:
        """
        Retrieve a reading session and its associated array data by session ID.
        Args:
            session_id (str): The session ID of the reading to retrieve.
        Returns:
            dict: A dictionary containing the reading session and its associated array data.
        """
        session = await async_db.readings.find_one({"session_id": session_id})
        if not session:
            raise ValueError("Session not found")
        
        array = await ReadingRepository.get_array(session["ecg_array_id"])
        combined = {
            **session,
            "data": array["data"]
        }
        return combined

    @staticmethod
    async def get_array(array_id: str) -> dict:
        """
        Retrieve an ECG array by its ObjectId.
        Args:
            array_id (str): The ObjectId of the ECG array to retrieve.
        Returns:
            dict: A dictionary containing the ECG array data.
        """
        if not ObjectId.is_valid(array_id):
            raise ValueError("Invalid ObjectId")
        array = await async_db.ecg_arrays.find_one({"_id": ObjectId(array_id)})
        if not array:
            raise ValueError("Array not found")
        return array

    @staticmethod
    async def get_all_arrays() -> List[dict]:
        cursor = async_db.ecg_arrays.find()
        return [doc async for doc in cursor]

    @staticmethod
    async def update_metadata(session_id: str, updates: dict) -> dict:
        result = await async_db.readings.find_one_and_update(
            {"session_id": session_id},
            {"$set": updates},
            return_document=True
        )
        if not result:
            raise ValueError("Reading not found")
        return result

    @staticmethod
    async def update_array(array_id: str, updates: dict) -> dict:
        if not ObjectId.is_valid(array_id):
            raise ValueError("Invalid ObjectId")
        result = await async_db.ecg_arrays.find_one_and_update(
            {"_id": ObjectId(array_id)},
            {"$set": updates},
            return_document=True
        )
        if not result:
            raise ValueError("Array not found")
        return result

    @staticmethod
    async def delete_reading(session_id: str) -> None:
        reading = await async_db.readings.find_one({"session_id": session_id})
        if not reading:
            raise ValueError("Reading not found")
        await async_db.readings.delete_one({"_id": reading["_id"]})
        await async_db.ecg_arrays.delete_one({"_id": reading["ecg_array_id"]})

    @staticmethod
    async def delete_array(array_id: str) -> None:
        if not ObjectId.is_valid(array_id):
            raise ValueError("Invalid ObjectId")
        result = await async_db.ecg_arrays.delete_one({"_id": ObjectId(array_id)})
        if result.deleted_count == 0:
            raise ValueError("Array not found")
        
    @staticmethod
    async def append_to_array(array_id: str, new_values: List[float]) -> dict:
        """
        Appends new values to an array in a MongoDB document, capping its total length.
        If adding new_values would cause the array to exceed 2000 elements,
        new data will be discarded or truncated to fit the cap.

        Args:
            array_id (str): The ObjectId string of the document to update.
            new_values (List[float]): A list of float values to append.

        Returns:
            dict: The updated document from the database.

        Raises:
            ValueError: If array_id is invalid, new_values is not a list of numbers,
                        or the document is not found/failed to update.
        """
        # Validate input array_id
        if not ObjectId.is_valid(array_id):
            raise ValueError("Invalid ObjectId")

        # Validate new_values content
        if not isinstance(new_values, list) or not all(isinstance(v, (int, float)) for v in new_values):
            raise ValueError("new_values must be a list of numbers")

        MAX_ARRAY_LENGTH = 2000

        # 1. Fetch the document to get the current length of the 'data' array
        # This step is crucial to determine how many new values can be added.
        document = await async_db.ecg_arrays.find_one({"_id": ObjectId(array_id)})
        if not document:
            raise ValueError(f"Array document with ID {array_id} not found.")

        current_data = document.get("data", [])
        current_length = len(current_data)

        # 2. Determine how many new values can actually be appended
        if current_length >= MAX_ARRAY_LENGTH:
            # If the array is already at or above the maximum length,
            # discard all new incoming data.
            print(f"Array {array_id} is already at max length ({MAX_ARRAY_LENGTH}). New data discarded.")
            return document  # Return the existing document without modification

        # Calculate the remaining capacity in the array
        remaining_capacity = MAX_ARRAY_LENGTH - current_length

        # Take only the values that fit within the remaining capacity
        values_to_add = new_values[:remaining_capacity]

        if not values_to_add:
            # If no values are left to add after capping (e.g., new_values was empty
            # or too large to fit any part), return the current document.
            print(f"No new values to add to array {array_id} after capping.")
            return document

        # 3. Append the (potentially truncated) list of new values to the array
        # "$push" with "$each" is used to append multiple elements.
        result = await async_db.ecg_arrays.find_one_and_update(
            {"_id": ObjectId(array_id)},
            {"$push": {"data": {"$each": values_to_add}}},
            return_document=True  # Returns the modified document
        )

        if not result:
            # This case should ideally not happen if the document was found initially,
            # but it's good practice to handle potential update failures.
            raise ValueError(f"Failed to update array document with ID {array_id}.")

        return result