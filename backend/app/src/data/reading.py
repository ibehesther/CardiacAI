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
    async def get_all_metadata() -> List[dict]:
        """
        Retrieve metadata for all ECG readings.
        Returns:
            List[dict]: A list of dictionaries containing metadata for all ECG readings.
        """
        cursor = async_db.readings.find()
        return [doc async for doc in cursor]

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
        if not ObjectId.is_valid(array_id):
            raise ValueError("Invalid ObjectId")
        if not isinstance(new_values, list) or not all(isinstance(v, (int, float)) for v in new_values):
            raise ValueError("new_values must be a list of numbers")

        result = await async_db.ecg_arrays.find_one_and_update(
            {"_id": ObjectId(array_id)},
            {"$push": {"data": {"$each": new_values}}},
            return_document=True
        )
        if not result:
            raise ValueError("Array not found")
        return result
