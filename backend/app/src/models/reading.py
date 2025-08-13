from pydantic import BaseModel, Field
from bson import ObjectId
from typing import Annotated, List
from datetime import datetime
from app.src.models.device import ObjectIdPydanticAnnotation

class ECGReading(BaseModel):
    """
    Represents an ECG (Electrocardiogram) reading entry.
    Attributes:
        id (ObjectId): Unique identifier for the ECG reading, mapped from MongoDB's '_id' field.
        device_id (str): Identifier for the device that recorded the ECG.
        session_id (str): Identifier for the session during which the ECG was recorded.
        timestamp (datetime): The UTC timestamp when the ECG reading was created. Defaults to the current time.
        ecg_array_id (ObjectId): Reference to the stored ECG data array.
    Config:
        - Allows arbitrary types (e.g., ObjectId).
        - Serializes ObjectId as a string in JSON.
        - Enables validation by field name.
    """

    id: Annotated[ObjectId, ObjectIdPydanticAnnotation] = Field(default_factory=ObjectId, alias="_id")
    device_id: str
    session_id: str
    timestamp: datetime = Field(default_factory=datetime.utcnow)
    ecg_array_id: Annotated[ObjectId, ObjectIdPydanticAnnotation]
    save_status: bool = Field(default=False, description="Indicates if the reading is saved or not")

    class Config:
        arbitrary_types_allowed = True
        json_encoders = {ObjectId: str}
        validate_by_field_name = True

class ECGArray(BaseModel):
    """
    Represents an ECG data array.
    Attributes:
        id (ObjectId): Unique identifier for the ECG data array, mapped from MongoDB's '_id' field.
        data (List[float]): List of float values representing the ECG data points.
    Config:
        - Allows arbitrary types (e.g., ObjectId).
        - Serializes ObjectId as a string in JSON.
    """
    id: Annotated[ObjectId, ObjectIdPydanticAnnotation] = Field(default_factory=ObjectId, alias="_id")
    data: List[float]

    class Config:
        arbitrary_types_allowed = True
        json_encoders = {ObjectId: str}
