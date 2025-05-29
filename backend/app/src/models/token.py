from pydantic import BaseModel

class Token(BaseModel):
    """
    Token model for user authentication.
    Attributes:
        access_token (str): The JWT access token.
        token_type (str): The type of the token, typically "bearer".
        role (str): The role of the user, e.g., "user" or "admin".
    """
    access_token: str
    token_type: str
    role: str # user, admin
