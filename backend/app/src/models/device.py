from pydantic import BaseModel, Field
from bson import ObjectId
from typing import Annotated, Any, TypeVar
from bson import ObjectId
from pydantic_core import core_schema
from pydantic import Field
from pydantic.json_schema import JsonSchemaValue

class ObjectIdPydanticAnnotation:
    """
    Custom Pydantic annotation for handling MongoDB ObjectId fields.
    This class provides methods to validate and serialize ObjectId fields in Pydantic models.
    """
    @classmethod
    def validate_object_id(cls, v: Any, handler) -> ObjectId:
        if isinstance(v, ObjectId):
            return v

        s = handler(v)
        if ObjectId.is_valid(s):
            return ObjectId(s)
        else:
            raise ValueError("Invalid ObjectId")

    @classmethod
    def __get_pydantic_core_schema__(
        cls, source_type, _handler
    ) -> core_schema.CoreSchema:
        assert source_type is ObjectId
        return core_schema.no_info_wrap_validator_function(
            cls.validate_object_id,
            core_schema.str_schema(),
            serialization=core_schema.to_string_ser_schema(),
        )

    @classmethod
    def __get_pydantic_json_schema__(
        cls, _core_schema, handler
    ) -> JsonSchemaValue:
        return handler(core_schema.str_schema())


T = TypeVar("T")
ExcludedField = Annotated[T, Field(exclude=True)]


# Public model
class DeviceCreate(BaseModel):
    """
    DeviceCreate is a Pydantic model for creating a new device.
    """
    device_id: str
    password: str
    public_password: str


# Stored in DB
class DeviceInDB(BaseModel):
    """
    DeviceInDB is a Pydantic model representing a device stored in the database.
    """
    id: Annotated[ObjectId, ObjectIdPydanticAnnotation] = Field(
        default_factory=ObjectId, alias="_id"
    )
    device_id: str
    hashed_password: str
    hashed_public_password: str

    class Config:
        """
        Configuration class for Pydantic model.

        Attributes:
            arbitrary_types_allowed (bool): Allows arbitrary user-defined types (such as ObjectId) to be used as fields in the model.
            json_encoders (dict): Specifies custom JSON encoders for specific types. Here, ObjectId instances will be serialized as strings.
        """
        arbitrary_types_allowed = True
        json_encoders = {ObjectId: str}
