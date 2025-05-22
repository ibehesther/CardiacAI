from app.src.utils.security import verify_password
from app.src.data.devices import DeviceRepository
from app.src.models.token import Token
from fastapi import HTTPException, status
import jwt
from jwt import decode as jwt_decode, PyJWTError
from datetime import datetime, timedelta
from app.src.config import settings
import datetime as dt

async def authenticate_user(device_id: str, password: str) -> Token:
    """
    Authenticate a user by username and password.

    Args:
    - device_id (str): The device_id of the user.
    - password (str): The password of the user.

    Returns:
    - UserOut: The user object if the user is authenticated, None otherwise.
    """
    device = await DeviceRepository.get_device_by_id(device_id)
    if not device:
        print(f"Device with ID {device_id} not found.")
        return None
    
    if verify_password(password, device.hashed_password):
        print(f"Authenticated device with ID {device_id} as admin.")
        return Token(
            access_token=await create_access_token(device_id, "admin"),
            token_type="bearer",
            role="admin",
        )
    elif verify_password(password, device.hashed_public_password):
        print(f"Authenticated device with ID {device_id} as user.")
        return Token(
            access_token=await create_access_token(device_id, "user"),
            token_type="bearer",
            role="user",
        )
    return None

async def create_access_token(device_id: str, role: str) -> str:
    """
    Create an access token for the user.

    Args:
    - device_id (str): The ID of the device.
    - role (str): The role of the user (user, admin).

    Returns:
    - str: The access token.
    """
    print(f"About access token for device_id: {device_id} with role: {role}")
    payload = {
        "device_id": device_id,
        "exp": int(
            dt.datetime.now(dt.timezone.utc).timestamp() + settings.ACCESS_TOKEN_EXPIRE_SECONDS
        ),
        "role": role,
    }

    print(f"Creating access token for device_id: {device_id} with role: {role}")
    
    return jwt.encode(
        payload, settings.SECRET_KEY, algorithm=settings.ALGORITHM
    )

async def check_for_admin(token: str) -> bool:
    """
    Check if the token is for an admin user.

    Args:
    - token (str): The access token.

    Returns:
    - bool: True if the user is an admin, False otherwise.
    """
    credentials_exception = HTTPException(
        status_code=status.HTTP_401_UNAUTHORIZED,
        detail="Only admin can access this resource",
        headers={"WWW-Authenticate": "Bearer"},
    )
    
    try:
        payload = jwt.decode(token, settings.SECRET_KEY, algorithms=[settings.ALGORITHM])
        device_id: str = payload.get("device_id")
        role: str = payload.get("role")
        if device_id is None or role is None:
            raise credentials_exception
        if role == "admin":
            return True
        raise credentials_exception
    except PyJWTError:
        raise credentials_exception