"""
This module provides utility functions for hashing and verifying passwords.
"""

from bcrypt import hashpw, gensalt, checkpw


def hash_password(password: str) -> str:
    """
    Hash the provided password.

    Args:
        password (str): The plain text password to hash.

    Returns:
        str: The hashed password.
    """
    return hashpw(password.encode(), gensalt()).decode()


def verify_password(password: str, hashed_password: str) -> bool:
    """
    Verify if the provided password matches the hashed password.

    Args:
        password (str): The plain text password to verify.
        hashed_password (str): The hashed password to compare against.

    Returns:
        bool: True if the password matches the hashed password, False otherwise.
    """
    return checkpw(password.encode(), hashed_password.encode())
