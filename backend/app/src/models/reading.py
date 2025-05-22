from pydantic import BaseModel, Field
from bson import ObjectId
from typing import Annotated, List
from datetime import datetime
from app.src.models.device import ObjectIdPydanticAnnotation

class ECGReading(BaseModel):
    id: Annotated[ObjectId, ObjectIdPydanticAnnotation] = Field(default_factory=ObjectId, alias="_id")
    device_id: str
    session_id: str
    timestamp: datetime = Field(default_factory=datetime.utcnow)
    ecg_array_id: Annotated[ObjectId, ObjectIdPydanticAnnotation]

    class Config:
        arbitrary_types_allowed = True
        json_encoders = {ObjectId: str}
        validate_by_field_name = True

class ECGArray(BaseModel):
    id: Annotated[ObjectId, ObjectIdPydanticAnnotation] = Field(default_factory=ObjectId, alias="_id")
    data: List[float]

    class Config:
        arbitrary_types_allowed = True
        json_encoders = {ObjectId: str}
