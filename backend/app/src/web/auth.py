from fastapi import APIRouter, Depends, HTTPException, status
from fastapi.security import OAuth2PasswordRequestForm, OAuth2PasswordBearer
from app.src.service.auth import authenticate_user
from app.src.models.token import Token

router = APIRouter(prefix="/auth", tags=["Auth"])
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="/api/auth/token")

@router.post("/token")
async def login(form_data: OAuth2PasswordRequestForm = Depends()) -> Token:
    """
    Authenticate a user and return an access token.

    Args:
        form_data (OAuth2PasswordRequestForm): The user credentials.

    Returns:
        Token: The access token.
    """
    print(f"Authenticating user with device_id: {form_data.username} and password: {form_data.password}")
    token = await authenticate_user(
        form_data.username, form_data.password
    )
    if not token:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Incorrect username or password",
        )
    return token